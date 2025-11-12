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
#include <ByteOrder.h>
#include <LibArchive.h>
#include <RefreshCollection.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>

#ifndef USE_OPENMP
#include <pthread.h>
#include <thread>
#ifdef _WIN32
#include <errhandlingapi.h>
#include <winbase.h>
#endif
#endif

RefreshCollection::RefreshCollection(
    const std::shared_ptr<AuxFunc> &af, const std::string &collection_name,
    const int &num_threads, const bool &remove_empty, const bool &fast_refresh,
    const bool &refresh_bookmarks, const std::shared_ptr<BookMarks> &bookmarks)
    : CreateCollection(af, num_threads)
{
  this->af = af;
  if(num_threads > 0)
    {
      this->num_threads = num_threads;
    }
  base_path = getBasePath(collection_name);
  this->collection_name = collection_name;
  books_path = getBooksPath();
  this->remove_empty = remove_empty;
  this->fast_refresh = fast_refresh;
  this->refresh_bookmarks = refresh_bookmarks;
  this->bookmarks = bookmarks;
  rar_support = true;

#ifdef USE_OPENMP
  omp_init_lock(&basemtx);
  omp_init_lock(&already_hashedmtx);
  omp_init_lock(&need_to_parsemtx);
#else
  if(this->num_threads > static_cast<int>(std::thread::hardware_concurrency()))
    {
      unsigned lim
          = static_cast<unsigned>(std::thread::hardware_concurrency());
      thr_pool.reserve(lim);
      for(unsigned i = 0; i < lim; i++)
        {
          thr_pool.push_back(std::make_tuple(i, true));
        }
    }
  else
    {
      unsigned lim = static_cast<unsigned>(this->num_threads);
      thr_pool.reserve(lim);
      for(unsigned i = 0; i < lim; i++)
        {
          thr_pool.push_back(std::make_tuple(i, true));
        }
    }
#endif
}

RefreshCollection::~RefreshCollection()
{
#ifdef USE_OPENMP
  omp_destroy_lock(&basemtx);
  omp_destroy_lock(&already_hashedmtx);
  omp_destroy_lock(&need_to_parsemtx);
#endif
}

std::filesystem::path
RefreshCollection::getBasePath(const std::string &collection_name)
{
  std::filesystem::path result;

  result = af->homePath();
  result /= std::filesystem::u8path(".local")
            / std::filesystem::u8path("share")
            / std::filesystem::u8path("MyLibrary")
            / std::filesystem::u8path("Collections");
  result /= std::filesystem::u8path(collection_name);
  result /= std::filesystem::u8path("base");

  return result;
}

std::filesystem::path
RefreshCollection::getBooksPath()
{
  std::filesystem::path result;

  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.seekg(0, std::ios_base::end);
      size_t fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);
      uint16_t sz;
      if(fsz >= sizeof(sz))
        {
          f.read(reinterpret_cast<char *>(&sz), sizeof(sz));
        }
      else
        {
          f.close();
          throw std::runtime_error(
              "RefreshCollection::get_books_path: incorrect base file");
        }
      ByteOrder bo;
      bo.set_little(sz);
      sz = bo;

      if(fsz >= sizeof(sz) + static_cast<size_t>(sz))
        {
          std::string buf;
          buf.resize(sz);
          f.read(buf.data(), buf.size());
          result = std::filesystem::u8path(buf);
        }
      else
        {
          f.close();
          throw std::runtime_error(
              "RefreshCollection::get_books_path: incorrect base file(2)");
        }

      f.close();
    }

  return result;
}

void
RefreshCollection::refreshCollection()
{
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();
  if(std::filesystem::exists(books_path))
    {
      std::vector<std::filesystem::path> books_files;
      std::vector<std::filesystem::path> empty_paths;
      for(auto &dirit : std::filesystem::recursive_directory_iterator(
              books_path,
              std::filesystem::directory_options::follow_directory_symlink))
        {
          if(pulse)
            {
              pulse();
            }
          std::filesystem::path p = dirit.path();
          if(remove_empty)
            {
              if(std::filesystem::is_empty(p))
                {
                  empty_paths.emplace_back(p);
                }
              else
                {
                  if(!std::filesystem::is_directory(p))
                    {
                      std::filesystem::path check_type;
                      if(!(std::filesystem::symlink_status(p).type()
                           == std::filesystem::file_type::symlink))
                        {
                          check_type = p;
                        }
                      else
                        {
                          check_type = std::filesystem::read_symlink(p);
                        }
                      if(af->if_supported_type(check_type))
                        {
                          books_files.emplace_back(p);
                        }
                    }
                }
            }
          else
            {
              if(!std::filesystem::is_directory(p))
                {
                  std::filesystem::path check_type;
                  if(!(std::filesystem::symlink_status(p).type()
                       == std::filesystem::file_type::symlink))
                    {
                      check_type = p;
                    }
                  else
                    {
                      check_type = std::filesystem::read_symlink(p);
                    }
                  if(af->if_supported_type(check_type))
                    {
                      books_files.emplace_back(p);
                    }
                }
            }
        }
      for(auto it = empty_paths.begin(); it != empty_paths.end(); it++)
        {
          std::filesystem::remove_all(*it);
        }

      if(!rar_support)
        {
          std::string sstr = ".rar";
          books_files.erase(
              std::remove_if(books_files.begin(), books_files.end(),
                             [sstr, this](std::filesystem::path &el)
                               {
                                 std::string ext = el.extension().u8string();
                                 ext = af->stringToLower(ext);
                                 return ext == sstr;
                               }),
              books_files.end());
        }

      compareVectors(base, books_files);
      if(!fast_refresh)
        {
          checkHashes(&base, &books_files);
        }

      std::filesystem::remove_all(base_path);
      openBaseFile();
      for(auto it = base.begin(); it != base.end(); it++)
        {
          write_file_to_base(*it);
        }
      closeBaseFile();

      if(signal_total_bytes)
        {
          double tmp = 0.0;
          for(auto it = need_to_parse.begin(); it != need_to_parse.end(); it++)
            {
              std::error_code ec;
              tmp += static_cast<double>(std::filesystem::file_size(*it, ec));
              if(ec)
                {
                  std::cout
                      << "CreateCollection::createCollection: " << ec.message()
                      << std::endl;
                }
            }
          signal_total_bytes(tmp);
        }
      threadRegulator();

      if(refresh_bookmarks)
        {
          refreshBookMarks(bk);
        }
    }
  else
    {
      throw std::runtime_error(
          "RefreshCollection::refreshCollection: books not found");
    }
}

void
RefreshCollection::compareVectors(
    std::vector<FileParseEntry> &base,
    std::vector<std::filesystem::path> &books_files)
{
  for(auto it = base.begin(); it != base.end();)
    {
      if(pulse)
        {
          pulse();
        }
      auto itbf = std::find_if(books_files.begin(), books_files.end(),
                               std::bind(&RefreshCollection::compareFunction1,
                                         this, std::placeholders::_1, *it));
      if(itbf == books_files.end())
        {
          base.erase(it);
        }
      else
        {
          it++;
        }
    }

  std::unique_ptr<LibArchive> la(new LibArchive(af));

#ifdef USE_OPENMP
  int num_threads_default = omp_get_max_threads();
  omp_set_num_threads(num_threads);
  omp_set_max_active_levels(omp_get_supported_active_levels());
  omp_set_dynamic(true);
  omp_lock_t base_mtx;
  omp_init_lock(&base_mtx);
#pragma omp parallel
#pragma omp for
  for(auto it = books_files.begin(); it != books_files.end(); it++)
    {
      bool cncl;
#pragma omp atomic read
      cncl = cancel;
      if(cncl)
        {
#pragma omp cancel for
          continue;
        }
      if(pulse)
        {
          pulse();
        }
      omp_set_lock(&base_mtx);
      auto itbase
          = std::find_if(base.begin(), base.end(),
                         std::bind(&RefreshCollection::compareFunction2, this,
                                   std::placeholders::_1, *it));
      if(itbase == base.end())
        {
          omp_unset_lock(&base_mtx);
          omp_set_lock(&need_to_parsemtx);
          need_to_parse.push_back(*it);
          omp_unset_lock(&need_to_parsemtx);
        }
      else
        {
          FileParseEntry fpe = *itbase;
          omp_unset_lock(&base_mtx);
          if(af->ifSupportedArchiveUnpackaingType(*it))
            {
              std::vector<std::string> filenames;
              getFilenamesFromArchives(*it, "", filenames, la.get());

              // Base file existence check.
              bool to_check = true;
              for(auto it_fnm = filenames.begin(); it_fnm != filenames.end();
                  it_fnm++)
                {
                  auto it_bpe
                      = std::find_if(fpe.books.begin(), fpe.books.end(),
                                     [it_fnm](const BookParseEntry &el)
                                       {
                                         return el.book_path == *it_fnm;
                                       });
                  if(it_bpe == fpe.books.end())
                    {
                      omp_set_lock(&need_to_parsemtx);
                      need_to_parse.push_back(*it);
                      omp_unset_lock(&need_to_parsemtx);
                      omp_set_lock(&base_mtx);
                      base.erase(std::remove_if(base.begin(), base.end(),
                                                [fpe](const FileParseEntry &el)
                                                  {
                                                    return fpe.file_rel_path
                                                           == el.file_rel_path;
                                                  }),
                                 base.end());
                      omp_unset_lock(&base_mtx);
                      to_check = false;
                      break;
                    }                  
                }

              // Archive file existence check
              if(to_check)
                {
                  for(auto it_fpe = fpe.books.begin();
                      it_fpe != fpe.books.end(); it_fpe++)
                    {
                      auto it_fnm
                          = std::find(filenames.begin(), filenames.end(),
                                      it_fpe->book_path);
                      if(it_fnm == filenames.end())
                        {
                          omp_set_lock(&need_to_parsemtx);
                          need_to_parse.push_back(*it);
                          omp_unset_lock(&need_to_parsemtx);
                          omp_set_lock(&base_mtx);
                          base.erase(
                              std::remove_if(base.begin(), base.end(),
                                             [fpe](const FileParseEntry &el)
                                               {
                                                 return fpe.file_rel_path
                                                        == el.file_rel_path;
                                               }),
                              base.end());
                          omp_unset_lock(&base_mtx);
                          break;
                        }
                    }
                }
            }
        }
    }
  omp_destroy_lock(&base_mtx);
  omp_set_num_threads(num_threads_default);
#else
  std::mutex base_mtx;

  for(auto it = books_files.begin(); it != books_files.end(); it++)
    {
      if(cancel.load(std::memory_order_relaxed))
        {
          break;
        }
      std::unique_lock<std::mutex> run_thr_lock(newthrmtx);
      std::vector<std::tuple<unsigned, bool>>::iterator free;
      continue_hashing.wait(run_thr_lock,
                            [this, &free]
                              {
                                bool result = false;
                                for(auto it = thr_pool.begin();
                                    it != thr_pool.end(); it++)
                                  {
                                    if(std::get<1>(*it))
                                      {
                                        free = it;
                                        result = true;
                                        break;
                                      }
                                  }
                                return result;
                              });
      std::get<1>(*free) = false;
      std::thread thr(
          [this, &base_mtx, &base, &la, it, free]
            {
              if(pulse)
                {
                  pulse();
                }
              base_mtx.lock();
              auto itbase = std::find_if(
                  base.begin(), base.end(),
                  std::bind(&RefreshCollection::compareFunction2, this,
                            std::placeholders::_1, *it));
              if(itbase == base.end())
                {
                  base_mtx.unlock();
                  need_to_parsemtx.lock();
                  need_to_parse.push_back(*it);
                  need_to_parsemtx.unlock();
                }
              else
                {
                  FileParseEntry fpe = *itbase;
                  base_mtx.unlock();
                  if(af->ifSupportedArchiveUnpackaingType(*it))
                    {
                      std::vector<std::string> filenames;
                      getFilenamesFromArchives(*it, "", filenames, la.get());

                      // Base file existence check.
                      bool to_check = true;
                      for(auto it_fnm = filenames.begin();
                          it_fnm != filenames.end(); it_fnm++)
                        {
                          auto it_bpe = std::find_if(
                              fpe.books.begin(), fpe.books.end(),
                              [it_fnm](const BookParseEntry &el)
                                {
                                  return el.book_path == *it_fnm;
                                });
                          if(it_bpe == fpe.books.end())
                            {
                              need_to_parsemtx.lock();
                              need_to_parse.push_back(*it);
                              need_to_parsemtx.unlock();
                              base_mtx.lock();
                              base.erase(std::remove_if(
                                             base.begin(), base.end(),
                                             [fpe](const FileParseEntry &el)
                                               {
                                                 return fpe.file_rel_path
                                                        == el.file_rel_path;
                                               }),
                                         base.end());
                              base_mtx.unlock();
                              to_check = false;
                              break;
                            }
                        }

                      // Archive file existence check
                      if(to_check)
                        {
                          for(auto it_fpe = fpe.books.begin();
                              it_fpe != fpe.books.end(); it_fpe++)
                            {
                              auto it_fnm = std::find(filenames.begin(),
                                                      filenames.end(),
                                                      it_fpe->book_path);
                              if(it_fnm == filenames.end())
                                {
                                  need_to_parsemtx.lock();
                                  need_to_parse.push_back(*it);
                                  need_to_parsemtx.unlock();
                                  base_mtx.lock();
                                  base.erase(
                                      std::remove_if(
                                          base.begin(), base.end(),
                                          [fpe](const FileParseEntry &el)
                                            {
                                              return fpe.file_rel_path
                                                     == el.file_rel_path;
                                            }),
                                      base.end());
                                  base_mtx.unlock();
                                  break;
                                }
                            }
                        }
                    }
                }
              std::lock_guard<std::mutex> lglock(newthrmtx);
              std::get<1>(*free) = true;
              continue_hashing.notify_all();
            });
#ifdef __linux
      cpu_set_t cpu_set;
      CPU_ZERO(&cpu_set);
      CPU_SET(std::get<0>(*free), &cpu_set);
      int er = pthread_setaffinity_np(thr.native_handle(), sizeof(cpu_set_t),
                                      &cpu_set);
      if(er != 0)
        {
          std::cout << "RefreshCollection::refreshCollection "
                       "pthread_setaffinity_np: "
                    << std::strerror(er) << std::endl;
        }
#elif defined(_WIN32)
      DWORD_PTR mask = 1;
      mask = mask << std::get<0>(*free);
      HANDLE handle = pthread_gethandle(thr.native_handle());
      if(handle)
        {
          if(SetThreadAffinityMask(handle, mask) == 0)
            {
              std::cout << "RefreshCollection::refreshCollection "
                           "SetThreadAffinityMask: "
                        << std::strerror(GetLastError()) << std::endl;
            }
        }
      else
        {
          std::cout << "RefreshCollection::refreshCollection: handle is null!"
                    << std::endl;
        }
#endif
      thr.detach();
    }
  std::unique_lock<std::mutex> ullock(newthrmtx);
  continue_hashing.wait(ullock,
                        [this]
                          {
                            bool result = true;
                            for(auto it = thr_pool.begin();
                                it != thr_pool.end(); it++)
                              {
                                if(!std::get<1>(*it))
                                  {
                                    result = false;
                                    break;
                                  }
                              }
                            return result;
                          });
#endif
}

bool
RefreshCollection::compareFunction1(const std::filesystem::path &book_path,
                                    const FileParseEntry &ent)
{
  std::filesystem::path cur = std::filesystem::u8path(ent.file_rel_path);
  std::filesystem::path comp = book_path.lexically_proximate(books_path);
  if(cur == comp)
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool
RefreshCollection::compareFunction2(const FileParseEntry &ent,
                                    const std::filesystem::path &book_path)
{
  return compareFunction1(book_path, ent);
}

void
RefreshCollection::refreshFile(const BookBaseEntry &bbe)
{
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();
  for(auto it = base.begin(); it != base.end();)
    {
      if(compareFunction1(bbe.file_path, *it))
        {
          base.erase(it);
          break;
        }
      else
        {
          it++;
        }
    }

  std::filesystem::remove_all(base_path);

  openBaseFile();
  for(auto it = base.begin(); it != base.end(); it++)
    {
      write_file_to_base(*it);
    }
  closeBaseFile();
  if(std::filesystem::exists(bbe.file_path))
    {
      need_to_parse.push_back(bbe.file_path);
      threadRegulator();
    }
  if(refresh_bookmarks)
    {
      refreshBookMarks(bk);
    }
}

void
RefreshCollection::checkHashes(std::vector<FileParseEntry> *base,
                               std::vector<std::filesystem::path> *books_files)
{
  uintmax_t summ = 0;
  if(total_bytes_to_hash)
    {
      for(auto it = books_files->begin(); it != books_files->end(); it++)
        {
          if(pulse)
            {
              pulse();
            }
          summ += std::filesystem::file_size(*it);
        }
      total_bytes_to_hash(static_cast<double>(summ));
    }

#ifndef USE_OPENMP
  bytes_summ.store(0);
  for(auto it = thr_pool.begin(); it != thr_pool.end(); it++)
    {
      std::get<1>(*it) = true;
    }
  for(auto it = books_files->begin(); it != books_files->end(); it++)
    {
      if(cancel.load(std::memory_order_relaxed))
        {
          break;
        }
      std::unique_lock<std::mutex> ullock(newthrmtx);
      std::vector<std::tuple<unsigned, bool>>::iterator free;
      continue_hashing.wait(ullock,
                            [this, &free]
                              {
                                bool result = false;
                                for(auto it = thr_pool.begin();
                                    it != thr_pool.end(); it++)
                                  {
                                    if(std::get<1>(*it))
                                      {
                                        result = true;
                                        free = it;
                                        break;
                                      }
                                  }
                                return result;
                              });
      std::get<1>(*free) = false;
      std::thread thr(
          [this, free, it, base]
            {
              hashThread(*it, base);
              std::lock_guard<std::mutex> lglock(newthrmtx);
              std::get<1>(*free) = true;
              continue_hashing.notify_all();
            });
#ifdef __linux
      cpu_set_t cpu_set;
      CPU_ZERO(&cpu_set);
      CPU_SET(std::get<0>(*free), &cpu_set);
      int er = pthread_setaffinity_np(thr.native_handle(), sizeof(cpu_set_t),
                                      &cpu_set);
      if(er != 0)
        {
          std::cout
              << "RefreshCollection::checkHashes pthread_setaffinity_np: "
              << std::strerror(er) << std::endl;
        }
#elif defined(_WIN32)
      DWORD_PTR mask = 1;
      mask = mask << std::get<0>(*free);
      HANDLE handle = pthread_gethandle(thr.native_handle());
      if(handle)
        {
          if(SetThreadAffinityMask(handle, mask) == 0)
            {
              std::cout << "RefreshCollection::checkHashes "
                           "SetThreadAffinityMask: "
                        << std::strerror(GetLastError()) << std::endl;
            }
        }
      else
        {
          std::cout << "RefreshCollection::checkHashes: handle is null!"
                    << std::endl;
        }
#endif
      thr.detach();
    }

  std::unique_lock<std::mutex> ullock(newthrmtx);
  continue_hashing.wait(ullock,
                        [this]
                          {
                            bool result = true;
                            for(auto it = thr_pool.begin();
                                it != thr_pool.end(); it++)
                              {
                                if(!std::get<1>(*it))
                                  {
                                    result = false;
                                    break;
                                  }
                              }
                            return result;
                          });
#else
  int num_threads_default = omp_get_max_threads();
  omp_set_num_threads(num_threads);
#pragma omp parallel
#pragma omp for
  for(auto it = books_files->begin(); it != books_files->end(); it++)
    {
      bool cncl;
#pragma omp atomic read
      cncl = cancel;
      if(cncl)
        {
#pragma omp cancel for
          continue;
        }
      hashThread(*it, base);
    }
  omp_set_num_threads(num_threads_default);
#endif
}

void
RefreshCollection::hashThread(const std::filesystem::path &file_to_hash,
                              std::vector<FileParseEntry> *base)
{
  std::string hash;
#ifndef USE_OPENMP
  try
    {
      hash = file_hashing(file_to_hash);
    }
  catch(std::exception &er)
    {
      std::cout << "RefreshCollection::hashThread: \"" << er.what() << "\""
                << std::endl;
    }

  if(!hash.empty() && !cancel.load())
    {
      already_hashedmtx.lock();
      already_hashed.emplace_back(std::make_tuple(file_to_hash, hash));
      basemtx.lock();
      auto itbase
          = std::find_if(base->begin(), base->end(),
                         std::bind(&RefreshCollection::compareFunction2, this,
                                   std::placeholders::_1, file_to_hash));
      if(itbase != base->end())
        {
          if(hash != itbase->file_hash)
            {
              need_to_parsemtx.lock();
              auto itntp = std::find(need_to_parse.begin(),
                                     need_to_parse.end(), file_to_hash);
              if(itntp == need_to_parse.end())
                {
                  need_to_parse.push_back(file_to_hash);
                }
              need_to_parsemtx.unlock();
              base->erase(itbase);
            }
        }
      else
        {
          need_to_parsemtx.lock();
          auto itntp = std::find(need_to_parse.begin(), need_to_parse.end(),
                                 file_to_hash);
          if(itntp == need_to_parse.end())
            {
              need_to_parse.push_back(file_to_hash);
            }
          need_to_parsemtx.unlock();
        }
      basemtx.unlock();
      already_hashedmtx.unlock();
      bytes_summ.store(bytes_summ.load()
                       + std::filesystem::file_size(file_to_hash));
      if(bytes_hashed)
        {
          bytes_hashed(static_cast<double>(bytes_summ.load()));
        }
    }
#else
  try
    {
      hash = file_hashing(file_to_hash);
    }
  catch(std::exception &er)
    {
      std::cout << "RefreshCollection::hashThread: \"" << er.what() << "\""
                << std::endl;
    }

  bool cncl = false;
#pragma omp atomic read
  cncl = cancel;
  if(!hash.empty() && !cncl)
    {
      omp_set_lock(&already_hashedmtx);
      already_hashed.emplace_back(std::make_tuple(file_to_hash, hash));
      omp_set_lock(&basemtx);
      auto itbase
          = std::find_if(base->begin(), base->end(),
                         std::bind(&RefreshCollection::compareFunction2, this,
                                   std::placeholders::_1, file_to_hash));
      if(itbase != base->end())
        {
          if(hash != itbase->file_hash)
            {
              omp_set_lock(&need_to_parsemtx);
              auto itntp = std::find(need_to_parse.begin(),
                                     need_to_parse.end(), file_to_hash);
              if(itntp == need_to_parse.end())
                {
                  need_to_parse.push_back(file_to_hash);
                }
              omp_unset_lock(&need_to_parsemtx);
              base->erase(itbase);
            }
        }
      else
        {
          omp_set_lock(&need_to_parsemtx);
          auto itntp = std::find(need_to_parse.begin(), need_to_parse.end(),
                                 file_to_hash);
          if(itntp == need_to_parse.end())
            {
              need_to_parse.push_back(file_to_hash);
            }
          omp_unset_lock(&need_to_parsemtx);
        }
      omp_unset_lock(&basemtx);
      omp_unset_lock(&already_hashedmtx);

      uintmax_t b_summ;
#pragma omp atomic capture
      {
        bytes_summ += std::filesystem::file_size(file_to_hash);
        b_summ = bytes_summ;
      }
      if(bytes_hashed)
        {
          bytes_hashed(static_cast<double>(b_summ));
        }
    }
#endif
}

bool
RefreshCollection::editBook(const BookBaseEntry &bbe_old,
                            const BookBaseEntry &bbe_new)
{
  bool result = false;
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();

  auto itbase
      = std::find_if(base.begin(), base.end(),
                     std::bind(&RefreshCollection::compareFunction2, this,
                               std::placeholders::_1, bbe_old.file_path));
  if(itbase != base.end())
    {
      for(auto it = itbase->books.begin(); it != itbase->books.end(); it++)
        {
          std::filesystem::path p = books_path;
          p /= std::filesystem::u8path(itbase->file_rel_path);
          BookBaseEntry bbe(*it, p);
          if(bbe == bbe_old)
            {
              *it = bbe_new.bpe;
              result = true;
            }
        }
    }

  if(result)
    {
      std::filesystem::remove_all(base_path);
      openBaseFile();
      for(auto it = base.begin(); it != base.end(); it++)
        {
          write_file_to_base(*it);
        }
      closeBaseFile();
      if(refresh_bookmarks)
        {
          std::vector<std::tuple<std::string, BookBaseEntry>> bmv
              = bookmarks->getBookMarks();
          std::tuple<std::string, BookBaseEntry> s_tup
              = std::make_tuple(collection_name, bbe_old);
          auto itbmv = std::find_if(
              bmv.begin(), bmv.end(),
              [s_tup](std::tuple<std::string, BookBaseEntry> &el)
                {
                  if(std::get<0>(el) == std::get<0>(s_tup)
                     && std::get<1>(el) == std::get<1>(s_tup))
                    {
                      return true;
                    }
                  else
                    {
                      return false;
                    }
                });
          if(itbmv != bmv.end())
            {
              bookmarks->removeBookMark(std::get<0>(*itbmv),
                                        std::get<1>(*itbmv));
              bookmarks->createBookMark(collection_name, bbe_new);
            }
        }
    }

  return result;
}

bool
RefreshCollection::refreshBook(const BookBaseEntry &bbe)
{
  bool result = false;

  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();

  auto itbase
      = std::find_if(base.begin(), base.end(),
                     std::bind(&RefreshCollection::compareFunction2, this,
                               std::placeholders::_1, bbe.file_path));
  if(itbase != base.end())
    {
      auto itbpe = std::find_if(itbase->books.begin(), itbase->books.end(),
                                [bbe](BookParseEntry &el)
                                  {
                                    return el == bbe.bpe;
                                  });
      if(itbpe != itbase->books.end())
        {
          *itbpe = bbe.bpe;
          std::filesystem::remove_all(base_path);
          openBaseFile();
          for(auto it = base.begin(); it != base.end(); it++)
            {
              write_file_to_base(*it);
            }
          closeBaseFile();
          result = true;
        }
    }

  return result;
}

void
RefreshCollection::set_rar_support(const bool &rar_support)
{
  this->rar_support = rar_support;
}

void
RefreshCollection::refreshBookMarks(const std::shared_ptr<BaseKeeper> &bk)
{
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();

  std::vector<std::tuple<std::string, BookBaseEntry>> bmv
      = bookmarks->getBookMarks();
  for(auto it = bmv.begin(); it != bmv.end(); it++)
    {
      auto itbase = std::find_if(
          base.begin(), base.end(),
          std::bind(&RefreshCollection::compareFunction2, this,
                    std::placeholders::_1, std::get<1>(*it).file_path));
      if(itbase != base.end())
        {
          auto itbpe = std::find_if(itbase->books.begin(), itbase->books.end(),
                                    [it](BookParseEntry &el)
                                      {
                                        return el == std::get<1>(*it).bpe;
                                      });
          if(itbpe == itbase->books.end())
            {
              bookmarks->removeBookMark(collection_name, std::get<1>(*it));
            }
        }
      else
        {
          bookmarks->removeBookMark(collection_name, std::get<1>(*it));
        }
    }
}

void
RefreshCollection::getFilenamesFromArchives(
    const std::filesystem::path &arch_path, const std::string &prefix,
    std::vector<std::string> &result, void *la)
{
  std::string ext = arch_path.extension().u8string();
  std::vector<ArchEntry> files;
  LibArchive *l_la = reinterpret_cast<LibArchive *>(la);
  bool zip = false;
  if(ext == ".zip" || ext == ".ZIP")
    {
      l_la->fileNames(arch_path, files);
      zip = true;
    }
  else
    {
      l_la->fileNamesStream(arch_path, files);
    }

  std::vector<std::string> arch_types
      = af->get_supported_archive_types_unpacking();
  std::vector<std::string> types = af->get_supported_types();
  for(auto it = files.begin(); it != files.end(); it++)
    {
      ext = af->getExtension(it->filename);
      if(ext.size() > 0)
        {
          ext.erase(ext.begin());
        }

      auto it_t = std::find(arch_types.begin(), arch_types.end(), ext);

      if(it_t != arch_types.end())
        {
          std::string l_prefix;
          if(prefix.empty())
            {
              l_prefix = it->filename;
            }
          else
            {
              l_prefix = prefix + "\n" + it->filename;
            }
          std::filesystem::path p
              = arch_path.parent_path()
                / std::filesystem::u8path(af->randomFileName());
          std::filesystem::path unpacked;
          if(zip)
            {
              unpacked = l_la->unpackByPosition(arch_path, p, *it);
            }
          else
            {
              unpacked
                  = l_la->unpackByFileNameStream(arch_path, p, it->filename);
            }
          getFilenamesFromArchives(unpacked, l_prefix, result, la);
          std::filesystem::remove_all(p);
        }
      else
        {
          it_t = std::find(types.begin(), types.end(), ext);
          if(it_t != types.end())
            {
              if(prefix.empty())
                {
                  result.push_back(it->filename);
                }
              else
                {
                  result.push_back(prefix + "\n" + it->filename);
                }
            }
        }
    }
}
