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
#pragma omp taskwait
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
  books_in_base = 0;
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
              books_in_base = 0;
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
  books_in_base += fpe.books.size();
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
BaseKeeper::searchBook(const BookBaseEntry &search,
                       const double &coef_coincedence)
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
  result.reserve(books_in_base);
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
#ifndef USE_GPUOFFLOADING
                if(!searchSurname(search, result, coef_coincedence))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
#else
                if(!searchSurnameGPU(search, result, coef_coincedence))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
#endif
                break;
              }
            case 2:
              {
#ifndef USE_GPUOFFLOADING
                if(!searchFirstName(search, result, coef_coincedence))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
#else
                if(!searchFirstNameGPU(search, result, coef_coincedence))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
#endif
                break;
              }
            case 3:
              {
#ifndef USE_GPUOFFLOADING
                if(!searchLastName(search, result, coef_coincedence))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
#else
                if(!searchLastNameGPU(search, result, coef_coincedence))
                  {
                    if(result.size() == 0)
                      {
                        stop_search = true;
                      }
                    all_empty = false;
                  }
#endif
                break;
              }
            case 4:
              {
                if(!search.bpe.book_name.empty())
                  {
#ifndef USE_GPUOFFLOADING
                    searchBook(search, result, coef_coincedence);
#else
                    searchBookGPU(search, result, coef_coincedence);
#endif
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
#ifndef USE_GPUOFFLOADING
                    searchSeries(search, result, coef_coincedence);
#else
                    searchSeriesGPU(search, result, coef_coincedence);
#endif
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
#ifndef USE_GPUOFFLOADING
                    searchGenre(search, result, coef_coincedence);
#else
                    searchGenreGPU(search, result, coef_coincedence);
#endif
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
#pragma omp parallel
#pragma omp for
      for(auto it = base.begin(); it != base.end(); it++)
        {
#pragma omp atomic read
          c_s = cancel_search;
          if(c_s)
            {
#pragma omp cancel for
              continue;
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
#pragma omp critical
              {
                result.emplace_back(BookBaseEntry(*itb, book_file_path));
              }
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

  result.shrink_to_fit();

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
  result.reserve(books_in_base);
  std::string find_str = ", ";
#ifdef USE_OPENMP

#pragma omp parallel
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
                      result.push_back(auth);
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

#else
#ifdef USE_PE
  std::mutex auth_v_mtx;
  std::for_each(
      std::execution::par, base.begin(), base.end(),
      [this, &result, find_str, &auth_v_mtx](FileParseEntry &fpe) {
        if(cancel_search.load())
          {
            return void();
          }

        std::for_each(
            std::execution::par, fpe.books.begin(), fpe.books.end(),
            [this, find_str, &result, &auth_v_mtx](BookParseEntry &bpe) {
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
                      auth_v_mtx.lock();
                      result.push_back(auth);
                      auth_v_mtx.unlock();
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
  std::for_each(base.begin(), base.end(),
                [this, &result, find_str](FileParseEntry &fpe) {
                  if(cancel_search.load())
                    {
                      return void();
                    }

                  std::for_each(
                      fpe.books.begin(), fpe.books.end(),
                      [this, find_str, &result](BookParseEntry &bpe) {
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
                                result.push_back(auth);
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
  std::vector<std::string>::iterator end_it = result.end();
  double progr;
  double sz;

  bool c_s = false;

  int lvls = omp_get_max_active_levels();
  omp_set_max_active_levels(1);
  for(auto it = result.begin(); it != end_it; it++)
    {
#pragma omp atomic read
      c_s = cancel_search;
      if(c_s)
        {
          break;
        }
      end_it = AuxFunc::parallelRemove(it + 1, end_it, *it);
      if(auth_show_progr)
        {
          progr = static_cast<double>(std::distance(result.begin(), it));
          sz = static_cast<double>(std::distance(result.begin(), end_it));
          auth_show_progr(progr, sz);
        }
    }
  omp_set_max_active_levels(lvls);
  result.erase(end_it, result.end());

#pragma omp atomic read
  c_s = cancel_search;
  if(c_s)
    {
      result.clear();
    }
#else
  std::vector<std::string>::iterator end_it = result.end();
  double progr;
  double sz;
  for(auto it = result.begin(); it != end_it; it++)
    {
      if(cancel_search.load())
        {
          break;
        }
#ifdef USE_PE
      end_it = std::remove(std::execution::par, it + 1, end_it, *it);
#else
      end_it = std::remove(it + 1, end_it, *it);
#endif
      if(auth_show_progr)
        {
          progr = static_cast<double>(std::distance(result.begin(), it));
          sz = static_cast<double>(std::distance(result.begin(), end_it));
          auth_show_progr(progr, sz);
        }
    }
  result.erase(end_it, result.end());

  if(cancel_search.load())
    {
      result.clear();
    }
#endif
  result.shrink_to_fit();
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
          = collection_path / std::filesystem::u8path(it_base->file_rel_path);
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
                           const std::string &source,
                           const double &coef_coincidence)
{
  std::string loc_source = af->stringToLower(source);

  if(loc_source.size() == 0 || to_search.size() == 0
     || to_search.size() > loc_source.size())
    {
      return false;
    }

  double weight;
  double incr = 1.0 / static_cast<double>(to_search.size());
  for(auto it = loc_source.begin();
      it
      != loc_source.begin() + loc_source.size()
             - (to_search.size() * (1 - coef_coincidence));
      it++)
    {
      if(it == loc_source.begin())
        {
          weight = 0.0;
          for(size_t i = 0; i < to_search.size() && it + i != loc_source.end();
              i++)
            {
              if(*(it + i) == to_search[i])
                {
                  weight += incr;
                }
              else
                {
                  break;
                }
              if(weight >= coef_coincidence)
                {
                  return true;
                }
            }
        }
      else if(*it == ' ')
        {
          if(it + 1 != loc_source.end())
            {
              weight = 0.0;
              for(size_t i = 0;
                  i < to_search.size() && it + i + 1 != loc_source.end(); i++)
                {
                  if(*(it + i + 1) == to_search[i])
                    {
                      weight += incr;
                    }
                  else
                    {
                      break;
                    }
                  if(weight >= coef_coincidence)
                    {
                      return true;
                    }
                }
            }
        }
    }

  return false;
}

bool
BaseKeeper::searchSurname(const BookBaseEntry &search,
                          std::vector<BookBaseEntry> &result,
                          const double &coef_coincidence)
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
          surname = af->stringToLower(surname);
          for(auto it = surname.begin(); it != surname.end();)
            {
              if((*it) == ' ')
                {
                  surname.erase(it);
                }
              else
                {
                  break;
                }
            }
#ifdef USE_OPENMP
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
                      if(searchLineFunc(surname, itb->book_author,
                                        coef_coincidence))
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
#else
          std::mutex result_mtx;
#ifdef USE_PE
          std::for_each(
              std::execution::par, base.begin(), base.end(),
              [&result_mtx, &result, surname, this,
               coef_coincidence](FileParseEntry &el) {
                if(cancel_search.load())
                  {
                    return void();
                  }
                std::filesystem::path book_file_path
                    = collection_path
                      / std::filesystem::u8path(el.file_rel_path);
                std::for_each(
                    std::execution::par, el.books.begin(), el.books.end(),
                    [this, surname, &result, &result_mtx, book_file_path,
                     coef_coincidence](BookParseEntry &el) {
                      if(cancel_search.load())
                        {
                          return void();
                        }
                      if(searchLineFunc(surname, el.book_author,
                                        coef_coincidence))
                        {
                          result_mtx.lock();
                          result.emplace_back(
                              BookBaseEntry(el, book_file_path));
                          result_mtx.unlock();
                        }
                    });
              });
#else
          std::for_each(base.begin(), base.end(),
                        [&result_mtx, &result, surname, coef_coincidence,
                         this](FileParseEntry &el) {
                          if(cancel_search.load())
                            {
                              return void();
                            }
                          std::filesystem::path book_file_path
                              = collection_path
                                / std::filesystem::u8path(el.file_rel_path);
                          std::for_each(
                              el.books.begin(), el.books.end(),
                              [this, surname, &result, &result_mtx,
                               book_file_path,
                               coef_coincidence](BookParseEntry &el) {
                                if(cancel_search.load())
                                  {
                                    return void();
                                  }
                                if(searchLineFunc(surname, el.book_author,
                                                  coef_coincidence))
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
                       std::vector<BookBaseEntry> &result,
                       const double &coef_coincidence)
{
  if(result.size() == 0)
    {
#ifdef USE_OPENMP
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
                  if(searchLineFunc(search.bpe.book_name, itb->book_name,
                                    coef_coincidence))
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
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, search, book_file_path, &result, &result_mtx,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_name, el.book_name,
                                    coef_coincidence))
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
          [this, search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(el.books.begin(), el.books.end(),
                          [this, search, book_file_path, &result, &result_mtx,
                           coef_coincidence](BookParseEntry &el) {
                            if(cancel_search.load())
                              {
                                return void();
                              }
                            if(searchLineFunc(search.bpe.book_name,
                                              el.book_name, coef_coincidence))
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
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_name,
                                           el.bpe.book_name, coef_coincidence))
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
      result.erase(std::remove_if(
                       std::execution::par, result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_name,
                                           el.bpe.book_name, coef_coincidence))
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
      result.erase(std::remove_if(
                       result.begin(), result.end(),
                       [search, coef_coincidence, this](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_name,
                                           el.bpe.book_name, coef_coincidence))
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
                         std::vector<BookBaseEntry> &result,
                         const double &coef_coincidence)
{
  if(result.size() == 0)
    {
#ifdef USE_OPENMP
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
                  if(searchLineFunc(search.bpe.book_series, itb->book_series,
                                    coef_coincidence))
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
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_series, el.book_series,
                                    coef_coincidence))
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
          [this, search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_series, el.book_series,
                                    coef_coincidence))
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
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_series,
                                           el.bpe.book_series,
                                           coef_coincidence))
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
          std::remove_if(std::execution::par, result.begin(), result.end(),
                         [search, this, coef_coincidence](BookBaseEntry &el) {
                           if(searchLineFunc(search.bpe.book_series,
                                             el.bpe.book_series,
                                             coef_coincidence))
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
                         [search, coef_coincidence, this](BookBaseEntry &el) {
                           if(searchLineFunc(search.bpe.book_series,
                                             el.bpe.book_series,
                                             coef_coincidence))
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
  books_in_base = 0;
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

size_t
BaseKeeper::getBookQuantity()
{
#ifdef USE_OPENMP
  OmpLockGuard olg(basemtx);
#else
  std::lock_guard<std::mutex> lglock(basemtx);
#endif
  return books_in_base;
}

void
BaseKeeper::searchGenre(const BookBaseEntry &search,
                        std::vector<BookBaseEntry> &result,
                        const double &coef_coincidence)
{
  if(result.size() == 0)
    {
#ifdef USE_OPENMP
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
                  if(searchLineFunc(search.bpe.book_genre, itb->book_genre,
                                    coef_coincidence))
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
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);
            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(search.bpe.book_genre, el.book_genre,
                                    coef_coincidence))
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
          [this, search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);
            std::for_each(el.books.begin(), el.books.end(),
                          [this, search, &result, &result_mtx, book_file_path,
                           coef_coincidence](BookParseEntry &el) {
                            if(cancel_search.load())
                              {
                                return void();
                              }
                            if(searchLineFunc(search.bpe.book_genre,
                                              el.book_genre, coef_coincidence))
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
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_genre,
                                           el.bpe.book_genre,
                                           coef_coincidence))
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
          std::remove_if(std::execution::par, result.begin(), result.end(),
                         [search, this, coef_coincidence](BookBaseEntry &el) {
                           if(searchLineFunc(search.bpe.book_genre,
                                             el.bpe.book_genre,
                                             coef_coincidence))
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
                         [search, coef_coincidence, this](BookBaseEntry &el) {
                           if(searchLineFunc(search.bpe.book_genre,
                                             el.bpe.book_genre,
                                             coef_coincidence))
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

#ifdef USE_GPUOFFLOADING
bool
BaseKeeper::searchSurnameGPU(const BookBaseEntry &search,
                             std::vector<BookBaseEntry> &result,
                             const double &coef_coincidence)
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
          surname = af->stringToLower(surname);
          for(auto it = surname.begin(); it != surname.end();)
            {
              if((*it) == ' ')
                {
                  surname.erase(it);
                }
              else
                {
                  break;
                }
            }

          std::string src_surnames;
          std::vector<size_t> offset;
          offset.reserve(books_in_base);
          std::vector<size_t> auth_sz;
          auth_sz.reserve(books_in_base);
          struct index
          {
            size_t base_ind;
            size_t book_ind;
          };
          std::vector<index> ind_v;
          ind_v.reserve(books_in_base);
          std::vector<int> use;
          use.reserve(books_in_base);

#pragma omp parallel
#pragma omp for
          for(size_t i = 0; i < base.size(); i++)
            {
              for(size_t j = 0; j < base[i].books.size(); j++)
                {
                  index ind;
                  ind.base_ind = i;
                  ind.book_ind = j;
                  std::string lstr
                      = af->stringToLower(base[i].books[j].book_author);
#pragma omp critical
                  {
                    offset.push_back(src_surnames.size());
                    auth_sz.push_back(lstr.size());
                    ind_v.push_back(ind);
                    use.push_back(false);
                    src_surnames += lstr;
                  }
                }
            }

          char *surname_gpu = surname.data();
          size_t surname_gpu_sz = surname.size();
          char *src_surnames_gpu = src_surnames.data();
          size_t src_surnames_gpu_sz = src_surnames.size();
          size_t *offset_gpu = offset.data();
          size_t offset_gpu_sz = offset.size();
          size_t *auth_sz_gpu = auth_sz.data();
          int *use_gpu = use.data();
#pragma omp target map(                                                       \
        to : surname_gpu[0 : surname_gpu_sz], surname_gpu_sz,                 \
            src_surnames_gpu[0 : src_surnames_gpu_sz], src_surnames_gpu_sz,   \
            offset_gpu[0 : offset_gpu_sz], offset_gpu_sz,                     \
            auth_sz_gpu[0 : offset_gpu_sz], coef_coincidence, cancel_search)  \
    map(tofrom : use_gpu[0 : offset_gpu_sz])
          {
#pragma omp parallel
#pragma omp for
            for(size_t i = 0; i < offset_gpu_sz; i++)
              {
                if(cancel_search)
                  {
#pragma omp cancel for
                    continue;
                  }
                use_gpu[i]
                    = searchLineFuncGPU(surname_gpu, surname_gpu_sz,
                                        (src_surnames_gpu + offset_gpu[i]),
                                        auth_sz_gpu[i], coef_coincidence);
              }
          }

#pragma omp parallel
#pragma omp for
          for(size_t i = 0; i < use.size(); i++)
            {
              if(use[i])
                {
                  std::filesystem::path book_file_path
                      = collection_path
                        / std::filesystem::u8path(
                            base[ind_v[i].base_ind].file_rel_path);
                  BookBaseEntry bbe(
                      base[ind_v[i].base_ind].books[ind_v[i].book_ind],
                      book_file_path);
#pragma omp critical
                  {
                    result.emplace_back(bbe);
                  }
                }
            }
        }
    }
  return all_empty;
}

bool
BaseKeeper::searchFirstNameGPU(const BookBaseEntry &search,
                               std::vector<BookBaseEntry> &result,
                               const double &coef_coincidence)
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
              first_name = af->stringToLower(first_name);
              for(auto it = first_name.begin(); it != first_name.end();)
                {
                  if((*it) == ' ')
                    {
                      first_name.erase(it);
                    }
                  else
                    {
                      break;
                    }
                }
              if(result.size() == 0)
                {
                  std::string src_names;
                  std::vector<size_t> offset;
                  offset.reserve(books_in_base);
                  std::vector<size_t> auth_sz;
                  auth_sz.reserve(books_in_base);
                  struct index
                  {
                    size_t base_ind;
                    size_t book_ind;
                  };
                  std::vector<index> ind_v;
                  ind_v.reserve(books_in_base);
                  std::vector<int> use;
                  use.reserve(books_in_base);

#pragma omp parallel
#pragma omp for
                  for(size_t i = 0; i < base.size(); i++)
                    {
                      for(size_t j = 0; j < base[i].books.size(); j++)
                        {
                          index ind;
                          ind.base_ind = i;
                          ind.book_ind = j;
                          std::string lstr = af->stringToLower(
                              base[i].books[j].book_author);
#pragma omp critical
                          {
                            offset.push_back(src_names.size());
                            auth_sz.push_back(lstr.size());
                            ind_v.push_back(ind);
                            use.push_back(false);
                            src_names += lstr;
                          }
                        }
                    }

                  char *name_gpu = first_name.data();
                  size_t name_gpu_sz = first_name.size();
                  char *src_names_gpu = src_names.data();
                  size_t src_names_gpu_sz = src_names.size();
                  size_t *offset_gpu = offset.data();
                  size_t offset_gpu_sz = offset.size();
                  size_t *auth_sz_gpu = auth_sz.data();
                  int *use_gpu = use.data();
#pragma omp target map(to : name_gpu[0 : name_gpu_sz], name_gpu_sz,           \
                           src_names_gpu[0 : src_names_gpu_sz],               \
                           src_names_gpu_sz, offset_gpu[0 : offset_gpu_sz],   \
                           offset_gpu_sz, auth_sz_gpu[0 : offset_gpu_sz],     \
                           coef_coincidence, cancel_search)                   \
    map(tofrom : use_gpu[0 : offset_gpu_sz])
                  {
#pragma omp parallel
#pragma omp for
                    for(size_t i = 0; i < offset_gpu_sz; i++)
                      {
                        if(cancel_search)
                          {
#pragma omp cancel for
                            continue;
                          }
                        use_gpu[i] = searchLineFuncGPU(
                            name_gpu, name_gpu_sz,
                            (src_names_gpu + offset_gpu[i]), auth_sz_gpu[i],
                            coef_coincidence);
                      }
                  }

#pragma omp parallel
#pragma omp for
                  for(size_t i = 0; i < use.size(); i++)
                    {
                      if(use[i])
                        {
                          std::filesystem::path book_file_path
                              = collection_path
                                / std::filesystem::u8path(
                                    base[ind_v[i].base_ind].file_rel_path);
                          BookBaseEntry bbe(
                              base[ind_v[i].base_ind].books[ind_v[i].book_ind],
                              book_file_path);
#pragma omp critical
                          {
                            result.emplace_back(bbe);
                          }
                        }
                    }
                }
              else
                {
                  result.erase(AuxFunc::parallelRemoveIf(
                                   result.begin(), result.end(),
                                   [first_name, this,
                                    coef_coincidence](BookBaseEntry &el) {
                                     if(searchLineFunc(first_name,
                                                       el.bpe.book_author,
                                                       coef_coincidence))
                                       {
                                         return false;
                                       }
                                     else
                                       {
                                         return true;
                                       }
                                   }),
                               result.end());
                }
            }
        }
    }
  return all_empty;
}

bool
BaseKeeper::searchLastNameGPU(const BookBaseEntry &search,
                              std::vector<BookBaseEntry> &result,
                              const double &coef_coincidence)
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
              last_name = af->stringToLower(last_name);
              for(auto it = last_name.begin(); it != last_name.end();)
                {
                  if((*it) == ' ')
                    {
                      last_name.erase(it);
                    }
                  else
                    {
                      break;
                    }
                }
              if(result.size() == 0)
                {
                  std::string src_lastnames;
                  std::vector<size_t> offset;
                  offset.reserve(books_in_base);
                  std::vector<size_t> auth_sz;
                  auth_sz.reserve(books_in_base);
                  struct index
                  {
                    size_t base_ind;
                    size_t book_ind;
                  };
                  std::vector<index> ind_v;
                  ind_v.reserve(books_in_base);
                  std::vector<int> use;
                  use.reserve(books_in_base);

#pragma omp parallel
#pragma omp for
                  for(size_t i = 0; i < base.size(); i++)
                    {
                      for(size_t j = 0; j < base[i].books.size(); j++)
                        {
                          index ind;
                          ind.base_ind = i;
                          ind.book_ind = j;
                          std::string lstr = af->stringToLower(
                              base[i].books[j].book_author);
#pragma omp critical
                          {
                            offset.push_back(src_lastnames.size());
                            auth_sz.push_back(lstr.size());
                            ind_v.push_back(ind);
                            use.push_back(false);
                            src_lastnames += lstr;
                          }
                        }
                    }

                  char *lastname_gpu = last_name.data();
                  size_t lastname_gpu_sz = last_name.size();
                  char *src_lastnames_gpu = src_lastnames.data();
                  size_t src_lastnames_gpu_sz = src_lastnames.size();
                  size_t *offset_gpu = offset.data();
                  size_t offset_gpu_sz = offset.size();
                  size_t *auth_sz_gpu = auth_sz.data();
                  int *use_gpu = use.data();
#pragma omp target map(                                                       \
        to : lastname_gpu[0 : lastname_gpu_sz], lastname_gpu_sz,              \
            src_lastnames_gpu[0 : src_lastnames_gpu_sz],                      \
            src_lastnames_gpu_sz, offset_gpu[0 : offset_gpu_sz],              \
            offset_gpu_sz, auth_sz_gpu[0 : offset_gpu_sz], coef_coincidence,  \
            cancel_search) map(tofrom : use_gpu[0 : offset_gpu_sz])
                  {
#pragma omp parallel
#pragma omp for
                    for(size_t i = 0; i < offset_gpu_sz; i++)
                      {
                        if(cancel_search)
                          {
#pragma omp cancel for
                            continue;
                          }
                        use_gpu[i] = searchLineFuncGPU(
                            lastname_gpu, lastname_gpu_sz,
                            (src_lastnames_gpu + offset_gpu[i]),
                            auth_sz_gpu[i], coef_coincidence);
                      }
                  }

#pragma omp parallel
#pragma omp for
                  for(size_t i = 0; i < use.size(); i++)
                    {
                      if(use[i])
                        {
                          std::filesystem::path book_file_path
                              = collection_path
                                / std::filesystem::u8path(
                                    base[ind_v[i].base_ind].file_rel_path);
                          BookBaseEntry bbe(
                              base[ind_v[i].base_ind].books[ind_v[i].book_ind],
                              book_file_path);
#pragma omp critical
                          {
                            result.emplace_back(bbe);
                          }
                        }
                    }
                }
              else
                {
                  result.erase(AuxFunc::parallelRemoveIf(
                                   result.begin(), result.end(),
                                   [last_name, this,
                                    coef_coincidence](BookBaseEntry &el) {
                                     if(searchLineFunc(last_name,
                                                       el.bpe.book_author,
                                                       coef_coincidence))
                                       {
                                         return false;
                                       }
                                     else
                                       {
                                         return true;
                                       }
                                   }),
                               result.end());
                }
            }
        }
    }
  return all_empty;
}

void
BaseKeeper::searchBookGPU(const BookBaseEntry &search,
                          std::vector<BookBaseEntry> &result,
                          const double &coef_coincidence)
{
  if(result.size() == 0)
    {
      std::string src_books;
      std::vector<size_t> offset;
      offset.reserve(books_in_base);
      std::vector<size_t> auth_sz;
      auth_sz.reserve(books_in_base);
      struct index
      {
        size_t base_ind;
        size_t book_ind;
      };
      std::vector<index> ind_v;
      ind_v.reserve(books_in_base);
      std::vector<int> use;
      use.reserve(books_in_base);

#pragma omp parallel
#pragma omp for
      for(size_t i = 0; i < base.size(); i++)
        {
          for(size_t j = 0; j < base[i].books.size(); j++)
            {
              index ind;
              ind.base_ind = i;
              ind.book_ind = j;
              std::string lstr = af->stringToLower(base[i].books[j].book_name);
#pragma omp critical
              {
                offset.push_back(src_books.size());
                auth_sz.push_back(lstr.size());
                ind_v.push_back(ind);
                use.push_back(false);
                src_books += lstr;
              }
            }
        }

      char *searchbook_gpu = const_cast<char *>(search.bpe.book_name.data());
      size_t searchbook_gpu_sz = search.bpe.book_name.size();
      char *src_books_gpu = src_books.data();
      size_t src_books_gpu_sz = src_books.size();
      size_t *offset_gpu = offset.data();
      size_t offset_gpu_sz = offset.size();
      size_t *auth_sz_gpu = auth_sz.data();
      int *use_gpu = use.data();
#pragma omp target map(                                                       \
        to : searchbook_gpu[0 : searchbook_gpu_sz], searchbook_gpu_sz,        \
            src_books_gpu[0 : src_books_gpu_sz], src_books_gpu_sz,            \
            offset_gpu[0 : offset_gpu_sz], offset_gpu_sz,                     \
            auth_sz_gpu[0 : offset_gpu_sz], coef_coincidence, cancel_search)  \
    map(tofrom : use_gpu[0 : offset_gpu_sz])
      {
#pragma omp parallel
#pragma omp for
        for(size_t i = 0; i < offset_gpu_sz; i++)
          {
            if(cancel_search)
              {
#pragma omp cancel for
                continue;
              }
            use_gpu[i] = searchLineFuncGPU(searchbook_gpu, searchbook_gpu_sz,
                                           (src_books_gpu + offset_gpu[i]),
                                           auth_sz_gpu[i], coef_coincidence);
          }
      }

#pragma omp parallel
#pragma omp for
      for(size_t i = 0; i < use.size(); i++)
        {
          if(use[i])
            {
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(
                        base[ind_v[i].base_ind].file_rel_path);
              BookBaseEntry bbe(
                  base[ind_v[i].base_ind].books[ind_v[i].book_ind],
                  book_file_path);
#pragma omp critical
              {
                result.emplace_back(bbe);
              }
            }
        }
    }
  else
    {
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_name,
                                           el.bpe.book_name, coef_coincidence))
                           {
                             return false;
                           }
                         else
                           {
                             return true;
                           }
                       }),
                   result.end());
    }
}

void
BaseKeeper::searchSeriesGPU(const BookBaseEntry &search,
                            std::vector<BookBaseEntry> &result,
                            const double &coef_coincidence)
{
  if(result.size() == 0)
    {
      std::string src_series;
      std::vector<size_t> offset;
      offset.reserve(books_in_base);
      std::vector<size_t> auth_sz;
      auth_sz.reserve(books_in_base);
      struct index
      {
        size_t base_ind;
        size_t book_ind;
      };
      std::vector<index> ind_v;
      ind_v.reserve(books_in_base);
      std::vector<int> use;
      use.reserve(books_in_base);

#pragma omp parallel
#pragma omp for
      for(size_t i = 0; i < base.size(); i++)
        {
          for(size_t j = 0; j < base[i].books.size(); j++)
            {
              index ind;
              ind.base_ind = i;
              ind.book_ind = j;
              std::string lstr
                  = af->stringToLower(base[i].books[j].book_series);
#pragma omp critical
              {
                offset.push_back(src_series.size());
                auth_sz.push_back(lstr.size());
                ind_v.push_back(ind);
                use.push_back(false);
                src_series += lstr;
              }
            }
        }

      char *searchseries_gpu
          = const_cast<char *>(search.bpe.book_series.data());
      size_t searchseries_gpu_sz = search.bpe.book_series.size();
      char *src_series_gpu = src_series.data();
      size_t src_series_gpu_sz = src_series.size();
      size_t *offset_gpu = offset.data();
      size_t offset_gpu_sz = offset.size();
      size_t *auth_sz_gpu = auth_sz.data();
      int *use_gpu = use.data();
#pragma omp target map(                                                       \
        to : searchseries_gpu[0 : searchseries_gpu_sz], searchseries_gpu_sz,  \
            src_series_gpu[0 : src_series_gpu_sz], src_series_gpu_sz,         \
            offset_gpu[0 : offset_gpu_sz], offset_gpu_sz,                     \
            auth_sz_gpu[0 : offset_gpu_sz], coef_coincidence, cancel_search)  \
    map(tofrom : use_gpu[0 : offset_gpu_sz])
      {
#pragma omp parallel
#pragma omp for
        for(size_t i = 0; i < offset_gpu_sz; i++)
          {
            if(cancel_search)
              {
#pragma omp cancel for
                continue;
              }
            use_gpu[i]
                = searchLineFuncGPU(searchseries_gpu, searchseries_gpu_sz,
                                    (src_series_gpu + offset_gpu[i]),
                                    auth_sz_gpu[i], coef_coincidence);
          }
      }

#pragma omp parallel
#pragma omp for
      for(size_t i = 0; i < use.size(); i++)
        {
          if(use[i])
            {
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(
                        base[ind_v[i].base_ind].file_rel_path);
              BookBaseEntry bbe(
                  base[ind_v[i].base_ind].books[ind_v[i].book_ind],
                  book_file_path);
#pragma omp critical
              {
                result.emplace_back(bbe);
              }
            }
        }
    }
  else
    {
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_series,
                                           el.bpe.book_series,
                                           coef_coincidence))
                           {
                             return false;
                           }
                         else
                           {
                             return true;
                           }
                       }),
                   result.end());
    }
}

void
BaseKeeper::searchGenreGPU(const BookBaseEntry &search,
                           std::vector<BookBaseEntry> &result,
                           const double &coef_coincidence)
{
  if(result.size() == 0)
    {
      std::string src_genres;
      std::vector<size_t> offset;
      offset.reserve(books_in_base);
      std::vector<size_t> auth_sz;
      auth_sz.reserve(books_in_base);
      struct index
      {
        size_t base_ind;
        size_t book_ind;
      };
      std::vector<index> ind_v;
      ind_v.reserve(books_in_base);
      std::vector<int> use;
      use.reserve(books_in_base);

#pragma omp parallel
#pragma omp for
      for(size_t i = 0; i < base.size(); i++)
        {
          for(size_t j = 0; j < base[i].books.size(); j++)
            {
              index ind;
              ind.base_ind = i;
              ind.book_ind = j;
              std::string lstr
                  = af->stringToLower(base[i].books[j].book_genre);
#pragma omp critical
              {
                offset.push_back(src_genres.size());
                auth_sz.push_back(lstr.size());
                ind_v.push_back(ind);
                use.push_back(false);
                src_genres += lstr;
              }
            }
        }

      char *searchgenres_gpu
          = const_cast<char *>(search.bpe.book_genre.data());
      size_t searchgenres_gpu_sz = search.bpe.book_genre.size();
      char *src_genres_gpu = src_genres.data();
      size_t src_genres_gpu_sz = src_genres.size();
      size_t *offset_gpu = offset.data();
      size_t offset_gpu_sz = offset.size();
      size_t *auth_sz_gpu = auth_sz.data();
      int *use_gpu = use.data();
#pragma omp target map(                                                       \
        to : searchgenres_gpu[0 : searchgenres_gpu_sz], searchgenres_gpu_sz,  \
            src_genres_gpu[0 : src_genres_gpu_sz], src_genres_gpu_sz,         \
            offset_gpu[0 : offset_gpu_sz], offset_gpu_sz,                     \
            auth_sz_gpu[0 : offset_gpu_sz], coef_coincidence, cancel_search)  \
    map(tofrom : use_gpu[0 : offset_gpu_sz])
      {
#pragma omp parallel
#pragma omp for
        for(size_t i = 0; i < offset_gpu_sz; i++)
          {
            if(cancel_search)
              {
#pragma omp cancel for
                continue;
              }
            use_gpu[i]
                = searchLineFuncGPU(searchgenres_gpu, searchgenres_gpu_sz,
                                    (src_genres_gpu + offset_gpu[i]),
                                    auth_sz_gpu[i], coef_coincidence);
          }
      }

#pragma omp parallel
#pragma omp for
      for(size_t i = 0; i < use.size(); i++)
        {
          if(use[i])
            {
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(
                        base[ind_v[i].base_ind].file_rel_path);
              BookBaseEntry bbe(
                  base[ind_v[i].base_ind].books[ind_v[i].book_ind],
                  book_file_path);
#pragma omp critical
              {
                result.emplace_back(bbe);
              }
            }
        }
    }
  else
    {
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(search.bpe.book_genre,
                                           el.bpe.book_genre,
                                           coef_coincidence))
                           {
                             return false;
                           }
                         else
                           {
                             return true;
                           }
                       }),
                   result.end());
    }
}

#pragma omp declare target
bool
BaseKeeper::searchLineFuncGPU(const char *to_search,
                              const size_t &to_search_size, const char *source,
                              const size_t &source_sz,
                              const double &coef_coincidence)
{
  if(source_sz == 0 || to_search_size == 0 || to_search_size > source_sz)
    {
      return false;
    }

  double weight;
  double incr = 1.0 / static_cast<double>(to_search_size);
  size_t dif = static_cast<size_t>(to_search_size * (1 - coef_coincidence));

  for(const char *it = source; it != source + source_sz - dif; it++)
    {
      if(it == source)
        {
          weight = 0.0;
          for(size_t i = 0;
              i < to_search_size && it + i != (source + source_sz); i++)
            {
              if(*(it + i) == to_search[i])
                {
                  weight += incr;
                }
              else
                {
                  break;
                }
              if(weight >= coef_coincidence)
                {
                  return true;
                }
            }
        }
      else if(*it == ' ')
        {
          if(it + 1 != (source + source_sz))
            {
              weight = 0.0;
              for(size_t i = 0;
                  i < to_search_size && it + i + 1 != (source + source_sz);
                  i++)
                {
                  if(*(it + i + 1) == to_search[i])
                    {
                      weight += incr;
                    }
                  else
                    {
                      break;
                    }
                  if(weight >= coef_coincidence)
                    {
                      return true;
                    }
                }
            }
        }
    }

  return false;
}
#pragma omp end declare target
#endif

void
BaseKeeper::stopSearch()
{
#ifdef USE_OPENMP
#pragma atomic write
  cancel_search = true;
#ifdef USE_GPUOFFLOADING
#pragma omp target update to(cancel_search)
#endif
#else
  cancel_search.store(true);
#endif
}

bool
BaseKeeper::searchLastName(const BookBaseEntry &search,
                           std::vector<BookBaseEntry> &result,
                           const double &coef_coincidence)
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
          last_name = af->stringToLower(last_name);
          for(auto it = last_name.begin(); it != last_name.end();)
            {
              if((*it) == ' ')
                {
                  last_name.erase(it);
                }
              else
                {
                  break;
                }
            }
          if(!last_name.empty())
            {
              all_empty = false;
              if(result.size() == 0)
                {
#ifdef USE_OPENMP
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
                              if(searchLineFunc(last_name, itb->book_author,
                                                coef_coincidence))
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
#else
                  std::mutex result_mtx;
#ifdef USE_PE
                  std::for_each(
                      std::execution::par, base.begin(), base.end(),
                      [this, last_name, &result, &result_mtx,
                       coef_coincidence](FileParseEntry &el) {
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
                             book_file_path,
                             coef_coincidence](BookParseEntry &el) {
                              if(cancel_search.load())
                                return void();
                              {
                              }
                              if(searchLineFunc(last_name, el.book_author,
                                                coef_coincidence))
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
                      [this, last_name, &result, &result_mtx,
                       coef_coincidence](FileParseEntry &el) {
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
                             book_file_path,
                             coef_coincidence](BookParseEntry &el) {
                              if(cancel_search.load())
                                return void();
                              {
                              }
                              if(searchLineFunc(last_name, el.book_author,
                                                coef_coincidence))
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
                  result.erase(AuxFunc::parallelRemoveIf(
                                   result.begin(), result.end(),
                                   [last_name, this,
                                    coef_coincidence](BookBaseEntry &el) {
                                     if(searchLineFunc(last_name,
                                                       el.bpe.book_author,
                                                       coef_coincidence))
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
                          [last_name, this,
                           coef_coincidence](BookBaseEntry &el) {
                            if(searchLineFunc(last_name, el.bpe.book_author,
                                              coef_coincidence))
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
                                     [last_name, coef_coincidence,
                                      this](BookBaseEntry &el) {
                                       if(searchLineFunc(last_name,
                                                         el.bpe.book_author,
                                                         coef_coincidence))
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
                            std::vector<BookBaseEntry> &result,
                            const double &coef_coincidence)
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
              first_name = af->stringToLower(first_name);
              for(auto it = first_name.begin(); it != first_name.end();)
                {
                  if((*it) == ' ')
                    {
                      first_name.erase(it);
                    }
                  else
                    {
                      break;
                    }
                }

              if(result.size() == 0)
                {
#ifdef USE_OPENMP
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
                              if(searchLineFunc(first_name, itb->book_author,
                                                coef_coincidence))
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
#else
                  std::mutex result_mtx;
#ifdef USE_PE
                  std::for_each(
                      std::execution::par, base.begin(), base.end(),
                      [this, first_name, &result, &result_mtx,
                       coef_coincidence](FileParseEntry &el) {
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
                             book_file_path,
                             coef_coincidence](BookParseEntry &el) {
                              if(cancel_search.load())
                                {
                                  return void();
                                }
                              if(searchLineFunc(first_name, el.book_author,
                                                coef_coincidence))
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
                      [this, first_name, &result, &result_mtx,
                       coef_coincidence](FileParseEntry &el) {
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
                             book_file_path,
                             coef_coincidence](BookParseEntry &el) {
                              if(cancel_search.load())
                                {
                                  return void();
                                }
                              if(searchLineFunc(first_name, el.book_author,
                                                coef_coincidence))
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
                  result.erase(AuxFunc::parallelRemoveIf(
                                   result.begin(), result.end(),
                                   [first_name, this,
                                    coef_coincidence](BookBaseEntry &el) {
                                     if(searchLineFunc(first_name,
                                                       el.bpe.book_author,
                                                       coef_coincidence))
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
                          [first_name, this,
                           coef_coincidence](BookBaseEntry &el) {
                            if(searchLineFunc(first_name, el.bpe.book_author,
                                              coef_coincidence))
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
                                     [first_name, coef_coincidence,
                                      this](BookBaseEntry &el) {
                                       if(searchLineFunc(first_name,
                                                         el.bpe.book_author,
                                                         coef_coincidence))
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
