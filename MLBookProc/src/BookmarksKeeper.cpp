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
#include <BookmarksKeeper.h>
#include <ByteOrder.h>
#include <algorithm>
#include <fstream>
#include <iostream>

BookmarksKeeper::BookmarksKeeper(const std::shared_ptr<MLBookProc> &mlbp)
    : UDBase()
{
  this->mlbp = mlbp;
}

BookmarksKeeper::~BookmarksKeeper()
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
}

void
BookmarksKeeper::loadBookmarksBase(
    const std::filesystem::path &bookmarks_base_path)
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
  base_path = bookmarks_base_path;
  if(!std::filesystem::exists(base_path))
    {
      return void();
    }

  try
    {
      readFromFile(base_path);
    }
  catch(std::exception &er)
    {
      std::cout << "BookmarksKeeper::loadBookmarksBase: \"" << er.what()
                << "\"" << std::endl;
      loadLegacyBase();
    }
  this->shrinkToFit();
}

void
BookmarksKeeper::addToBookmarks(const UDBElement &book_search_result)
{
  UDBElement bookmark = book_search_result;
  bid.setId(bookmark, BaseID::BookMark);

  auto it_fl
      = std::find_if(bookmark.subelements.begin(), bookmark.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::File;
                       });
  if(it_fl == bookmark.subelements.end())
    {
      return void();
    }

  auto it_book
      = std::find_if(bookmark.subelements.begin(), bookmark.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Book;
                       });
  if(it_book == bookmark.subelements.end())
    {
      return void();
    }

  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  std::filesystem::path sp
      = std::u8string(it_fl->content.begin(), it_fl->content.end());

  std::lock_guard<std::shared_mutex> lglock(base_mtx);

  if(it_path == it_book->subelements.end())
    {
      Algorithm alg;
      auto it_base = alg.parallelFindIf(
          base.begin(), base.end(),
          [sp, this](const UDBElement &el)
            {
              if(bid.getId(el) == BaseID::BookMark)
                {
                  auto it = std::find_if(
                      el.subelements.begin(), el.subelements.end(),
                      [this](const UDBElement &el)
                        {
                          return bid.getId(el) == BaseID::File;
                        });
                  if(it != el.subelements.end())
                    {
                      std::filesystem::path p = std::u8string(
                          it->content.begin(), it->content.end());
                      return p == sp;
                    }
                }
              return false;
            });
      if(it_base != base.end())
        {
          return void();
        }
    }
  else
    {
      Algorithm alg;
      auto it_base = alg.parallelFindIf(
          base.begin(), base.end(),
          [it_path, this, sp](const UDBElement &el)
            {
              if(bid.getId(el) == BaseID::BookMark)
                {
                  auto it_fl = std::find_if(
                      el.subelements.begin(), el.subelements.end(),
                      [this](const UDBElement &el)
                        {
                          return bid.getId(el) == BaseID::File;
                        });
                  if(it_fl == el.subelements.end())
                    {
                      return false;
                    }
                  else
                    {
                      if(sp
                         != std::filesystem::path(std::u8string(
                             it_fl->content.begin(), it_fl->content.end())))
                        {
                          return false;
                        }
                    }
                  auto it_book = std::find_if(
                      el.subelements.begin(), el.subelements.end(),
                      [this](const UDBElement &el)
                        {
                          return bid.getId(el) == BaseID::Book;
                        });
                  if(it_book != el.subelements.end())
                    {
                      auto it_lpth = std::find_if(
                          it_book->subelements.begin(),
                          it_book->subelements.end(),
                          [this](const UDBElement &el)
                            {
                              return bid.getId(el) == BaseID::PathInFile;
                            });
                      if(it_lpth != it_book->subelements.end())
                        {
                          return *it_path == *it_lpth;
                        }
                    }
                }
              return false;
            });
      if(it_base != base.end())
        {
          return void();
        }
    }  
  addElement(bookmark);

  std::filesystem::create_directories(base_path.parent_path());

  std::filesystem::path tmp = base_path.parent_path() / mlbp->randomFileName();
  writeToFile(tmp);
  if(std::filesystem::exists(tmp))
    {
      std::filesystem::remove_all(base_path);
      std::filesystem::rename(tmp, base_path);
    }
}

void
BookmarksKeeper::removeBookmark(const UDBElement &bookmark)
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
  auto it_fl
      = std::find_if(bookmark.subelements.begin(), bookmark.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::File;
                       });
  if(it_fl == bookmark.subelements.end())
    {
      return void();
    }

  auto it_book
      = std::find_if(bookmark.subelements.begin(), bookmark.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Book;
                       });
  if(it_book == bookmark.subelements.end())
    {
      return void();
    }

  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  std::filesystem::path sp
      = std::u8string(it_fl->content.begin(), it_fl->content.end());

  if(it_path == it_book->subelements.end())
    {
      removeElements(
          [sp, this](const UDBElement &el)
            {
              if(bid.getId(el) != BaseID::BookMark)
                {
                  return false;
                }
              auto it
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::File;
                                   });
              if(it == el.subelements.end())
                {
                  return false;
                }

              if(sp
                 != std::filesystem::path(
                     std::u8string(it->content.begin(), it->content.end())))
                {
                  return false;
                }

              return true;
            });
    }
  else
    {
      removeElements(
          [this, it_path, sp](const UDBElement &el)
            {
              if(bid.getId(el) != BaseID::BookMark)
                {
                  return false;
                }
              auto it
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::File;
                                   });
              if(it == el.subelements.end())
                {
                  return false;
                }
              if(sp
                 != std::filesystem::path(
                     std::u8string(it->content.begin(), it->content.end())))
                {
                  return false;
                }

              auto it_book
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::Book;
                                   });
              if(it_book == el.subelements.end())
                {
                  return false;
                }

              auto it_p = std::find_if(
                  it_book->subelements.begin(), it_book->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_p == it_book->subelements.end())
                {
                  return false;
                }

              if(*it_path != *it_p)
                {
                  return false;
                }

              return true;
            });
    }

  if(baseSize() > 0)
    {
      std::filesystem::create_directories(base_path.parent_path());
      std::filesystem::path tmp
          = base_path.parent_path() / mlbp->randomFileName();
      writeToFile(tmp);
      if(std::filesystem::exists(tmp))
        {
          std::filesystem::remove_all(base_path);
          std::filesystem::rename(tmp, base_path);
        }
    }
  else
    {
      std::filesystem::remove_all(base_path.parent_path());
    }
}

std::shared_mutex *
BookmarksKeeper::getRawMutex()
{
  return &base_mtx;
}

void
BookmarksKeeper::loadLegacyBase()
{
  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      return void();
    }
  std::string buf;
  f.seekg(0, std::ios_base::end);
  std::fpos pos = f.tellg();
  if(pos <= 0)
    {
      return void();
    }
  buf.resize(static_cast<size_t>(pos));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  size_t rb = 0;
  uint64_t val64;
  size_t sz_64 = sizeof(val64);
  ByteOrder bo;
  size_t buf_sz = buf.size();

#pragma omp parallel
#pragma omp masked
  {
    while(rb < buf_sz)
      {
        if(rb + sz_64 > buf_sz)
          {
            throw std::runtime_error(
                "BookmarksKeeper::loadLegacyBase: incorrect entry size");
          }
        char *ptr = reinterpret_cast<char *>(&val64);
        for(size_t i = 0; i < sz_64; i++)
          {
            ptr[i] = buf[rb + i];
          }
        rb += sz_64;

        bo.setLittle(val64);
        val64 = bo;
        size_t sz = static_cast<size_t>(val64);
        if(rb + sz > buf.size())
          {
            throw std::runtime_error(
                "BookmarksKeeper::loadLegacyBase: incorrect entry");
          }
        std::string entry;
        entry.reserve(sz);
        std::copy(buf.begin() + rb, buf.begin() + rb + sz,
                  std::back_inserter(entry));
        rb += entry.size();

#pragma omp task
        {
          parseLegacyEntry(entry);
        }
      }
  }
}

void
BookmarksKeeper::parseLegacyEntry(const std::string &entry)
{
  UDBElement book_mark;
  bid.setId(book_mark, BaseID::BookMark);

  uint16_t val16;
  size_t sz_16 = sizeof(val16);
  size_t rb = 0;
  size_t ent_sz = entry.size();

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect collection name size");
    }
  char *ptr = reinterpret_cast<char *>(&val16);
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  ByteOrder bo;
  bo.setLittle(val16);
  val16 = bo;
  size_t sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect collection name");
    }
  rb += sz;

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect file path size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect file path");
    }
  UDBElement el;
  bid.setId(el, BaseID::File);
  el.content.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(el.content));
  rb += el.content.size();
  book_mark.subelements.emplace_back(el);

  UDBElement book;
  bid.setId(book, BaseID::Book);

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect book path size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect book path");
    }
  std::string str;
  str.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(str));
  rb += str.size();
  if(str.size() > 0)
    {
      UDBElement path;
      bid.setId(path, BaseID::PathInFile);
      UDBElement *prev = &path;
      std::string::size_type n = 0;
      std::string find_str("\n");
      while(n != std::string::npos)
        {
          n = str.find(find_str);
          if(n != std::string::npos)
            {
              prev->content = str.substr(0, n);
              str.erase(0, n + find_str.size());
              el = UDBElement();
              bid.setId(el, BaseID::PathInFile);
              prev->subelements.emplace_back(el);
              prev = &prev->subelements[prev->subelements.size() - 1];
            }
          else if(str.size() > 0)
            {
              prev->content = str;
            }
        }
      book.subelements.emplace_back(path);
    }

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect author size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect author");
    }
  str.clear();
  str.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(str));
  rb += str.size();
  if(str.size() > 0)
    {
      std::string find_str(", ");
      std::string::size_type n = 0;
      while(n != std::string::npos)
        {
          n = str.find(find_str);
          if(n != std::string::npos)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Author);
              el.content = str.substr(0, n);
              str.erase(0, n + find_str.size());
              book.subelements.emplace_back(el);
            }
          else if(str.size() > 0)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Author);
              el.content = str;
              book.subelements.emplace_back(el);
            }
        }
    }

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect book title size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect book title");
    }
  el = UDBElement();
  bid.setId(el, BaseID::BookTitle);
  el.content.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(el.content));
  rb += el.content.size();
  book.subelements.emplace_back(el);

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect series size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect series");
    }
  str.clear();
  str.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(str));
  rb += str.size();
  if(str.size() > 0)
    {
      std::string find_str(", ");
      std::string::size_type n = 0;
      while(n != std::string::npos)
        {
          n = str.find(find_str);
          if(n != std::string::npos)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Sequence);
              el.content = str.substr(0, n);
              str.erase(0, n + find_str.size());
              book.subelements.emplace_back(el);
            }
          else if(str.size() > 0)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Sequence);
              el.content = str;
              book.subelements.emplace_back(el);
            }
        }
    }

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect genre size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect genre");
    }
  str.clear();
  str.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(str));
  rb += str.size();
  if(str.size() > 0)
    {
      std::string find_str(", ");
      std::string::size_type n = 0;
      while(n != std::string::npos)
        {
          n = str.find(find_str);
          if(n != std::string::npos)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Genre);
              el.content = str.substr(0, n);
              str.erase(0, n + find_str.size());
              book.subelements.emplace_back(el);
            }
          else if(str.size() > 0)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Genre);
              el.content = str;
              book.subelements.emplace_back(el);
            }
        }
    }

  if(rb + sz_16 > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect date size");
    }
  for(size_t i = 0; i < sz_16; i++)
    {
      ptr[i] = entry[rb + i];
    }
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);

  if(rb + sz > ent_sz)
    {
      throw std::runtime_error(
          "BookmarksKeeper::parseLegacyEntry: incorrect date");
    }
  el = UDBElement();
  bid.setId(el, BaseID::Date);
  el.content.reserve(sz);
  std::copy(entry.begin() + rb, entry.begin() + rb + sz,
            std::back_inserter(el.content));
  rb += el.content.size();
  book.subelements.emplace_back(el);

  book_mark.subelements.emplace_back(book);

#pragma omp critical
  {
    addElement(book_mark);
  }
}
