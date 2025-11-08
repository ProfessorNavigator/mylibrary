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
#include <pthread.h>
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
#else
  std::lock_guard<std::mutex> lglock(basemtx);
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
          throw std::runtime_error(
              "BaseKeeper::loadCollection: collection path size "
              "reading error");
        }

      ByteOrder bo;
      bo.set_little(val16);
      val16 = bo;
      if(static_cast<size_t>(val16) > base_str.size() - rb)
        {
          collection_path.clear();
          collection_name.clear();
          throw std::runtime_error(
              "BaseKeeper::loadCollection: collection path reading error");
        }
      std::string ent(base_str.begin() + rb, base_str.begin() + rb + val16);
      rb += ent.size();
      collection_path = std::filesystem::u8path(ent);
      if(!std::filesystem::exists(collection_path))
        {
          collection_path.clear();
          collection_name.clear();
          throw std::runtime_error(
              "BaseKeeper::loadCollection: collection path error");
        }

      while(rb < base_str.size())
        {
          try
            {
              base.emplace_back(readFileEntry(base_str, rb));
            }
          catch(std::exception &er)
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
}

FileParseEntry
BaseKeeper::readFileEntry(const std::string &base_str, size_t &rb)
{
  uint64_t val64;
  size_t sz = sizeof(val64);
  if(base_str.size() + rb < sz)
    {
      throw std::runtime_error(
          "BaseKeeper::readFileEntry: incorrect base size");
    }
  std::memcpy(&val64, &base_str[rb], sz);
  rb += sz;

  ByteOrder bo;
  bo.set_little(val64);
  val64 = bo;

  if(val64 == 0 || base_str.size() - rb < static_cast<size_t>(val64))
    {
      throw std::runtime_error(
          "BaseKeeper::readFileEntry: incorrect entry size");
    }

  std::string entry(base_str.begin() + rb, base_str.begin() + rb + val64);
  rb += entry.size();

  uint16_t val16;
  sz = sizeof(val16);
  if(entry.size() < sz)
    {
      throw std::runtime_error(
          "BaseKeeper::readFileEntry: incorrect file address entry size");
    }
  std::memcpy(&val16, &entry[0], sz);
  size_t lrb = sz;
  bo.set_little(val16);
  val16 = bo;

  if(val16 == 0 || entry.size() - lrb < static_cast<size_t>(val16))
    {
      throw std::runtime_error(
          "BaseKeeper::readFileEntry: incorrect file address size");
    }

  FileParseEntry fpe;

  std::string e(entry.begin() + lrb, entry.begin() + lrb + val16);
  lrb += e.size();
  fpe.file_rel_path = e;

  sz = sizeof(val16);
  if(entry.size() - lrb < sz)
    {
      throw std::runtime_error(
          "BaseKeeper::readFileEntry: incorrect file hash entry size");
    }
  std::memcpy(&val16, &entry[lrb], sz);
  lrb += sz;
  bo.set_little(val16);
  val16 = bo;

  if(val16 == 0 || entry.size() - lrb < static_cast<size_t>(val16))
    {
      throw std::runtime_error(
          "BaseKeeper::readFileEntry: incorrect file hash size");
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
  size_t sz = sizeof(val64);
#ifdef USE_OPENMP
#pragma omp parallel masked
  {
    while(rb < entry.size())
      {
        std::string book_e;
        if(entry.size() - rb < sz)
          {
            throw std::runtime_error(
                "BaseKeeper::readBookEntry: incorrect book entry size");
          }
        std::memcpy(&val64, &entry[rb], sz);
        rb += sz;

        bo.set_little(val64);
        val64 = bo;

        if(val64 == 0 || entry.size() - rb < static_cast<size_t>(val64))
          {
            throw std::runtime_error(
                "BaseKeeper::readBookEntry: incorrect book entry size(2)");
          }
        book_e = std::string(entry.begin() + rb, entry.begin() + rb + val64);
        rb += book_e.size();
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          BookParseEntry bpe;
          size_t lrb = 0;
          parseBookEntry(book_e, bpe.book_path, lrb);
          parseBookEntry(book_e, bpe.book_author, lrb);
          parseBookEntry(book_e, bpe.book_name, lrb);
          parseBookEntry(book_e, bpe.book_series, lrb);
          parseBookEntry(book_e, bpe.book_genre, lrb);
          parseBookEntry(book_e, bpe.book_date, lrb);
#pragma omp critical
          {
            result.emplace_back(bpe);
          }
          omp_fulfill_event(event);
        }
      }
  }
#else
  while(rb < entry.size())
    {
      std::string book_e;
      if(entry.size() - rb < sz)
        {
          throw std::runtime_error(
              "BaseKeeper::readBookEntry: incorrect book entry size");
        }
      std::memcpy(&val64, &entry[rb], sz);
      rb += sz;

      bo.set_little(val64);
      val64 = bo;

      if(val64 == 0 || entry.size() - rb < static_cast<size_t>(val64))
        {
          throw std::runtime_error(
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
#endif
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
      throw std::runtime_error(
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
  double l_cc;
  if(coef_coincedence > 1.0 || coef_coincedence < 0.0)
    {
      l_cc = 0.99;
    }
  else
    {
      l_cc = coef_coincedence;
    }
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lock_base(basemtx);
  cancel_search.store(false, std::memory_order_relaxed);
#else
  omp_set_max_active_levels(omp_get_supported_active_levels());
  omp_set_dynamic(true);
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
          if(cancel_search.load(std::memory_order_relaxed))
            {
              break;
            }
#endif
          switch(i)
            {
            case 1:
              {
                if(!searchSurname(search, result, l_cc))
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
                if(!searchFirstName(search, result, l_cc))
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
                if(!searchLastName(search, result, l_cc))
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
                    searchBook(search, result, l_cc);

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
                    searchSeries(search, result, l_cc);

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
                    searchGenre(search, result, l_cc);
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
  if(all_empty && !cancel_search.load(std::memory_order_relaxed))
    {
      for(auto it = base.begin(); it != base.end(); it++)
        {
          if(cancel_search.load(std::memory_order_relaxed))
            {
              break;
            }
          std::filesystem::path book_file_path = collection_path;
          book_file_path /= std::filesystem::u8path(it->file_rel_path);
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  break;
                }
              result.emplace_back(BookBaseEntry(*itb, book_file_path));
            }
        }
    }
  if(cancel_search.load(std::memory_order_relaxed))
    {
      result.clear();
    }
#else
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
#pragma omp parallel for
          for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
            {
              BookBaseEntry bbe(*itb, book_file_path);
#pragma omp critical
              {
                result.emplace_back(bbe);
              }
            }
        }
    }

  if(c_s)
    {
      result.clear();
    }
#endif

  if(coef_coincedence > 1.0)
    {
      BookBaseEntry l_search = search;
      for(auto it = l_search.bpe.book_author.begin();
          it != l_search.bpe.book_author.end(); it++)
        {
          if(*it == 7)
            {
              *it = ' ';
            }
        }

      normalizeString(l_search.bpe.book_author, Normalization::Author);
      normalizeString(l_search.bpe.book_date, Normalization::Other);
      normalizeString(l_search.bpe.book_genre, Normalization::Other);
      normalizeString(l_search.bpe.book_name, Normalization::Other);
      normalizeString(l_search.bpe.book_series, Normalization::Other);

#ifdef USE_OPENMP
      result.erase(AuxFunc::parallelRemoveIf(
                       result.begin(), result.end(),
                       [l_search, this](const BookBaseEntry &el)
                         {
                           return !exactMatchSearchFunc(el, l_search);
                         }),
                   result.end());
#else
#ifdef USE_PE
      result.erase(
          std::remove_if(std::execution::par, result.begin(), result.end(),
                         [l_search, this](const BookBaseEntry &el)
                           {
                             return !exactMatchSearchFunc(el, l_search);
                           }),
          result.end());
#else
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [l_search, this](const BookBaseEntry &el)
                                    {
                                      return !exactMatchSearchFunc(el,
                                                                   l_search);
                                    }),
                   result.end());
#endif
#endif
    }

#ifdef USE_OPENMP
#pragma omp parallel for
  for(auto it = result.begin(); it != result.end(); it++)
    {
      normalizeString(it->bpe.book_author, Normalization::Other);
      normalizeString(it->bpe.book_date, Normalization::Other);
      normalizeString(it->bpe.book_genre, Normalization::Other);
      normalizeString(it->bpe.book_name, Normalization::Other);
      normalizeString(it->bpe.book_series, Normalization::Other);
    }
#else
#ifdef USE_PE
  std::for_each(std::execution::par, result.begin(), result.end(),
                [this](BookBaseEntry &el)
                  {
                    normalizeString(el.bpe.book_author, Normalization::Other);
                    normalizeString(el.bpe.book_date, Normalization::Other);
                    normalizeString(el.bpe.book_genre, Normalization::Other);
                    normalizeString(el.bpe.book_name, Normalization::Other);
                    normalizeString(el.bpe.book_series, Normalization::Other);
                  });
#else
  for(auto it = result.begin(); it != result.end(); it++)
    {
      normalizeString(it->bpe.book_author, Normalization::Other);
      normalizeString(it->bpe.book_date, Normalization::Other);
      normalizeString(it->bpe.book_genre, Normalization::Other);
      normalizeString(it->bpe.book_name, Normalization::Other);
      normalizeString(it->bpe.book_series, Normalization::Other);
    }
#endif
#endif

  result.shrink_to_fit();

  return result;
}

std::vector<std::string>
BaseKeeper::collectionAuthors()
{
  std::vector<std::string> result;
  std::vector<std::string> lower_result;
  std::string find_str = ", ";
#ifdef USE_OPENMP
#pragma omp atomic write
  cancel_search = false;
  omp_set_lock(&basemtx);
  result.reserve(books_in_base);
  lower_result.reserve(books_in_base);
  double sz = static_cast<double>(books_in_base);
  double progr = 0.0;
  bool c_s = false;
  for(auto it = base.begin(); it != base.end(); it++)
    {
#pragma omp atomic read
      c_s = cancel_search;
      if(c_s)
        {
          break;
        }
      for(auto it_b = it->books.begin(); it_b != it->books.end(); it_b++)
        {
          if(!it_b->book_author.empty())
            {
              std::string::size_type n_beg = 0;
              std::string::size_type n_end;
              bool stop = false;
              for(;;)
                {
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
                  normalizeString(auth, Normalization::Author);
                  std::string lower = af->stringToLower(auth);
                  if(!lower.empty())
                    {
                      std::string *end = lower_result.data() - 1;
                      std::string *it_str = end;
#pragma omp parallel
#pragma omp for
                      for(std::string *i
                          = lower_result.data() + lower_result.size() - 1;
                          i > end; i--)
                        {
                          if(*i == lower)
                            {
#pragma omp critical
                              {
                                if(i > it_str)
                                  {
                                    it_str = i;
                                  }
                              }
#pragma omp cancel for
                            }
                        }
                      if(it_str == end)
                        {
                          lower_result.emplace_back(lower);
                          result.emplace_back(auth);
                        }
                    }
                  n_beg = n_end + find_str.size() - 1;
                  if(stop)
                    {
                      break;
                    }
                }
            }
          if(auth_show_progr)
            {
              progr += 1.0;
              auth_show_progr(progr, sz);
            }
        }
    }
  omp_unset_lock(&basemtx);
  if(c_s)
    {
      result.clear();
    }
#else
  cancel_search.store(false, std::memory_order_relaxed);
  basemtx.lock();
  result.reserve(books_in_base);
  lower_result.reserve(books_in_base);
  double sz = static_cast<double>(books_in_base);
  double progr = 0.0;
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(cancel_search.load(std::memory_order_relaxed))
        {
          break;
        }
      for(auto it_b = it->books.begin(); it_b != it->books.end(); it_b++)
        {
          if(!it_b->book_author.empty())
            {
              std::string::size_type n_beg = 0;
              std::string::size_type n_end;
              bool stop = false;
              for(;;)
                {
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
                  normalizeString(auth, Normalization::Author);
                  std::string lower = af->stringToLower(auth);
                  if(!lower.empty())
                    {
#ifdef USE_PE
                      auto it = std::find(std::execution::par,
                                          lower_result.rbegin(),
                                          lower_result.rend(), lower);
#else
                      auto it = std::find(lower_result.rbegin(),
                                          lower_result.rend(), lower);
#endif
                      if(it == lower_result.rend())
                        {
                          result.emplace_back(auth);
                          lower_result.emplace_back(lower);
                        }
                    }
                  n_beg = n_end + find_str.size() - 1;
                  if(stop)
                    {
                      break;
                    }
                }
            }
          if(auth_show_progr)
            {
              progr += 1.0;
              auth_show_progr(progr, sz);
            }
        }
    }
  basemtx.unlock();
  if(cancel_search.load(std::memory_order_relaxed))
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
  cancel_search.store(false, std::memory_order_relaxed);
  std::mutex result_mtx;
  basemtx.lock();
#ifdef USE_PE
  std::for_each(
      std::execution::par, base.begin(), base.end(),
      [&result, &result_mtx, notes, this](FileParseEntry &ent)
        {
          if(cancel_search.load(std::memory_order_relaxed))
            {
              return void();
            }
          std::filesystem::path f_p
              = collection_path / std::filesystem::u8path(ent.file_rel_path);
          auto it = std::find_if(notes.begin(), notes.end(),
                                 [f_p](const NotesBaseEntry &el_n)
                                   {
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
                                       [bp](BookParseEntry &el)
                                         {
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
      [&result, &result_mtx, notes, this](FileParseEntry &ent)
        {
          if(cancel_search.load(std::memory_order_relaxed))
            {
              return void();
            }
          std::filesystem::path f_p
              = collection_path / std::filesystem::u8path(ent.file_rel_path);
          auto it = std::find_if(notes.begin(), notes.end(),
                                 [f_p](const NotesBaseEntry &el_n)
                                   {
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
                                       [bp](BookParseEntry &el)
                                         {
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
#pragma omp atomic write
  cancel_search = false;
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
                             [f_p](const NotesBaseEntry &el_n)
                               {
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
                             [bp](BookParseEntry &el)
                               {
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
  if(cancel_search.load(std::memory_order_relaxed))
    {
      result.clear();
    }
#endif

  return result;
}

bool
BaseKeeper::searchLineFunc(const std::string &to_search,
                           const std::string &source,
                           const double &coef_coincidence,
                           const Normalization &variant)
{
  std::string loc_source = af->stringToLower(source);
  normalizeString(loc_source, variant);  

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
  std::string::size_type n = surname.find("\7");
  if(n != std::string::npos)
    {
      surname = surname.substr(0, n);
      normalizeString(surname, Normalization::Author);
      surname = af->stringToLower(surname);

      if(!surname.empty())
        {
          all_empty = false;
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
                                    coef_coincidence, Normalization::Author))
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
               coef_coincidence](FileParseEntry &el)
                {
                  if(cancel_search.load(std::memory_order_relaxed))
                    {
                      return void();
                    }
                  std::filesystem::path book_file_path
                      = collection_path
                        / std::filesystem::u8path(el.file_rel_path);
                  std::for_each(
                      std::execution::par, el.books.begin(), el.books.end(),
                      [this, surname, &result, &result_mtx, book_file_path,
                       coef_coincidence](BookParseEntry &el)
                        {
                          if(cancel_search.load(std::memory_order_relaxed))
                            {
                              return void();
                            }
                          if(searchLineFunc(surname, el.book_author,
                                            coef_coincidence,
                                            Normalization::Author))
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
              [&result_mtx, &result, surname, coef_coincidence,
               this](FileParseEntry &el)
                {
                  if(cancel_search.load(std::memory_order_relaxed))
                    {
                      return void();
                    }
                  std::filesystem::path book_file_path
                      = collection_path
                        / std::filesystem::u8path(el.file_rel_path);
                  std::for_each(
                      el.books.begin(), el.books.end(),
                      [this, surname, &result, &result_mtx, book_file_path,
                       coef_coincidence](BookParseEntry &el)
                        {
                          if(cancel_search.load(std::memory_order_relaxed))
                            {
                              return void();
                            }
                          if(searchLineFunc(surname, el.book_author,
                                            coef_coincidence,
                                            Normalization::Author))
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
  normalizeString(l_search, Normalization::Other);
  if(l_search.empty())
    {
      return void();
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
           coef_coincidence](FileParseEntry &el)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  return void();
                }
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(el.file_rel_path);

              std::for_each(
                  std::execution::par, el.books.begin(), el.books.end(),
                  [this, l_search, book_file_path, &result, &result_mtx,
                   coef_coincidence](BookParseEntry &el)
                    {
                      if(cancel_search.load(std::memory_order_relaxed))
                        {
                          return void();
                        }
                      if(searchLineFunc(l_search, el.book_name,
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
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  return void();
                }
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(el.file_rel_path);

              std::for_each(
                  el.books.begin(), el.books.end(),
                  [this, l_search, book_file_path, &result, &result_mtx,
                   coef_coincidence](BookParseEntry &el)
                    {
                      if(cancel_search.load(std::memory_order_relaxed))
                        {
                          return void();
                        }
                      if(searchLineFunc(l_search, el.book_name,
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
                       [l_search, this, coef_coincidence](BookBaseEntry &el)
                         {
                           return !searchLineFunc(l_search, el.bpe.book_name,
                                                  coef_coincidence);
                         }),
                   result.end());
#else
#ifdef USE_PE
      result.erase(
          std::remove_if(std::execution::par, result.begin(), result.end(),
                         [search, this, coef_coincidence](BookBaseEntry &el)
                           {
                             if(searchLineFunc(search.bpe.book_name,
                                               el.bpe.book_name,
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
                         [search, coef_coincidence, this](BookBaseEntry &el)
                           {
                             if(searchLineFunc(search.bpe.book_name,
                                               el.bpe.book_name,
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
BaseKeeper::searchSeries(const BookBaseEntry &search,
                         std::vector<BookBaseEntry> &result,
                         const double &coef_coincidence)
{
  std::string l_search = af->stringToLower(search.bpe.book_series);
  normalizeString(l_search, Normalization::Other);
  if(l_search.empty())
    {
      return void();
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
           coef_coincidence](FileParseEntry &el)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  return void();
                }
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(el.file_rel_path);

              std::for_each(
                  std::execution::par, el.books.begin(), el.books.end(),
                  [this, l_search, &result, &result_mtx, book_file_path,
                   coef_coincidence](BookParseEntry &el)
                    {
                      if(cancel_search.load(std::memory_order_relaxed))
                        {
                          return void();
                        }
                      if(searchLineFunc(l_search, el.book_series,
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
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  return void();
                }
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(el.file_rel_path);

              std::for_each(
                  el.books.begin(), el.books.end(),
                  [this, l_search, &result, &result_mtx, book_file_path,
                   coef_coincidence](BookParseEntry &el)
                    {
                      if(cancel_search.load(std::memory_order_relaxed))
                        {
                          return void();
                        }
                      if(searchLineFunc(l_search, el.book_series,
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
                       [l_search, this, coef_coincidence](BookBaseEntry &el)
                         {
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
      result.erase(
          std::remove_if(std::execution::par, result.begin(), result.end(),
                         [l_search, this, coef_coincidence](BookBaseEntry &el)
                           {
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
      result.erase(
          std::remove_if(result.begin(), result.end(),
                         [l_search, coef_coincidence, this](BookBaseEntry &el)
                           {
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
          throw std::runtime_error(
              "BaseKeeper::get_books_path: base file error");
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
          throw std::runtime_error(
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
  normalizeString(l_search, Normalization::Other);
  if(l_search.empty())
    {
      return void();
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
           coef_coincidence](FileParseEntry &el)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  return void();
                }
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(el.file_rel_path);
              std::for_each(
                  std::execution::par, el.books.begin(), el.books.end(),
                  [this, l_search, &result, &result_mtx, book_file_path,
                   coef_coincidence](BookParseEntry &el)
                    {
                      if(cancel_search.load(std::memory_order_relaxed))
                        {
                          return void();
                        }
                      if(searchLineFunc(l_search, el.book_genre,
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
          [this, l_search, &result, &result_mtx,
           coef_coincidence](FileParseEntry &el)
            {
              if(cancel_search.load(std::memory_order_relaxed))
                {
                  return void();
                }
              std::filesystem::path book_file_path
                  = collection_path
                    / std::filesystem::u8path(el.file_rel_path);
              std::for_each(
                  el.books.begin(), el.books.end(),
                  [this, l_search, &result, &result_mtx, book_file_path,
                   coef_coincidence](BookParseEntry &el)
                    {
                      if(cancel_search.load(std::memory_order_relaxed))
                        {
                          return void();
                        }
                      if(searchLineFunc(l_search, el.book_genre,
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
                       [l_search, this, coef_coincidence](BookBaseEntry &el)
                         {
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
      result.erase(
          std::remove_if(std::execution::par, result.begin(), result.end(),
                         [l_search, this, coef_coincidence](BookBaseEntry &el)
                           {
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
      result.erase(
          std::remove_if(result.begin(), result.end(),
                         [l_search, coef_coincidence, this](BookBaseEntry &el)
                           {
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

bool
BaseKeeper::exactMatchSearchFunc(const BookBaseEntry &el,
                                 const BookBaseEntry &search)
{
  bool result = false;
  if(!search.bpe.book_author.empty())
    {
      std::string search_str = el.bpe.book_author;
      normalizeString(search_str, Normalization::Author);

      if(search_str == search.bpe.book_author)
        {
          result = true;
        }
      else
        {
          std::string::size_type n1 = 0;
          std::string::size_type n2;
          std::string find_str = ", ";
          for(;;)
            {
              n2 = search_str.find(find_str, n1);
              if(n2 == std::string::npos)
                {
                  std::string str(search_str.begin() + n1, search_str.end());
                  if(str == search.bpe.book_author)
                    {
                      result = true;
                    }
                  break;
                }
              else
                {
                  std::string str(search_str.begin() + n1,
                                  search_str.begin() + n2);
                  if(str == search.bpe.book_author)
                    {
                      result = true;
                      break;
                    }
                }
              n1 = n2 + find_str.size();
            }
        }

      if(!result)
        {
          return result;
        }
    }

  if(!search.bpe.book_name.empty())
    {
      std::string search_str = el.bpe.book_name;
      normalizeString(search_str, Normalization::Other);
      result = false;
      if(search_str == search.bpe.book_name)
        {
          result = true;
        }
      else
        {
          std::string::size_type n1 = 0;
          std::string::size_type n2;
          std::string find_str = ", ";
          for(;;)
            {
              n2 = search_str.find(find_str, n1);
              if(n2 == std::string::npos)
                {
                  break;
                }
              else
                {
                  std::string str(search_str.begin() + n1,
                                  search_str.begin() + n2);
                  if(str == search.bpe.book_name)
                    {
                      result = true;
                      break;
                    }
                }
              n1 = n2 + find_str.size();
            }
        }
      if(!result)
        {
          return result;
        }
    }

  if(!search.bpe.book_series.empty())
    {
      std::string search_str = el.bpe.book_series;
      normalizeString(search_str, Normalization::Other);
      result = false;
      if(search_str == search.bpe.book_series)
        {
          result = true;
        }
      else
        {
          std::string::size_type n1 = 0;
          std::string::size_type n2;
          std::string find_str = ", ";
          for(;;)
            {
              n2 = search_str.find(find_str, n1);
              if(n2 == std::string::npos)
                {
                  break;
                }
              else
                {
                  std::string str(search_str.begin() + n1,
                                  search_str.begin() + n2);
                  if(str == search.bpe.book_series)
                    {
                      result = true;
                      break;
                    }
                }
              n1 = n2 + find_str.size();
            }
        }
      if(!result)
        {
          return result;
        }
    }

  if(!search.bpe.book_genre.empty())
    {
      std::string search_str = el.bpe.book_genre;
      normalizeString(search_str, Normalization::Other);
      result = false;
      if(search_str == search.bpe.book_genre)
        {
          result = true;
        }
      else
        {
          std::string::size_type n1 = 0;
          std::string::size_type n2;
          std::string find_str = ", ";
          for(;;)
            {
              n2 = search_str.find(find_str, n1);
              if(n2 == std::string::npos)
                {
                  break;
                }
              else
                {
                  std::string str(search_str.begin() + n1,
                                  search_str.begin() + n2);
                  if(str == search.bpe.book_genre)
                    {
                      result = true;
                      break;
                    }
                }
              n1 = n2 + find_str.size();
            }
        }
      if(!result)
        {
          return result;
        }
    }

  if(!search.bpe.book_date.empty())
    {
      std::string search_str = el.bpe.book_date;
      normalizeString(search_str, Normalization::Other);
      result = false;
      if(search_str == search.bpe.book_date)
        {
          result = true;
        }
      else
        {
          std::string::size_type n1 = 0;
          std::string::size_type n2;
          std::string find_str = ", ";
          for(;;)
            {
              n2 = search_str.find(find_str, n1);
              if(n2 == std::string::npos)
                {
                  break;
                }
              else
                {
                  std::string str(search_str.begin() + n1,
                                  search_str.begin() + n2);
                  if(str == search.bpe.book_date)
                    {
                      result = true;
                      break;
                    }
                }
              n1 = n2 + find_str.size();
            }
        }
      if(!result)
        {
          return result;
        }
    }

  return result;
}

void
BaseKeeper::normalizeString(std::string &str, const Normalization &variant)
{
  for(auto it = str.begin(); it != str.end();)
    {
      char el = *it;
      if(el >= 0 && el <= 32)
        {
          str.erase(it);
        }
      else
        {
          break;
        }
    }

  while(str.size() > 0)
    {
      char el = *str.rbegin();
      if(el >= 0 && el <= 32)
        {
          str.pop_back();
        }
      else
        {
          break;
        }
    }

  std::string find_str = "  ";
  std::string::size_type n = 0;
  for(;;)
    {
      n = str.find(find_str, n);
      if(n == std::string::npos)
        {
          break;
        }
      str.erase(str.begin() + n);
    }

  if(variant == Normalization::Author)
    {
      find_str = ". ";
      n = 0;
      for(;;)
        {
          n = str.find(find_str, n);
          if(n == std::string::npos)
            {
              break;
            }
          str.erase(str.begin() + n + 1);
        }
    }
  str.shrink_to_fit();
}

void
BaseKeeper::stopSearch()
{
#ifdef USE_OPENMP
#pragma atomic write
  cancel_search = true;
#else
  cancel_search.store(true, std::memory_order_relaxed);
#endif
}

bool
BaseKeeper::searchLastName(const BookBaseEntry &search,
                           std::vector<BookBaseEntry> &result,
                           const double &coef_coincidence)
{
  bool all_empty = true;
  std::string last_name = search.bpe.book_author;
  std::string sstr = "\7";
  std::string::size_type n = last_name.find(sstr);
  if(n != std::string::npos)
    {
      last_name.erase(0, n + sstr.size());
      n = last_name.find(sstr);
      if(n != std::string::npos)
        {
          last_name.erase(0, n + sstr.size());
          normalizeString(last_name, Normalization::Author);
          last_name = af->stringToLower(last_name);          
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
                                            coef_coincidence,
                                            Normalization::Author))
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
                       coef_coincidence](FileParseEntry &el)
                        {
                          if(cancel_search.load(std::memory_order_relaxed))
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
                               coef_coincidence](BookParseEntry &el)
                                {
                                  if(cancel_search.load(
                                         std::memory_order_relaxed))
                                    {
                                      return void();
                                    }
                                  if(searchLineFunc(last_name, el.book_author,
                                                    coef_coincidence,
                                                    Normalization::Author))
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
                       coef_coincidence](FileParseEntry &el)
                        {
                          if(cancel_search.load(std::memory_order_relaxed))
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
                               coef_coincidence](BookParseEntry &el)
                                {
                                  if(cancel_search.load(
                                         std::memory_order_relaxed))
                                    {
                                      return void();
                                    }
                                  if(searchLineFunc(last_name, el.book_author,
                                                    coef_coincidence,
                                                    Normalization::Author))
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
                          [last_name, this,
                           coef_coincidence](BookBaseEntry &el)
                            {
                              if(searchLineFunc(last_name, el.bpe.book_author,
                                                coef_coincidence,
                                                Normalization::Author))
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
                           coef_coincidence](BookBaseEntry &el)
                            {
                              if(searchLineFunc(last_name, el.bpe.book_author,
                                                coef_coincidence,
                                                Normalization::Author))
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
                                      this](BookBaseEntry &el)
                                       {
                                         if(searchLineFunc(
                                                last_name, el.bpe.book_author,
                                                coef_coincidence,
                                                Normalization::Author))
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
  std::string sstr = "\7";
  std::string::size_type n = first_name.find(sstr);
  if(n != std::string::npos)
    {
      first_name.erase(0, n + sstr.size());
      n = first_name.find(sstr);
      if(n != std::string::npos)
        {
          first_name = first_name.substr(0, n);
          normalizeString(first_name, Normalization::Author);
          first_name = af->stringToLower(first_name);          
          if(!first_name.empty())
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
                          if(searchLineFunc(first_name, itb->book_author,
                                            coef_coincidence,
                                            Normalization::Author))
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
                       coef_coincidence](FileParseEntry &el)
                        {
                          if(cancel_search.load(std::memory_order_relaxed))
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
                               coef_coincidence](BookParseEntry &el)
                                {
                                  if(cancel_search.load(
                                         std::memory_order_relaxed))
                                    {
                                      return void();
                                    }
                                  if(searchLineFunc(first_name, el.book_author,
                                                    coef_coincidence,
                                                    Normalization::Author))
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
                       coef_coincidence](FileParseEntry &el)
                        {
                          if(cancel_search.load(std::memory_order_relaxed))
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
                               coef_coincidence](BookParseEntry &el)
                                {
                                  if(cancel_search.load(
                                         std::memory_order_relaxed))
                                    {
                                      return void();
                                    }
                                  if(searchLineFunc(first_name, el.book_author,
                                                    coef_coincidence,
                                                    Normalization::Author))
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
                          [first_name, this,
                           coef_coincidence](BookBaseEntry &el)
                            {
                              if(searchLineFunc(first_name, el.bpe.book_author,
                                                coef_coincidence,
                                                Normalization::Author))
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
                           coef_coincidence](BookBaseEntry &el)
                            {
                              if(searchLineFunc(first_name, el.bpe.book_author,
                                                coef_coincidence,
                                                Normalization::Author))
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
                                      this](BookBaseEntry &el)
                                       {
                                         if(searchLineFunc(
                                                first_name, el.bpe.book_author,
                                                coef_coincidence,
                                                Normalization::Author))
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
