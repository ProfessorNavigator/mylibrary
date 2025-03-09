/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <BaseKeeper.h>
#include <ByteOrder.h>
#include <MLException.h>
#include <NotesKeeper.h>
#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <thread>

#ifdef USE_OPENMP
#include <omp.h>
#endif

NotesKeeper::NotesKeeper(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  base_directory_path = af->homePath();
  base_directory_path
      /= std::filesystem::u8path(".local/share/MyLibrary/Notes");

  std::thread thr(std::bind(&NotesKeeper::loadBase, this));
  thr.detach();
}

NotesKeeper::~NotesKeeper()
{
  std::lock_guard<std::mutex> lglock(base_mtx);
}

void
NotesKeeper::loadBase()
{
  std::lock_guard<std::mutex> lglock(base_mtx);

  std::filesystem::path bp = base_directory_path;
  bp /= std::filesystem::u8path("notes_base");

  std::fstream f;
  f.open(bp, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string raw_base;
      f.seekg(0, std::ios_base::end);
      raw_base.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(raw_base.data(), raw_base.size());
      f.close();
      try
        {
          parseRawBase(raw_base);
        }
      catch(MLException &e)
        {
          std::cout << e.what() << std::endl;
        }
    }

  base.shrink_to_fit();
}

void
NotesKeeper::editNote(const NotesBaseEntry &nbe, const std::string &note)
{
  base_mtx.lock();
  auto it = std::find_if(std::execution::par, base.begin(), base.end(),
                         [nbe](NotesBaseEntry &el) {
                           return el == nbe;
                         });
  if(it != base.end())
    {
      if(note.size() == 0)
        {
          base.erase(it);
        }
    }
  else
    {
      if(note.size() > 0)
        {
          base.push_back(nbe);
          base.shrink_to_fit();
        }
    }

  std::filesystem::create_directories(nbe.note_file_full_path.parent_path());
  std::filesystem::remove_all(nbe.note_file_full_path);
  if(note.size() > 0)
    {
      std::fstream f;
      f.open(nbe.note_file_full_path,
             std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
        {
          std::string entry = nbe.collection_name + "\n";
          entry += nbe.book_file_full_path.u8string() + "\n";
          size_t sz = entry.size();
          for(auto it = nbe.book_path.begin(); it != nbe.book_path.end(); it++)
            {
              char v = *it;
              if(v == '\n')
                {
                  v = '/';
                }
              entry.push_back(v);
            }
          if(sz < entry.size())
            {
              entry += "\n\n";
            }
          else
            {
              entry += "\n";
            }
          entry += note;
          f.write(entry.c_str(), entry.size());
          f.close();
        }
    }
  base_mtx.unlock();
  saveBase();
}

NotesBaseEntry
NotesKeeper::getNote(const std::string &collection_name,
                     const std::filesystem::path &book_file_full_path,
                     const std::string &book_path)
{
  NotesBaseEntry nbe(collection_name, book_file_full_path, book_path);

  base_mtx.lock();
  auto it = std::find_if(std::execution::par, base.begin(), base.end(),
                         [nbe](NotesBaseEntry &el) {
                           return el == nbe;
                         });
  if(it != base.end())
    {
      nbe.note_file_full_path = it->note_file_full_path;
    }
  else
    {
      std::string rnd = af->randomFileName();
      nbe.note_file_full_path = base_directory_path;
      nbe.note_file_full_path /= std::filesystem::u8path(rnd);

      while(std::filesystem::exists(nbe.note_file_full_path))
        {
          rnd = af->randomFileName();
          nbe.note_file_full_path = base_directory_path;
          nbe.note_file_full_path /= std::filesystem::u8path(rnd);
        }
    }
  base_mtx.unlock();

  return nbe;
}

void
NotesKeeper::removeNotes(const NotesBaseEntry &nbe,
                         const std::filesystem::path &reserve_directory,
                         const bool &make_reserve)
{
  std::vector<std::filesystem::path> to_remove;

  base_mtx.lock();
  if(nbe.book_file_full_path.extension().u8string() == ".rar")
    {
      base.erase(std::remove_if(
                     base.begin(), base.end(),
                     [nbe, &to_remove](NotesBaseEntry &el) {
                       if(el.book_file_full_path == nbe.book_file_full_path)
                         {
                           to_remove.push_back(el.note_file_full_path);
                           return true;
                         }
                       else
                         {
                           return false;
                         }
                     }),
                 base.end());
    }
  else
    {
      base.erase(std::remove_if(base.begin(), base.end(),
                                [nbe, &to_remove](NotesBaseEntry &el) {
                                  if(el == nbe)
                                    {
                                      to_remove.push_back(
                                          el.note_file_full_path);
                                      return true;
                                    }
                                  else
                                    {
                                      return false;
                                    }
                                }),
                 base.end());
    }
  base_mtx.unlock();
  if(make_reserve)
    {
      if(to_remove.size() > 0)
        {
          std::filesystem::create_directories(reserve_directory);
        }
      for(auto it = to_remove.begin(); it != to_remove.end(); it++)
        {
          std::error_code ec;
          std::filesystem::path p = reserve_directory;
          p /= it->filename();
          std::filesystem::copy(*it, p, ec);
          if(ec)
            {
              std::cout << "NotesKeeper::removeNotes " << *it << ": "
                        << ec.message() << std::endl;
            }
          else
            {
              std::filesystem::remove_all(*it);
            }
        }
    }
  else
    {
      for(auto it = to_remove.begin(); it != to_remove.end(); it++)
        {
          std::filesystem::remove_all(*it);
        }
    }

  saveBase();
}

void
NotesKeeper::removeCollection(const std::string &collection_name,
                              const std::filesystem::path &reserve_directory,
                              const bool &make_reserve)
{
  std::vector<std::filesystem::path> to_remove;

  base_mtx.lock();
  base.erase(std::remove_if(base.begin(), base.end(),
                            [collection_name, &to_remove](NotesBaseEntry &el) {
                              if(collection_name == el.collection_name)
                                {
                                  to_remove.push_back(el.note_file_full_path);
                                  return true;
                                }
                              else
                                {
                                  return false;
                                }
                            }),
             base.end());
  base_mtx.unlock();

  saveBase();

  if(make_reserve)
    {
      if(to_remove.size() > 0)
        {
          std::filesystem::create_directories(reserve_directory);
        }
      for(auto it = to_remove.begin(); it != to_remove.end(); it++)
        {
          std::error_code ec;
          std::filesystem::path p = reserve_directory;
          p /= it->filename();
          std::filesystem::copy(*it, p, ec);
          if(ec)
            {
              std::cout << "NotesKeeper::removeCollection " << *it << " "
                        << ec.message() << std::endl;
            }
          else
            {
              std::filesystem::remove_all(*it);
            }
        }
    }
  else
    {
      for(auto it = to_remove.begin(); it != to_remove.end(); it++)
        {
          std::filesystem::remove_all(*it);
        }
    }
}

void
NotesKeeper::refreshCollection(const std::string &collection_name,
                               const std::filesystem::path &reserve_directory,
                               const bool &make_reserve)
{
  std::vector<NotesBaseEntry> to_remove;
  std::vector<NotesBaseEntry> to_check;
  base_mtx.lock();
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->collection_name == collection_name)
        {
          to_check.push_back(*it);
        }
    }
  base_mtx.unlock();

  if(to_check.size() > 0)
    {
      BaseKeeper bk(af);
      bk.loadCollection(collection_name);
      std::vector<FileParseEntry> bs = bk.get_base_vector();
      std::filesystem::path books_path
          = BaseKeeper::get_books_path(collection_name, af);
      for(auto it = to_check.begin(); it != to_check.end(); it++)
        {
          std::filesystem::path file_path = it->book_file_full_path;
          std::string b_p = it->book_path;
          if(b_p.empty())
            {
              auto it_bs = std::find_if(
                  bs.begin(), bs.end(),
                  [books_path, file_path](FileParseEntry &el) {
                    std::filesystem::path l_path = books_path;
                    l_path /= std::filesystem::u8path(el.file_rel_path);
                    return l_path == file_path;
                  });
              if(it_bs == bs.end())
                {
                  to_remove.push_back(*it);
                }
            }
          else
            {
              auto it_bs = std::find_if(
                  bs.begin(), bs.end(),
                  [books_path, file_path, b_p](FileParseEntry &el) {
                    std::filesystem::path l_path = books_path;
                    l_path /= std::filesystem::u8path(el.file_rel_path);
                    if(l_path == file_path)
                      {
                        auto it
                            = std::find_if(el.books.begin(), el.books.end(),
                                           [b_p](BookParseEntry &bbe) {
                                             return bbe.book_path == b_p;
                                           });
                        if(it != el.books.end())
                          {
                            return true;
                          }
                        else
                          {
                            return false;
                          }
                      }
                    else
                      {
                        return false;
                      }
                  });
              if(it_bs == bs.end())
                {
                  to_remove.push_back(*it);
                }
            }
        }
    }

  if(to_remove.size() > 0)
    {
      base_mtx.lock();
      base.erase(std::remove_if(base.begin(), base.end(),
                                [to_remove](NotesBaseEntry &el) {
                                  auto it = std::find(to_remove.begin(),
                                                      to_remove.end(), el);
                                  if(it != to_remove.end())
                                    {
                                      return true;
                                    }
                                  else
                                    {
                                      return false;
                                    }
                                }),
                 base.end());
      base.shrink_to_fit();
      base_mtx.unlock();
      saveBase();
    }
  if(make_reserve)
    {
      if(to_remove.size() > 0)
        {
          std::filesystem::create_directories(reserve_directory);
        }
      for(auto it = to_remove.begin(); it != to_remove.end(); it++)
        {
          std::filesystem::path p = reserve_directory;
          p /= it->note_file_full_path.filename();
          std::error_code ec;
          std::filesystem::copy(it->note_file_full_path, p, ec);
          if(ec)
            {
              std::cout << "NotesKeeper::refreshCollection "
                        << it->note_file_full_path << " " << ec.message()
                        << std::endl;
            }
          else
            {
              std::filesystem::remove_all(it->note_file_full_path);
            }
        }
    }
  else
    {
      for(auto it = to_remove.begin(); it != to_remove.end(); it++)
        {
          std::filesystem::remove_all(it->note_file_full_path);
        }
    }
}

std::vector<NotesBaseEntry>
NotesKeeper::getNotesForCollection(const std::string &collection_name)
{
  std::vector<NotesBaseEntry> result;
  std::mutex result_mtx;

  base_mtx.lock();
#ifndef USE_OPENMP
  std::for_each(std::execution::par, base.begin(), base.end(),
                [collection_name, &result, &result_mtx](NotesBaseEntry &el) {
                  if(el.collection_name == collection_name)
                    {
                      result_mtx.lock();
                      result.push_back(el);
                      result_mtx.unlock();
                    }
                });
#endif
#ifdef USE_OPENMP
#pragma omp parallel for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->collection_name == collection_name)
        {
          result_mtx.lock();
          result.push_back(*it);
          result_mtx.unlock();
        }
    }
#endif
  base_mtx.unlock();

  return result;
}

void
NotesKeeper::parseRawBase(const std::string &raw_base)
{
  size_t rb = 0;
  ByteOrder bo;
  uint64_t val64;
  size_t sz_64 = sizeof(val64);
  size_t limit = raw_base.size();
  size_t sz;
  while(rb < limit)
    {
      if(rb + sz_64 <= limit)
        {
          std::memcpy(&val64, &raw_base[rb], sz_64);
          rb += sz_64;
          bo.set_little(val64);
          val64 = bo;
        }
      else
        {
          throw MLException("NotesKeeper::parseRawBase: incorrect entry size");
        }

      sz = static_cast<size_t>(val64);

      if(rb + sz <= limit)
        {
          std::string entry = raw_base.substr(rb, sz);
          rb += sz;
          try
            {
              parseEntry(entry);
            }
          catch(MLException &e)
            {
              std::cout << e.what() << std::endl;
            }
        }
      else
        {
          throw MLException("NotesKeeper::parseRawBase: incorrect entry");
        }
    }
}

void
NotesKeeper::parseEntry(const std::string &entry)
{
  uint32_t val32;
  size_t sz_32 = sizeof(val32);
  size_t rb = 0;
  size_t limit = entry.size();
  ByteOrder bo;
  NotesBaseEntry nbe;
  size_t sz;
  for(int i = 1; i <= 4; i++)
    {
      if(rb + sz_32 <= limit)
        {
          std::memcpy(&val32, &entry[rb], sz_32);
          rb += sz_32;
          bo.set_little(val32);
          val32 = bo;
        }
      else
        {
          throw MLException("NotesKeeper::parseEntry: incorrect entry size");
        }
      sz = static_cast<size_t>(val32);
      if(rb + sz <= limit)
        {
          switch(i)
            {
            case 1:
              {
                nbe.collection_name = entry.substr(rb, sz);
                break;
              }
            case 2:
              {
                nbe.book_file_full_path
                    = std::filesystem::u8path(entry.substr(rb, sz));
                break;
              }
            case 3:
              {
                nbe.book_path = entry.substr(rb, sz);
                break;
              }
            case 4:
              {
                nbe.note_file_full_path
                    = std::filesystem::u8path(entry.substr(rb, sz));
                break;
              }
            }
          rb += sz;
        }
      else
        {
          throw MLException("NotesKeeper::parseEntry: incorrect entry");
        }
    }
  base.emplace_back(nbe);
}

void
NotesKeeper::saveBase()
{
  uint64_t val64;
  uint32_t val32;
  size_t sz_64 = sizeof(val64);
  size_t sz_32 = sizeof(val32);
  ByteOrder bo;
  size_t sz;

  std::filesystem::path bp = base_directory_path;
  bp /= std::filesystem::u8path("notes_base");
  std::filesystem::create_directories(bp.parent_path());

  std::lock_guard<std::mutex> lglock(base_mtx);
  std::filesystem::remove_all(bp);

  std::fstream f;
  f.open(bp, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      for(auto it = base.begin(); it != base.end(); it++)
        {
          std::string entry;
          for(int i = 1; i <= 4; i++)
            {
              switch(i)
                {
                case 1:
                  {
                    val32 = static_cast<uint32_t>(it->collection_name.size());
                    break;
                  }
                case 2:
                  {
                    val32 = static_cast<uint32_t>(
                        it->book_file_full_path.u8string().size());
                    break;
                  }
                case 3:
                  {
                    val32 = static_cast<uint32_t>(it->book_path.size());
                    break;
                  }
                case 4:
                  {
                    val32 = static_cast<uint32_t>(
                        it->note_file_full_path.u8string().size());
                    break;
                  }
                }
              bo = val32;
              bo.get_little(val32);

              sz = entry.size();
              entry.resize(sz + sz_32);
              std::memcpy(&entry[sz], &val32, sz_32);

              switch(i)
                {
                case 1:
                  {
                    entry += it->collection_name;
                    break;
                  }
                case 2:
                  {
                    entry += it->book_file_full_path.u8string();
                    break;
                  }
                case 3:
                  {
                    entry += it->book_path;
                    break;
                  }
                case 4:
                  {
                    entry += it->note_file_full_path.u8string();
                    break;
                  }
                }
            }

          val64 = entry.size();
          bo = val64;
          bo.get_little(val64);

          f.write(reinterpret_cast<char *>(&val64), sz_64);
          f.write(entry.c_str(), entry.size());
        }
      f.close();
    }
  else
    {
      std::cout << "NotesKeeper::saveBase: cannot save base!" << std::endl;
    }
}
