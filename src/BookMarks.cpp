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

#include <BookMarks.h>
#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <MLException.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stddef.h>
#include <string>
#include <thread>

BookMarks::BookMarks(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  bookmp = af->homePath();
  bookmp
      /= std::filesystem::u8path(".local/share/MyLibrary/BookMarks/bookmarks");
  std::thread thr([this] {
    try
      {
        loadBookMarks();
      }
    catch(MLException &e)
      {
        std::cout << e.what() << std::endl;
      };
  });
  thr.detach();
}

BookMarks::~BookMarks()
{
  saveBookMarks();
}

void
BookMarks::loadBookMarks()
{
  std::unique_lock<std::mutex> un_l(bookmarksmtx);
  std::fstream f;
  f.open(bookmp, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t fsz;

      f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);

      size_t bytesr = 0;
      uint64_t val64;
      std::string buf;
      ByteOrder bo;
      const size_t val64_sz = sizeof(val64);
      while(bytesr < fsz)
        {
          if(val64_sz > fsz - bytesr)
            {
              f.close();
              throw MLException("BookMarks::loadBookMarks: wrong entry");
            }
          else
            {
              f.read(reinterpret_cast<char *>(&val64), val64_sz);
              bytesr += val64_sz;
              bo.set_little(val64);
              val64 = bo;
            }

          if(static_cast<size_t>(val64) > fsz - bytesr)
            {
              f.close();
              throw MLException("BookMarks::loadBookMarks: wrong entry size");
            }
          else
            {
              buf.clear();
              buf.resize(static_cast<std::string::size_type>(val64));
              f.read(buf.data(), buf.size());
              bytesr += buf.size();
            }
          try
            {
              std::tuple<std::string, BookBaseEntry> bm_tup = parse_entry(buf);
              bookmarks.emplace_back(bm_tup);
            }
          catch(MLException &e)
            {
              bookmarks.clear();
              std::cout << e.what() << std::endl;
              break;
            }
        }
      f.close();
    }
}

std::tuple<std::string, BookBaseEntry>
BookMarks::parse_entry(const std::string &buf)
{
  std::tuple<std::string, BookBaseEntry> result;

  uint16_t val16;
  ByteOrder bo;
  const size_t val16_sz = sizeof(val16);
  std::string readval;
  size_t r_b = 0;
  for(int i = 1; i <= 8; i++)
    {
      if(buf.size() < r_b + val16_sz)
        {
          if(i == 8)
            {
              std::get<0>(result).clear();
              result = parse_entry_legacy(buf);
              break;
            }
          else
            {
              throw MLException(
                  "BookMarks::parse_entry: incorrect entry size");
            }
        }
      else
        {
          std::memcpy(&val16, &buf[r_b], val16_sz);
          r_b += val16_sz;
          bo.set_little(val16);
          val16 = bo;
        }
      if(buf.size() < r_b + static_cast<size_t>(val16))
        {
          throw MLException("BookMarks::parse_entry: wrong size(2)");
        }
      else
        {
          readval.clear();
          readval
              = std::string(buf.begin() + r_b,
                            buf.begin() + r_b + static_cast<size_t>(val16));
          r_b += static_cast<size_t>(val16);
        }
      switch(i)
        {
        case 1:
          {
            std::get<0>(result) = readval;
            break;
          }
        case 2:
          {
            std::get<1>(result).file_path = std::filesystem::u8path(readval);
            break;
          }
        case 3:
          {
            std::get<1>(result).bpe.book_path = readval;
            break;
          }
        case 4:
          {
            std::get<1>(result).bpe.book_author = readval;
            break;
          }
        case 5:
          {
            std::get<1>(result).bpe.book_name = readval;
            break;
          }
        case 6:
          {
            std::get<1>(result).bpe.book_series = readval;
            break;
          }
        case 7:
          {
            std::get<1>(result).bpe.book_genre = readval;
            break;
          }
        case 8:
          {
            std::get<1>(result).bpe.book_date = readval;
            break;
          }
        default:
          break;
        }
    }

  return result;
}

// TODO remove legacy code in next releases
std::tuple<std::string, BookBaseEntry>
BookMarks::parse_entry_legacy(const std::string &buf)
{
  std::tuple<std::string, BookBaseEntry> result;

  uint16_t val16;
  ByteOrder bo;
  const size_t val16_sz = sizeof(val16);
  std::string readval;
  size_t r_b = 0;
  for(int i = 1; i <= 7; i++)
    {
      if(buf.size() < val16_sz)
        {
          throw MLException("BookMarks::parse_entry_legacy: wrong size");
        }
      else
        {
          std::memcpy(&val16, &buf[r_b], val16_sz);
          r_b += val16_sz;
          bo.set_little(val16);
          val16 = bo;
        }
      if(buf.size() < static_cast<size_t>(val16))
        {
          throw MLException("BookMarks::parse_entry_legacy: wrong size(2)");
        }
      else
        {
          readval.clear();
          readval
              = std::string(buf.begin() + r_b,
                            buf.begin() + r_b + static_cast<size_t>(val16));
          r_b += static_cast<size_t>(val16);
        }
      switch(i)
        {
        case 1:
          {
            std::get<1>(result).file_path = std::filesystem::u8path(readval);
            break;
          }
        case 2:
          {
            std::get<1>(result).bpe.book_path = readval;
            break;
          }
        case 3:
          {
            std::get<1>(result).bpe.book_author = readval;
            break;
          }
        case 4:
          {
            std::get<1>(result).bpe.book_name = readval;
            break;
          }
        case 5:
          {
            std::get<1>(result).bpe.book_series = readval;
            break;
          }
        case 6:
          {
            std::get<1>(result).bpe.book_genre = readval;
            break;
          }
        case 7:
          {
            std::get<1>(result).bpe.book_date = readval;
            break;
          }
        default:
          break;
        }
    }

  return result;
}

bool
BookMarks::saveBookMarks()
{
  bool result = false;

  bookmarksmtx.lock();
  if(bookmarks.size() > 0)
    {
      std::filesystem::create_directories(bookmp.parent_path());
      std::fstream f;
      f.open(bookmp, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
        {
          result = true;
          uint64_t val64;
          const size_t val64_sz = sizeof(val64);
          std::string entry;
          ByteOrder bo;

          for(auto it = bookmarks.begin(); it != bookmarks.end(); it++)
            {
              entry = form_entry(std::get<0>(*it), std::get<1>(*it));
              val64 = static_cast<uint64_t>(entry.size());
              bo = val64;
              bo.get_little(val64);
              f.write(reinterpret_cast<char *>(&val64), val64_sz);
              f.write(entry.c_str(), entry.size());
            }
          f.close();
        }
    }
  else
    {
      std::filesystem::remove_all(bookmp);
    }
  bookmarksmtx.unlock();

  return result;
}

int
BookMarks::createBookMark(const std::string &col_name,
                          const BookBaseEntry &bbe)
{
  int result = 0;

  bool save = false;

  std::tuple<std::string, BookBaseEntry> bm_tup
      = std::make_tuple(col_name, bbe);
  bookmarksmtx.lock();
  auto itbm
      = std::find_if(bookmarks.begin(), bookmarks.end(),
                     [bm_tup](std::tuple<std::string, BookBaseEntry> &el) {
                       if(std::get<0>(el) == std::get<0>(bm_tup)
                          && std::get<1>(el) == std::get<1>(bm_tup))
                         {
                           return true;
                         }
                       else
                         {
                           return false;
                         }
                     });
  if(itbm == bookmarks.end())
    {
      bookmarks.emplace_back(bm_tup);
      save = true;
    }
  bookmarksmtx.unlock();

  if(save)
    {
      if(saveBookMarks())
        {
          result = 1;
        }
      else
        {
          result = -1;
        }
    }

  return result;
}

std::vector<std::tuple<std::string, BookBaseEntry>>
BookMarks::getBookMarks()
{
  std::vector<std::tuple<std::string, BookBaseEntry>> result;

  bookmarksmtx.lock();
  result = bookmarks;
  bookmarksmtx.unlock();

  return result;
}

void
BookMarks::removeBookMark(const std::string &col_name,
                          const BookBaseEntry &bbe)
{
  bool save = false;

  std::tuple<std::string, BookBaseEntry> bm_tup
      = std::make_tuple(col_name, bbe);

  bookmarksmtx.lock();
  auto itbm
      = std::find_if(bookmarks.begin(), bookmarks.end(),
                     [bm_tup](std::tuple<std::string, BookBaseEntry> &el) {
                       if(std::get<0>(el) == std::get<0>(bm_tup)
                          && std::get<1>(el) == std::get<1>(bm_tup))
                         {
                           return true;
                         }
                       else
                         {
                           return false;
                         }
                     });
  if(itbm != bookmarks.end())
    {
      bookmarks.erase(itbm);
      save = true;
    }
  bookmarksmtx.unlock();

  if(save)
    {
      saveBookMarks();
    }
}

std::string
BookMarks::form_entry(const std::string &col_name, const BookBaseEntry &bbe)
{
  std::string result;
  uint16_t val16;
  ByteOrder bo;
  size_t sz;
  std::string val;
  const size_t val16_sz = sizeof(val16);
  for(int i = 1; i <= 8; i++)
    {
      switch(i)
        {
        case 1:
          {
            val = col_name;
            break;
          }
        case 2:
          {
            val = bbe.file_path.u8string();
            break;
          }
        case 3:
          {
            val = bbe.bpe.book_path;
            break;
          }
        case 4:
          {
            val = bbe.bpe.book_author;
            break;
          }
        case 5:
          {
            val = bbe.bpe.book_name;
            break;
          }
        case 6:
          {
            val = bbe.bpe.book_series;
            break;
          }
        case 7:
          {
            val = bbe.bpe.book_genre;
            break;
          }
        case 8:
          {
            val = bbe.bpe.book_date;
            break;
          }
        default:
          break;
        }

      val16 = static_cast<uint16_t>(val.size());
      bo = val16;
      bo.get_little(val16);
      sz = result.size();
      result.resize(sz + val16_sz);
      std::memcpy(&result[sz], &val16, val16_sz);

      std::copy(val.begin(), val.end(), std::back_inserter(result));
    }

  return result;
}
