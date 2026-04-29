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
#include <ByteOrder.h>
#include <RefreshCollection.h>
#include <algorithm>
#include <iostream>

RefreshCollection::RefreshCollection(const std::shared_ptr<MLBookProc> &mlbp,
                                     const int &threads_num)
    : CreateCollection(mlbp, threads_num)
{
  base_keeper = new BaseKeeper(mlbp);
}

RefreshCollection::~RefreshCollection()
{
  delete base_keeper;
}

void
RefreshCollection::refreshCollection(const std::filesystem::path &base_path,
                                     const bool &fast_refresh)
{
  base_keeper->loadCollection(base_path);
  std::string base_type = base_keeper->getCollectionType();
  if(base_type == "legacy" || base_type == "inpx")
    {
      std::filesystem::path books_path = base_keeper->getBooksDirectory();
      std::filesystem::path temp_base_path
          = base_path.parent_path() / mlbp->randomFileName();
      std::vector<std::filesystem::path> files;
      files.resize(1);
      files[0] = books_path;
      if(signal_file_hashed)
        {
          signal_file_hashed(1.0, 1.0);
        }
      createCollection(files, temp_base_path);
      if(std::filesystem::exists(temp_base_path))
        {
          std::filesystem::remove_all(base_path);
          std::filesystem::rename(temp_base_path, base_path);
        }
    }
  else if(base_type == "native")
    {
      l_processed = 0.0;
      total = static_cast<double>(base_keeper->getFilesQuantity());

      std::vector<std::filesystem::path> files_and_dirs;
      std::vector<UDBElement> *raw_base = base_keeper->getRawBase();
      for(auto it = raw_base->begin(); it != raw_base->end(); it++)
        {
          switch(bid.getId(*it))
            {
            case BaseID::File:
            case BaseID::Dir:
            case BaseID::Symlink:
              {
                files_and_dirs.push_back(std::filesystem::path(
                    std::u8string(it->content.begin(), it->content.end())));
                break;
              }
            default:
              break;
            }
        }

      base_keeper->removeElements(std::bind(&RefreshCollection::elementRemove,
                                            this, std::placeholders::_1,
                                            fast_refresh));

      UDBase new_base;

      filesCollecting(files_and_dirs, new_base);
      std::vector<UDBElement> *new_raw_base = new_base.getRawBase();

      total = 0.0;
#pragma omp parallel
#pragma omp masked
      {
        compareVectors(*new_raw_base, *raw_base);
      }

      if(signal_files_collecting)
        {
          signal_files_collecting(static_cast<size_t>(total));
        }

      UDBElement coll_info;
      bid.setId(coll_info, BaseID::CollectionInfo);

      UDBElement el;
      bid.setId(el, BaseID::CollectionType);
      el.content = "native";
      coll_info.subelements.emplace_back(el);

      new_raw_base->insert(new_raw_base->begin(), coll_info);

      l_processed = 0.0;
      processFiles(*new_raw_base);
      std::unique_lock<std::mutex> ullock(*threads_v_mtx);
      threads_v_var->wait(ullock,
                          [this]
                            {
                              for(auto it = threads_v->begin();
                                  it != threads_v->end(); it++)
                                {
                                  if(!std::get<1>(*it))
                                    {
                                      return false;
                                    }
                                }
                              return true;
                            });

      std::vector<std::tuple<const std::vector<UDBElement> *,
                             std::vector<UDBElement>::const_iterator>>
          files = getAllFiles(*raw_base);

      removeDublicates(files);

      cleanBase(*new_raw_base);

      if(cancel.load(std::memory_order::relaxed))
        {
          return void();
        }
      std::filesystem::remove_all(base_path);
      saveBase(base_path, new_base);
    }
}

void
RefreshCollection::addFilesAndDirs(
    const std::filesystem::path &base_path,
    const std::vector<std::filesystem::path> &files_and_dirs)
{
  base_keeper->loadCollection(base_path);
  std::string base_type = base_keeper->getCollectionType();
  if(base_type == "legacy" || base_type == "inpx")
    {
      std::filesystem::path books_path = base_keeper->getBooksDirectory();
      std::filesystem::path temp_base_path
          = base_path.parent_path() / mlbp->randomFileName();
      std::vector<std::filesystem::path> files;
      files.reserve(files_and_dirs.size() + 1);
      files.resize(1);
      files[0] = books_path;
      std::copy(files_and_dirs.begin(), files_and_dirs.end(),
                std::back_inserter(files));

      if(signal_file_hashed)
        {
          signal_file_hashed(1.0, 1.0);
        }
      createCollection(files, temp_base_path);
      if(std::filesystem::exists(temp_base_path))
        {
          std::filesystem::remove_all(base_path);
          std::filesystem::rename(temp_base_path, base_path);
        }
    }
  else if(base_type == "native")
    {
      l_processed = 0.0;
      total = static_cast<double>(base_keeper->getFilesQuantity());

      std::vector<UDBElement> *raw_base = base_keeper->getRawBase();

      UDBase new_base;

      filesCollecting(files_and_dirs, new_base);
      std::vector<UDBElement> *new_raw_base = new_base.getRawBase();

      total = static_cast<double>(countFiles(*new_raw_base));

      if(signal_files_collecting)
        {
          signal_files_collecting(static_cast<size_t>(total));
        }

      if(signal_file_hashed)
        {
          signal_file_hashed(1.0, 1.0);
        }

      processed.store(0.0, std::memory_order_relaxed);
      processFiles(*new_raw_base);
      std::unique_lock<std::mutex> ullock(*threads_v_mtx);
      threads_v_var->wait(ullock,
                          [this]
                            {
                              for(auto it = threads_v->begin();
                                  it != threads_v->end(); it++)
                                {
                                  if(!std::get<1>(*it))
                                    {
                                      return false;
                                    }
                                }
                              return true;
                            });

      base_keeper->operator+=(new_base);

      std::vector<std::tuple<const std::vector<UDBElement> *,
                             std::vector<UDBElement>::const_iterator>>
          files = getAllFiles(*raw_base);

      removeDublicates(files);

      cleanBase(*raw_base);

      if(cancel.load(std::memory_order::relaxed))
        {
          return void();
        }
      std::filesystem::remove_all(base_path);
      saveBase(base_path, *base_keeper);
    }
}

bool
RefreshCollection::elementRemove(const UDBElement &el,
                                 const bool &fast_refresh)
{
  BaseID::ID id = bid.getId(el);
  if(id == BaseID::File || id == BaseID::Dir || id == BaseID::Symlink)
    {
      std::filesystem::path p
          = std::u8string(el.content.begin(), el.content.end());
      if(!std::filesystem::exists(p))
        {
          return true;
        }
      if(bid.getId(el) == BaseID::File)
        {
          if(fast_refresh)
            {
              auto it
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::FileSize;
                                   });
              if(it == el.subelements.end())
                {
#pragma omp atomic update
                  l_processed += 1.0;
                  if(signal_file_hashed)
                    {
                      signal_file_hashed(l_processed, total);
                    }
                  return true;
                }
              uint64_t sz;
              size_t sz_64 = sizeof(sz);
              if(sz_64 != it->content.size())
                {
#pragma omp atomic update
                  l_processed += 1.0;
                  if(signal_file_hashed)
                    {
                      signal_file_hashed(l_processed, total);
                    }
                  return true;
                }
              char *ptr = reinterpret_cast<char *>(&sz);
              for(size_t i = 0; i < sz_64; i++)
                {
                  ptr[i] = it->content[i];
                }
              ByteOrder bo;
              bo.setLittle(sz);
              sz = bo;
              std::error_code ec;
              uint64_t fsz
                  = static_cast<uint64_t>(std::filesystem::file_size(p, ec));
              if(ec)
                {
#pragma omp critical
                  {
                    std::cout << "RefreshCollection::elementRemove: \""
                              << ec.message() << "\" " << p << std::endl;
                  }
#pragma omp atomic update
                  l_processed += 1.0;
                  if(signal_file_hashed)
                    {
                      signal_file_hashed(l_processed, total);
                    }
                  return true;
                }
              if(sz != fsz)
                {
#pragma omp atomic update
                  l_processed += 1.0;
                  if(signal_file_hashed)
                    {
                      signal_file_hashed(l_processed, total);
                    }
                  return true;
                }
#pragma omp atomic update
              l_processed += 1.0;
              if(signal_file_hashed)
                {
                  signal_file_hashed(l_processed, total);
                }
            }
          else
            {
              auto it
                  = std::find_if(el.subelements.begin(), el.subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::FileHash;
                                   });
              if(it == el.subelements.end())
                {
#pragma omp atomic update
                  l_processed += 1.0;
                  if(signal_file_hashed)
                    {
                      signal_file_hashed(l_processed, total);
                    }
                  return true;
                }
              std::string hash = fileHash(p);
#pragma omp critical
              {
                already_hashed.push_back(std::make_tuple(p, hash));
              }
              if(hash != it->content)
                {
#pragma omp atomic update
                  l_processed += 1.0;
                  if(signal_file_hashed)
                    {
                      signal_file_hashed(l_processed, total);
                    }
                  return true;
                }
#pragma omp atomic update
              l_processed += 1.0;
              if(signal_file_hashed)
                {
                  signal_file_hashed(l_processed, total);
                }
            }
        }
      else
        {
          UDBElement *el_ptr = const_cast<UDBElement *>(&el);
          Algorithm alg;
          el_ptr->subelements.erase(
              alg.parallelRemoveIf(
                  el_ptr->subelements.begin(), el_ptr->subelements.end(),
                  std::bind(&RefreshCollection::elementRemove, this,
                            std::placeholders::_1, fast_refresh)),
              el_ptr->subelements.end());
          if(el.subelements.size() == 0)
            {
              return true;
            }
        }
    }

  return false;
}

void
RefreshCollection::compareVectors(std::vector<UDBElement> &new_v,
                                  const std::vector<UDBElement> &old_v)
{
  for(auto it = new_v.begin(); it != new_v.end(); it++)
    {
#pragma omp task
      {
        auto it_old = std::find_if(
            old_v.begin(), old_v.end(),
            [it](const UDBElement &el)
              {
                if(it->id == el.id)
                  {
                    if(std::filesystem::path(std::u8string(it->content.begin(),
                                                           it->content.end()))
                       == std::filesystem::path(std::u8string(
                           el.content.begin(), el.content.end())))
                      {
                        return true;
                      }
                  }
                return false;
              });
        if(it_old != old_v.end())
          {
            if(bid.getId(*it_old) == BaseID::File)
              {
                *it = *it_old;
              }
            else
              {
                compareVectors(it->subelements, it_old->subelements);
              }
          }
        else if(bid.getId(*it) == BaseID::File)
          {
#pragma omp atomic update
            total += 1.0;
          }
      }
    }
}
