/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <ARCHParser.h>
#include <ArchiveFileEntry.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <MLException.h>
#include <PDFParser.h>
#include <SelfRemovingPath.h>
#include <algorithm>
#include <archive_entry.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

ARCHParser::ARCHParser(const std::shared_ptr<AuxFunc> &af,
                       const bool &rar_support, std::atomic<bool> *cancel)
{
  this->af = af;
  this->rar_support = rar_support;
  this->cancel = cancel;
  parse_thr = std::thread([] {
  });
}

ARCHParser::~ARCHParser()
{
  if(parse_thr.joinable())
    {
      parse_thr.join();
    }
}

std::vector<BookParseEntry>
ARCHParser::arch_parser(const std::filesystem::path &filepath)
{
  std::shared_ptr<ArchiveFileEntry> fl = createArchFile(filepath, 0);
  if(fl)
    {
      std::shared_ptr<archive> a = libarchive_read_init_fallback(fl);
      if(a)
        {
          arch_process(a);
        }
      else
        {
          throw MLException(
              "ARCHParser::arch_parser: initialize archive error: "
              + filepath.u8string());
        }
    }
  else
    {
      throw MLException("ARCHParser::arch_parser: cannot open file: "
                        + filepath.u8string());
    }

  parse_thr.join();

  check_for_fbd();

  return result;
}

void
ARCHParser::arch_process(const std::shared_ptr<archive> &a)
{
  std::signal(SIGILL, &ARCHParser::signalHandler);
  std::signal(SIGSEGV, &ARCHParser::signalHandler);
  std::signal(SIGFPE, &ARCHParser::signalHandler);
  std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()),
                                       [](archive_entry *e) {
                                         archive_entry_free(e);
                                       });
  bool interrupt = false;
  int er;
  std::string filename;
  while(!interrupt)
    {
      if(cancel->load())
        {
          break;
        }
      archive_entry_clear(entry.get());
      er = archive_read_next_header2(a.get(), entry.get());
      switch(er)
        {
        case ARCHIVE_OK:
          {
            filename.clear();
            char *fnm
                = const_cast<char *>(archive_entry_pathname_utf8(entry.get()));
            if(fnm)
              {
                filename = fnm;
              }
            else
              {
                fnm = const_cast<char *>(archive_entry_pathname(entry.get()));
                if(fnm)
                  {
                    filename = fnm;
                  }
                else
                  {
                    std::cout << "ARCHParser::arch_process file name error"
                              << std::endl;
                  }
              }
            if(archive_entry_filetype(entry.get()) == AE_IFREG
               && !filename.empty())
              {
                std::filesystem::path ch_p = std::filesystem::u8path(filename);
                unpack_entry(ch_p, a, entry);
              }
            break;
          }
        case ARCHIVE_EOF:
          {
            interrupt = true;
            break;
          }
        case ARCHIVE_FATAL:
          {
            libarchive_error(a,
                             "ARCHParser::arch_process fatal read error:", er);
            interrupt = true;
            break;
          }
        default:
          {
            libarchive_error(a, "ARCHParser::arch_process read error:", er);
            break;
          }
        }
    }
}

void
ARCHParser::unpack_entry(const std::filesystem::path &ch_p,
                         const std::shared_ptr<archive> &a,
                         const std::shared_ptr<archive_entry> &e)
{
  std::string ext = af->get_extension(ch_p);
  ext = af->stringToLower(ext);
  if(!rar_support && ext == ".rar")
    {
      return void();
    }
  if(af->if_supported_type(ch_p) || ext == ".fbd")
    {
      std::string buf;
      if(ext == ".fb2")
        {
          buf = libarchive_read_entry_str(a.get(), e.get());
          parse_thr.join();
          parse_thr = std::thread([this, buf, ch_p] {
            BookParseEntry bpe;
            FB2Parser fb2(af);
            try
              {
                bpe = fb2.fb2_parser(buf);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error: " << er.what()
                          << std::endl;
                return void();
              }
            bpe.book_path = ch_p.u8string();
            if(bpe.book_name.empty())
              {
                bpe.book_name = ch_p.stem().u8string();
              }
            result.push_back(bpe);
          });
        }
      else if(ext == ".epub")
        {
          std::filesystem::path temp = af->temp_path();
          temp /= std::filesystem::u8path(af->randomFileName());
          SelfRemovingPath srp(temp);
          std::filesystem::path out
              = libarchive_read_entry(a.get(), e.get(), srp.path);

          if(std::filesystem::exists(out))
            {
              parse_thr.join();
              parse_thr = std::thread([this, srp, out, ch_p] {
                EPUBParser epub(af);
                BookParseEntry bpe;
                try
                  {
                    bpe = epub.epub_parser(out);
                  }
                catch(MLException &er)
                  {
                    std::cout
                        << "ARCHParser::unpack_entry error: " << er.what()
                        << std::endl;
                    return void();
                  }
                bpe.book_path = ch_p.u8string();
                if(bpe.book_name.empty())
                  {
                    bpe.book_name = ch_p.stem().u8string();
                  }
                result.push_back(bpe);
              });
            }
          else
            {
              std::cout << "ARCHParser::unpack_entry epub unpacking error"
                        << std::endl;
              return void();
            }
        }
      else if(ext == ".pdf")
        {
          buf = libarchive_read_entry_str(a.get(), e.get());
          parse_thr.join();
          parse_thr = std::thread([this, buf, ch_p] {
            PDFParser pdf(af);
            BookParseEntry bpe;
            try
              {
                bpe = pdf.pdf_parser(buf);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error: " << er.what()
                          << std::endl;
                return void();
              }
            bpe.book_path = ch_p.u8string();
            if(bpe.book_name.empty())
              {
                bpe.book_name = ch_p.stem().u8string();
              }
            result.push_back(bpe);
          });
        }
      else if(ext == ".djvu")
        {
          std::filesystem::path temp = af->temp_path();
          temp /= std::filesystem::u8path(af->randomFileName());
          SelfRemovingPath srp(temp);
          std::filesystem::path out
              = libarchive_read_entry(a.get(), e.get(), srp.path);
          std::string book_date
              = af->time_t_to_date(archive_entry_birthtime(e.get()));
          if(std::filesystem::exists(out))
            {
              parse_thr.join();
              parse_thr = std::thread([this, out, srp, book_date, ch_p] {
                DJVUParser djvu(af);
                BookParseEntry bpe;
                try
                  {
                    bpe = djvu.djvu_parser(out);
                  }
                catch(MLException &er)
                  {
                    std::cout
                        << "ARCHParser::unpack_entry error: " << er.what()
                        << std::endl;
                    return void();
                  }
                bpe.book_date = book_date;
                bpe.book_path = ch_p.u8string();
                if(bpe.book_name.empty())
                  {
                    bpe.book_name = ch_p.stem().u8string();
                  }
                result.push_back(bpe);
              });
            }
          else
            {
              std::cout << "ARCHParser::unpack_entry djvu unpacking error"
                        << std::endl;
              return void();
            }
        }
      else if(ext == ".fbd")
        {
          buf = libarchive_read_entry_str(a.get(), e.get());
          parse_thr.join();
          parse_thr = std::thread([this, buf, ch_p] {
            FB2Parser fb2(af);
            BookParseEntry bpe;
            try
              {
                bpe = fb2.fb2_parser(buf);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error: " << er.what()
                          << std::endl;
                return void();
              }
            bpe.book_path = ch_p.u8string();
            if(bpe.book_name.empty())
              {
                bpe.book_name = ch_p.stem().u8string();
              }
            fbd.push_back(bpe);
          });
        }
      else
        {
          std::filesystem::path temp = af->temp_path();
          temp /= std::filesystem::u8path(af->randomFileName());
          SelfRemovingPath srp(temp);
          std::filesystem::path out
              = libarchive_read_entry(a.get(), e.get(), srp.path);
          if(std::filesystem::exists(out))
            {
              parse_thr.join();
              parse_thr = std::thread([this, srp, out, ch_p] {
                std::vector<BookParseEntry> rec_v;
                ARCHParser arch(af, rar_support, cancel);
                try
                  {
                    rec_v = arch.arch_parser(out);
                  }
                catch(MLException &er)
                  {
                    std::cout << "ARCHParser::unpack_entry: " << er.what()
                              << std::endl;
                    return void();
                  }
                for(auto it = rec_v.begin(); it != rec_v.end(); it++)
                  {
                    it->book_path = ch_p.u8string() + "\n" + it->book_path;
                    result.push_back(*it);
                  }
              });
            }
          else
            {
              std::cout << "ARCHParser::unpack_entry archive unpacking error"
                        << std::endl;
              return void();
            }
        }
    }
}

void
ARCHParser::check_for_fbd()
{
  for(auto it = result.begin(); it != result.end(); it++)
    {
      std::filesystem::path ch_p = std::filesystem::u8path(it->book_path);
      ch_p = ch_p.replace_extension(std::filesystem::u8path(""));
      auto itfbd
          = std::find_if(fbd.begin(), fbd.end(), [ch_p](BookParseEntry &el) {
              std::filesystem::path p = std::filesystem::u8path(el.book_path);
              p = p.replace_extension(std::filesystem::u8path(""));
              if(p == ch_p)
                {
                  return true;
                }
              else
                {
                  return false;
                }
            });
      if(itfbd != fbd.end())
        {
          std::string bp = it->book_path;
          *it = *itfbd;
          it->book_path = bp;
        }
    }
}

void
ARCHParser::signalHandler(int sig)
{
  std::string msg = std::string("ARCHParser received signal: \"")
                    + std::strerror(sig) + "\"";
  throw MLException(msg);
}
