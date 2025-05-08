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

#include <BaseKeeper.h>
#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <MLException.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

#ifdef USE_OPENMP
#include <OmpLockGuard.h>
#else
#ifdef USE_PE
#include <execution>
#endif
#endif

BaseKeeper::BaseKeeper(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
#ifdef USE_OPENMP
  omp_init_lock(&basemtx);
#pragma atomic write
  cancel_search = true;
#else
  cancel_search.store(false);
#endif
}

BaseKeeper::~BaseKeeper()
{
#ifdef USE_OPENMP
  omp_destroy_lock(&basemtx);
#endif
}

void
BaseKeeper::loadCollection(const std::string &col_name)
{
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lock_base(basemtx);
#else
  OmpLockGuard lock_base(basemtx);
#endif
  base.clear();
  collection_path.clear();
  collection_name = col_name;
  std::filesystem::path base_path = af->homePath();
  base_path /= std::filesystem::u8path(".local")
               / std::filesystem::u8path("share")
               / std::filesystem::u8path("MyLibrary")
               / std::filesystem::u8path("Collections");
  base_path /= std::filesystem::u8path(col_name);
  base_path /= std::filesystem::u8path("base");

  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string base_str;
      f.seekg(0, std::ios_base::end);
      base_str.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(base_str.data(), base_str.size());
      f.close();

      uint16_t val16;
      size_t rb;
      size_t sz = sizeof(val16);
      if(base_str.size() >= sz)
        {
          std::memcpy(&val16, &base_str[0], sz);
          rb = sz;
        }
      else
        {
          collection_path.clear();
          collection_name.clear();
          throw MLException("BaseKeeper::loadCollection: collection path size "
                            "reading error");
        }

      ByteOrder bo;
      bo.set_little(val16);
      val16 = bo;
      if(static_cast<size_t>(val16) > base_str.size() - rb)
        {
          collection_path.clear();
          collection_name.clear();
          throw MLException(
              "BaseKeeper::loadCollection: collection path reading error");
        }
      std::string ent(base_str.begin() + rb, base_str.begin() + rb + val16);
      rb += ent.size();
      collection_path = std::filesystem::u8path(ent);
      if(!std::filesystem::exists(collection_path))
        {
          collection_path.clear();
          collection_name.clear();
          throw MLException(
              "BaseKeeper::loadCollection: collection path error");
        }

      while(rb < base_str.size())
        {
          try
            {
              base.emplace_back(readFileEntry(base_str, rb));
            }
          catch(MLException &er)
            {
              collection_path.clear();
              collection_name.clear();
              base.clear();
              std::cout << er.what() << std::endl;
              return void();
            }
        }
      base.shrink_to_fit();
    }
}

FileParseEntry
BaseKeeper::readFileEntry(const std::string &base_str, size_t &rb)
{
  uint64_t val64;
  size_t sz = sizeof(val64);
  if(base_str.size() + rb < sz)
    {
      throw MLException("BaseKeeper::readFileEntry: incorrect base size");
    }
  std::memcpy(&val64, &base_str[rb], sz);
  rb += sz;

  ByteOrder bo;
  bo.set_little(val64);
  val64 = bo;

  if(val64 == 0 || base_str.size() - rb < static_cast<size_t>(val64))
    {
      throw MLException("BaseKeeper::readFileEntry: incorrect entry size");
    }

  std::string entry(base_str.begin() + rb, base_str.begin() + rb + val64);
  rb += entry.size();

  uint16_t val16;
  sz = sizeof(val16);
  if(entry.size() < sz)
    {
      throw MLException(
          "BaseKeeper::readFileEntry: incorrect file address entry size");
    }
  std::memcpy(&val16, &entry[0], sz);
  size_t lrb = sz;
  bo.set_little(val16);
  val16 = bo;

  if(val16 == 0 || entry.size() - lrb < static_cast<size_t>(val16))
    {
      throw MLException(
          "BaseKeeper::readFileEntry: incorrect file address size");
    }

  FileParseEntry fpe;

  std::string e(entry.begin() + lrb, entry.begin() + lrb + val16);
  lrb += e.size();
  fpe.file_rel_path = e;

  sz = sizeof(val16);
  if(entry.size() - lrb < sz)
    {
      throw MLException(
          "BaseKeeper::readFileEntry: incorrect file hash entry size");
    }
  std::memcpy(&val16, &entry[lrb], sz);
  lrb += sz;
  bo.set_little(val16);
  val16 = bo;

  if(val16 == 0 || entry.size() - lrb < static_cast<size_t>(val16))
    {
      throw MLException("BaseKeeper::readFileEntry: incorrect file hash size");
    }
  e = std::string(entry.begin() + lrb, entry.begin() + lrb + val16);
  lrb += static_cast<size_t>(val16);
  fpe.file_hash = e;

  fpe.books = readBookEntry(entry, lrb);

  return fpe;
}

std::vector<BookParseEntry>
BaseKeeper::readBookEntry(const std::string &entry, size_t &rb)
{
  std::vector<BookParseEntry> result;
  uint64_t val64;
  ByteOrder bo;
  std::string book_e;
  size_t sz = sizeof(val64);
  while(rb < entry.size())
    {
      if(entry.size() - rb < sz)
        {
          throw MLException(
              "BaseKeeper::readBookEntry: incorrect book entry size");
        }
      std::memcpy(&val64, &entry[rb], sz);
      rb += sz;

      bo.set_little(val64);
      val64 = bo;

      if(val64 == 0 || entry.size() - rb < static_cast<size_t>(val64))
        {
          throw MLException(
              "BaseKeeper::readBookEntry: incorrect book entry size(2)");
        }
      book_e = std::string(entry.begin() + rb, entry.begin() + rb + val64);
      rb += book_e.size();

      BookParseEntry bpe;
      size_t lrb = 0;
      parseBookEntry(book_e, bpe.book_path, lrb);
      parseBookEntry(book_e, bpe.book_author, lrb);
      parseBookEntry(book_e, bpe.book_name, lrb);
      parseBookEntry(book_e, bpe.book_series, lrb);
      parseBookEntry(book_e, bpe.book_genre, lrb);
      parseBookEntry(book_e, bpe.book_date, lrb);
      result.emplace_back(bpe);
    }

  return result;
}

void
BaseKeeper::parseBookEntry(const std::string &e, std::string &read_val,
                           size_t &rb)
{
  uint16_t val16;
  size_t sz = sizeof(val16);
  if(e.size() - rb < sz)
    {
      throw MLException(
          "BaseKeeper::parseBookEntry: incorrect book entry size");
    }
  std::memcpy(&val16, &e[rb], sz);
  rb += sz;

  ByteOrder bo;
  bo.set_little(val16);
  val16 = bo;
  if(val16 > 0)
    {
      read_val = std::string(e.begin() + rb, e.begin() + rb + val16);
      rb += read_val.size();
    }
}

std::vector<BookBaseEntry>
BaseKeeper::searchBook(const BookBaseEntry &search)
{
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lock_base(basemtx);
  cancel_search.store(false);
#else
  OmpLockGuard lock_base(basemtx);
#pragma omp atomic write
  cancel_search = false;
#endif
  std::vector<BookBaseEntry> result;
  result.reserve(base.size());
  bool stop_search = false;
  bool all_empty = true;
  if(!search.bpe.book_author.empty() || !search.bpe.book_date.empty()
     || !search.bpe.book_genre.empty() || !search.bpe.book_name.empty()
     || !search.bpe.book_series.empty())
    {
#ifdef USE_OPENMP
      bool c_s;
#endif
      for(int i = 1; i <= 6; i++)
        {
#ifdef USE_OPENMP
#pragma omp atomic read
          c_s = cancel_search;
          if(c_s)
            {
              break;
            }
#else
          if(cancel_search.load())
            {
              break;
            }
#endif
          switch(i)
            {
            case 1:
              {
                if(!searchSurname(search, result))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
                break;
              }
            case 2:
              {
                if(!searchFirstName(search, result))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
                break;
              }
            case 3:
              {
                if(!searchLastName(search, result))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
                break;
              }
            case 4:
              {
                if(!search.bpe.book_name.empty())
                  {
                    searchBook(search, result);
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
                break;
              }
            case 5:
              {
                if(!search.bpe.book_series.empty())
                  {
                    searchSeries(search, result);
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
                break;
              }
            case 6:
              {
                if(!search.bpe.book_genre.empty())
                  {
                    searchGenre(search, result);
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
                break;
              }
            default:
              break;
            }
          if(stop_search)
            {
              break;
            }
        }
    }
#ifndef USE_OPENMP
  if(all_empty && !cancel_search.load())
    {
      for(auto it = base.begin(); it != base.end(); it++)
        {
          if(cancel_search.load())
            {
              break;
            }
          std::filesystem::path book_file_path = collection_path;
          book_file_path /= std::filesystem::u8path(it->file_rel_path);
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
              if(cancel_search.load())
                {
                  break;
                }
              result.emplace_back(BookBaseEntry(*itb, book_file_path));
            }
        }
    }
  if(cancel_search.load())
    {
      result.clear();
    }
#endif

#ifdef USE_OPENMP
  bool c_s;
#pragma omp atomic read
  c_s = cancel_search;
  if(all_empty && !c_s)
    {
      for(auto it = base.begin(); it != base.end(); it++)
        {
#pragma omp atomic read
          c_s = cancel_search;
          if(c_s)
            {
              break;
            }
          std::filesystem::path book_file_path = collection_path;
          book_file_path /= std::filesystem::u8path(it->file_rel_path);
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
#pragma omp atomic read
              c_s = cancel_search;
              if(c_s)
                {
                  break;
                }
              result.emplace_back(BookBaseEntry(*itb, book_file_path));
            }
        }
    }
#pragma omp atomic read
  c_s = cancel_search;
  if(c_s)
    {
      result.clear();
    }
#endif

  return result;
}

std::vector<std::string>
BaseKeeper::collectionAuthors()
{
  std::vector<std::string> result;
#ifdef USE_OPENMP
#pragma omp atomic write
  cancel_search = false;
  omp_set_lock(&basemtx);
#else
  cancel_search.store(false);
  basemtx.lock();
#endif
  std::vector<std::tuple<std::string, bool>> auth_v;
  {
    size_t sz = 0;
    for(auto it = base.begin(); it != base.end(); it++)
      {
        if(it->books.size() == 0)
          {
            sz += 1;
          }
        else
          {
            sz += it->books.size();
          }
      }
    auth_v.reserve(sz);
  }
  std::string find_str = ", ";
#ifdef USE_OPENMP
  omp_set_dynamic(true);
  int lvls = omp_get_max_active_levels();
  omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
  {
#pragma omp for
    for(auto it = base.begin(); it != base.end(); it++)
      {
        bool c_s;
#pragma omp atomic read
        c_s = cancel_search;
        if(c_s)
          {
#pragma omp cancel for
            continue;
          }
#pragma omp parallel
        {
#pragma omp for
          for(auto it_b = it->books.begin(); it_b != it->books.end(); it_b++)
            {
              bool c_s2;
#pragma omp atomic read
              c_s2 = cancel_search;
              if(c_s2)
                {
#pragma omp cancel for
                  continue;
                }
              std::string::size_type n_beg = 0;
              std::string::size_type n_end;
              bool stop = false;
              for(;;)
                {
#pragma omp atomic read
                  c_s2 = cancel_search;
                  if(c_s2)
                    {
                      break;
                    }
                  n_end = it_b->book_author.find(find_str, n_beg);
                  std::string auth;
                  if(n_end != std::string::npos)
                    {
                      auth = it_b->book_author.substr(n_beg, n_end - n_beg);
                    }
                  else
                    {
                      if(n_beg < it_b->book_author.size())
                        {
                          std::copy(it_b->book_author.begin() + n_beg,
                                    it_b->book_author.end(),
                                    std::back_inserter(auth));
                        }
                      else
                        {
                          break;
                        }
                      stop = true;
                    }
                  if(!auth.empty())
                    {
                      while(auth.size() > 0)
                        {
                          char ch = *auth.begin();
                          if(ch >= 0 && ch <= 32)
                            {
                              auth.erase(auth.begin());
                            }
                          else
                            {
                              break;
                            }
                        }
                      while(auth.size() > 0)
                        {
                          char ch = *auth.rbegin();
                          if(ch >= 0 && ch <= 32)
                            {
                              auth.pop_back();
                            }
                          else
                            {
                              break;
                            }
                        }
#pragma omp critical
                      {
                        auth_v.push_back(std::make_tuple(auth, true));
                      }
                    }
                  n_beg = n_end + find_str.size() - 1;
                  if(stop)
                    {
                      break;
                    }
                }
            }
        }
      }
  }
#else
  std::mutex result_mtx;
#ifdef USE_PE
  std::for_each(
      std::execution::par, base.begin(), base.end(),
      [this, &result, find_str, &result_mtx, &auth_v](FileParseEntry &fpe) {
        if(cancel_search.load())
          {
            return void();
          }

        std::for_each(std::execution::par, fpe.books.begin(), fpe.books.end(),
                      [this, find_str, &result, &result_mtx,
                       &auth_v](BookParseEntry &bpe) {
                        if(cancel_search.load())
                          {
                            return void();
                          }
                        std::string::size_type n_beg = 0;
                        std::string::size_type n_end;
                        bool stop = false;
                        for(;;)
                          {
                            if(cancel_search.load())
                              {
                                break;
                              }
                            n_end = bpe.book_author.find(find_str, n_beg);
                            std::string auth;
                            if(n_end != std::string::npos)
                              {
                                auth = bpe.book_author.substr(n_beg,
                                                              n_end - n_beg);
                              }
                            else
                              {
                                if(n_beg < bpe.book_author.size())
                                  {
                                    std::copy(bpe.book_author.begin() + n_beg,
                                              bpe.book_author.end(),
                                              std::back_inserter(auth));
                                  }
                                else
                                  {
                                    break;
                                  }
                                stop = true;
                              }
                            if(!auth.empty())
                              {
                                while(auth.size() > 0)
                                  {
                                    char ch = *auth.begin();
                                    if(ch >= 0 && ch <= 32)
                                      {
                                        auth.erase(auth.begin());
                                      }
                                    else
                                      {
                                        break;
                                      }
                                  }
                                while(auth.size() > 0)
                                  {
                                    char ch = *auth.rbegin();
                                    if(ch >= 0 && ch <= 32)
                                      {
                                        auth.pop_back();
                                      }
                                    else
                                      {
                                        break;
                                      }
                                  }
                                result_mtx.lock();
                                auth_v.push_back(std::make_tuple(auth, true));
                                result_mtx.unlock();
                              }
                            n_beg = n_end + find_str.size() - 1;
                            if(stop)
                              {
                                break;
                              }
                          }
                      });
      });
#else
  std::for_each(
      base.begin(), base.end(),
      [this, &result, find_str, &result_mtx](FileParseEntry &fpe) {
        if(cancel_search.load())
          {
            return void();
          }

        std::for_each(
            fpe.books.begin(), fpe.books.end(),
            [this, find_str, &result, &result_mtx](BookParseEntry &bpe) {
              if(cancel_search.load())
                {
                  return void();
                }
              std::string::size_type n_beg = 0;
              std::string::size_type n_end;
              bool stop = false;
              for(;;)
                {
                  if(cancel_search.load())
                    {
                      break;
                    }
                  n_end = bpe.book_author.find(find_str, n_beg);
                  std::string auth;
                  if(n_end != std::string::npos)
                    {
                      auth = bpe.book_author.substr(n_beg, n_end - n_beg);
                    }
                  else
                    {
                      if(n_beg < bpe.book_author.size())
                        {
                          std::copy(bpe.book_author.begin() + n_beg,
                                    bpe.book_author.end(),
                                    std::back_inserter(auth));
                        }
                      else
                        {
                          break;
                        }
                      stop = true;
                    }
                  if(!auth.empty())
                    {
                      while(auth.size() > 0)
                        {
                          char ch = *auth.begin();
                          if(ch >= 0 && ch <= 32)
                            {
                              auth.erase(auth.begin());
                            }
                          else
                            {
                              break;
                            }
                        }
                      while(auth.size() > 0)
                        {
                          char ch = *auth.rbegin();
                          if(ch >= 0 && ch <= 32)
                            {
                              auth.pop_back();
                            }
                          else
                            {
                              break;
                            }
                        }
                      result_mtx.lock();
                      result.push_back(auth);
                      result_mtx.unlock();
                    }
                  n_beg = n_end + find_str.size() - 1;
                  if(stop)
                    {
                      break;
                    }
                }
            });
      });
#endif
  basemtx.unlock();
#endif
#ifdef USE_OPENMP
  omp_unset_lock(&basemtx);
#endif
  result.reserve(auth_v.size());
  double sz = static_cast<double>(auth_v.size());
  double progr = 1.0;
#ifdef USE_OPENMP
  for(auto it = auth_v.begin(); it != auth_v.end(); it++)
    {
      bool c_s;
#pragma omp atomic read
      c_s = cancel_search;
      if(c_s)
        {
          break;
        }
      if(std::get<1>(*it))
        {
          std::string s_str = std::move(std::get<0>(*it));
#pragma omp parallel
#pragma omp for
          for(auto it_s = it + 1; it_s != auth_v.end(); it_s++)
            {
              if(std::get<1>(*it_s))
                {
                  if(std::get<0>(*it_s) == s_str)
                    {
                      std::get<1>(*it_s) = false;
                    }
                }
            }
          result.emplace_back(s_str);
        }
      if(auth_show_progr)
        {
          auth_show_progr(progr, sz);
        }
      progr += 1.0;
    }
  omp_set_dynamic(false);
  omp_set_max_active_levels(lvls);
  bool c_s;
#pragma omp atomic read
  c_s = cancel_search;
  if(c_s)
    {
      result.clear();
    }
#else
  for(auto it = auth_v.begin(); it != auth_v.end(); it++)
    {
      if(cancel_search.load())
        {
          break;
        }
      if(std::get<1>(*it))
        {
          std::string s_str = std::move(std::get<0>(*it));
#ifdef USE_PE
          std::for_each(std::execution::par, it + 1, auth_v.end(),
                        [s_str](std::tuple<std::string, bool> &val) {
                          if(std::get<1>(val))
                            {
                              if(std::get<0>(val) == s_str)
                                {
                                  std::get<1>(val) = false;
                                }
                            }
                        });
#else
          std::for_each(it + 1, auth_v.end(),
                        [s_str](std::tuple<std::string, bool> &val) {
                          if(std::get<1>(val))
                            {
                              if(std::get<0>(val) == s_str)
                                {
                                  std::get<1>(val) = false;
                                }
                            }
                        });
#endif
          result.emplace_back(s_str);
          if(auth_show_progr)
            {
              auth_show_progr(progr, sz);
            }
          progr += 1.0;
        }
    }

  if(cancel_search.load())
    {
      result.clear();
    }
#endif
  return result;
}

std::vector<BookBaseEntry>
BaseKeeper::booksWithNotes(const std::vector<NotesBaseEntry> &notes)
{
  std::vector<BookBaseEntry> result;

#ifndef USE_OPENMP
  std::mutex result_mtx;
  basemtx.lock();
#ifdef USE_PE
  std::for_each(
      std::execution::par, base.begin(), base.end(),
      [&result, &result_mtx, notes, this](FileParseEntry &ent) {
        if(cancel_search.load())
          {
            return void();
          }
        std::filesystem::path f_p
            = collection_path / std::filesystem::u8path(ent.file_rel_path);
        auto it = std::find_if(notes.begin(), notes.end(),
                               [f_p](const NotesBaseEntry &el_n) {
                                 if(f_p == el_n.book_file_full_path)
                                   {
                                     return true;
                                   }
                                 else
                                   {
                                     return false;
                                   }
                               });
        if(it != notes.end())
          {
            std::string bp = it->book_path;

            auto it_b = std::find_if(ent.books.begin(), ent.books.end(),
                                     [bp](BookParseEntry &el) {
                                       return bp == el.book_path;
                                     });
            if(it_b != ent.books.end())
              {
                BookBaseEntry bbe(*it_b, f_p);
                result_mtx.lock();
                result.emplace_back(bbe);
                result_mtx.unlock();
              }
          }
      });
#else
  std::for_each(
      base.begin(), base.end(),
      [&result, &result_mtx, notes, this](FileParseEntry &ent) {
        if(cancel_search.load())
          {
            return void();
          }
        std::filesystem::path f_p
            = collection_path / std::filesystem::u8path(ent.file_rel_path);
        auto it = std::find_if(notes.begin(), notes.end(),
                               [f_p](const NotesBaseEntry &el_n) {
                                 if(f_p == el_n.book_file_full_path)
                                   {
                                     return true;
                                   }
                                 else
                                   {
                                     return false;
                                   }
                               });
        if(it != notes.end())
          {
            std::string bp = it->book_path;

            auto it_b = std::find_if(ent.books.begin(), ent.books.end(),
                                     [bp](BookParseEntry &el) {
                                       return bp == el.book_path;
                                     });
            if(it_b != ent.books.end())
              {
                BookBaseEntry bbe(*it_b, f_p);
                result_mtx.lock();
                result.emplace_back(bbe);
                result_mtx.unlock();
              }
          }
      });
#endif
#else
  omp_set_lock(&basemtx);
#pragma omp parallel
  {
#pragma omp for
    for(auto it_base = base.begin(); it_base != base.end(); it_base++)
      {
        bool c_s;
#pragma omp atomic read
        c_s = cancel_search;
        if(c_s)
          {
#pragma omp cancel for
            continue;
          }
        std::filesystem::path f_p
            = collection_path
              / std::filesystem::u8path(it_base->file_rel_path);
        auto it = std::find_if(notes.begin(), notes.end(),
                               [f_p](const NotesBaseEntry &el_n) {
                                 if(f_p == el_n.book_file_full_path)
                                   {
                                     return true;
                                   }
                                 else
                                   {
                                     return false;
                                   }
                               });
        if(it != notes.end())
          {
            std::string bp = it->book_path;

            auto it_b
                = std::find_if(it_base->books.begin(), it_base->books.end(),
                               [bp](BookParseEntry &el) {
                                 return bp == el.book_path;
                               });
            if(it_b != it_base->books.end())
              {
                BookBaseEntry bbe(*it_b, f_p);
#pragma omp critical
                {
                  result.emplace_back(bbe);
                }
              }
          }
      }
  }
  omp_unset_lock(&basemtx);
  bool c_s;
#pragma omp atomic read
  c_s = cancel_search;
  if(c_s)
    {
      result.clear();
    }
#endif
#ifndef USE_OPENMP
  basemtx.unlock();
  if(cancel_search.load())
    {
      result.clear();
    }
#endif

  return result;
}

bool
BaseKeeper::searchLineFunc(const std::string &to_search,
                           const std::string &source)
{
  std::string loc_search = af->stringToLower(to_search);
  std::string loc_source = af->stringToLower(source);
  std::string::size_type n;
  n = loc_source.find(loc_search);
  if(n != std::string::npos)
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool
BaseKeeper::searchSurname(const BookBaseEntry &search,
                          std::vector<BookBaseEntry> &result)
{
  bool all_empty = true;
  std::string surname = search.bpe.book_author;
  std::string::size_type n = surname.find("\n");
  if(n != std::string::npos)
    {
      surname = surname.substr(0, n);
      if(!surname.empty())
        {
          all_empty = false;
#ifdef USE_OPENMP
          omp_set_dynamic(true);
          int lvls = omp_get_max_active_levels();
          omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
          {
#pragma omp for
            for(auto it = base.begin(); it != base.end(); it++)
              {
                bool c_s;
#pragma omp atomic read
                c_s = cancel_search;
                if(c_s)
                  {
#pragma omp cancel for
                    continue;
                  }
                std::filesystem::path book_file_path = collection_path;
                book_file_path /= std::filesystem::u8path(it->file_rel_path);
#pragma omp parallel
                {
#pragma omp for
                  for(auto itb = it->books.begin(); itb != it->books.end();
                      itb++)
                    {
                      bool c_s2;
#pragma omp atomic read
                      c_s2 = cancel_search;
                      if(c_s2)
                        {
#pragma omp cancel for
                          continue;
                        }
                      if(searchLineFunc(surname, itb->book_author))
                        {
#pragma omp critical
                          {
                            result.emplace_back(
                                BookBaseEntry(*itb, book_file_path));
                          }
                        }
                    }
                }
              }
          }
          omp_set_dynamic(false);
          omp_set_max_active_levels(lvls);
#else
          std::mutex result_mtx;
#ifdef USE_PE
          std::for_each(
              std::execution::par, base.begin(), base.end(),
              [&result_mtx, &result, surname, this](FileParseEntry &el) {
                if(cancel_search.load())
                  {
                    return void();
                  }
                std::filesystem::path book_file_path
                    = collection_path
                      / std::filesystem::u8path(el.file_rel_path);
                std::for_each(std::execution::par, el.books.begin(),
                              el.books.end(),
                              [this, surname, &result, &result_mtx,
                               book_file_path](BookParseEntry &el) {
                                if(cancel_search.load())
                                  {
                                    return void();
                                  }
                                if(searchLineFunc(surname, el.book_author))
                                  {
                                    result_mtx.lock();
                                    result.emplace_back(
                                        BookBaseEntry(el, book_file_path));
                                    result_mtx.unlock();
                                  }
                              });
              });
#else
          std::for_each(
              base.begin(), base.end(),
              [&result_mtx, &result, surname, this](FileParseEntry &el) {
                if(cancel_search.load())
                  {
                    return void();
                  }
                std::filesystem::path book_file_path
                    = collection_path
                      / std::filesystem::u8path(el.file_rel_path);
                std::for_each(el.books.begin(), el.books.end(),
                              [this, surname, &result, &result_mtx,
                               book_file_path](BookParseEntry &el) {
                                if(cancel_search.load())
                                  {
                                    return void();
                                  }
                                if(searchLineFunc(surname, el.book_author))
                                  {
                                    result_mtx.lock();
                                    result.emplace_back(
                                        BookBaseEntry(el, book_file_path));
                                    result_mtx.unlock();
                                  }
                              });
              });
#endif
#endif
        }
    }
  return all_empty;
}

void
BaseKeeper::searchBook(const BookBaseEntry &search,
                       std::vector<BookBaseEntry> &result)
{
  if(result.size() == 0)
    {
#ifdef USE_OPENMP
      omp_set_dynamic(true);
      int lvls = omp_get_max_active_levels();
      omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
      {
#pragma omp for
        for(auto it = base.begin(); it != base.end(); it++)
          {
            bool c_s;
#pragma omp atomic read
            c_s = cancel_search;
            if(c_s)
              {
#pragma omp cancel for
                continue;
              }
            std::filesystem::path book_file_path = collection_path;
            book_file_path /= std::filesystem::u8path(it->file_rel_path);
#pragma omp parallel
            {
#pragma omp for
              for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
                {
                  bool c_s2;
#pragma omp atomic read
                  c_s2 = cancel_search;
                  if(c_s2)
                    {
#pragma omp cancel for
                      continue;
                    }
                  if(searchLineFunc(search.bpe.book_name, itb->book_name))
                    {
#pragma omp critical
                      {
                        result.emplace_back(
                            BookBaseEntry(*itb, book_file_path));
                      }
                    }
                }
            }
          }
      }
      omp_set_dynamic(false);
      omp_set_max_active_levels(lvls);
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, search, &result, &result_mtx](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, search, book_file_path, &result,
                 &result_mtx](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_name, el.book_name))
                    {
                      result_mtx.lock();
                      result.emplace_back(BookBaseEntry(el, book_file_path));
                      result_mtx.unlock();
                    }
                });
          });
#else
      std::for_each(
          base.begin(), base.end(),
          [this, search, &result, &result_mtx](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                el.books.begin(), el.books.end(),
                [this, search, book_file_path, &result,
                 &result_mtx](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_name, el.book_name))
                    {
                      result_mtx.lock();
                      result.emplace_back(BookBaseEntry(el, book_file_path));
                      result_mtx.unlock();
                    }
                });
          });
#endif
#endif
    }
  else
    {
#ifdef USE_OPENMP
      result.erase(
          AuxFunc::parallelRemoveIf(result.begin(), result.end(),
                                    [search, this](BookBaseEntry &el) {
                                      if(searchLineFunc(search.bpe.book_name,
                                                        el.bpe.book_name))
                                        {
                                          return false;
                                        }
                                      else
                                        {
                                          return true;
                                        }
                                    }),
          result.end());
#else
#ifdef USE_PE
      result.erase(std::remove_if(std::execution::par, result.begin(),
                                  result.end(),
                                  [search, this](BookBaseEntry &el) {
                                    if(searchLineFunc(search.bpe.book_name,
                                                      el.bpe.book_name))
                                      {
                                        return false;
                                      }
                                    else
                                      {
                                        return true;
                                      }
                                  }),
                   result.end());
#else
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [search, this](BookBaseEntry &el) {
                                    if(searchLineFunc(search.bpe.book_name,
                                                      el.bpe.book_name))
                                      {
                                        return false;
                                      }
                                    else
                                      {
                                        return true;
                                      }
                                  }),
                   result.end());
#endif
#endif
    }
}

void
BaseKeeper::searchSeries(const BookBaseEntry &search,
                         std::vector<BookBaseEntry> &result)
{
  if(result.size() == 0)
    {
#ifdef USE_OPENMP
      omp_set_dynamic(true);
      int lvls = omp_get_max_active_levels();
      omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
      {
#pragma omp for
        for(auto it = base.begin(); it != base.end(); it++)
          {
            bool c_s;
#pragma omp atomic read
            c_s = cancel_search;
            if(c_s)
              {
#pragma omp cancel for
                continue;
              }
            std::filesystem::path book_file_path = collection_path;
            book_file_path /= std::filesystem::u8path(it->file_rel_path);
#pragma omp parallel
            {
#pragma omp for
              for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
                {
                  bool c_s2;
#pragma omp atomic read
                  c_s2 = cancel_search;
                  if(c_s2)
                    {
#pragma omp cancel for
                      continue;
                    }
                  if(searchLineFunc(search.bpe.book_series, itb->book_series))
                    {
#pragma omp critical
                      {
                        result.emplace_back(
                            BookBaseEntry(*itb, book_file_path));
                      }
                    }
                }
            }
          }
      }
      omp_set_dynamic(false);
      omp_set_max_active_levels(lvls);
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, search, &result, &result_mtx](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx,
                 book_file_path](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_series, el.book_series))
                    {
                      result_mtx.lock();
                      result.emplace_back(BookBaseEntry(el, book_file_path));
                      result_mtx.unlock();
                    }
                });
          });
#else
      std::for_each(
          base.begin(), base.end(),
          [this, search, &result, &result_mtx](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx,
                 book_file_path](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_series, el.book_series))
                    {
                      result_mtx.lock();
                      result.emplace_back(BookBaseEntry(el, book_file_path));
                      result_mtx.unlock();
                    }
                });
          });
#endif
#endif
    }
  else
    {
#ifdef USE_OPENMP
      result.erase(
          AuxFunc::parallelRemoveIf(result.begin(), result.end(),
                                    [search, this](BookBaseEntry &el) {
                                      if(searchLineFunc(search.bpe.book_series,
                                                        el.bpe.book_series))
                                        {
                                          return false;
                                        }
                                      else
                                        {
                                          return true;
                                        }
                                    }),
          result.end());
#else
#ifdef USE_PE
      result.erase(std::remove_if(std::execution::par, result.begin(),
                                  result.end(),
                                  [search, this](BookBaseEntry &el) {
                                    if(searchLineFunc(search.bpe.book_series,
                                                      el.bpe.book_series))
                                      {
                                        return false;
                                      }
                                    else
                                      {
                                        return true;
                                      }
                                  }),
                   result.end());
#else
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [search, this](BookBaseEntry &el) {
                                    if(searchLineFunc(search.bpe.book_series,
                                                      el.bpe.book_series))
                                      {
                                        return false;
                                      }
                                    else
                                      {
                                        return true;
                                      }
                                  }),
                   result.end());
#endif
#endif
    }
}

void
BaseKeeper::clearBase()
{
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lock_base(basemtx);
#else
  OmpLockGuard lock_base(basemtx);
#endif
  collection_path.clear();
  collection_name.clear();
  base.clear();
  base.shrink_to_fit();
}

std::vector<FileParseEntry>
BaseKeeper::get_base_vector()
{
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lock_base(basemtx);
#else
  OmpLockGuard lock_base(basemtx);
#endif
  return base;
}

std::filesystem::path
BaseKeeper::get_books_path(const std::string &collection_name,
                           const std::shared_ptr<AuxFunc> &af)
{
  std::filesystem::path result;

  std::filesystem::path base_path = af->homePath();
  base_path /= std::filesystem::u8path(".local")
               / std::filesystem::u8path("share")
               / std::filesystem::u8path("MyLibrary")
               / std::filesystem::u8path("Collections");
  base_path /= std::filesystem::u8path(collection_name);
  base_path /= std::filesystem::u8path("base");

  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t fsz;
      f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);
      uint16_t val16;
      size_t sz = sizeof(val16);
      if(fsz < sz)
        {
          f.close();
          throw MLException("BaseKeeper::get_books_path: base file error");
        }
      else
        {
          f.read(reinterpret_cast<char *>(&val16), sz);
          ByteOrder bo;
          bo.set_little(val16);
          val16 = bo;
        }

      if(static_cast<size_t>(val16) + sz > fsz)
        {
          f.close();
          throw MLException(
              "BaseKeeper::get_books_path: wrong books path size");
        }
      else
        {
          std::string buf;
          buf.resize(val16);
          f.read(buf.data(), buf.size());
          result = std::filesystem::u8path(buf);
        }

      f.close();
    }

  return result;
}

void
BaseKeeper::searchGenre(const BookBaseEntry &search,
                        std::vector<BookBaseEntry> &result)
{
  if(result.size() == 0)
    {
#ifdef USE_OPENMP
      omp_set_dynamic(true);
      int lvls = omp_get_max_active_levels();
      omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
      {
#pragma omp for
        for(auto it = base.begin(); it != base.end(); it++)
          {
            bool c_s;
#pragma omp atomic read
            c_s = cancel_search;
            if(c_s)
              {
#pragma omp cancel for
                continue;
              }
            std::filesystem::path book_file_path = collection_path;
            book_file_path /= std::filesystem::u8path(it->file_rel_path);
#pragma omp parallel
            {
#pragma omp for
              for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
                {
                  bool c_s2;
#pragma omp atomic read
                  c_s2 = cancel_search;
                  if(c_s2)
                    {
#pragma omp cancel for
                      continue;
                    }
                  if(searchLineFunc(search.bpe.book_genre, itb->book_genre))
                    {
#pragma omp critical
                      {
                        result.emplace_back(
                            BookBaseEntry(*itb, book_file_path));
                      }
                    }
                }
            }
          }
      }
      omp_set_dynamic(false);
      omp_set_max_active_levels(lvls);
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, search, &result, &result_mtx](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);
            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx,
                 book_file_path](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_genre, el.book_genre))
                    {
                      result_mtx.lock();
                      result.emplace_back(BookBaseEntry(el, book_file_path));
                      result_mtx.unlock();
                    }
                });
          });
#else
      std::for_each(
          base.begin(), base.end(),
          [this, search, &result, &result_mtx](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);
            std::for_each(
                el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx,
                 book_file_path](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_genre, el.book_genre))
                    {
                      result_mtx.lock();
                      result.emplace_back(BookBaseEntry(el, book_file_path));
                      result_mtx.unlock();
                    }
                });
          });
#endif
#endif
    }
  else
    {
#ifdef USE_OPENMP
      result.erase(
          AuxFunc::parallelRemoveIf(result.begin(), result.end(),
                                    [search, this](BookBaseEntry &el) {
                                      if(searchLineFunc(search.bpe.book_genre,
                                                        el.bpe.book_genre))
                                        {
                                          return false;
                                        }
                                      else
                                        {
                                          return true;
                                        }
                                    }),
          result.end());
#else
#ifdef USE_PE
      result.erase(std::remove_if(std::execution::par, result.begin(),
                                  result.end(),
                                  [search, this](BookBaseEntry &el) {
                                    if(searchLineFunc(search.bpe.book_genre,
                                                      el.bpe.book_genre))
                                      {
                                        return false;
                                      }
                                    else
                                      {
                                        return true;
                                      }
                                  }),
                   result.end());
#else
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [search, this](BookBaseEntry &el) {
                                    if(searchLineFunc(search.bpe.book_genre,
                                                      el.bpe.book_genre))
                                      {
                                        return false;
                                      }
                                    else
                                      {
                                        return true;
                                      }
                                  }),
                   result.end());
#endif
#endif
    }
}

void
BaseKeeper::stopSearch()
{
#ifdef USE_OPENMP
#pragma atomic write
  cancel_search = true;
#else
  cancel_search.store(true);
#endif
}

bool
BaseKeeper::searchLastName(const BookBaseEntry &search,
                           std::vector<BookBaseEntry> &result)
{
  bool all_empty = true;
  std::string last_name = search.bpe.book_author;
  std::string sstr = "\n";
  std::string::size_type n = last_name.find(sstr);
  if(n != std::string::npos)
    {
      last_name.erase(0, n + sstr.size());
      n = last_name.find(sstr);
      if(n != std::string::npos)
        {
          last_name.erase(0, n + sstr.size());
          if(!last_name.empty())
            {
              all_empty = false;
              if(result.size() == 0)
                {
#ifdef USE_OPENMP
                  omp_set_dynamic(true);
                  int lvls = omp_get_max_active_levels();
                  omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
                  {
#pragma omp for
                    for(auto it = base.begin(); it != base.end(); it++)
                      {
                        bool c_s;
#pragma omp atomic read
                        c_s = cancel_search;
                        if(c_s)
                          {
#pragma omp cancel for
                            continue;
                          }
                        std::filesystem::path book_file_path = collection_path;
                        book_file_path
                            /= std::filesystem::u8path(it->file_rel_path);
#pragma omp parallel
                        {
#pragma omp for
                          for(auto itb = it->books.begin();
                              itb != it->books.end(); itb++)
                            {
                              bool c_s2;
#pragma omp atomic read
                              c_s2 = cancel_search;
                              if(c_s2)
                                {
#pragma omp cancel for
                                  continue;
                                }
                              if(searchLineFunc(last_name, itb->book_author))
                                {
#pragma omp critical
                                  {
                                    result.emplace_back(
                                        BookBaseEntry(*itb, book_file_path));
                                  }
                                }
                            }
                        }
                      }
                  }
                  omp_set_dynamic(false);
                  omp_set_max_active_levels(lvls);
#else
                  std::mutex result_mtx;
#ifdef USE_PE
                  std::for_each(
                      std::execution::par, base.begin(), base.end(),
                      [this, last_name, &result,
                       &result_mtx](FileParseEntry &el) {
                        if(cancel_search.load())
                          {
                            return void();
                          }
                        std::filesystem::path book_file_path
                            = collection_path
                              / std::filesystem::u8path(el.file_rel_path);

                        std::for_each(
                            std::execution::par, el.books.begin(),
                            el.books.end(),
                            [this, last_name, &result, &result_mtx,
                             book_file_path](BookParseEntry &el) {
                              if(cancel_search.load())
                                return void();
                              {
                              }
                              if(searchLineFunc(last_name, el.book_author))
                                {
                                  result_mtx.lock();
                                  result.emplace_back(
                                      BookBaseEntry(el, book_file_path));
                                  result_mtx.unlock();
                                }
                            });
                      });
#else
                  std::for_each(
                      base.begin(), base.end(),
                      [this, last_name, &result,
                       &result_mtx](FileParseEntry &el) {
                        if(cancel_search.load())
                          {
                            return void();
                          }
                        std::filesystem::path book_file_path
                            = collection_path
                              / std::filesystem::u8path(el.file_rel_path);

                        std::for_each(
                            el.books.begin(), el.books.end(),
                            [this, last_name, &result, &result_mtx,
                             book_file_path](BookParseEntry &el) {
                              if(cancel_search.load())
                                return void();
                              {
                              }
                              if(searchLineFunc(last_name, el.book_author))
                                {
                                  result_mtx.lock();
                                  result.emplace_back(
                                      BookBaseEntry(el, book_file_path));
                                  result_mtx.unlock();
                                }
                            });
                      });
#endif
#endif
                }
              else
                {
#ifdef USE_OPENMP
                  result.erase(
                      AuxFunc::parallelRemoveIf(
                          result.begin(), result.end(),
                          [last_name, this](BookBaseEntry &el) {
                            if(searchLineFunc(last_name, el.bpe.book_author))
                              {
                                return false;
                              }
                            else
                              {
                                return true;
                              }
                          }),
                      result.end());
#else
#ifdef USE_PE
                  result.erase(
                      std::remove_if(
                          std::execution::par, result.begin(), result.end(),
                          [last_name, this](BookBaseEntry &el) {
                            if(searchLineFunc(last_name, el.bpe.book_author))
                              {
                                return false;
                              }
                            else
                              {
                                return true;
                              }
                          }),
                      result.end());
#else
                  result.erase(
                      std::remove_if(result.begin(), result.end(),
                                     [last_name, this](BookBaseEntry &el) {
                                       if(searchLineFunc(last_name,
                                                         el.bpe.book_author))
                                         {
                                           return false;
                                         }
                                       else
                                         {
                                           return true;
                                         }
                                     }),
                      result.end());
#endif
#endif
                }
            }
        }
    }
  return all_empty;
}

bool
BaseKeeper::searchFirstName(const BookBaseEntry &search,
                            std::vector<BookBaseEntry> &result)
{
  bool all_empty = true;
  std::string first_name = search.bpe.book_author;
  std::string sstr = "\n";
  std::string::size_type n = first_name.find(sstr);
  if(n != std::string::npos)
    {
      first_name.erase(0, n + sstr.size());
      n = first_name.find(sstr);
      if(n != std::string::npos)
        {
          first_name = first_name.substr(0, n);
          if(!first_name.empty())
            {
              all_empty = false;
              if(result.size() == 0)
                {
#ifdef USE_OPENMP
                  omp_set_dynamic(true);
                  int lvls = omp_get_max_active_levels();
                  omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
                  {
#pragma omp for
                    for(auto it = base.begin(); it != base.end(); it++)
                      {
                        bool c_s;
#pragma omp atomic read
                        c_s = cancel_search;
                        if(c_s)
                          {
#pragma omp cancel for
                            continue;
                          }
                        std::filesystem::path book_file_path = collection_path;
                        book_file_path
                            /= std::filesystem::u8path(it->file_rel_path);
#pragma omp parallel
                        {
#pragma omp for
                          for(auto itb = it->books.begin();
                              itb != it->books.end(); itb++)
                            {
                              bool c_s2;
#pragma omp atomic read
                              c_s2 = cancel_search;
                              if(c_s2)
                                {
#pragma omp cancel for
                                  continue;
                                }
                              if(searchLineFunc(first_name, itb->book_author))
                                {
#pragma omp critical
                                  {
                                    result.emplace_back(
                                        BookBaseEntry(*itb, book_file_path));
                                  }
                                }
                            }
                        }
                      }
                  }
                  omp_set_dynamic(false);
                  omp_set_max_active_levels(lvls);
#else
                  std::mutex result_mtx;
#ifdef USE_PE
                  std::for_each(
                      std::execution::par, base.begin(), base.end(),
                      [this, first_name, &result,
                       &result_mtx](FileParseEntry &el) {
                        if(cancel_search.load())
                          {
                            return void();
                          }
                        std::filesystem::path book_file_path
                            = collection_path
                              / std::filesystem::u8path(el.file_rel_path);

                        std::for_each(
                            std::execution::par, el.books.begin(),
                            el.books.end(),
                            [this, first_name, &result, &result_mtx,
                             book_file_path](BookParseEntry &el) {
                              if(cancel_search.load())
                                {
                                  return void();
                                }
                              if(searchLineFunc(first_name, el.book_author))
                                {
                                  result_mtx.lock();
                                  result.emplace_back(
                                      BookBaseEntry(el, book_file_path));
                                  result_mtx.unlock();
                                }
                            });
                      });
#else
                  std::for_each(
                      base.begin(), base.end(),
                      [this, first_name, &result,
                       &result_mtx](FileParseEntry &el) {
                        if(cancel_search.load())
                          {
                            return void();
                          }
                        std::filesystem::path book_file_path
                            = collection_path
                              / std::filesystem::u8path(el.file_rel_path);

                        std::for_each(
                            el.books.begin(), el.books.end(),
                            [this, first_name, &result, &result_mtx,
                             book_file_path](BookParseEntry &el) {
                              if(cancel_search.load())
                                {
                                  return void();
                                }
                              if(searchLineFunc(first_name, el.book_author))
                                {
                                  result_mtx.lock();
                                  result.emplace_back(
                                      BookBaseEntry(el, book_file_path));
                                  result_mtx.unlock();
                                }
                            });
                      });
#endif
#endif
                }
              else
                {
#ifdef USE_OPENMP
                  result.erase(
                      AuxFunc::parallelRemoveIf(
                          result.begin(), result.end(),
                          [first_name, this](BookBaseEntry &el) {
                            if(searchLineFunc(first_name, el.bpe.book_author))
                              {
                                return false;
                              }
                            else
                              {
                                return true;
                              }
                          }),
                      result.end());
#else
#ifdef USE_PE
                  result.erase(
                      std::remove_if(
                          std::execution::par, result.begin(), result.end(),
                          [first_name, this](BookBaseEntry &el) {
                            if(searchLineFunc(first_name, el.bpe.book_author))
                              {
                                return false;
                              }
                            else
                              {
                                return true;
                              }
                          }),
                      result.end());
#else
                  result.erase(
                      std::remove_if(result.begin(), result.end(),
                                     [first_name, this](BookBaseEntry &el) {
                                       if(searchLineFunc(first_name,
                                                         el.bpe.book_author))
                                         {
                                           return false;
                                         }
                                       else
                                         {
                                           return true;
                                         }
                                     }),
                      result.end());
#endif
#endif
                }
            }
        }
    }
  return all_empty;
}
