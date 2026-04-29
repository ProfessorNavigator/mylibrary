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

#include <ByteOrder.h>
#include <NotesKeeper.h>
#include <algorithm>
#include <fstream>
#include <iostream>

NotesKeeper::NotesKeeper(const std::shared_ptr<MLBookProc> &mlbp) : UDBase()
{
  this->mlbp = mlbp;
}

void
NotesKeeper::loadNotesBase(const std::filesystem::path &base_path)
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
  this->base_path = base_path;

  try
    {
      readFromFile(base_path);
    }
  catch(std::exception &er)
    {
      std::cout << "NotesKeeper::loadNotesBase: \"" << er.what() << "\""
                << std::endl;      
      loadLegacyBase(base_path);
    }
  shrinkToFit();
}

UDBase
NotesKeeper::searchNotes(const UDBElement &book_search_result)
{
  UDBase result;

  auto it_fl = std::find_if(book_search_result.subelements.begin(),
                            book_search_result.subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::File;
                              });
  if(it_fl == book_search_result.subelements.end())
    {
      return result;
    }

  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == book_search_result.subelements.end())
    {
      return result;
    }

  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });

  std::filesystem::path fp
      = std::u8string(it_fl->content.begin(), it_fl->content.end());
  std::shared_lock shlock(base_mtx);

  if(it_path == it_book->subelements.end())
    {
      result = std::move(searchElement(
          [fp, this](const UDBElement &el)
            {
              auto it_fl
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::File;
                                   });
              if(it_fl == el.subelements.end())
                {
                  return false;
                }
              std::filesystem::path p = std::u8string(it_fl->content.begin(),
                                                      it_fl->content.end());
              if(fp != p)
                {
                  return false;
                }
              return true;
            }));
    }
  else
    {
      result = std::move(searchElement(
          [this, fp, it_path](const UDBElement &el)
            {
              auto it_fl
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::File;
                                   });
              if(it_fl == el.subelements.end())
                {
                  return false;
                }
              std::filesystem::path p = std::u8string(it_fl->content.begin(),
                                                      it_fl->content.end());
              if(fp != p)
                {
                  return false;
                }

              auto it_p = std::find_if(
                  el.subelements.begin(), el.subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_p == el.subelements.end())
                {
                  return false;
                }
              if(*it_p != *it_path)
                {
                  return false;
                }
              return true;
            }));
    }

  return result;
}

void
NotesKeeper::addNote(const UDBElement &book_search_result,
                     const std::string &note_txt)
{
  auto it_fl = std::find_if(book_search_result.subelements.begin(),
                            book_search_result.subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::File;
                              });
  if(it_fl == book_search_result.subelements.end())
    {
      return void();
    }

  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == book_search_result.subelements.end())
    {
      return void();
    }

  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });

  std::filesystem::path fp
      = std::u8string(it_fl->content.begin(), it_fl->content.end());

  std::lock_guard<std::shared_mutex> lglock(base_mtx);

  if(it_path == it_book->subelements.end())
    {
      auto it_note = std::find_if(
          base.begin(), base.end(),
          [fp, this](const UDBElement &el)
            {
              auto it_fl
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::File;
                                   });
              if(it_fl == el.subelements.end())
                {
                  return false;
                }
              std::filesystem::path p = std::u8string(it_fl->content.begin(),
                                                      it_fl->content.end());
              if(fp != p)
                {
                  return false;
                }
              return true;
            });
      if(it_note == base.end())
        {
          if(note_txt.empty())
            {
              return void();
            }
          UDBElement note;
          bid.setId(note, BaseID::BookNote);

          note.subelements.push_back(*it_fl);

          std::filesystem::path p
              = base_path.parent_path() / mlbp->randomFileName();
          while(std::filesystem::exists(p))
            {
              p = base_path.parent_path() / mlbp->randomFileName();
            }

          std::filesystem::create_directories(p.parent_path());
          std::fstream f;
          f.open(p, std::ios_base::out | std::ios_base::binary);
          if(!f.is_open())
            {
              throw std::runtime_error(
                  "NotesKeeper::addNote: cannot open note file");
            }
          std::string w_str = it_fl->content + "\n\n";
          f.write(w_str.c_str(), w_str.size());
          f.write(note_txt.c_str(), note_txt.size());
          f.close();

          UDBElement el;
          bid.setId(el, BaseID::BookNoteFile);
          std::u8string u8str = p.u8string();
          el.content = std::string(u8str.begin(), u8str.end());
          note.subelements.emplace_back(el);

          base.emplace_back(note);
        }
      else
        {
          auto it_p = std::find_if(
              it_note->subelements.begin(), it_note->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::BookNoteFile;
                });
          if(it_p == it_note->subelements.end())
            {
              throw std::runtime_error(
                  "NotesKeeper::addNote: incorrect note entry");
            }

          if(note_txt.empty())
            {
              std::filesystem::path p
                  = std::u8string(it_p->content.begin(), it_p->content.end());
              std::filesystem::remove_all(p);
              base.erase(it_note);
              std::filesystem::path tmp
                  = base_path.parent_path() / mlbp->randomFileName();
              while(std::filesystem::exists(tmp))
                {
                  tmp = base_path.parent_path() / mlbp->randomFileName();
                }
              writeToFile(tmp);
              std::filesystem::remove_all(base_path);
              std::filesystem::rename(tmp, base_path);
              return void();
            }

          std::filesystem::path tmp
              = base_path.parent_path() / mlbp->randomFileName();
          while(std::filesystem::exists(tmp))
            {
              tmp = base_path.parent_path() / mlbp->randomFileName();
            }

          std::filesystem::create_directories(tmp.parent_path());
          std::fstream f;
          f.open(tmp, std::ios_base::out | std::ios_base::binary);
          if(!f.is_open())
            {
              throw std::runtime_error(
                  "NotesKeeper::addNote: cannot open note file");
            }
          std::string w_str = it_fl->content + "\n\n";
          f.write(w_str.c_str(), w_str.size());
          f.write(note_txt.c_str(), note_txt.size());
          f.close();

          std::filesystem::path p
              = std::u8string(it_p->content.begin(), it_p->content.end());
          std::filesystem::remove_all(p);
          std::filesystem::rename(tmp, p);
        }
    }
  else
    {
      auto it_note = std::find_if(
          base.begin(), base.end(),
          [this, fp, it_path](const UDBElement &el)
            {
              auto it_fl
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::File;
                                   });
              if(it_fl == el.subelements.end())
                {
                  return false;
                }
              std::filesystem::path p = std::u8string(it_fl->content.begin(),
                                                      it_fl->content.end());
              if(fp != p)
                {
                  return false;
                }

              auto it_p = std::find_if(
                  el.subelements.begin(), el.subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_p == el.subelements.end())
                {
                  return false;
                }
              if(*it_p != *it_path)
                {
                  return false;
                }
              return true;
            });
      if(it_note == base.end())
        {
          if(note_txt.empty())
            {
              return void();
            }
          UDBElement note;
          bid.setId(note, BaseID::BookNote);

          note.subelements.push_back(*it_fl);
          note.subelements.push_back(*it_path);

          std::filesystem::path p
              = base_path.parent_path() / mlbp->randomFileName();
          while(std::filesystem::exists(p))
            {
              p = base_path.parent_path() / mlbp->randomFileName();
            }

          std::filesystem::create_directories(p.parent_path());
          std::fstream f;
          f.open(p, std::ios_base::out | std::ios_base::binary);
          if(!f.is_open())
            {
              throw std::runtime_error(
                  "NotesKeeper::addNote: cannot open note file");
            }
          std::string w_str = it_fl->content + "\n";
          w_str += it_path->content;
          for(;;)
            {
              auto it_p = std::find_if(
                  it_path->subelements.begin(), it_path->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_p == it_path->subelements.end())
                {
                  break;
                }
              else
                {
                  w_str += "/";
                  w_str += it_p->content;
                  it_path = it_p;
                }
            }
          w_str += "\n\n";

          f.write(w_str.c_str(), w_str.size());
          f.write(note_txt.c_str(), note_txt.size());
          f.close();

          UDBElement el;
          bid.setId(el, BaseID::BookNoteFile);
          std::u8string u8str = p.u8string();
          el.content = std::string(u8str.begin(), u8str.end());
          note.subelements.emplace_back(el);

          base.emplace_back(note);
        }
      else
        {
          auto it_p = std::find_if(
              it_note->subelements.begin(), it_note->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::BookNoteFile;
                });
          if(it_p == it_note->subelements.end())
            {
              throw std::runtime_error(
                  "NotesKeeper::addNote: incorrect note entry");
            }

          if(note_txt.empty())
            {
              std::filesystem::path p
                  = std::u8string(it_p->content.begin(), it_p->content.end());
              std::filesystem::remove_all(p);
              base.erase(it_note);
              std::filesystem::path tmp
                  = base_path.parent_path() / mlbp->randomFileName();
              while(std::filesystem::exists(tmp))
                {
                  tmp = base_path.parent_path() / mlbp->randomFileName();
                }
              writeToFile(tmp);
              std::filesystem::remove_all(base_path);
              std::filesystem::rename(tmp, base_path);
              return void();
            }

          std::filesystem::path tmp
              = base_path.parent_path() / mlbp->randomFileName();
          while(std::filesystem::exists(tmp))
            {
              tmp = base_path.parent_path() / mlbp->randomFileName();
            }

          std::filesystem::create_directories(tmp.parent_path());
          std::fstream f;
          f.open(tmp, std::ios_base::out | std::ios_base::binary);
          if(!f.is_open())
            {
              throw std::runtime_error(
                  "NotesKeeper::addNote: cannot open note file");
            }
          std::string w_str = it_fl->content + "\n";
          w_str += it_path->content;
          for(;;)
            {
              auto it_pin = std::find_if(
                  it_path->subelements.begin(), it_path->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_pin == it_path->subelements.end())
                {
                  break;
                }
              else
                {
                  w_str += "/";
                  w_str += it_p->content;
                  it_path = it_pin;
                }
            }
          w_str += "\n\n";
          f.write(w_str.c_str(), w_str.size());
          f.write(note_txt.c_str(), note_txt.size());
          f.close();

          std::filesystem::path p
              = std::u8string(it_p->content.begin(), it_p->content.end());
          std::filesystem::remove_all(p);
          std::filesystem::rename(tmp, p);
        }
    }

  std::filesystem::path tmp = base_path.parent_path() / mlbp->randomFileName();
  while(std::filesystem::exists(tmp))
    {
      tmp = base_path.parent_path() / mlbp->randomFileName();
    }
  writeToFile(tmp);
  std::filesystem::remove_all(base_path);
  std::filesystem::rename(tmp, base_path);
}

void
NotesKeeper::removeNote(const UDBElement &note)
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
  auto it = std::find(base.begin(), base.end(), note);
  if(it == base.end())
    {
      return void();
    }
  auto it_fl = std::find_if(it->subelements.begin(), it->subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::BookNoteFile;
                              });
  if(it_fl != it->subelements.end())
    {
      std::filesystem::path p
          = std::u8string(it_fl->content.begin(), it_fl->content.end());
      std::filesystem::remove_all(p);
    }
  base.erase(it);

  std::filesystem::path tmp = base_path.parent_path() / mlbp->randomFileName();
  while(std::filesystem::exists(tmp))
    {
      tmp = base_path.parent_path() / mlbp->randomFileName();
    }
  writeToFile(tmp);
  std::filesystem::remove_all(base_path);
  std::filesystem::rename(tmp, base_path);
}

void
NotesKeeper::loadLegacyBase(const std::filesystem::path &base_path)
{
  std::string buf;
  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      return void();
    }
  f.seekg(0, std::ios_base::end);
  buf.resize(static_cast<size_t>(f.tellg()));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  size_t rb = 0;
  size_t buf_sz = buf.size();
  uint64_t val64;
  size_t sz_64 = sizeof(val64);
  char *ptr = reinterpret_cast<char *>(&val64);
  ByteOrder bo;

#pragma omp parallel
#pragma omp masked
  {
    while(rb < buf_sz)
      {
        if(rb + sz_64 > buf_sz)
          {
            base.clear();
            throw std::runtime_error(
                "NotesKeeper::loadLegacyBase: incorrect entry size");
          }
        for(size_t i = 0; i < sz_64; i++)
          {
            ptr[i] = buf[rb + i];
          }
        rb += sz_64;

        bo.setLittle(val64);
        val64 = bo;

        size_t sz = static_cast<size_t>(val64);
        if(rb + sz > buf_sz)
          {
            base.clear();
            throw std::runtime_error(
                "NotesKeeper::loadLegacyBase: incorrect entry");
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
NotesKeeper::parseLegacyEntry(const std::string &entry)
{
  size_t rb = 0;
  uint32_t val32;
  size_t sz_32 = sizeof(val32);
  char *ptr = reinterpret_cast<char *>(&val32);
  ByteOrder bo;
  size_t entry_sz = entry.size();

  UDBElement note_entry;
  bid.setId(note_entry, BaseID::BookNote);

  int count = 1;
  while(rb < entry_sz && count <= 4)
    {
      if(rb + sz_32 > entry_sz)
        {          
          throw std::runtime_error(
              "NotesKeeper::parseLegacyEntry: incorrect size");
        }
      for(size_t j = 0; j < sz_32; j++)
        {
          ptr[j] = entry[rb + j];
        }
      rb += sz_32;

      bo.setLittle(val32);
      val32 = bo;

      size_t sz = static_cast<size_t>(val32);
      if(rb + sz > entry_sz)
        {
          throw std::runtime_error(
              "NotesKeeper::parseLegacyEntry: incorrect entry");
        }
      std::string l_entry;
      l_entry.reserve(sz);
      std::copy(entry.begin() + rb, entry.begin() + rb + sz,
                std::back_inserter(l_entry));
      rb += l_entry.size();

      switch(count)
        {
        case 2:
          {
            UDBElement el;
            bid.setId(el, BaseID::File);
            el.content = std::move(l_entry);
            note_entry.subelements.emplace_back(el);
            break;
          }
        case 3:
          {
            if(l_entry.size() > 0)
              {
                UDBElement path;
                bid.setId(path, BaseID::PathInFile);
                UDBElement *prev = &path;
                std::string::size_type n = 0;
                std::string find_str("\n");
                while(n != std::string::npos)
                  {
                    n = l_entry.find(find_str);
                    if(n != std::string::npos)
                      {
                        prev->content = l_entry.substr(0, n);
                        l_entry.erase(0, n + find_str.size());
                        UDBElement el;
                        bid.setId(el, BaseID::PathInFile);
                        prev->subelements.emplace_back(el);
                        prev
                            = &prev->subelements[prev->subelements.size() - 1];
                      }
                    else if(l_entry.size() > 0)
                      {
                        prev->content = l_entry;
                      }
                  }
                note_entry.subelements.emplace_back(path);
              }
            break;
          }
        case 4:
          {
            UDBElement el;
            bid.setId(el, BaseID::BookNoteFile);
            el.content = std::move(l_entry);
            note_entry.subelements.emplace_back(el);
            break;
          }
        default:
          break;
        }

      count++;
    }

#pragma omp critical
  {
    base.emplace_back(note_entry);
  }
}
