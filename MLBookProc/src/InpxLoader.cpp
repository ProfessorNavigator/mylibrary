/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <Algorithm.h>
#include <InpxLoader.h>
#include <XMLTextEncoding.h>
#include <cstring>
#include <iostream>
#include <omp.h>
#include <stdexcept>

InpxLoader::InpxLoader(const std::shared_ptr<MLBookProc> &mlbp)
    : LibArchive(mlbp)
{
}

UDBase
InpxLoader::loadInpxCollection(const std::filesystem::path &books_directory,
                               const std::filesystem::path &inpx_file_path)
{
  if(!std::filesystem::is_directory(books_directory)
     || !std::filesystem::exists(inpx_file_path))
    {
      throw std::runtime_error("InpxLoader::loadInpxCollection: books "
                               "directory or inpx file does not exist");
    }

  for(auto &dir_it :
      std::filesystem::recursive_directory_iterator(books_directory))
    {
      std::filesystem::path p = dir_it.path();
      std::filesystem::file_status stat = std::filesystem::symlink_status(p);
      switch(stat.type())
        {
        case std::filesystem::file_type::regular:
        case std::filesystem::file_type::symlink:
          {
            books_directory_files.emplace_back(p);
            break;
          }
        default:
          break;
        }
    }

  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->path = inpx_file_path;
  fd->open_mode = std::ios_base::in | std::ios_base::binary;

  std::shared_ptr<archive> a = initForReading(fd);

  int er = archive_read_set_seek_callback(a.get(), &LibArchive::seekCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a, "InpxLoader::loadInpxCollection:");
    }

  er = archive_read_open2(a.get(), fd.get(), &LibArchive::openCallBack,
                          &LibArchive::readCallBack, &LibArchive::skipCallback,
                          &LibArchive::closeCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a, "InpxLoader::loadInpxCollection:");
    }

#pragma omp parallel
#pragma omp masked
  {
    int retry_count = 0;
    while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
      {
        std::shared_ptr<archive_entry> e(archive_entry_new(),
                                         [](archive_entry *e)
                                           {
                                             archive_entry_free(e);
                                           });
        er = archive_read_next_header2(a.get(), e.get());
        switch(er)
          {
          case ARCHIVE_WARN:
            {
              const char *str = archive_error_string(a.get());
              std::string err;
              if(str)
                {
                  err = std::string("InpxLoader::loadInpxCollection: \"") + str
                        + "\"";
                }
              else
                {
                  err = std::string("InpxLoader::loadInpxCollection: ")
                        + std::strerror(archive_errno(a.get()));
                }
              std::cout << err << std::endl;
            }
          case ARCHIVE_OK:
            {
              retry_count = 0;
              parseEntry(a, e, books_directory);
              break;
            }
          case ARCHIVE_EOF:
            {
              break;
            }
          case ARCHIVE_RETRY:
            {
              retry_count++;
              break;
            }
          default:
            {
              result.clearBase();
              archiveError(a, "InpxLoader::loadInpxCollection:");
              break;
            }
          }
      }
  }

  return result;
}

void
InpxLoader::parseEntry(std::shared_ptr<archive> a,
                       std::shared_ptr<archive_entry> e,
                       const std::filesystem::path &books_directory)
{
  la_int64_t sz = 0;
  if(archive_entry_size_is_set(e.get()))
    {
      sz = archive_entry_size(e.get());
      if(sz < 0)
        {
          sz = 0;
        }
    }
  if(sz == 0)
    {
      return void();
    }

  const char *val = archive_entry_pathname_utf8(e.get());
  if(val == nullptr)
    {
      return void();
    }

  std::filesystem::path inp_p
      = std::u8string(reinterpret_cast<const char8_t *>(val));
  if(inp_p.extension().u8string() != u8".inp")
    {
      return void();
    }
  inp_p.replace_extension("");

  auto it_res = Algorithm::parallelFindIf(
      books_directory_files.begin(), books_directory_files.end(),
      [books_directory, inp_p](const std::filesystem::path &el)
        {
          std::filesystem::path ch_p = el.lexically_relative(books_directory);
          if(ch_p.has_extension())
            {
              ch_p.replace_extension("");
            }
          return inp_p == ch_p;
        });
  if(it_res == books_directory_files.end())
    {
      return void();
    }
  std::filesystem::path file_path = *it_res;
  books_directory_files.erase(it_res);
  std::string buf = unpackEntryToBuffer(a, e);
  if(buf.size() == 0)
    {
      return void();
    }

#pragma omp task
  {
    parseInpFile(buf, file_path);
  }
}

void
InpxLoader::parseInpFile(const std::string &buf,
                         const std::filesystem::path &result_path)
{
  UDBElement file;
  bid.setId(file, BaseID::File);
  std::u8string u8str = result_path.u8string();
  file.content = std::string(u8str.begin(), u8str.end());

  std::string book_entry_end;
  book_entry_end.reserve(2);
  book_entry_end.push_back(13);
  book_entry_end.push_back(10);

  std::string::size_type n1 = 0;
  std::string::size_type n2;
  for(;;)
    {
      n2 = buf.find(book_entry_end, n1);
      if(n2 == std::string::npos)
        {
          break;
        }

      std::string book_entry(buf.begin() + n1, buf.begin() + n2);
      if(book_entry.size() > 0)
        {
          UDBElement el = parseBookEntry(book_entry);
          if(!el.id.empty())
            {
              file.subelements.emplace_back(el);
            }
        }

      n1 = n2 + book_entry_end.size();
    }
  if(file.subelements.size() > 0)
    {
#pragma omp critical
      {
        result.addElement(file);
      }
    }
}

UDBElement
InpxLoader::parseBookEntry(const std::string &buf)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);

  std::string field_end;
  field_end.reserve(1);
  field_end.push_back(4);

  int count = 0;
  std::string::size_type n1 = 0;
  std::string::size_type n2;
  UDBElement sequence;
  bid.setId(sequence, BaseID::Sequence);

  UDBElement path;
  bid.setId(path, BaseID::PathInFile);
  for(;;)
    {
      n2 = buf.find(field_end, n1);
      if(n2 == std::string::npos)
        {
          break;
        }

      switch(count)
        {
        case 0:
          {
            std::string auth_buf;
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                break;
              }

            XMLTextEncoding::convertToEncoding(field, auth_buf, code_pages[0],
                                               "UTF-8");
            if(auth_buf.empty())
              {
                break;
              }
            std::vector<UDBElement> authors = parseAuthors(auth_buf);
            std::copy(authors.begin(), authors.end(),
                      std::back_inserter(result.subelements));
            break;
          }
        case 1:
          {
            std::string genre_buf;
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                break;
              }

            XMLTextEncoding::convertToEncoding(field, genre_buf, code_pages[0],
                                               "UTF-8");
            if(genre_buf.empty())
              {
                break;
              }
            std::vector<UDBElement> genres = parseGenres(genre_buf);
            std::copy(genres.begin(), genres.end(),
                      std::back_inserter(result.subelements));
            break;
          }
        case 2:
          {
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                result.id.clear();
                return result;
              }
            UDBElement el;
            XMLTextEncoding::convertToEncoding(field, el.content,
                                               code_pages[0], "UTF-8");
            if(el.content.empty())
              {
                result.id.clear();
                return result;
              }
            else
              {
                bid.setId(el, BaseID::BookTitle);
                result.subelements.emplace_back(el);
              }
            break;
          }
        case 3:
          {
            UDBElement el;
            bid.setId(el, BaseID::SequenceName);
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                break;
              }

            XMLTextEncoding::convertToEncoding(field, el.content,
                                               code_pages[0], "UTF-8");
            if(el.content.empty())
              {
                break;
              }
            sequence.subelements.emplace_back(el);
            break;
          }
        case 4:
          {
            if(sequence.subelements.size() > 0)
              {
                UDBElement el;
                bid.setId(el, BaseID::SequenceNumber);
                std::string field(buf.begin() + n1, buf.begin() + n2);

                std::vector<std::string> code_pages
                    = XMLTextEncoding::detectStringEncoding(field);
                if(code_pages.size() == 0)
                  {
                    break;
                  }

                XMLTextEncoding::convertToEncoding(field, el.content,
                                                   code_pages[0], "UTF-8");
                if(!el.content.empty())
                  {
                    sequence.subelements.emplace_back(el);
                  }
                result.subelements.emplace_back(sequence);
              }
            break;
          }
        case 5:
          {
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                result.id.clear();
                return result;
              }

            XMLTextEncoding::convertToEncoding(field, path.content,
                                               code_pages[0], "UTF-8");

            if(path.content.empty())
              {
                result.id.clear();
                return result;
              }
            break;
          }
        case 9:
          {
            std::string ext;
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                result.id.clear();
                return result;
              }

            XMLTextEncoding::convertToEncoding(field, ext, code_pages[0],
                                               "UTF-8");

            if(ext.empty())
              {
                result.id.clear();
                return result;
              }

            path.content += "." + ext;

            result.subelements.emplace_back(path);
            break;
          }
        case 10:
          {
            UDBElement el;
            bid.setId(el, BaseID::Date);
            std::string field(buf.begin() + n1, buf.begin() + n2);

            std::vector<std::string> code_pages
                = XMLTextEncoding::detectStringEncoding(field);
            if(code_pages.size() == 0)
              {
                break;
              }

            XMLTextEncoding::convertToEncoding(field, el.content,
                                               code_pages[0], "UTF-8");

            if(el.content.empty())
              {
                break;
              }

            result.subelements.emplace_back(el);

            break;
          }
        default:
          break;
        }

      n1 = n2 + field_end.size();
      count++;
    }

  return result;
}

std::vector<UDBElement>
InpxLoader::parseAuthors(const std::string &buf)
{
  std::vector<UDBElement> result;

  std::string separator(":");
  std::string::size_type n1 = 0;
  std::string::size_type n2;
  for(;;)
    {
      n2 = buf.find(separator, n1);
      if(n2 == std::string::npos)
        {
          break;
        }

      std::string author(buf.begin() + n1, buf.begin() + n2);
      if(!author.empty())
        {
          UDBElement el = parseAuthor(author);
          if(el.subelements.size() > 0)
            {
              result.emplace_back(el);
            }
        }

      n1 = n2 + separator.size();
    }

  return result;
}

UDBElement
InpxLoader::parseAuthor(const std::string &buf)
{
  UDBElement result;
  bid.setId(result, BaseID::Author);

  std::string separator(",");
  std::string::size_type n1 = 0;
  std::string::size_type n2;
  for(int i = 0; i < 3; i++)
    {
      UDBElement el;
      switch(i)
        {
        case 0:
          {
            bid.setId(el, BaseID::LastName);
            break;
          }
        case 1:
          {
            bid.setId(el, BaseID::FirstName);
            break;
          }
        case 2:
          {
            bid.setId(el, BaseID::MiddleName);
            break;
          }
        }
      n2 = buf.find(separator, n1);
      if(n2 == std::string::npos)
        {
          std::copy(buf.begin() + n1, buf.end(),
                    std::back_inserter(el.content));
          break;
        }
      else
        {
          std::copy(buf.begin() + n1, buf.begin() + n2,
                    std::back_inserter(el.content));
        }
      if(!el.content.empty())
        {
          result.subelements.emplace_back(el);
        }
      n1 = n2 + separator.size();
    }

  return result;
}

std::vector<UDBElement>
InpxLoader::parseGenres(const std::string &buf)
{
  std::vector<UDBElement> result;

  std::string separator(":");
  std::string::size_type n1 = 0;
  std::string::size_type n2;
  for(;;)
    {
      n2 = buf.find(separator, n1);
      if(n2 == std::string::npos)
        {
          break;
        }
      UDBElement el;
      bid.setId(el, BaseID::Genre);
      std::copy(buf.begin() + n1, buf.begin() + n2,
                std::back_inserter(el.content));
      if(!el.content.empty())
        {
          result.emplace_back(el);
        }
      n1 = n2 + separator.size();
    }

  return result;
}
