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

#include <BookParseEntry.h>
#include <LibArchive.h>
#include <MLException.h>
#include <OpenBook.h>
#include <algorithm>
#include <iostream>

OpenBook::OpenBook(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

std::filesystem::path
OpenBook::open_book(
    const BookBaseEntry &bbe, const bool &copy,
    const std::filesystem::path &copy_path, const bool &find_fbd,
    std::function<void(const std::filesystem::path &path)> open_callback)
{
  std::filesystem::path result;
  std::string ext;
  std::error_code ec;
  std::filesystem::file_status fstat
      = std::filesystem::symlink_status(bbe.file_path, ec);
  if(ec)
    {
      throw MLException(std::string("\nOpenBook::open_book: ")
                        + bbe.file_path.u8string() + "\n" + ec.message()
                        + "\n");
    }
  if(fstat.type() == std::filesystem::file_type::symlink)
    {
      std::filesystem::path resolved
          = std::filesystem::read_symlink(bbe.file_path);
      ext = resolved.extension().u8string();
    }
  else
    {
      ext = bbe.file_path.extension().u8string();
    }
  ext = af->stringToLower(ext);
  if(ext == ".fb2" || ext == ".epub" || ext == ".pdf" || ext == ".djvu"
     || ext == ".odt")
    {
      if(copy)
        {
          std::filesystem::path tmp = copy_path;
          tmp /= bbe.file_path.filename();
          std::filesystem::create_directories(tmp.parent_path());
          std::filesystem::copy(bbe.file_path, tmp, ec);
          if(ec)
            {
              throw MLException("OpenBook::open_book error: " + ec.message());
            }
          if(find_fbd)
            {
              std::string sstr = bbe.file_path.stem().u8string() + ".fbd";
              std::filesystem::path ch_fbd = std::filesystem::u8path(sstr);
              std::filesystem::path copy_path = tmp.parent_path();
              copy_path /= std::filesystem::u8path(sstr);
              for(auto &dirit : std::filesystem::recursive_directory_iterator(
                      bbe.file_path.parent_path()))
                {
                  std::filesystem::path p = dirit.path();
                  std::string ext = p.extension().u8string();
                  ext = af->stringToLower(ext);
                  if(!std::filesystem::is_directory(p)
                     && p.stem() == ch_fbd.stem()
                     && ext == ch_fbd.extension().u8string())
                    {
                      std::filesystem::copy(p, copy_path, ec);
                      if(ec)
                        {
                          std::cout << "OpenBook::open_book fbd error: "
                                    << ec.message() << std::endl;
                        }
                      break;
                    }
                }
            }
          result = tmp;
        }
      else
        {
          result = bbe.file_path;
          if(open_callback)
            {
              open_callback(result);
            }
        }
    }
  else
    {
      result = open_archive(bbe, ext, copy_path, find_fbd);
      if(!copy)
        {
          BookBaseEntry bbe;
          bbe.file_path = result;
          reading = result.parent_path();
          OpenBook *ob = new OpenBook(af);
          ob->open_book(bbe, false, std::filesystem::u8path(""), false,
                        open_callback);
          delete ob;
        }
    }
  return result;
}

std::filesystem::path
OpenBook::open_archive(const BookBaseEntry &bbe, const std::string &ext,
                       const std::filesystem::path &copy_path,
                       const bool &find_fbd)
{
  std::filesystem::path result;
  std::filesystem::path tmp = af->temp_path();
  tmp /= std::filesystem::u8path(af->randomFileName());
  SelfRemovingPath p(tmp);
  BookBaseEntry bber = bbe;
  bool loc_find_fbd = false;
  std::string::size_type n;
  std::string unpack_path;
  std::string sstr = "\n";
  n = bbe.bpe.book_path.find(sstr);
  if(n != std::string::npos)
    {
      unpack_path = bber.bpe.book_path.substr(0, n);
      bber.bpe.book_path.erase(0, n + sstr.size());
    }
  else
    {
      unpack_path = bber.bpe.book_path;
      loc_find_fbd = true;
    }
  std::vector<ArchEntry> files;

  LibArchive la(af);

  if(ext == ".zip")
    {
      la.fileNames(bbe.file_path, files);

      correct_separators(files);
      std::string search_p;
      std::string conv_nm;
      bool encoding = false;
      for(int32_t i = -1; i < af->get_charset_conv_quantity(); i++)
        {
          if(i < 0)
            {
              search_p = unpack_path;
            }
          else
            {
              conv_nm = af->get_converter_by_number(i);
              search_p = af->utf_8_to(unpack_path, conv_nm.c_str());
              encoding = true;
            }
          auto it = std::find_if(files.begin(), files.end(),
                                 [search_p](ArchEntry &el) {
                                   return el.filename == search_p;
                                 });
          if(it != files.end())
            {
              bber.file_path = la.unpackByPosition(bbe.file_path, tmp, *it);

              if(find_fbd && loc_find_fbd)
                {
                  std::filesystem::path ch_fbd
                      = std::filesystem::u8path(unpack_path);
                  ch_fbd.replace_extension(std::filesystem::u8path(".fbd"));
                  auto itfbd
                      = std::find_if(files.begin(), files.end(),
                                     std::bind(&OpenBook::compare_func, this,
                                               std::placeholders::_1, encoding,
                                               conv_nm, ch_fbd));
                  if(itfbd != files.end())
                    {
                      la.unpackByPosition(bbe.file_path, tmp, *itfbd);
                    }
                }

              OpenBook *ob = new OpenBook(af);
              result = ob->open_book(bber, true, copy_path, find_fbd, nullptr);
              delete ob;
              break;
            }
        }
    }
  else
    {
      bber.file_path
          = la.unpackByFileNameStream(bbe.file_path, tmp, unpack_path);
      if(find_fbd && loc_find_fbd)
        {
          la.fileNamesStream(bbe.file_path, files);
          std::filesystem::path ch_fbd = std::filesystem::u8path(unpack_path);
          ch_fbd.replace_extension(std::filesystem::u8path(".fbd"));
          std::string find_fbd = ch_fbd.u8string();
          auto itfbd = std::find_if(files.begin(), files.end(),
                                    std::bind(&OpenBook::compare_func, this,
                                              std::placeholders::_1, false, "",
                                              ch_fbd));
          if(itfbd != files.end())
            {
              la.unpackByFileNameStream(bbe.file_path, tmp, itfbd->filename);
            }
        }
      OpenBook *ob = new OpenBook(af);
      result = ob->open_book(bber, true, copy_path, find_fbd, nullptr);
      delete ob;
    }

  return result;
}

void
OpenBook::correct_separators(std::vector<ArchEntry> &files)
{
  std::string::size_type n;
  std::string sstr = "\\";
  for(auto it = files.begin(); it != files.end(); it++)
    {
      if(it->filename.size() > 0)
        {
          n = 0;
          for(;;)
            {
              n = it->filename.find(sstr, n);
              if(n != std::string::npos)
                {
                  it->filename.erase(n, sstr.size());
                  it->filename.insert(n, "/");
                }
              else
                {
                  break;
                }
            }
        }
    }
}

bool
OpenBook::compare_func(const ArchEntry &ent, const bool &encoding,
                       const std::string &conv_nm,
                       const std::filesystem::path &ch_fbd)
{
  std::string val;
  if(encoding)
    {
      val = af->to_utf_8(ent.filename, conv_nm.c_str());
    }
  else
    {
      val = ent.filename;
    }
  std::filesystem::path ch_p = std::filesystem::u8path(val);
  std::string ext = af->stringToLower(ch_p.extension().u8string());
  if(ext == ch_fbd.extension().u8string() && ch_fbd.stem() == ch_p.stem())
    {
      return true;
    }
  else
    {
      return false;
    }
}
