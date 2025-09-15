/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <ARCHParser.h>
#include <ArchiveFileEntry.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <MLException.h>
#include <ODTParser.h>
#include <PDFParser.h>
#include <SelfRemovingPath.h>
#include <TXTParser.h>
#include <algorithm>
#include <archive_entry.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

#ifndef USE_OPENMP
#include <thread>
#endif

ARCHParser::ARCHParser(const std::shared_ptr<AuxFunc> &af,
                       const bool &rar_support)
    : LibArchive(af)
{
  this->af = af;
  this->rar_support = rar_support;
#ifndef USE_OPENMP
  cancel.store(false);
#else
  omp_init_lock(&archp_obj_mtx);
#endif
}

ARCHParser::~ARCHParser()
{
#ifndef USE_OPENMP
  std::unique_lock<std::mutex> ullock(extra_run_mtx);
  extra_run_var.wait(ullock, [this] {
    return !extra_run;
  });
#else
  omp_destroy_lock(&archp_obj_mtx);
#endif
}

std::vector<BookParseEntry>
ARCHParser::arch_parser(const std::filesystem::path &filepath)
{
  arch_path = filepath;
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

#ifndef USE_OPENMP
  std::unique_lock<std::mutex> ullock(extra_run_mtx);
  extra_run_var.wait(ullock, [this] {
    return !extra_run;
  });
#endif

  check_for_fbd();

  return result;
}

void
ARCHParser::stopAll()
{
#ifndef USE_OPENMP
  archp_obj_mtx.lock();
  for(size_t i = 0; i < archp_obj.size(); i++)
    {
      archp_obj[i]->stopAll();
    }
  archp_obj_mtx.unlock();
  cancel.store(true);
#else
  omp_set_lock(&archp_obj_mtx);
  for(size_t i = 0; i < archp_obj.size(); i++)
    {
      archp_obj[i]->stopAll();
    }
  omp_unset_lock(&archp_obj_mtx);
#pragma omp atomic write
  cancel = true;
#endif
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
#ifndef USE_OPENMP
  while(!interrupt)
    {
      if(cancel.load())
        {
          break;
        }
      archive_entry_clear(entry.get());
      try
        {
          er = archive_read_next_header2(a.get(), entry.get());
        }
      catch(MLException &ml_e)
        {
          std::cout << "ARCHParser::arch_process: " << ml_e.what()
                    << std::endl;
          break;
        }

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
#else
#pragma omp parallel
  {
#pragma omp masked
    {
      while(!interrupt)
        {
          bool cncl;
#pragma omp atomic read
          cncl = cancel;
          if(cncl)
            {
              break;
            }
          archive_entry_clear(entry.get());
          try
            {
              er = archive_read_next_header2(a.get(), entry.get());
            }
          catch(MLException &ml_e)
            {
              std::cout << "ARCHParser::arch_process: " << ml_e.what()
                        << std::endl;
              break;
            }

          switch(er)
            {
            case ARCHIVE_OK:
              {
                filename.clear();
                char *fnm = const_cast<char *>(
                    archive_entry_pathname_utf8(entry.get()));
                if(fnm)
                  {
                    filename = fnm;
                  }
                else
                  {
                    fnm = const_cast<char *>(
                        archive_entry_pathname(entry.get()));
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
                    std::filesystem::path ch_p
                        = std::filesystem::u8path(filename);
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
                libarchive_error(
                    a, "ARCHParser::arch_process fatal read error:", er);
                interrupt = true;
                break;
              }
            default:
              {
                libarchive_error(a,
                                 "ARCHParser::arch_process read error:", er);
                break;
              }
            }
        }
#pragma omp taskwait
    }
  }
#endif
}

void
ARCHParser::unpack_entry(const std::filesystem::path &ch_p,
                         const std::shared_ptr<archive> &a,
                         const std::shared_ptr<archive_entry> &e)
{
#ifndef USE_OPENMP
  std::string ext = af->get_extension(ch_p);
  ext = af->stringToLower(ext);
  if(!rar_support && ext == ".rar")
    {
      return void();
    }

  std::string buf;
  if(ext == ".fb2")
    {
      buf = libarchive_read_entry_str(a.get(), e.get());

      std::unique_lock<std::mutex> ullock(extra_run_mtx);
      extra_run_var.wait(ullock, [this] {
        return !extra_run;
      });
      extra_run = true;
      ullock.unlock();

      std::thread parse_thr([this, buf, ch_p] {
        BookParseEntry bpe;
        FB2Parser fb2(af);
        try
          {
            bpe = fb2.fb2_parser(buf);
            bpe.book_path = ch_p.u8string();
            if(bpe.book_name.empty())
              {
                bpe.book_name = ch_p.stem().u8string();
              }
            result.emplace_back(bpe);
          }
        catch(MLException &er)
          {
            std::cout << "ARCHParser::unpack_entry error " << arch_path << " "
                      << ch_p << " " << er.what() << std::endl;
          }
        std::lock_guard<std::mutex> lglock(extra_run_mtx);
        extra_run = false;
        extra_run_var.notify_one();
      });
      parse_thr.detach();
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
          std::unique_lock<std::mutex> ullock(extra_run_mtx);
          extra_run_var.wait(ullock, [this] {
            return !extra_run;
          });
          extra_run = true;
          ullock.unlock();

          std::thread parse_thr([this, srp, out, ch_p] {
            EPUBParser epub(af);
            BookParseEntry bpe;
            try
              {
                bpe = epub.epub_parser(out);

                bpe.book_path = ch_p.u8string();
                if(bpe.book_name.empty())
                  {
                    bpe.book_name = ch_p.stem().u8string();
                  }
                result.emplace_back(bpe);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error " << arch_path
                          << " " << ch_p << " " << er.what() << std::endl;
              }
            std::lock_guard<std::mutex> lglock(extra_run_mtx);
            extra_run = false;
            extra_run_var.notify_one();
          });
          parse_thr.detach();
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

      std::unique_lock<std::mutex> ullock(extra_run_mtx);
      extra_run_var.wait(ullock, [this] {
        return !extra_run;
      });
      extra_run = true;
      ullock.unlock();

      std::thread parse_thr([this, buf, ch_p] {
        PDFParser pdf(af);
        BookParseEntry bpe;
        try
          {
            bpe = pdf.pdf_parser(buf);
            bpe.book_path = ch_p.u8string();
            if(bpe.book_name.empty())
              {
                bpe.book_name = ch_p.stem().u8string();
              }
            result.emplace_back(bpe);
          }
        catch(MLException &er)
          {
            std::cout << "ARCHParser::unpack_entry error " << arch_path << " "
                      << ch_p << " " << er.what() << std::endl;
          }

        std::lock_guard<std::mutex> lglock(extra_run_mtx);
        extra_run = false;
        extra_run_var.notify_one();
      });
      parse_thr.detach();
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
          std::unique_lock<std::mutex> ullock(extra_run_mtx);
          extra_run_var.wait(ullock, [this] {
            return !extra_run;
          });
          extra_run = true;
          ullock.unlock();

          std::thread parse_thr([this, out, srp, book_date, ch_p] {
            DJVUParser djvu(af);
            BookParseEntry bpe;
            try
              {
                bpe = djvu.djvu_parser(out);
                bpe.book_date = book_date;
                bpe.book_path = ch_p.u8string();
                if(bpe.book_name.empty())
                  {
                    bpe.book_name = ch_p.stem().u8string();
                  }
                result.emplace_back(bpe);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error " << arch_path
                          << " " << ch_p << " " << er.what() << std::endl;
                return void();
              }

            std::lock_guard<std::mutex> lglock(extra_run_mtx);
            extra_run = false;
            extra_run_var.notify_one();
          });
          parse_thr.detach();
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

      std::unique_lock<std::mutex> ullock(extra_run_mtx);
      extra_run_var.wait(ullock, [this] {
        return !extra_run;
      });
      extra_run = true;
      ullock.unlock();

      std::thread parse_thr([this, buf, ch_p] {
        FB2Parser fb2(af);
        BookParseEntry bpe;
        try
          {
            bpe = fb2.fb2_parser(buf);

            bpe.book_path = ch_p.u8string();
            if(bpe.book_name.empty())
              {
                bpe.book_name = ch_p.stem().u8string();
              }
            fbd.emplace_back(bpe);
          }
        catch(MLException &er)
          {
            std::cout << "ARCHParser::unpack_entry error " << arch_path << " "
                      << ch_p << " " << er.what() << std::endl;
          }

        std::lock_guard<std::mutex> lglock(extra_run_mtx);
        extra_run = false;
        extra_run_var.notify_one();
      });
      parse_thr.detach();
    }
  else if(ext == ".odt")
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
          std::unique_lock<std::mutex> ullock(extra_run_mtx);
          extra_run_var.wait(ullock, [this] {
            return !extra_run;
          });
          extra_run = true;
          ullock.unlock();

          std::thread parse_thr([this, out, srp, book_date, ch_p] {
            ODTParser odt(af);
            BookParseEntry bpe;
            try
              {
                bpe = odt.odtParser(out);
                if(bpe.book_date.empty())
                  {
                    bpe.book_date = book_date;
                  }
                bpe.book_path = ch_p.u8string();
                if(bpe.book_name.empty())
                  {
                    bpe.book_name = ch_p.stem().u8string();
                  }
                result.emplace_back(bpe);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error " << arch_path
                          << " " << ch_p << " " << er.what() << std::endl;
                return void();
              }

            std::lock_guard<std::mutex> lglock(extra_run_mtx);
            extra_run = false;
            extra_run_var.notify_one();
          });
          parse_thr.detach();
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry odt unpacking error"
                    << std::endl;
          return void();
        }
    }
  else if(ext == ".txt" || ext == ".md")
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
          std::unique_lock<std::mutex> ullock(extra_run_mtx);
          extra_run_var.wait(ullock, [this] {
            return !extra_run;
          });
          extra_run = true;
          ullock.unlock();

          std::thread parse_thr([this, out, srp, book_date, ch_p] {
            TXTParser txt(af);
            BookParseEntry bpe;
            try
              {
                bpe = txt.txtParser(out);
                if(bpe.book_date.empty())
                  {
                    bpe.book_date = book_date;
                  }
                bpe.book_path = ch_p.u8string();
                if(bpe.book_name.empty())
                  {
                    bpe.book_name = ch_p.stem().u8string();
                  }
                result.emplace_back(bpe);
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error " << arch_path
                          << " " << ch_p << " " << er.what() << std::endl;
                return void();
              }

            std::lock_guard<std::mutex> lglock(extra_run_mtx);
            extra_run = false;
            extra_run_var.notify_one();
          });
          parse_thr.detach();
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry odt unpacking error"
                    << std::endl;
          return void();
        }
    }
  else if(af->ifSupportedArchiveUnpackaingType(ch_p))
    {
      std::filesystem::path temp = af->temp_path();
      temp /= std::filesystem::u8path(af->randomFileName());
      SelfRemovingPath srp(temp);
      std::filesystem::path out
          = libarchive_read_entry(a.get(), e.get(), srp.path);
      if(std::filesystem::exists(out))
        {
          std::unique_lock<std::mutex> ullock(extra_run_mtx);
          extra_run_var.wait(ullock, [this] {
            return !extra_run;
          });
          extra_run = true;
          ullock.unlock();

          std::thread parse_thr([this, srp, out, ch_p] {
            std::vector<BookParseEntry> rec_v;
            ARCHParser arch(af, rar_support);
            archp_obj_mtx.lock();
            archp_obj.push_back(&arch);
            archp_obj_mtx.unlock();
            try
              {
                rec_v = arch.arch_parser(out);
                for(auto it = rec_v.begin(); it != rec_v.end(); it++)
                  {
                    it->book_path = ch_p.u8string() + "\n" + it->book_path;
                    result.push_back(*it);
                  }
              }
            catch(MLException &er)
              {
                std::cout << "ARCHParser::unpack_entry error " << arch_path
                          << " " << ch_p << " " << er.what() << std::endl;
              }
            archp_obj_mtx.lock();
            archp_obj.erase(
                std::remove(archp_obj.begin(), archp_obj.end(), &arch),
                archp_obj.end());
            archp_obj_mtx.unlock();

            std::lock_guard<std::mutex> lglock(extra_run_mtx);
            extra_run = false;
            extra_run_var.notify_one();
          });
          parse_thr.detach();
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry archive unpacking error"
                    << std::endl;
          return void();
        }
    }
  else
    {
      std::string book_date
          = af->time_t_to_date(archive_entry_birthtime(e.get()));
      std::unique_lock<std::mutex> ullock(extra_run_mtx);
      extra_run_var.wait(ullock, [this] {
        return !extra_run;
      });
      extra_run = true;
      ullock.unlock();
      std::thread parse_thr([this, book_date, ch_p] {
        BookParseEntry bpe;
        bpe.book_name = ch_p.stem().u8string();
        bpe.book_date = book_date;
        bpe.book_path = ch_p.u8string();
        result.emplace_back(bpe);

        std::lock_guard<std::mutex> lglock(extra_run_mtx);
        extra_run = false;
        extra_run_var.notify_one();
      });
      parse_thr.detach();
    }
#else
  std::string ext = af->get_extension(ch_p);
  ext = af->stringToLower(ext);
  if(!rar_support && ext == ".rar")
    {
      return void();
    }

  std::string buf;
  if(ext == ".fb2")
    {
      buf = libarchive_read_entry_str(a.get(), e.get());
#pragma omp taskwait
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          BookParseEntry bpe;
          FB2Parser fb2(af);
          try
            {
              bpe = fb2.fb2_parser(buf);
              bpe.book_path = ch_p.u8string();
              if(bpe.book_name.empty())
                {
                  bpe.book_name = ch_p.stem().u8string();
                }
              result.emplace_back(bpe);
            }
          catch(MLException &er)
            {
              std::cout << "ARCHParser::unpack_entry error " << arch_path
                        << " " << ch_p << " " << er.what() << std::endl;
            }
          omp_fulfill_event(event);
        }
      }
    }
  else if(ext == ".epub")
    {
      std::filesystem::path temp = af->temp_path();
      temp /= std::filesystem::u8path(af->randomFileName());
      std::filesystem::path out
          = libarchive_read_entry(a.get(), e.get(), temp);

      if(std::filesystem::exists(out))
        {
#pragma omp taskwait
#pragma omp masked
          {
            omp_event_handle_t event;
#pragma omp task detach(event)
            {
              EPUBParser epub(af);
              BookParseEntry bpe;
              try
                {
                  bpe = epub.epub_parser(out);
                  bpe.book_path = ch_p.u8string();
                  if(bpe.book_name.empty())
                    {
                      bpe.book_name = ch_p.stem().u8string();
                    }
                  result.emplace_back(bpe);
                }
              catch(MLException &er)
                {
                  std::cout << "ARCHParser::unpack_entry error " << arch_path
                            << " " << ch_p << " " << er.what() << std::endl;
                }
              std::filesystem::remove_all(temp);
              omp_fulfill_event(event);
            }
          }
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry epub unpacking error"
                    << std::endl;
          std::filesystem::remove_all(temp);
          return void();
        }
    }
  else if(ext == ".pdf")
    {
      buf = libarchive_read_entry_str(a.get(), e.get());
#pragma omp taskwait
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          PDFParser pdf(af);
          BookParseEntry bpe;
          try
            {
              bpe = pdf.pdf_parser(buf);
              bpe.book_path = ch_p.u8string();
              if(bpe.book_name.empty())
                {
                  bpe.book_name = ch_p.stem().u8string();
                }
              result.emplace_back(bpe);
            }
          catch(MLException &er)
            {
              std::cout << "ARCHParser::unpack_entry error " << arch_path
                        << " " << ch_p << " " << er.what() << std::endl;
            }
          omp_fulfill_event(event);
        }
      }
    }
  else if(ext == ".djvu")
    {
      std::filesystem::path temp = af->temp_path();
      temp /= std::filesystem::u8path(af->randomFileName());
      std::filesystem::path out
          = libarchive_read_entry(a.get(), e.get(), temp);
      std::string book_date
          = af->time_t_to_date(archive_entry_birthtime(e.get()));
      if(std::filesystem::exists(out))
        {
#pragma omp taskwait
#pragma omp masked
          {
            omp_event_handle_t event;
#pragma omp task detach(event)
            {
              DJVUParser djvu(af);
              BookParseEntry bpe;
              try
                {
                  bpe = djvu.djvu_parser(out);
                  bpe.book_date = book_date;
                  bpe.book_path = ch_p.u8string();
                  if(bpe.book_name.empty())
                    {
                      bpe.book_name = ch_p.stem().u8string();
                    }
                  result.emplace_back(bpe);
                }
              catch(MLException &er)
                {
                  std::cout << "ARCHParser::unpack_entry error " << arch_path
                            << " " << ch_p << " " << er.what() << std::endl;
                }
              std::filesystem::remove_all(temp);
              omp_fulfill_event(event);
            }
          }
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry djvu unpacking error"
                    << std::endl;
          std::filesystem::remove_all(temp);
          return void();
        }
    }
  else if(ext == ".fbd")
    {
      buf = libarchive_read_entry_str(a.get(), e.get());
#pragma omp taskwait
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          FB2Parser fb2(af);
          BookParseEntry bpe;
          try
            {
              bpe = fb2.fb2_parser(buf);
              bpe.book_path = ch_p.u8string();
              if(bpe.book_name.empty())
                {
                  bpe.book_name = ch_p.stem().u8string();
                }
              fbd.emplace_back(bpe);
            }
          catch(MLException &er)
            {
              std::cout << "ARCHParser::unpack_entry error " << arch_path
                        << " " << ch_p << " " << er.what() << std::endl;
            }
          omp_fulfill_event(event);
        }
      }
    }
  else if(ext == ".odt")
    {
      std::filesystem::path temp = af->temp_path();
      temp /= std::filesystem::u8path(af->randomFileName());
      std::filesystem::path out
          = libarchive_read_entry(a.get(), e.get(), temp);
      std::string book_date
          = af->time_t_to_date(archive_entry_birthtime(e.get()));
      if(std::filesystem::exists(out))
        {
#pragma omp taskwait
#pragma omp masked
          {
            omp_event_handle_t event;
#pragma omp task detach(event)
            {
              ODTParser odt(af);
              BookParseEntry bpe;
              try
                {
                  bpe = odt.odtParser(out);
                  if(bpe.book_date.empty())
                    {
                      bpe.book_date = book_date;
                    }
                  bpe.book_path = ch_p.u8string();
                  if(bpe.book_name.empty())
                    {
                      bpe.book_name = ch_p.stem().u8string();
                    }
                  result.emplace_back(bpe);
                }
              catch(MLException &er)
                {
                  std::cout << "ARCHParser::unpack_entry error " << arch_path
                            << " " << ch_p << " " << er.what() << std::endl;
                }
              std::filesystem::remove_all(temp);
              omp_fulfill_event(event);
            }
          }
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry odt unpacking error"
                    << std::endl;
          std::filesystem::remove_all(temp);
          return void();
        }
    }
  else if(ext == ".txt" || ext == ".md")
    {
      std::filesystem::path temp = af->temp_path();
      temp /= std::filesystem::u8path(af->randomFileName());
      std::filesystem::path out
          = libarchive_read_entry(a.get(), e.get(), temp);
      std::string book_date
          = af->time_t_to_date(archive_entry_birthtime(e.get()));
      if(std::filesystem::exists(out))
        {
#pragma omp taskwait
#pragma omp masked
          {
            omp_event_handle_t event;
#pragma omp task detach(event)
            {
              TXTParser txt(af);
              BookParseEntry bpe;
              try
                {
                  bpe = txt.txtParser(out);
                  if(bpe.book_date.empty())
                    {
                      bpe.book_date = book_date;
                    }
                  bpe.book_path = ch_p.u8string();
                  if(bpe.book_name.empty())
                    {
                      bpe.book_name = ch_p.stem().u8string();
                    }
                  result.emplace_back(bpe);
                }
              catch(MLException &er)
                {
                  std::cout << "ARCHParser::unpack_entry error " << arch_path
                            << " " << ch_p << " " << er.what() << std::endl;
                }
              std::filesystem::remove_all(temp);
              omp_fulfill_event(event);
            }
          }
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry txt unpacking error"
                    << std::endl;
          std::filesystem::remove_all(temp);
          return void();
        }
    }
  else if(af->ifSupportedArchiveUnpackaingType(ch_p))
    {
      std::filesystem::path temp = af->temp_path();
      temp /= std::filesystem::u8path(af->randomFileName());
      std::filesystem::path out
          = libarchive_read_entry(a.get(), e.get(), temp);
      if(std::filesystem::exists(out))
        {
#pragma omp taskwait
#pragma omp masked
          {
            omp_event_handle_t event;
#pragma omp task detach(event)
            {
              std::vector<BookParseEntry> rec_v;
              ARCHParser arch(af, rar_support);
              omp_set_lock(&archp_obj_mtx);
              archp_obj.push_back(&arch);
              omp_unset_lock(&archp_obj_mtx);
              try
                {
                  rec_v = arch.arch_parser(out);
                  for(auto it = rec_v.begin(); it != rec_v.end(); it++)
                    {
                      it->book_path = ch_p.u8string() + "\n" + it->book_path;
                      result.push_back(*it);
                    }
                }
              catch(MLException &er)
                {
                  std::cout << "ARCHParser::unpack_entry error " << arch_path
                            << " " << ch_p << " " << er.what() << std::endl;
                }
              omp_set_lock(&archp_obj_mtx);
              archp_obj.erase(
                  std::remove(archp_obj.begin(), archp_obj.end(), &arch),
                  archp_obj.end());
              omp_unset_lock(&archp_obj_mtx);
              std::filesystem::remove_all(temp);
              omp_fulfill_event(event);
            }
          }
        }
      else
        {
          std::cout << "ARCHParser::unpack_entry archive unpacking error"
                    << std::endl;
          std::filesystem::remove_all(temp);
          return void();
        }
    }
  else
    {
      std::string book_date
          = af->time_t_to_date(archive_entry_birthtime(e.get()));
#pragma omp taskwait
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          BookParseEntry bpe;
          bpe.book_name = ch_p.stem().u8string();
          bpe.book_date = book_date;
          bpe.book_path = ch_p.u8string();
          result.emplace_back(bpe);
          omp_fulfill_event(event);
        }
      }
    }
#endif
}

void
ARCHParser::check_for_fbd()
{
  std::string find_str(".fbd");
  std::string find_str2("\n");
  for(auto it = result.begin(); it != result.end();)
    {
      auto itfbd = std::find_if(
          fbd.begin(), fbd.end(), [it, find_str](const BookParseEntry &el) {
            if(it->book_path.size() >= el.book_path.size() - find_str.size())
              {
                return it->book_path.substr(0, el.book_path.size()
                                                   - find_str.size())
                       == el.book_path.substr(0, el.book_path.size()
                                                     - find_str.size());
              }
            return false;
          });
      if(itfbd != fbd.end())
        {
          std::string bp = it->book_path;
          *it = *itfbd;
          it->book_path = bp;
          it++;
        }
      else
        {
          std::string bp = it->book_path;
          std::string::size_type n = bp.rfind(find_str2);
          if(n != std::string::npos)
            {
              bp.erase(bp.begin(), bp.begin() + n + find_str2.size());
            }
          if(af->if_supported_type(std::filesystem::u8path(bp)))
            {
              it++;
            }
          else
            {
              result.erase(it);
            }
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
