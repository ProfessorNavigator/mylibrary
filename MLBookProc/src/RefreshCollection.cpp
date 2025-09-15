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
#include <MLException.h>
#include <RefreshCollection.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <tuple>

#ifndef USE_OPENMP
#include <thread>
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
  base_path = get_base_path(collection_name);
  this->collection_name = collection_name;
  books_path = get_books_path();
  this->remove_empty = remove_empty;
  this->fast_refresh = fast_refresh;
  this->refresh_bookmarks = refresh_bookmarks;
  this->bookmarks = bookmarks;
  rar_support = true;

#ifdef USE_OPENMP
  omp_init_lock(&basemtx);
  omp_init_lock(&already_hashedmtx);
  omp_init_lock(&need_to_parsemtx);
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
RefreshCollection::get_base_path(const std::string &collection_name)
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
RefreshCollection::get_books_path()
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
          throw MLException(
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
          throw MLException(
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
                             [sstr, this](std::filesystem::path &el) {
                               std::string ext = el.extension().u8string();
                               ext = af->stringToLower(ext);
                               return ext == sstr;
                             }),
              books_files.end());
        }

      compaire_vectors(base, books_files);
      if(!fast_refresh)
        {
          check_hashes(&base, &books_files);
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
      throw MLException(
          "RefreshCollection::refreshCollection: books not found");
    }
}

void
RefreshCollection::compaire_vectors(
    std::vector<FileParseEntry> &base,
    std::vector<std::filesystem::path> &books_files)
{
  for(auto it = base.begin(); it != base.end();)
    {
      auto itbf = std::find_if(books_files.begin(), books_files.end(),
                               std::bind(&RefreshCollection::compare_function1,
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

  for(auto it = books_files.begin(); it != books_files.end(); it++)
    {
      auto itbase
          = std::find_if(base.begin(), base.end(),
                         std::bind(&RefreshCollection::compare_function2, this,
                                   std::placeholders::_1, *it));
      if(itbase == base.end())
        {
          need_to_parse.push_back(*it);
        }
    }
}

bool
RefreshCollection::compare_function1(const std::filesystem::path &book_path,
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
RefreshCollection::compare_function2(const FileParseEntry &ent,
                                     const std::filesystem::path &book_path)
{
  return compare_function1(book_path, ent);
}

void
RefreshCollection::refreshFile(const BookBaseEntry &bbe)
{
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();
  for(auto it = base.begin(); it != base.end();)
    {
      if(compare_function1(bbe.file_path, *it))
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
RefreshCollection::check_hashes(
    std::vector<FileParseEntry> *base,
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

  run_threads = 0;
  for(auto it = books_files->begin(); it != books_files->end(); it++)
    {
      if(cancel.load())
        {
          break;
        }
      std::unique_lock<std::mutex> lk(newthrmtx);
      run_threads++;
      std::thread thr(
          std::bind(&RefreshCollection::hash_thread, this, *it, base));
      thr.detach();
      continue_hashing.wait(lk, [this] {
        if(num_threads > 1)
          {
            return run_threads < num_threads - 1;
          }
        else
          {
            return run_threads < 1;
          }
      });
    }

  std::unique_lock<std::mutex> lk(newthrmtx);
  continue_hashing.wait(lk, [this] {
    return run_threads <= 0;
  });
#else
  int num_threads_default = omp_get_num_threads();
  omp_set_num_threads(num_threads);
#pragma omp parallel
  {
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
        hash_thread(*it, base);
      }
  }
  omp_set_num_threads(num_threads_default);
#endif
}

void
RefreshCollection::hash_thread(const std::filesystem::path &file_to_hash,
                               std::vector<FileParseEntry> *base)
{
  std::string hash;
#ifndef USE_OPENMP
  try
    {
      std::unique_lock<std::mutex> ullock(newthrmtx);
      continue_hashing.wait(ullock, [this] {
        if(num_threads > 1)
          {
            return run_threads < num_threads;
          }
        else
          {
            return run_threads < 2;
          }
      });
      run_threads++;
      ullock.unlock();
      std::shared_ptr<int> thr_finish(&run_threads, [this](int *) {
        std::lock_guard<std::mutex> lglock(newthrmtx);
        run_threads--;
        continue_hashing.notify_one();
      });
      hash = file_hashing(file_to_hash);
    }
  catch(MLException &er)
    {
      std::cout << er.what() << std::endl;
      std::lock_guard<std::mutex> lk(newthrmtx);
      run_threads--;
      continue_hashing.notify_one();
    }

  already_hashedmtx.lock();
  already_hashed.emplace_back(std::make_tuple(file_to_hash, hash));
  basemtx.lock();
  auto itbase
      = std::find_if(base->begin(), base->end(),
                     std::bind(&RefreshCollection::compare_function2, this,
                               std::placeholders::_1, file_to_hash));
  if(itbase != base->end())
    {
      if(hash != itbase->file_hash)
        {
          need_to_parsemtx.lock();
          auto itntp = std::find(need_to_parse.begin(), need_to_parse.end(),
                                 file_to_hash);
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
  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  continue_hashing.notify_one();
#else
  try
    {
      hash = file_hashing(file_to_hash);
    }
  catch(MLException &er)
    {
      std::cout << er.what() << std::endl;
    }

  omp_set_lock(&already_hashedmtx);
  already_hashed.emplace_back(std::make_tuple(file_to_hash, hash));
  omp_set_lock(&basemtx);
  auto itbase
      = std::find_if(base->begin(), base->end(),
                     std::bind(&RefreshCollection::compare_function2, this,
                               std::placeholders::_1, file_to_hash));
  if(itbase != base->end())
    {
      if(hash != itbase->file_hash)
        {
          omp_set_lock(&need_to_parsemtx);
          auto itntp = std::find(need_to_parse.begin(), need_to_parse.end(),
                                 file_to_hash);
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
                     std::bind(&RefreshCollection::compare_function2, this,
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
              [s_tup](std::tuple<std::string, BookBaseEntry> &el) {
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
                     std::bind(&RefreshCollection::compare_function2, this,
                               std::placeholders::_1, bbe.file_path));
  if(itbase != base.end())
    {
      auto itbpe = std::find_if(itbase->books.begin(), itbase->books.end(),
                                [bbe](BookParseEntry &el) {
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
          std::bind(&RefreshCollection::compare_function2, this,
                    std::placeholders::_1, std::get<1>(*it).file_path));
      if(itbase != base.end())
        {
          auto itbpe = std::find_if(itbase->books.begin(), itbase->books.end(),
                                    [it](BookParseEntry &el) {
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
