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
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

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
  omp_set_lock(&basemtx);
  omp_unset_lock(&basemtx);
  omp_destroy_lock(&basemtx);
#ifdef USE_GPUOFFLOADING
  unloadBaseFromGPUMemory();
#endif
#else
  std::lock_guard<std::mutex> lglock(basemtx);
#endif
}

#ifdef USE_GPUOFFLOADING
void
BaseKeeper::loadCollection(const std::string &col_name,
                           const bool &offload_to_gpu)
#else
void
BaseKeeper::loadCollection(const std::string &col_name)
#endif
{
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lock_base(basemtx);
#else
  OmpLockGuard lock_base(basemtx);
#ifdef USE_GPUOFFLOADING
  unloadBaseFromGPUMemory();
#endif
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
    }
  base.shrink_to_fit();
#ifdef USE_OPENMP
#ifdef USE_GPUOFFLOADING
  if(offload_to_gpu)
    {
      loadBaseToGPUMemory();
    }
#endif
#endif
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
#ifdef USE_GPUOFFLOADING
  return collectionAuthorsGPU();
#endif
  std::vector<std::string> result;
  std::string find_str = ", ";
#ifdef USE_OPENMP
#pragma omp atomic write
  cancel_search = false;
  omp_set_lock(&basemtx);
  std::vector<std::string> l_result;
  l_result.reserve(books_in_base);
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
                    l_result.emplace_back(auth);
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
#else
  cancel_search.store(false);
  basemtx.lock();
  // result.reserve(books_in_base);
  // TODO check
  std::vector<std::string> l_result;
  l_result.reserve(books_in_base);
#ifdef USE_PE
  std::mutex auth_v_mtx;
  std::for_each(
      std::execution::par, base.begin(), base.end(),
      [this, &l_result, find_str, &auth_v_mtx](FileParseEntry &fpe) {
        if(cancel_search.load())
          {
            return void();
          }

        std::for_each(
            std::execution::par, fpe.books.begin(), fpe.books.end(),
            [this, find_str, &l_result, &auth_v_mtx](BookParseEntry &bpe) {
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
                      // TODO check
                      auth_v_mtx.lock();
                      l_result.emplace_back(auth);
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
                [this, &l_result, find_str](FileParseEntry &fpe) {
                  if(cancel_search.load())
                    {
                      return void();
                    }

                  std::for_each(
                      fpe.books.begin(), fpe.books.end(),
                      [this, find_str, &l_result](BookParseEntry &bpe) {
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
                                l_result.emplace_back(auth);
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

  double total_sz = 0;
  size_t l_result_sz = l_result.size();
  std::vector<std::string *> result_ptr;
  result_ptr.reserve(l_result_sz);
  for(size_t i = 0; i < l_result_sz; i++)
    {
      result_ptr.push_back(&l_result[i]);
      total_sz += static_cast<double>(l_result_sz - (i + 1));
    }

  std::atomic<double> progr = 0.0;

  bool stop_progr = false;
  std::mutex stop_progr_mtx;
  std::condition_variable stop_progr_var;

  std::thread progr_thr(
      [this, &stop_progr, &stop_progr_mtx, &stop_progr_var, &progr, total_sz] {
        if(auth_cpu_show_progr)
          {
            for(;;)
              {
                std::unique_lock<std::mutex> ullock(stop_progr_mtx);
                stop_progr_var.wait_for(ullock, std::chrono::milliseconds(300),
                                        [&stop_progr] {
                                          return stop_progr;
                                        });
                if(stop_progr)
                  {
                    break;
                  }
                else
                  {
                    auth_cpu_show_progr(progr.load(std::memory_order_relaxed),
                                        total_sz);
                  }
              }
          }
      });

  result.reserve(l_result_sz);
  bool c_s = false;
  double l_progr = -static_cast<double>(l_result_sz);
  for(size_t i = 0; i < l_result_sz; i++)
    {
#pragma omp atomic read
      c_s = cancel_search;
      if(c_s)
        {
          break;
        }
      l_progr += static_cast<double>(l_result_sz - i);
      progr.store(l_progr, std::memory_order_relaxed);
      if(result_ptr[i])
        {
#pragma omp parallel
#pragma omp for
          for(size_t j = i + 1; j < l_result_sz; j++)
            {
              if(result_ptr[j])
                {
                  if(*result_ptr[j] == *result_ptr[i])
                    {
                      result_ptr[j] = nullptr;
                    }
                }
            }
          result.emplace_back(*result_ptr[i]);
        }
    }

  stop_progr_mtx.lock();
  stop_progr = true;
  stop_progr_var.notify_all();
  stop_progr_mtx.unlock();
  progr_thr.join();

#pragma omp atomic read
  c_s = cancel_search;
  if(c_s)
    {
      result.clear();
    }
#else
  size_t l_result_sz = l_result.size();
  double total_sz = 0.0;
  std::vector<std::string *> result_ptr;
  result_ptr.reserve(l_result_sz);
  for(size_t i = 0; i < l_result_sz; i++)
    {
      result_ptr.push_back(&l_result[i]);
      total_sz += static_cast<double>(l_result_sz - (i + 1));
    }

  std::atomic<double> progr = 0.0;

  bool stop_progr = false;
  std::mutex stop_progr_mtx;
  std::condition_variable stop_progr_var;

  std::thread progr_thr(
      [this, &stop_progr, &stop_progr_mtx, &stop_progr_var, &progr, total_sz] {
        if(auth_cpu_show_progr)
          {
            for(;;)
              {
                std::unique_lock<std::mutex> ullock(stop_progr_mtx);
                stop_progr_var.wait_for(ullock, std::chrono::milliseconds(300),
                                        [&stop_progr] {
                                          return stop_progr;
                                        });
                if(stop_progr)
                  {
                    break;
                  }
                else
                  {
                    auth_cpu_show_progr(progr.load(std::memory_order_relaxed),
                                        total_sz);
                  }
              }
          }
      });

  result.reserve(l_result_sz);
  double l_progr = -static_cast<double>(l_result_sz);
  for(size_t i = 0; i < l_result_sz; i++)
    {
      if(cancel_search.load())
        {
          break;
        }
      l_progr += static_cast<double>(l_result_sz - i);
      progr.store(l_progr, std::memory_order_relaxed);
      if(result_ptr[i])
        {
          std::string val = *result_ptr[i];
#ifdef USE_PE
          std::for_each(std::execution::par, result_ptr.begin() + i + 1,
                        result_ptr.end(), [val](std::string *&el) {
                          if(el)
                            {
                              if(*el == val)
                                {
                                  el = nullptr;
                                }
                            }
                        });
#else
          std::for_each(result_ptr.begin() + i + 1, result_ptr.end(),
                        [val](std::string *&el) {
                          if(el)
                            {
                              if(*el == val)
                                {
                                  el = nullptr;
                                }
                            }
                        });
#endif
          result.emplace_back(*result_ptr[i]);
        }
    }

  stop_progr_mtx.lock();
  stop_progr = true;
  stop_progr_var.notify_all();
  stop_progr_mtx.unlock();

  progr_thr.join();

  if(cancel_search.load())
    {
      result.clear();
    }
    /*std::vector<std::string>::iterator end_it = result.end();
    std::atomic<double> progr = 0;
    double sz = static_cast<double>(
        std::ptrdiff_t(result.end().base() - result.data()));

    bool stop_progr = false;
    std::mutex stop_progr_mtx;
    std::condition_variable stop_progr_var;

    std::thread progr_thr(
        [this, &stop_progr, &stop_progr_mtx, &stop_progr_var, &progr, sz] {
          if(auth_show_progr)
            {
              for(;;)
                {
                  std::unique_lock<std::mutex> ullock(stop_progr_mtx);
                  stop_progr_var.wait_for(ullock,
  std::chrono::milliseconds(100),
                                          [&stop_progr] {
                                            return stop_progr;
                                          });
                  if(stop_progr)
                    {
                      break;
                    }
                  else
                    {
                      auth_show_progr(progr.load(std::memory_order_relaxed),
  sz);
                    }
                }
            }
        });

    std::ptrdiff_t dif;
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
        dif = end_it.base() - it.base();
        progr.store(sz - static_cast<double>(dif), std::memory_order_relaxed);
      }

    stop_progr_mtx.lock();
    stop_progr = true;
    stop_progr_var.notify_all();
    stop_progr_mtx.unlock();

    progr_thr.join();

    result.erase(end_it, result.end());

    if(cancel_search.load())
      {
        result.clear();
      }*/
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
  for(auto it = loc_source.begin(); it != loc_source.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          loc_source.erase(it);
        }
      else
        {
          break;
        }
    }

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
              if(*it >= 0 && *it <= ' ')
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
#pragma omp parallel for
              for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
                {
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
  std::string l_search = af->stringToLower(search.bpe.book_name);
  for(auto it = l_search.begin(); it != l_search.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          l_search.erase(it);
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
#pragma omp parallel for
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
              if(searchLineFunc(l_search, itb->book_name, coef_coincidence))
                {
#pragma omp critical
                  {
                    result.emplace_back(BookBaseEntry(*itb, book_file_path));
                  }
                }
            }
        }
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, l_search, book_file_path, &result, &result_mtx,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(l_search, el.book_name, coef_coincidence))
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
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                el.books.begin(), el.books.end(),
                [this, l_search, book_file_path, &result, &result_mtx,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(l_search, el.book_name, coef_coincidence))
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
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         return !searchLineFunc(l_search, el.bpe.book_name,
                                                coef_coincidence);
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
  std::string l_search = af->stringToLower(search.bpe.book_series);
  for(auto it = l_search.begin(); it != l_search.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          l_search.erase(it);
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
#pragma omp parallel for
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
              if(searchLineFunc(l_search, itb->book_series, coef_coincidence))
                {
#pragma omp critical
                  {
                    result.emplace_back(BookBaseEntry(*itb, book_file_path));
                  }
                }
            }
        }
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, l_search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(l_search, el.book_series,
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
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);

            std::for_each(
                el.books.begin(), el.books.end(),
                [this, l_search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(l_search, el.book_series,
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
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_series,
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
      result.erase(std::remove_if(
                       std::execution::par, result.begin(), result.end(),
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_series,
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
      result.erase(std::remove_if(
                       result.begin(), result.end(),
                       [l_search, coef_coincidence, this](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_series,
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
BaseKeeper::getBooksQuantity()
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
  std::string l_search = af->stringToLower(search.bpe.book_genre);
  for(auto it = l_search.begin(); it != l_search.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          l_search.erase(it);
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
#pragma omp parallel for
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
              if(searchLineFunc(l_search, itb->book_genre, coef_coincidence))
                {
#pragma omp critical
                  {
                    result.emplace_back(BookBaseEntry(*itb, book_file_path));
                  }
                }
            }
        }
#else
      std::mutex result_mtx;
#ifdef USE_PE
      std::for_each(
          std::execution::par, base.begin(), base.end(),
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);
            std::for_each(
                std::execution::par, el.books.begin(), el.books.end(),
                [this, l_search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(l_search, el.book_genre, coef_coincidence))
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
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el) {
            if(cancel_search.load())
              {
                return void();
              }
            std::filesystem::path book_file_path
                = collection_path / std::filesystem::u8path(el.file_rel_path);
            std::for_each(
                el.books.begin(), el.books.end(),
                [this, l_search, &result, &result_mtx, book_file_path,
                 coef_coincidence](BookParseEntry &el) {
                  if(cancel_search.load())
                    {
                      return void();
                    }
                  if(searchLineFunc(l_search, el.book_genre, coef_coincidence))
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
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_genre,
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
      result.erase(std::remove_if(
                       std::execution::par, result.begin(), result.end(),
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_genre,
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
      result.erase(std::remove_if(
                       result.begin(), result.end(),
                       [l_search, coef_coincidence, this](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_genre,
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
              if(*it >= 0 && *it <= ' ')
                {
                  surname.erase(it);
                }
              else
                {
                  break;
                }
            }

          omp_lock_t result_mtx;
          omp_init_lock(&result_mtx);

          std::thread gpu_thr;
          if(gpu_base && surname.size() > 0)
            {
              std::thread thr([this, surname, &coef_coincidence, &result,
                               &result_mtx] {
                size_t surname_gpu_sz = surname.size();
                char *surname_gpu = reinterpret_cast<char *>(omp_target_alloc(
                    surname_gpu_sz, omp_get_default_device()));

                int er = omp_target_memcpy(
                    surname_gpu, surname.data(), surname_gpu_sz, 0, 0,
                    omp_get_default_device(), omp_get_initial_device());
                if(er != 0)
                  {
                    omp_target_free(surname_gpu, omp_get_default_device());
                    return void();
                  }

                gpu_base_entry *result_gpu
                    = reinterpret_cast<gpu_base_entry *>(
                        omp_target_alloc(gpu_base_sz * sizeof(gpu_base_entry),
                                         omp_get_default_device()));

                size_t result_gpu_sz = 0;

#pragma omp target map(to : gpu_base, gpu_base_sz, surname_gpu,               \
                           surname_gpu_sz, result_gpu, coef_coincidence)      \
    map(tofrom : result_gpu_sz)
                {
#pragma omp parallel for
                  for(gpu_base_entry *i = gpu_base; i < gpu_base + gpu_base_sz;
                      i++)
                    {
                      if(searchLineFuncGPU(surname_gpu, surname_gpu_sz,
                                           i->book_author, i->book_author_sz,
                                           coef_coincidence))
                        {
#pragma omp critical
                          {
                            result_gpu[result_gpu_sz] = *i;
                            result_gpu_sz++;
                          }
                        }
                    }
                }

                if(result_gpu_sz > 0)
                  {
                    std::vector<gpu_base_entry> gp;
                    gp.resize(result_gpu_sz);

                    er = omp_target_memcpy(
                        gp.data(), result_gpu,
                        result_gpu_sz * sizeof(gpu_base_entry), 0, 0,
                        omp_get_initial_device(), omp_get_default_device());
                    if(er != 0)
                      {
                        gp.clear();
                      }
#pragma omp parallel
#pragma omp for
                    for(auto it = gp.begin(); it != gp.end(); it++)
                      {
                        std::filesystem::path p
                            = collection_path
                              / std::filesystem::u8path(
                                  it->base_entry->file_rel_path);
                        BookBaseEntry bbe(*(it->book_entry), p);
                        omp_set_lock(&result_mtx);
                        result.emplace_back(bbe);
                        omp_unset_lock(&result_mtx);
                      }
                  }

                omp_target_free(result_gpu, omp_get_default_device());
                omp_target_free(surname_gpu, omp_get_default_device());
              });
              gpu_thr = std::move(thr);
            }

#pragma omp parallel
#pragma omp for
          for(auto it = cpu_base.begin(); it != cpu_base.end(); it++)
            {
              if(searchLineFunc(surname, it->book_entry->book_author,
                                coef_coincidence))
                {
                  std::filesystem::path p = collection_path
                                            / std::filesystem::u8path(
                                                it->base_entry->file_rel_path);
                  BookBaseEntry bbe(*(it->book_entry), p);
                  omp_set_lock(&result_mtx);
                  result.emplace_back(bbe);
                  omp_unset_lock(&result_mtx);
                }
            }

          if(gpu_thr.joinable())
            {
              gpu_thr.join();
            }

          omp_destroy_lock(&result_mtx);
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
                  if(*it >= 0 && *it <= ' ')
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
                  omp_lock_t result_mtx;
                  omp_init_lock(&result_mtx);

                  std::thread gpu_thr;
                  if(gpu_base && first_name.size() > 0)
                    {
                      std::thread thr([this, first_name, &coef_coincidence,
                                       &result, &result_mtx] {
                        size_t first_name_gpu_sz = first_name.size();
                        char *first_name_gpu
                            = reinterpret_cast<char *>(omp_target_alloc(
                                first_name_gpu_sz, omp_get_default_device()));

                        int er = omp_target_memcpy(
                            first_name_gpu, first_name.data(),
                            first_name_gpu_sz, 0, 0, omp_get_default_device(),
                            omp_get_initial_device());
                        if(er != 0)
                          {
                            omp_target_free(first_name_gpu,
                                            omp_get_default_device());
                            return void();
                          }

                        gpu_base_entry *result_gpu
                            = reinterpret_cast<gpu_base_entry *>(
                                omp_target_alloc(gpu_base_sz
                                                     * sizeof(gpu_base_entry),
                                                 omp_get_default_device()));

                        size_t result_gpu_sz = 0;

#pragma omp target map(to : gpu_base, gpu_base_sz, first_name_gpu,            \
                           first_name_gpu_sz, result_gpu, coef_coincidence)   \
    map(tofrom : result_gpu_sz)
                        {
#pragma omp parallel for
                          for(gpu_base_entry *i = gpu_base;
                              i < gpu_base + gpu_base_sz; i++)
                            {
                              if(searchLineFuncGPU(
                                     first_name_gpu, first_name_gpu_sz,
                                     i->book_author, i->book_author_sz,
                                     coef_coincidence))
                                {
#pragma omp critical
                                  {
                                    result_gpu[result_gpu_sz] = *i;
                                    result_gpu_sz++;
                                  }
                                }
                            }
                        }

                        if(result_gpu_sz > 0)
                          {
                            std::vector<gpu_base_entry> gp;
                            gp.resize(result_gpu_sz);

                            er = omp_target_memcpy(
                                gp.data(), result_gpu,
                                result_gpu_sz * sizeof(gpu_base_entry), 0, 0,
                                omp_get_initial_device(),
                                omp_get_default_device());
                            if(er != 0)
                              {
                                gp.clear();
                              }
#pragma omp parallel
#pragma omp for
                            for(auto it = gp.begin(); it != gp.end(); it++)
                              {
                                std::filesystem::path p
                                    = collection_path
                                      / std::filesystem::u8path(
                                          it->base_entry->file_rel_path);
                                BookBaseEntry bbe(*(it->book_entry), p);
                                omp_set_lock(&result_mtx);
                                result.emplace_back(bbe);
                                omp_unset_lock(&result_mtx);
                              }
                          }

                        omp_target_free(result_gpu, omp_get_default_device());
                        omp_target_free(first_name_gpu,
                                        omp_get_default_device());
                      });
                      gpu_thr = std::move(thr);
                    }

#pragma omp parallel
#pragma omp for
                  for(auto it = cpu_base.begin(); it != cpu_base.end(); it++)
                    {
                      if(searchLineFunc(first_name,
                                        it->book_entry->book_author,
                                        coef_coincidence))
                        {
                          std::filesystem::path p
                              = collection_path
                                / std::filesystem::u8path(
                                    it->base_entry->file_rel_path);
                          BookBaseEntry bbe(*(it->book_entry), p);
                          omp_set_lock(&result_mtx);
                          result.emplace_back(bbe);
                          omp_unset_lock(&result_mtx);
                        }
                    }

                  if(gpu_thr.joinable())
                    {
                      gpu_thr.join();
                    }

                  omp_destroy_lock(&result_mtx);
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
                  if(*it >= 0 && *it <= ' ')
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
                  omp_lock_t result_mtx;
                  omp_init_lock(&result_mtx);

                  std::thread gpu_thr;
                  if(gpu_base && last_name.size() > 0)
                    {
                      std::thread thr([this, last_name, &coef_coincidence,
                                       &result, &result_mtx] {
                        size_t last_name_gpu_sz = last_name.size();
                        char *last_name_gpu
                            = reinterpret_cast<char *>(omp_target_alloc(
                                last_name_gpu_sz, omp_get_default_device()));

                        int er = omp_target_memcpy(
                            last_name_gpu, last_name.data(), last_name_gpu_sz,
                            0, 0, omp_get_default_device(),
                            omp_get_initial_device());
                        if(er != 0)
                          {
                            omp_target_free(last_name_gpu,
                                            omp_get_default_device());
                            return void();
                          }

                        gpu_base_entry *result_gpu
                            = reinterpret_cast<gpu_base_entry *>(
                                omp_target_alloc(gpu_base_sz
                                                     * sizeof(gpu_base_entry),
                                                 omp_get_default_device()));

                        size_t result_gpu_sz = 0;

#pragma omp target map(to : gpu_base, gpu_base_sz, last_name_gpu,             \
                           last_name_gpu_sz, result_gpu, coef_coincidence)    \
    map(tofrom : result_gpu_sz)
                        {
#pragma omp parallel for
                          for(gpu_base_entry *i = gpu_base;
                              i < gpu_base + gpu_base_sz; i++)
                            {
                              if(searchLineFuncGPU(
                                     last_name_gpu, last_name_gpu_sz,
                                     i->book_author, i->book_author_sz,
                                     coef_coincidence))
                                {
#pragma omp critical
                                  {
                                    result_gpu[result_gpu_sz] = *i;
                                    result_gpu_sz++;
                                  }
                                }
                            }
                        }

                        if(result_gpu_sz > 0)
                          {
                            std::vector<gpu_base_entry> gp;
                            gp.resize(result_gpu_sz);

                            er = omp_target_memcpy(
                                gp.data(), result_gpu,
                                result_gpu_sz * sizeof(gpu_base_entry), 0, 0,
                                omp_get_initial_device(),
                                omp_get_default_device());
                            if(er != 0)
                              {
                                gp.clear();
                              }
#pragma omp parallel
#pragma omp for
                            for(auto it = gp.begin(); it != gp.end(); it++)
                              {
                                std::filesystem::path p
                                    = collection_path
                                      / std::filesystem::u8path(
                                          it->base_entry->file_rel_path);
                                BookBaseEntry bbe(*(it->book_entry), p);
                                omp_set_lock(&result_mtx);
                                result.emplace_back(bbe);
                                omp_unset_lock(&result_mtx);
                              }
                          }

                        omp_target_free(result_gpu, omp_get_default_device());
                        omp_target_free(last_name_gpu,
                                        omp_get_default_device());
                      });
                      gpu_thr = std::move(thr);
                    }

#pragma omp parallel
#pragma omp for
                  for(auto it = cpu_base.begin(); it != cpu_base.end(); it++)
                    {
                      if(searchLineFunc(last_name, it->book_entry->book_author,
                                        coef_coincidence))
                        {
                          std::filesystem::path p
                              = collection_path
                                / std::filesystem::u8path(
                                    it->base_entry->file_rel_path);
                          BookBaseEntry bbe(*(it->book_entry), p);
                          omp_set_lock(&result_mtx);
                          result.emplace_back(bbe);
                          omp_unset_lock(&result_mtx);
                        }
                    }

                  if(gpu_thr.joinable())
                    {
                      gpu_thr.join();
                    }

                  omp_destroy_lock(&result_mtx);
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
  std::string l_search = af->stringToLower(search.bpe.book_name);
  for(auto it = l_search.begin(); it != l_search.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          l_search.erase(it);
        }
      else
        {
          break;
        }
    }
  if(result.size() == 0)
    {
      omp_lock_t result_mtx;
      omp_init_lock(&result_mtx);

      std::thread gpu_thr;
      if(gpu_base && l_search.size() > 0)
        {
          std::thread thr([this, l_search, &coef_coincidence, &result,
                           &result_mtx] {
            size_t l_search_gpu_sz = l_search.size();
            char *l_search_gpu = reinterpret_cast<char *>(
                omp_target_alloc(l_search_gpu_sz, omp_get_default_device()));

            int er = omp_target_memcpy(
                l_search_gpu, l_search.data(), l_search_gpu_sz, 0, 0,
                omp_get_default_device(), omp_get_initial_device());
            if(er != 0)
              {
                omp_target_free(l_search_gpu, omp_get_default_device());
                return void();
              }

            gpu_base_entry *result_gpu = reinterpret_cast<gpu_base_entry *>(
                omp_target_alloc(gpu_base_sz * sizeof(gpu_base_entry),
                                 omp_get_default_device()));

            size_t result_gpu_sz = 0;

#pragma omp target map(to : gpu_base, gpu_base_sz, l_search_gpu,              \
                           l_search_gpu_sz, result_gpu, coef_coincidence)     \
    map(tofrom : result_gpu_sz)
            {
#pragma omp parallel for
              for(gpu_base_entry *i = gpu_base; i < gpu_base + gpu_base_sz;
                  i++)
                {
                  if(searchLineFuncGPU(l_search_gpu, l_search_gpu_sz,
                                       i->book_name, i->book_name_sz,
                                       coef_coincidence))
                    {
#pragma omp critical
                      {
                        result_gpu[result_gpu_sz] = *i;
                        result_gpu_sz++;
                      }
                    }
                }
            }

            if(result_gpu_sz > 0)
              {
                std::vector<gpu_base_entry> gp;
                gp.resize(result_gpu_sz);

                er = omp_target_memcpy(gp.data(), result_gpu,
                                       result_gpu_sz * sizeof(gpu_base_entry),
                                       0, 0, omp_get_initial_device(),
                                       omp_get_default_device());
                if(er != 0)
                  {
                    gp.clear();
                  }
#pragma omp parallel
#pragma omp for
                for(auto it = gp.begin(); it != gp.end(); it++)
                  {
                    std::filesystem::path p
                        = collection_path
                          / std::filesystem::u8path(
                              it->base_entry->file_rel_path);
                    BookBaseEntry bbe(*(it->book_entry), p);
                    omp_set_lock(&result_mtx);
                    result.emplace_back(bbe);
                    omp_unset_lock(&result_mtx);
                  }
              }

            omp_target_free(result_gpu, omp_get_default_device());
            omp_target_free(l_search_gpu, omp_get_default_device());
          });
          gpu_thr = std::move(thr);
        }

#pragma omp parallel
#pragma omp for
      for(auto it = cpu_base.begin(); it != cpu_base.end(); it++)
        {
          if(searchLineFunc(l_search, it->book_entry->book_name,
                            coef_coincidence))
            {
              std::filesystem::path p
                  = collection_path
                    / std::filesystem::u8path(it->base_entry->file_rel_path);
              BookBaseEntry bbe(*(it->book_entry), p);
              omp_set_lock(&result_mtx);
              result.emplace_back(bbe);
              omp_unset_lock(&result_mtx);
            }
        }

      if(gpu_thr.joinable())
        {
          gpu_thr.join();
        }

      omp_destroy_lock(&result_mtx);
    }
  else
    {
      result.erase(
          AuxFunc::parallelRemoveIf(
              result.begin(), result.end(),
              [l_search, this, coef_coincidence](const BookBaseEntry &el) {
                if(searchLineFunc(l_search, el.bpe.book_name,
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
BaseKeeper::searchSeriesGPU(const BookBaseEntry &search,
                            std::vector<BookBaseEntry> &result,
                            const double &coef_coincidence)
{
  std::string l_search = af->stringToLower(search.bpe.book_series);
  for(auto it = l_search.begin(); it != l_search.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          l_search.erase(it);
        }
      else
        {
          break;
        }
    }
  if(result.size() == 0)
    {
      omp_lock_t result_mtx;
      omp_init_lock(&result_mtx);

      std::thread gpu_thr;
      if(gpu_base && l_search.size() > 0)
        {
          std::thread thr([this, l_search, &coef_coincidence, &result,
                           &result_mtx] {
            size_t l_search_gpu_sz = l_search.size();
            char *l_search_gpu = reinterpret_cast<char *>(
                omp_target_alloc(l_search_gpu_sz, omp_get_default_device()));

            int er = omp_target_memcpy(
                l_search_gpu, l_search.data(), l_search_gpu_sz, 0, 0,
                omp_get_default_device(), omp_get_initial_device());
            if(er != 0)
              {
                omp_target_free(l_search_gpu, omp_get_default_device());
                return void();
              }

            gpu_base_entry *result_gpu = reinterpret_cast<gpu_base_entry *>(
                omp_target_alloc(gpu_base_sz * sizeof(gpu_base_entry),
                                 omp_get_default_device()));

            size_t result_gpu_sz = 0;

#pragma omp target map(to : gpu_base, gpu_base_sz, l_search_gpu,              \
                           l_search_gpu_sz, result_gpu, coef_coincidence)     \
    map(tofrom : result_gpu_sz)
            {
#pragma omp parallel for
              for(gpu_base_entry *i = gpu_base; i < gpu_base + gpu_base_sz;
                  i++)
                {
                  if(searchLineFuncGPU(l_search_gpu, l_search_gpu_sz,
                                       i->book_series, i->book_series_sz,
                                       coef_coincidence))
                    {
#pragma omp critical
                      {
                        result_gpu[result_gpu_sz] = *i;
                        result_gpu_sz++;
                      }
                    }
                }
            }

            if(result_gpu_sz > 0)
              {
                std::vector<gpu_base_entry> gp;
                gp.resize(result_gpu_sz);

                er = omp_target_memcpy(gp.data(), result_gpu,
                                       result_gpu_sz * sizeof(gpu_base_entry),
                                       0, 0, omp_get_initial_device(),
                                       omp_get_default_device());
                if(er != 0)
                  {
                    gp.clear();
                  }
#pragma omp parallel
#pragma omp for
                for(auto it = gp.begin(); it != gp.end(); it++)
                  {
                    std::filesystem::path p
                        = collection_path
                          / std::filesystem::u8path(
                              it->base_entry->file_rel_path);
                    BookBaseEntry bbe(*(it->book_entry), p);
                    omp_set_lock(&result_mtx);
                    result.emplace_back(bbe);
                    omp_unset_lock(&result_mtx);
                  }
              }

            omp_target_free(result_gpu, omp_get_default_device());
            omp_target_free(l_search_gpu, omp_get_default_device());
          });
          gpu_thr = std::move(thr);
        }

#pragma omp parallel
#pragma omp for
      for(auto it = cpu_base.begin(); it != cpu_base.end(); it++)
        {
          if(searchLineFunc(l_search, it->book_entry->book_series,
                            coef_coincidence))
            {
              std::filesystem::path p
                  = collection_path
                    / std::filesystem::u8path(it->base_entry->file_rel_path);
              BookBaseEntry bbe(*(it->book_entry), p);
              omp_set_lock(&result_mtx);
              result.emplace_back(bbe);
              omp_unset_lock(&result_mtx);
            }
        }

      if(gpu_thr.joinable())
        {
          gpu_thr.join();
        }

      omp_destroy_lock(&result_mtx);
    }
  else
    {
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_series,
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
  std::string l_search = af->stringToLower(search.bpe.book_genre);
  for(auto it = l_search.begin(); it != l_search.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          l_search.erase(it);
        }
      else
        {
          break;
        }
    }
  if(result.size() == 0)
    {
      omp_lock_t result_mtx;
      omp_init_lock(&result_mtx);

      std::thread gpu_thr;
      if(gpu_base && l_search.size() > 0)
        {
          std::thread thr([this, l_search, &coef_coincidence, &result,
                           &result_mtx] {
            size_t l_search_gpu_sz = l_search.size();
            char *l_search_gpu = reinterpret_cast<char *>(
                omp_target_alloc(l_search_gpu_sz, omp_get_default_device()));

            int er = omp_target_memcpy(
                l_search_gpu, l_search.data(), l_search_gpu_sz, 0, 0,
                omp_get_default_device(), omp_get_initial_device());
            if(er != 0)
              {
                omp_target_free(l_search_gpu, omp_get_default_device());
                return void();
              }

            gpu_base_entry *result_gpu = reinterpret_cast<gpu_base_entry *>(
                omp_target_alloc(gpu_base_sz * sizeof(gpu_base_entry),
                                 omp_get_default_device()));

            size_t result_gpu_sz = 0;

#pragma omp target map(to : gpu_base, gpu_base_sz, l_search_gpu,              \
                           l_search_gpu_sz, result_gpu, coef_coincidence)     \
    map(tofrom : result_gpu_sz)
            {
#pragma omp parallel for
              for(gpu_base_entry *i = gpu_base; i < gpu_base + gpu_base_sz;
                  i++)
                {
                  if(searchLineFuncGPU(l_search_gpu, l_search_gpu_sz,
                                       i->book_genre, i->book_genre_sz,
                                       coef_coincidence))
                    {
#pragma omp critical
                      {
                        result_gpu[result_gpu_sz] = *i;
                        result_gpu_sz++;
                      }
                    }
                }
            }

            if(result_gpu_sz > 0)
              {
                std::vector<gpu_base_entry> gp;
                gp.resize(result_gpu_sz);

                er = omp_target_memcpy(gp.data(), result_gpu,
                                       result_gpu_sz * sizeof(gpu_base_entry),
                                       0, 0, omp_get_initial_device(),
                                       omp_get_default_device());
                if(er != 0)
                  {
                    gp.clear();
                  }
#pragma omp parallel
#pragma omp for
                for(auto it = gp.begin(); it != gp.end(); it++)
                  {
                    std::filesystem::path p
                        = collection_path
                          / std::filesystem::u8path(
                              it->base_entry->file_rel_path);
                    BookBaseEntry bbe(*(it->book_entry), p);
                    omp_set_lock(&result_mtx);
                    result.emplace_back(bbe);
                    omp_unset_lock(&result_mtx);
                  }
              }

            omp_target_free(result_gpu, omp_get_default_device());
            omp_target_free(l_search_gpu, omp_get_default_device());
          });
          gpu_thr = std::move(thr);
        }

#pragma omp parallel
#pragma omp for
      for(auto it = cpu_base.begin(); it != cpu_base.end(); it++)
        {
          if(searchLineFunc(l_search, it->book_entry->book_genre,
                            coef_coincidence))
            {
              std::filesystem::path p
                  = collection_path
                    / std::filesystem::u8path(it->base_entry->file_rel_path);
              BookBaseEntry bbe(*(it->book_entry), p);
              omp_set_lock(&result_mtx);
              result.emplace_back(bbe);
              omp_unset_lock(&result_mtx);
            }
        }

      if(gpu_thr.joinable())
        {
          gpu_thr.join();
        }

      omp_destroy_lock(&result_mtx);
    }
  else
    {
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [l_search, this, coef_coincidence](BookBaseEntry &el) {
                         if(searchLineFunc(l_search, el.bpe.book_genre,
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

std::vector<std::string>
BaseKeeper::collectionAuthorsGPU()
{
  std::vector<std::string> result;
  OmpLockGuard olg(basemtx);
  if(base.size() == 0)
    {
      return result;
    }
  unloadBaseFromGPUMemory();
  std::string find_str = ", ";
#pragma omp atomic write
  cancel_search = false;
  std::vector<std::string> result_str;
  result_str.reserve(books_in_base);
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
                    result_str.emplace_back(auth);
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

  struct gpu_string
  {
    char *data = nullptr;
    size_t size = 0;
    std::string *str = nullptr;
  };

  std::vector<std::string *> result_ptr;
  std::vector<gpu_string> for_gpu;
  size_t cpu_limit = result_str.size() * af->getCpuGpuBalanceAuthors();
  result_ptr.reserve(cpu_limit);
  for_gpu.reserve(result_str.size() - cpu_limit);
  int def_device = omp_get_default_device();
  int init_device = omp_get_initial_device();
  for(auto it = result_str.begin(); it != result_str.end(); it++)
    {
      if(result_ptr.size() < cpu_limit || it->empty())
        {
          result_ptr.push_back(it.base());
        }
      else
        {
          gpu_string gp;
          gp.size = it->size();
          gp.data = reinterpret_cast<char *>(
              omp_target_alloc(gp.size, def_device));
          int er = omp_target_memcpy(gp.data, it->data(), gp.size, 0, 0,
                                     def_device, init_device);
          if(er != 0)
            {
              throw MLException(
                  std::string("BaseKeeper::collectionAuthorsGPU: ")
                  + std::strerror(er));
            }
          gp.str = it.base();
          for_gpu.emplace_back(gp);
        }
    }

  double sz_gpu = 0.0;
  size_t for_gpu_sz = for_gpu.size();
  for(size_t i = 0; i < for_gpu.size(); i++)
    {
      sz_gpu += static_cast<double>(for_gpu_sz - (i + 1));
    }

  std::atomic<double> progr_gpu = 0;

  std::thread gpu_thr(
      [this, &for_gpu, def_device, init_device, &progr_gpu, for_gpu_sz] {
        size_t *for_gpu_sz_gpu = reinterpret_cast<size_t *>(
            omp_target_alloc(sizeof(size_t), def_device));
        int er = omp_target_memcpy(for_gpu_sz_gpu, &for_gpu_sz, sizeof(size_t),
                                   0, 0, def_device, init_device);
        if(er != 0)
          {
            throw MLException(std::string("BaseKeeper::collectionAuthorsGPU: ")
                              + std::strerror(er));
          }
        gpu_string *for_gpu_gpu = reinterpret_cast<gpu_string *>(
            omp_target_alloc(for_gpu_sz * sizeof(gpu_string), def_device));

        er = omp_target_memcpy(for_gpu_gpu, for_gpu.data(),
                               for_gpu_sz * sizeof(gpu_string), 0, 0,
                               def_device, init_device);
        if(er != 0)
          {
            throw MLException(std::string("BaseKeeper::collectionAuthorsGPU: ")
                              + std::strerror(er));
          }

        bool cncl = false;
        double l_sz = -static_cast<double>(for_gpu_sz);
        for(size_t i = 0; i < for_gpu_sz; i++)
          {
#pragma omp atomic read
            cncl = cancel_search;
            if(cncl)
              {
                break;
              }
            l_sz += static_cast<double>(for_gpu_sz - i);
            progr_gpu.store(l_sz, std::memory_order_relaxed);
#pragma omp target map(to : i, for_gpu_gpu)
            {
              gpu_string *l_ptr = for_gpu_gpu + i;
              if(l_ptr->str)
                {
#pragma omp parallel for
                  for(gpu_string *j = l_ptr + 1;
                      j < for_gpu_gpu + *for_gpu_sz_gpu; j++)
                    {
                      if(j->str)
                        {
                          if(j->size == l_ptr->size)
                            {
                              bool equal = true;
                              for(size_t e = 0; e < j->size; e++)
                                {
                                  if(l_ptr->data[e] != j->data[e])
                                    {
                                      equal = false;
                                      break;
                                    }
                                }
                              if(equal)
                                {
                                  j->str = nullptr;
                                }
                            }
                        }
                    }
                }
            }
          }

        er = omp_target_memcpy(for_gpu.data(), for_gpu_gpu,
                               for_gpu_sz * sizeof(gpu_string), 0, 0,
                               init_device, def_device);
        if(er != 0)
          {
            throw MLException(std::string("BaseKeeper::collectionAuthorsGPU: ")
                              + std::strerror(er));
          }
        omp_target_free(for_gpu_gpu, def_device);
        omp_target_free(for_gpu_sz_gpu, def_device);
      });

  result.reserve(result_str.size());
  size_t result_ptr_sz = result_ptr.size();
  double sz_cpu = 0.0;
  for(size_t i = 0; i < result_ptr_sz; i++)
    {
      sz_cpu += static_cast<double>(result_ptr_sz - (i + 1));
    }
  std::atomic<double> progr_cpu = 0.0;

  bool stop_progr = false;
  std::mutex stop_progr_mtx;
  std::condition_variable stop_progr_var;

  std::thread progr_thr([this, sz_gpu, sz_cpu, &progr_gpu, &progr_cpu,
                         &stop_progr, &stop_progr_mtx, &stop_progr_var] {
    for(;;)
      {
        std::unique_lock<std::mutex> ullock(stop_progr_mtx);
        stop_progr_var.wait_for(ullock, std::chrono::milliseconds(300),
                                [&stop_progr] {
                                  return stop_progr;
                                });
        if(stop_progr)
          {
            break;
          }
        else
          {
            if(auth_gpu_show_progr)
              {
                auth_gpu_show_progr(progr_gpu.load(std::memory_order_relaxed),
                                    sz_gpu);
              }
            if(auth_cpu_show_progr)
              {
                auth_cpu_show_progr(static_cast<double>(progr_cpu.load(
                                        std::memory_order_relaxed)),
                                    sz_cpu);
              }
          }
      }
  });

  double l_sz = -static_cast<double>(result_ptr_sz);
  for(size_t i = 0; i < result_ptr_sz; i++)
    {
      bool cncl = false;
#pragma atomic read
      cncl = cancel_search;
      if(cncl)
        {
          break;
        }
      l_sz += static_cast<double>(result_ptr_sz - i);
      progr_cpu.store(l_sz, std::memory_order_relaxed);
      if(result_ptr[i])
        {
#pragma omp parallel
#pragma omp for
          for(size_t j = i + 1; j < result_ptr_sz; j++)
            {
              if(result_ptr[j])
                {
                  if(*result_ptr[j] == *result_ptr[i])
                    {
                      result_ptr[j] = nullptr;
                    }
                }
            }
          result.emplace_back(*result_ptr[i]);
        }
    }

  gpu_thr.join();

  stop_progr_mtx.lock();
  stop_progr = true;
  stop_progr_var.notify_all();
  stop_progr_mtx.unlock();

  progr_thr.join();

  size_t cpu_res_size = result.size();

  if(auth_collecting_results)
    {
      auth_collecting_results();
    }

  bool cncl;
#pragma omp atomic read
  cncl = cancel_search;
  if(cncl)
    {
#pragma omp parallel
#pragma omp for
      for(auto it = for_gpu.begin(); it != for_gpu.end(); it++)
        {
          omp_target_free(it->data, def_device);
        }
    }
  else
    {
#pragma omp parallel
#pragma omp for
      for(auto it = for_gpu.begin(); it != for_gpu.end(); it++)
        {
          if(it->str)
            {
              auto it_res = AuxFunc::parallelFind(
                  result.begin(), result.begin() + cpu_res_size, *(it->str));
              if(it_res == result.begin() + cpu_res_size)
                {
#pragma omp critical
                  {
                    result.emplace_back(*(it->str));
                  }
                }
            }
          omp_target_free(it->data, def_device);
        }
    }

#pragma omp atomic read
  cncl = cancel_search;
  if(cncl)
    {
      result.clear();
    }
  result.shrink_to_fit();

  loadBaseToGPUMemory();

  return result;
}

void
BaseKeeper::loadBaseToGPUMemory()
{
  if(books_in_base > 0)
    {
      size_t cpu_size = books_in_base * af->getCpuGpuBalanceSearch();
      cpu_base.reserve(cpu_size);
      size_t gpu_size = books_in_base - cpu_size;
      std::vector<gpu_base_entry> gp;
      gp.reserve(gpu_size);

      int def_device = omp_get_default_device();
      int init_device = omp_get_initial_device();
      int er;
      for(auto it_base = base.begin(); it_base != base.end(); it_base++)
        {
          for(auto it_books = it_base->books.begin();
              it_books != it_base->books.end(); it_books++)
            {
              if(gp.size() < gpu_size)
                {
                  gpu_base_entry gp_e;
                  gp_e.base_entry = it_base.base();
                  gp_e.book_entry = it_books.base();

                  std::string l_val = af->stringToLower(it_books->book_author);
                  for(auto it = l_val.begin(); it != l_val.end();)
                    {
                      if(*it >= 0 && *it <= ' ')
                        {
                          l_val.erase(it);
                        }
                      else
                        {
                          break;
                        }
                    }
                  gp_e.book_author_sz = l_val.size();
                  if(gp_e.book_author_sz > 0)
                    {
                      gp_e.book_author = reinterpret_cast<char *>(
                          omp_target_alloc(gp_e.book_author_sz, def_device));
                      er = omp_target_memcpy(gp_e.book_author, l_val.data(),
                                             gp_e.book_author_sz, 0, 0,
                                             def_device, init_device);
                      if(er)
                        {
                          throw MLException(
                              std::string("BaseKeeper::loadBaseToGPUMemory: ")
                              + std::strerror(er));
                        }
                    }

                  l_val = af->stringToLower(it_books->book_name);
                  for(auto it = l_val.begin(); it != l_val.end();)
                    {
                      if(*it >= 0 && *it <= ' ')
                        {
                          l_val.erase(it);
                        }
                      else
                        {
                          break;
                        }
                    }
                  gp_e.book_name_sz = l_val.size();
                  if(gp_e.book_name_sz > 0)
                    {
                      gp_e.book_name = reinterpret_cast<char *>(
                          omp_target_alloc(gp_e.book_name_sz, def_device));
                      er = omp_target_memcpy(gp_e.book_name, l_val.data(),
                                             gp_e.book_name_sz, 0, 0,
                                             def_device, init_device);
                      if(er)
                        {
                          throw MLException(
                              std::string("BaseKeeper::loadBaseToGPUMemory: ")
                              + std::strerror(er));
                        }
                    }

                  l_val = af->stringToLower(it_books->book_series);
                  for(auto it = l_val.begin(); it != l_val.end();)
                    {
                      if(*it >= 0 && *it <= ' ')
                        {
                          l_val.erase(it);
                        }
                      else
                        {
                          break;
                        }
                    }
                  gp_e.book_series_sz = l_val.size();
                  if(gp_e.book_series_sz > 0)
                    {
                      gp_e.book_series = reinterpret_cast<char *>(
                          omp_target_alloc(gp_e.book_series_sz, def_device));
                      er = omp_target_memcpy(gp_e.book_series, l_val.data(),
                                             gp_e.book_series_sz, 0, 0,
                                             def_device, init_device);
                      if(er)
                        {
                          throw MLException(
                              std::string("BaseKeeper::loadBaseToGPUMemory: ")
                              + std::strerror(er));
                        }
                    }

                  l_val = af->stringToLower(it_books->book_genre);
                  for(auto it = l_val.begin(); it != l_val.end();)
                    {
                      if(*it >= 0 && *it <= ' ')
                        {
                          l_val.erase(it);
                        }
                      else
                        {
                          break;
                        }
                    }
                  gp_e.book_genre_sz = l_val.size();
                  if(gp_e.book_genre_sz > 0)
                    {
                      gp_e.book_genre = reinterpret_cast<char *>(
                          omp_target_alloc(gp_e.book_genre_sz, def_device));
                      er = omp_target_memcpy(gp_e.book_genre, l_val.data(),
                                             gp_e.book_genre_sz, 0, 0,
                                             def_device, init_device);
                      if(er)
                        {
                          throw MLException(
                              std::string("BaseKeeper::loadBaseToGPUMemory: ")
                              + std::strerror(er));
                        }
                    }

                  l_val = af->stringToLower(it_books->book_date);
                  for(auto it = l_val.begin(); it != l_val.end();)
                    {
                      if(*it >= 0 && *it <= ' ')
                        {
                          l_val.erase(it);
                        }
                      else
                        {
                          break;
                        }
                    }
                  gp_e.book_date_sz = l_val.size();
                  if(gp_e.book_date_sz > 0)
                    {
                      gp_e.book_date = reinterpret_cast<char *>(
                          omp_target_alloc(gp_e.book_date_sz, def_device));
                      er = omp_target_memcpy(gp_e.book_date, l_val.data(),
                                             gp_e.book_date_sz, 0, 0,
                                             def_device, init_device);
                      if(er)
                        {
                          throw MLException(
                              std::string("BaseKeeper::loadBaseToGPUMemory: ")
                              + std::strerror(er));
                        }
                    }

                  gp.emplace_back(gp_e);
                }
              else
                {
                  cpu_base_entry cp_e;
                  cp_e.base_entry = it_base.base();
                  cp_e.book_entry = it_books.base();
                  cpu_base.emplace_back(cp_e);
                }
            }
        }

      gpu_size = gp.size();
      if(gpu_size > 0)
        {
          gpu_base = reinterpret_cast<gpu_base_entry *>(
              omp_target_alloc(gpu_size * sizeof(gpu_base_entry), def_device));
          er = omp_target_memcpy(gpu_base, gp.data(),
                                 gpu_size * sizeof(gpu_base_entry), 0, 0,
                                 def_device, init_device);
          if(er)
            {
              omp_target_free(gpu_base, def_device);
              gpu_base = nullptr;
              gpu_base_sz = 0;
              throw MLException(
                  std::string("BaseKeeper::loadBaseToGPUMemory: ")
                  + std::strerror(er));
            }
          gpu_base_sz = gpu_size;
        }
    }
  cpu_base.shrink_to_fit();
}

void
BaseKeeper::unloadBaseFromGPUMemory()
{
  if(gpu_base)
    {
      std::vector<gpu_base_entry> gp;
      gp.resize(gpu_base_sz);
      int er = omp_target_memcpy(
          gp.data(), gpu_base, gpu_base_sz * sizeof(gpu_base_entry), 0, 0,
          omp_get_initial_device(), omp_get_default_device());
      if(er != 0)
        {
          std::cout << "BaseKeeper::unloadBaseFromGPUMemory: "
                    << std::strerror(er);
          gp.clear();
        }
      omp_target_free(gpu_base, omp_get_default_device());
      gpu_base = nullptr;
      gpu_base_sz = 0;

      for(auto it = gp.begin(); it != gp.end(); it++)
        {
          if(it->book_author)
            {
              omp_target_free(it->book_author, omp_get_default_device());
            }
          if(it->book_name)
            {
              omp_target_free(it->book_name, omp_get_default_device());
            }
          if(it->book_series)
            {
              omp_target_free(it->book_series, omp_get_default_device());
            }
          if(it->book_genre)
            {
              omp_target_free(it->book_genre, omp_get_default_device());
            }
          if(it->book_date)
            {
              omp_target_free(it->book_date, omp_get_default_device());
            }
        }
    }
  cpu_base.clear();
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
              if(*it >= 0 && *it <= ' ')
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
#pragma omp parallel for
                      for(auto itb = it->books.begin(); itb != it->books.end();
                          itb++)
                        {
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
                  if(*it >= 0 && *it <= ' ')
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
#pragma omp parallel for
                      for(auto itb = it->books.begin(); itb != it->books.end();
                          itb++)
                        {
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
