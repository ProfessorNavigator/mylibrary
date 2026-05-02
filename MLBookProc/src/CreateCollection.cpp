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
#include <CreateCollection.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <ODTParser.h>
#include <PDFParser.h>
#include <TXTParser.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdexcept>
#include <syncstream>
#include <thread>

CreateCollection::CreateCollection(const std::shared_ptr<MLBookProc> &mlbp,
                                   const int &threads_num)
{
  this->mlbp = mlbp;

  threads_v = std::make_shared<std::vector<std::tuple<unsigned, bool>>>();
  threads_v_mtx = std::make_shared<std::mutex>();
  threads_v_var = std::make_shared<std::condition_variable>();

  if(threads_num > 0
     && threads_num <= static_cast<int>(std::thread::hardware_concurrency()))
    {
      for(unsigned i = 0; i < static_cast<unsigned>(threads_num); i++)
        {
          threads_v->push_back(std::make_tuple(i, true));
        }
    }
  else
    {
      threads_v->push_back(std::make_tuple(0, true));
    }
  threads_v->shrink_to_fit();

  cancel.store(false, std::memory_order_relaxed);
}

CreateCollection::~CreateCollection()
{
  CreateCollection::stopAll();
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
}

void
CreateCollection::createCollection(
    const std::vector<std::filesystem::path> &files_and_dirs,
    const std::filesystem::path &base_path)
{
  UDBase col_base;
  UDBElement coll_info;
  bid.setId(coll_info, BaseID::CollectionInfo);

  UDBElement el;
  bid.setId(el, BaseID::CollectionType);
  el.content = "native";
  coll_info.subelements.emplace_back(el);

  col_base.addElement(coll_info);

  filesCollecting(files_and_dirs, col_base);

  std::vector<UDBElement> *raw_base = col_base.getRawBase();

  size_t fl_count = countFiles(*raw_base);

  if(signal_files_collecting)
    {
      signal_files_collecting(fl_count);
    }

  processed.store(0.0, std::memory_order_relaxed);

  total = static_cast<double>(fl_count);

  if(signal_parsing_progress)
    {
      signal_parsing_progress(processed, total);
    }

  processFiles(*raw_base);
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

  if(cancel.load(std::memory_order_relaxed))
    {
      return void();
    }
  std::vector<std::tuple<const std::vector<UDBElement> *,
                         std::vector<UDBElement>::const_iterator>>
      files = getAllFiles(*raw_base);

  removeDublicates(files);

  cleanBase(*raw_base);

  if(cancel.load(std::memory_order_relaxed))
    {
      return void();
    }

  saveBase(base_path, col_base);
}

void
CreateCollection::createInpxCollection(
    const std::filesystem::path &path_to_inpx,
    const std::filesystem::path &base_path)
{
  if(!std::filesystem::exists(path_to_inpx))
    {
      throw std::runtime_error(
          "CreateCollection::createInpxCollection: inpx file does not exist");
    }
  if(path_to_inpx.extension().u8string() != u8".inpx")
    {
      throw std::runtime_error(
          "CreateCollection::createInpxCollection: inpx file does not exist");
    }

  UDBase base;

  UDBElement coll_info;
  bid.setId(coll_info, BaseID::CollectionInfo);

  UDBElement el;
  bid.setId(el, BaseID::CollectionType);
  el.content = "inpx";
  coll_info.subelements.emplace_back(el);

  el = UDBElement();
  bid.setId(el, BaseID::BooksDirectory);
  std::u8string u8str = path_to_inpx.parent_path().u8string();
  el.content = std::string(u8str.begin(), u8str.end());
  coll_info.subelements.emplace_back(el);

  el = UDBElement();
  bid.setId(el, BaseID::InpxPath);
  u8str = path_to_inpx.u8string();
  el.content = std::string(u8str.begin(), u8str.end());
  coll_info.subelements.emplace_back(el);

  base.addElement(coll_info);

  std::vector<char> buf;
  base.writeToBuffer(buf);

  std::filesystem::create_directories(base_path.parent_path());
  std::fstream f;
  f.open(base_path, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      uint64_t base_sz = static_cast<uint64_t>(buf.size());
      ByteOrder bo(base_sz);
      bo.getLittle(base_sz);
      f.write(reinterpret_cast<char *>(&base_sz), sizeof(base_sz));
      f.write(buf.data(), buf.size());
      f.close();
    }
  else
    {
      std::filesystem::remove_all(base_path.parent_path());
    }
}

void
CreateCollection::stopAll()
{
  cancel.store(true, std::memory_order_relaxed);

  std::lock_guard<std::mutex> lglock(arch_proc_mtx);
  for(size_t i = 0; i < arch_proc.size(); i++)
    {
      arch_proc[i]->stopAll();
    }
}

void
CreateCollection::filesCollecting(
    const std::vector<std::filesystem::path> &files_and_dirs, UDBase &col_base)
{
  for(auto it = files_and_dirs.begin(); it != files_and_dirs.end(); it++)
    {
      if(!std::filesystem::exists(*it))
        {
          continue;
        }
      std::error_code ec;
      std::filesystem::file_status stat
          = std::filesystem::symlink_status(*it, ec);
      if(ec)
        {
          std::cout << "CreateCollection::filesCollecting: \"" << ec.message()
                    << "\" " << *it << std::endl;
          continue;
        }
      switch(stat.type())
        {
        case std::filesystem::file_type::directory:
          {
            UDBElement dir;
            bid.setId(dir, BaseID::Dir);
            std::u8string u8str = it->u8string();
            dir.content = std::string(u8str.begin(), u8str.end());
            for(auto &dir_it : std::filesystem::recursive_directory_iterator(
                    *it,
                    std::filesystem::directory_options::
                            follow_directory_symlink
                        | std::filesystem::directory_options::
                            skip_permission_denied,
                    ec))
              {
                std::filesystem::path p = dir_it.path();
                if(std::filesystem::is_symlink(p))
                  {
                    std::error_code l_ec;
                    std::filesystem::path resolved
                        = std::filesystem::read_symlink(p, l_ec);
                    if(l_ec)
                      {
                        std::cout << "CreateCollection::filesCollecting: \""
                                  << l_ec.message() << "\" " << p << std::endl;
                        continue;
                      }
                    if(mlbp->ifSupportedFile(resolved))
                      {
                        UDBElement symlink;
                        bid.setId(symlink, BaseID::Symlink);
                        u8str = p.u8string();
                        symlink.content
                            = std::string(u8str.begin(), u8str.end());

                        UDBElement file;
                        bid.setId(file, BaseID::File);
                        u8str = resolved.u8string();
                        file.content = std::string(u8str.begin(), u8str.end());

                        symlink.subelements.emplace_back(file);

                        dir.subelements.emplace_back(symlink);
                      }
                  }
                else if(mlbp->ifSupportedFile(p))
                  {
                    UDBElement file;
                    bid.setId(file, BaseID::File);
                    u8str = p.u8string();
                    file.content = std::string(u8str.begin(), u8str.end());

                    dir.subelements.emplace_back(file);
                  }
              }

            if(ec)
              {
                std::cout << "CreateCollection::filesCollecting: \""
                          << ec.message() << "\" " << *it << std::endl;
                break;
              }
            if(dir.subelements.size() > 0)
              {
                col_base.addElement(dir);
              }
            break;
          }
        case std::filesystem::file_type::symlink:
          {
            UDBElement symlink;
            bid.setId(symlink, BaseID::Symlink);
            std::u8string u8str = it->u8string();
            symlink.content = std::string(u8str.begin(), u8str.end());

            std::error_code ec;
            std::filesystem::path resolved
                = std::filesystem::read_symlink(*it, ec);
            if(ec)
              {
                std::cout << "CreateCollection::filesCollecting: \""
                          << ec.message() << "\" " << *it << std::endl;
                break;
              }
            if(std::filesystem::is_directory(resolved))
              {
                bid.setId(symlink, BaseID::Dir);
                for(auto &dir_it :
                    std::filesystem::recursive_directory_iterator(
                        resolved,
                        std::filesystem::directory_options::
                                follow_directory_symlink
                            | std::filesystem::directory_options::
                                skip_permission_denied,
                        ec))
                  {
                    std::filesystem::path p = dir_it.path();
                    if(std::filesystem::is_symlink(p))
                      {
                        std::error_code l_ec;
                        std::filesystem::path l_resolved
                            = std::filesystem::read_symlink(p, l_ec);
                        if(l_ec)
                          {
                            std::cout
                                << "CreateCollection::filesCollecting: \""
                                << l_ec.message() << "\" " << p << std::endl;
                            continue;
                          }
                        if(mlbp->ifSupportedFile(l_resolved))
                          {
                            UDBElement sub_sim;
                            bid.setId(symlink, BaseID::Symlink);
                            u8str = p.u8string();
                            sub_sim.content
                                = std::string(u8str.begin(), u8str.end());

                            UDBElement file;
                            bid.setId(file, BaseID::File);
                            u8str = l_resolved.u8string();
                            file.content
                                = std::string(u8str.begin(), u8str.end());

                            sub_sim.subelements.emplace_back(file);

                            symlink.subelements.emplace_back(sub_sim);
                          }
                      }
                    else
                      {
                        UDBElement file;
                        bid.setId(file, BaseID::File);
                        u8str = p.u8string();
                        file.content = std::string(u8str.begin(), u8str.end());

                        symlink.subelements.emplace_back(file);
                      }
                  }
                if(ec)
                  {
                    std::cout << "CreateCollection::filesCollecting: \""
                              << ec.message() << "\" " << *it << std::endl;
                  }
              }
            else if(mlbp->ifSupportedFile(resolved))
              {
                UDBElement file;
                bid.setId(file, BaseID::File);
                u8str = resolved.u8string();
                file.content = std::string(u8str.begin(), u8str.end());
                symlink.subelements.emplace_back(file);
              }

            if(symlink.subelements.size() > 0)
              {
                col_base.addElement(symlink);
              }
            break;
          }
        default:
          {
            if(mlbp->ifSupportedFile(*it))
              {
                UDBElement file;
                bid.setId(file, BaseID::File);
                std::u8string u8str = it->u8string();
                file.content = std::string(u8str.begin(), u8str.end());

                col_base.addElement(file);
              }
            break;
          }
        }
    }
}

size_t
CreateCollection::countFiles(const std::vector<UDBElement> &el_v)
{
  size_t result = 0;

  for(auto it = el_v.begin(); it != el_v.end(); it++)
    {
      if(bid.getId(*it) == BaseID::File)
        {
          if(it->subelements.size() == 0)
            {
              result++;
            }
        }
      else
        {
          result += countFiles(it->subelements);
        }
    }

  return result;
}

std::vector<std::tuple<const std::vector<UDBElement> *,
                       std::vector<UDBElement>::const_iterator>>
CreateCollection::getAllFiles(const std::vector<UDBElement> &el_v)
{
  std::vector<std::tuple<const std::vector<UDBElement> *,
                         std::vector<UDBElement>::const_iterator>>
      result;

  for(auto it = el_v.begin(); it != el_v.end(); it++)
    {
      if(bid.getId(*it) == BaseID::File)
        {
          result.push_back(std::make_tuple(&el_v, it));
        }
      else
        {
          std::vector<std::tuple<const std::vector<UDBElement> *,
                                 std::vector<UDBElement>::const_iterator>>
              res = std::move(getAllFiles(it->subelements));
          std::copy(res.begin(), res.end(), std::back_inserter(result));
        }
    }
  return result;
}

void
CreateCollection::removeDublicates(
    std::vector<std::tuple<const std::vector<UDBElement> *,
                           std::vector<UDBElement>::const_iterator>> &files)
{
  Algorithm alg;
  alg.parallelSort(
      files.begin(), files.end(),
      [](const std::tuple<const std::vector<UDBElement> *,
                          std::vector<UDBElement>::const_iterator> &el1,
         const std::tuple<const std::vector<UDBElement> *,
                          std::vector<UDBElement>::const_iterator> &el2)
        {
          if(std::get<0>(el1) >= std::get<0>(el2))
            {
              if(std::get<1>(el1) >= std::get<1>(el2))
                {
                  return true;
                }
            }
          return false;
        });
  for(auto it = files.begin(); it != files.end(); it++)
    {
      auto it_h = std::find_if(std::get<1>(*it)->subelements.begin(),
                               std::get<1>(*it)->subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::FileHash;
                                 });
      if(it_h == std::get<1>(*it)->subelements.end())
        {
          continue;
        }
      std::string hash = it_h->content;

      auto it_f = alg.parallelFindIf(
          it + 1, files.end(),
          [hash,
           this](const std::tuple<const std::vector<UDBElement> *,
                                  std::vector<UDBElement>::const_iterator> &el)
            {
              auto it_h
                  = std::find_if(std::get<1>(el)->subelements.begin(),
                                 std::get<1>(el)->subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::FileHash;
                                   });
              if(it_h == std::get<1>(el)->subelements.end())
                {
                  return false;
                }
              return it_h->content == hash;
            });
      if(it_f != files.end())
        {
          std::vector<UDBElement> *v
              = const_cast<std::vector<UDBElement> *>(std::get<0>(*it));
          v->erase(std::get<1>(*it));
        }
    }
}

void
CreateCollection::processFiles(const std::vector<UDBElement> &items)
{
  UDBElement *elements = const_cast<UDBElement *>(items.data());
  size_t lim = items.size();
  for(size_t i = 0; i < lim; i++)
    {
      if(cancel.load(std::memory_order_relaxed))
        {
          break;
        }
      UDBElement *el = elements + i;
      if(bid.getId(*el) == BaseID::File)
        {
          if(el->subelements.size() > 0)
            {
              continue;
            }
          std::vector<std::tuple<unsigned, bool>>::iterator it;
          std::unique_lock<std::mutex> ullock(*threads_v_mtx);
          threads_v_var->wait(ullock,
                              [&it, this]
                                {
                                  for(it = threads_v->begin();
                                      it != threads_v->end(); it++)
                                    {
                                      if(std::get<1>(*it))
                                        {
                                          std::get<1>(*it) = false;
                                          return true;
                                        }
                                    }
                                  return false;
                                });
          std::thread thr(
              [this, it, el]
                {
                  fileParsing(el);

                  processed.store(processed.load(std::memory_order_relaxed)
                                      + 1.0,
                                  std::memory_order_relaxed);
                  if(signal_parsing_progress)
                    {
                      signal_parsing_progress(
                          processed.load(std::memory_order_relaxed), total);
                    }
                  std::lock_guard<std::mutex> lglock(*threads_v_mtx);
                  std::get<1>(*it) = true;
                  threads_v_var->notify_one();
                });
#ifdef __linux
          cpu_set_t cpu;
          CPU_ZERO(&cpu);
          CPU_SET(std::get<0>(*it), &cpu);
          int er = pthread_setaffinity_np(thr.native_handle(),
                                          sizeof(cpu_set_t), &cpu);
          if(er)
            {
              std::cout << "CreateCollection::processFiles: \""
                        << std::strerror(er) << "\"" << std::endl;
            }
#elif defined(_WIN32)
          GROUP_AFFINITY gaf{};
          gaf.Group = std::get<0>(*it) / (sizeof(KAFFINITY) * CHAR_BIT);
          gaf.Mask = (1 << std::get<0>(*it) % (sizeof(KAFFINITY) * CHAR_BIT));
          HANDLE handle = pthread_gethandle(thr.native_handle());
          if(handle != nullptr)
            {
              if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
                {
                  std::cout << "CreateCollection::processFiles "
                               "SetThreadAffinityMask: \""
                            << std::strerror(GetLastError()) << "\""
                            << std::endl;
                }
            }
          else
            {
              std::cout << "CreateCollection::processFiles: handle is null!"
                        << std::endl;
            }
#endif
          thr.detach();
        }
      else
        {
          processFiles(el->subelements);
        }
    }
}

void
CreateCollection::fileParsing(UDBElement *file)
{
  std::filesystem::path p
      = std::u8string(file->content.begin(), file->content.end());
  std::string ext = mlbp->getExtension(p);
  ext = mlbp->stringToLower(ext);

  std::osyncstream(std::cout) << "Start parsing: " << p << std::endl;

  if(ext == ".fb2")
    {
      fb2Parsing(file, p);
    }
  else if(ext == ".epub")
    {
      epubParsing(file, p);
    }
  else if(ext == ".pdf")
    {
      pdfParsing(file, p);
    }
  else if(ext == ".djvu")
    {
      djvuParsing(file, p);
    }
  else if(ext == ".odt")
    {
      odtParsing(file, p);
    }
  else if(ext == ".txt" || ext == ".md")
    {
      txtParsing(file, p);
    }
  else
    {
      archiveParsing(file, p);
    }

  std::osyncstream(std::cout) << "Finish parsing: " << p << std::endl;
}

void
CreateCollection::cleanBase(std::vector<UDBElement> &items)
{
  Algorithm alg;
  items.erase(alg.parallelRemoveIf(
                  items.begin(), items.end(),
                  [this](const UDBElement &el)
                    {
                      switch(bid.getId(el))
                        {
                        case BaseID::File:
                          {
                            UDBElement *it = const_cast<UDBElement *>(&el);
                            auto it_sub = std::find_if(
                                it->subelements.begin(), it->subelements.end(),
                                [this](const UDBElement &el)
                                  {
                                    return bid.getId(el) == BaseID::Book;
                                  });
                            if(it_sub == it->subelements.end())
                              {
                                return true;
                              }
                            else
                              {
                                return false;
                              }
                          }
                        case BaseID::Dir:
                        case BaseID::Symlink:
                          {
                            UDBElement *it = const_cast<UDBElement *>(&el);
                            cleanBase(it->subelements);
                            if(it->subelements.size() > 0)
                              {
                                return false;
                              }
                            else
                              {
                                return true;
                              }
                          }
                        default:
                          {
                            return false;
                          }
                        }
                    }),
              items.end());
}

void
CreateCollection::saveBase(const std::filesystem::path &base_path,
                           UDBase &col_base)
{
  std::vector<UDBase> bases = col_base.splitBase(10485760);
  std::filesystem::create_directories(base_path.parent_path());
  std::fstream f;
  f.open(base_path, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      uint64_t sz;
      char *sz_ptr = reinterpret_cast<char *>(&sz);
      size_t sz_64 = sizeof(sz);
      ByteOrder bo;
      for(auto it = bases.begin(); it != bases.end(); it++)
        {
          std::vector<char> buf;
          it->writeToBuffer(buf);
          sz = static_cast<uint64_t>(buf.size());
          bo = sz;
          bo.getLittle(sz);
          f.write(sz_ptr, sz_64);
          f.write(buf.data(), buf.size());
        }
      f.close();
    }
  else
    {
      std::filesystem::remove_all(base_path.parent_path());
    }
}

void
CreateCollection::fb2Parsing(UDBElement *file,
                             const std::filesystem::path &file_path)
{
  std::fstream f;
  f.open(file_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::osyncstream(std::cout)
          << "CreateCollection::fb2Parsing: cannot open " << file_path
          << std::endl;
      return void();
    }
  f.seekg(0, std::ios_base::end);

  std::fpos pos = f.tellg();
  if(pos <= 0)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::fb2Parsing: incorrect file size " << file_path
          << std::endl;
      return void();
    }
  std::string buf;
  buf.resize(static_cast<size_t>(pos));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  std::unique_ptr<std::thread> thr;
  threads_v_mtx->lock();
  auto it_thr = std::find_if(threads_v->begin(), threads_v->end(),
                             [](const std::tuple<unsigned, bool> &el)
                               {
                                 return std::get<1>(el);
                               });
  if(it_thr != threads_v->end())
    {
      std::get<1>(*it_thr) = false;
      thr = std::unique_ptr<std::thread>(new std::thread(
          [this, file, file_path, buf, it_thr]
            {
              bufHash(file, file_path, buf);
              std::lock_guard<std::mutex> lglock(*threads_v_mtx);
              std::get<1>(*it_thr) = true;
              threads_v_var->notify_one();
            }));
#ifdef __linux
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(std::get<0>(*it_thr), &cpu);
      int er = pthread_setaffinity_np(thr->native_handle(), sizeof(cpu_set_t),
                                      &cpu);
      if(er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::fb2Parsing: \"" << std::strerror(er)
              << "\"" << std::endl;
        }
#elif defined(_WIN32)
      GROUP_AFFINITY gaf{};
      gaf.Group = std::get<0>(*it_thr) / (sizeof(KAFFINITY) * CHAR_BIT);
      gaf.Mask = (1 << std::get<0>(*it_thr) % (sizeof(KAFFINITY) * CHAR_BIT));
      HANDLE handle = pthread_gethandle(thr->native_handle());
      if(handle != nullptr)
        {
          if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
            {
              std::osyncstream(std::cout)
                  << "CreateCollection::fb2Parsing "
                     "SetThreadAffinityMask: \""
                  << std::strerror(GetLastError()) << "\"" << std::endl;
            }
        }
      else
        {
          std::osyncstream(std::cout)
              << "CreateCollection::fb2Parsing: handle is null!" << std::endl;
        }
#endif
      threads_v_mtx->unlock();
    }
  else
    {
      threads_v_mtx->unlock();
      bufHash(file, file_path, buf);
    }

  UDBElement book;

  try
    {
      FB2Parser parser(mlbp);
      book = parser.parseBook(buf);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::fb2Parsing error: \"" << er.what() << "\" "
          << file_path << std::endl;
    }

  auto it = std::find_if(book.subelements.begin(), book.subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::BookTitle;
                           });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::u8string u8str = file_path.stem().u8string();
          it->content = std::string(u8str.begin(), u8str.end());
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str = file_path.stem().u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);
    }

  it = std::find_if(book.subelements.begin(), book.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Date;
                      });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::filesystem::file_time_type cr
              = std::filesystem::last_write_time(file_path);
          auto sctp = std::chrono::time_point_cast<
              std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
          time_t tt = std::chrono::system_clock::to_time_t(sctp);
          it->content = mlbp->timeToDate(tt);
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::Date);
      std::filesystem::file_time_type cr
          = std::filesystem::last_write_time(file_path);
      auto sctp
          = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
      time_t tt = std::chrono::system_clock::to_time_t(sctp);
      el.content = mlbp->timeToDate(tt);
      book.subelements.emplace_back(el);
    }

  if(book.id.empty())
    {
      bid.setId(book, BaseID::Book);
    }

  uint64_t fsz = static_cast<uint64_t>(pos);
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }
  if(thr)
    {
      thr->join();
    }

  file->subelements.emplace_back(size);

  file->subelements.emplace_back(book);
}

void
CreateCollection::epubParsing(UDBElement *file,
                              const std::filesystem::path &file_path)
{
  std::fstream f;
  f.open(file_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::osyncstream(std::cout)
          << "CreateCollection::epubParsing: cannot open " << file_path
          << std::endl;

      return void();
    }
  f.seekg(0, std::ios_base::end);
  std::fpos pos = f.tellg();
  if(pos <= 0)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::epubParsing: incorrect file size " << file_path
          << std::endl;

      return void();
    }
  std::string buf;
  buf.resize(static_cast<size_t>(pos));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  std::unique_ptr<std::thread> thr;
  threads_v_mtx->lock();
  auto it_thr = std::find_if(threads_v->begin(), threads_v->end(),
                             [](const std::tuple<unsigned, bool> &el)
                               {
                                 return std::get<1>(el);
                               });
  if(it_thr != threads_v->end())
    {
      std::get<1>(*it_thr) = false;
      thr = std::unique_ptr<std::thread>(new std::thread(
          [this, file, file_path, buf, it_thr]
            {
              bufHash(file, file_path, buf);
              std::lock_guard<std::mutex> lglock(*threads_v_mtx);
              std::get<1>(*it_thr) = true;
              threads_v_var->notify_one();
            }));
#ifdef __linux
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(std::get<0>(*it_thr), &cpu);
      int er = pthread_setaffinity_np(thr->native_handle(), sizeof(cpu_set_t),
                                      &cpu);
      if(er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::epubParsing: \"" << std::strerror(er)
              << "\"" << std::endl;
        }
#elif defined(_WIN32)
      GROUP_AFFINITY gaf{};
      gaf.Group = std::get<0>(*it_thr) / (sizeof(KAFFINITY) * CHAR_BIT);
      gaf.Mask = (1 << std::get<0>(*it_thr) % (sizeof(KAFFINITY) * CHAR_BIT));
      HANDLE handle = pthread_gethandle(thr->native_handle());
      if(handle != nullptr)
        {
          if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
            {
              std::osyncstream(std::cout)
                  << "CreateCollection::epubParsing "
                     "SetThreadAffinityMask: \""
                  << std::strerror(GetLastError()) << "\"" << std::endl;
            }
        }
      else
        {
          std::osyncstream(std::cout)
              << "CreateCollection::epubParsing: handle is null!" << std::endl;
        }
#endif
      threads_v_mtx->unlock();
    }
  else
    {
      threads_v_mtx->unlock();
      bufHash(file, file_path, buf);
    }

  UDBElement book;
  try
    {
      EPUBParser parser(mlbp);
      book = parser.parseBook(buf);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::epubParsing error: \"" << er.what() << "\" "
          << file_path << std::endl;
    }

  auto it = std::find_if(book.subelements.begin(), book.subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::BookTitle;
                           });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::u8string u8str = file_path.stem().u8string();
          it->content = std::string(u8str.begin(), u8str.end());
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str = file_path.stem().u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);
    }

  it = std::find_if(book.subelements.begin(), book.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Date;
                      });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::filesystem::file_time_type cr
              = std::filesystem::last_write_time(file_path);
          auto sctp = std::chrono::time_point_cast<
              std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
          time_t tt = std::chrono::system_clock::to_time_t(sctp);
          it->content = mlbp->timeToDate(tt);
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::Date);
      std::filesystem::file_time_type cr
          = std::filesystem::last_write_time(file_path);
      auto sctp
          = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
      time_t tt = std::chrono::system_clock::to_time_t(sctp);
      el.content = mlbp->timeToDate(tt);
      book.subelements.emplace_back(el);
    }

  if(book.id.empty())
    {
      bid.setId(book, BaseID::Book);
    }

  uint64_t fsz = static_cast<uint64_t>(pos);
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }

  if(thr)
    {
      thr->join();
    }

  file->subelements.emplace_back(size);

  file->subelements.emplace_back(book);
}

void
CreateCollection::pdfParsing(UDBElement *file,
                             const std::filesystem::path &file_path)
{
  std::fstream f;
  f.open(file_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::osyncstream(std::cout)
          << "CreateCollection::pdfParsing: cannot open file " << file_path
          << std::endl;

      return void();
    }
  f.seekg(0, std::ios_base::end);
  std::fpos pos = f.tellg();
  if(pos <= 0)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::pdfParsing: incorrect file size " << file_path
          << std::endl;

      return void();
    }

  std::string buf;
  buf.resize(static_cast<size_t>(pos));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  std::unique_ptr<std::thread> thr;
  threads_v_mtx->lock();
  auto it_thr = std::find_if(threads_v->begin(), threads_v->end(),
                             [](const std::tuple<unsigned, bool> &el)
                               {
                                 return std::get<1>(el);
                               });
  if(it_thr != threads_v->end())
    {
      std::get<1>(*it_thr) = false;
      thr = std::unique_ptr<std::thread>(new std::thread(
          [this, file, file_path, buf, it_thr]
            {
              bufHash(file, file_path, buf);
              std::lock_guard<std::mutex> lglock(*threads_v_mtx);
              std::get<1>(*it_thr) = true;
              threads_v_var->notify_one();
            }));
#ifdef __linux
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(std::get<0>(*it_thr), &cpu);
      int er = pthread_setaffinity_np(thr->native_handle(), sizeof(cpu_set_t),
                                      &cpu);
      if(er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::pdfParsing: \"" << std::strerror(er)
              << "\"" << std::endl;
        }
#elif defined(_WIN32)
      GROUP_AFFINITY gaf{};
      gaf.Group = std::get<0>(*it_thr) / (sizeof(KAFFINITY) * CHAR_BIT);
      gaf.Mask = (1 << std::get<0>(*it_thr) % (sizeof(KAFFINITY) * CHAR_BIT));
      HANDLE handle = pthread_gethandle(thr->native_handle());
      if(handle != nullptr)
        {
          if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
            {
              std::osyncstream(std::cout)
                  << "CreateCollection::pdfParsing "
                     "SetThreadAffinityMask: \""
                  << std::strerror(GetLastError()) << "\"" << std::endl;
            }
        }
      else
        {
          std::osyncstream(std::cout)
              << "CreateCollection::pdfParsing: handle is null!" << std::endl;
        }
#endif
      threads_v_mtx->unlock();
    }
  else
    {
      threads_v_mtx->unlock();
      bufHash(file, file_path, buf);
    }

  UDBElement book;
  try
    {
      PDFParser parser(mlbp);
      book = parser.parseBook(buf);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::pdfParsing: \"" << er.what() << "\" "
          << file_path << std::endl;
    }

  auto it = std::find_if(book.subelements.begin(), book.subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::BookTitle;
                           });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::u8string u8str = file_path.stem().u8string();
          it->content = std::string(u8str.begin(), u8str.end());
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str = file_path.stem().u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);
    }

  it = std::find_if(book.subelements.begin(), book.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Date;
                      });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::filesystem::file_time_type cr
              = std::filesystem::last_write_time(file_path);
          auto sctp = std::chrono::time_point_cast<
              std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
          time_t tt = std::chrono::system_clock::to_time_t(sctp);
          it->content = mlbp->timeToDate(tt);
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::Date);
      std::filesystem::file_time_type cr
          = std::filesystem::last_write_time(file_path);
      auto sctp
          = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
      time_t tt = std::chrono::system_clock::to_time_t(sctp);
      el.content = mlbp->timeToDate(tt);
      book.subelements.emplace_back(el);
    }

  if(book.id.empty())
    {
      bid.setId(book, BaseID::Book);
    }

  uint64_t fsz = static_cast<uint64_t>(pos);
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }

  if(thr)
    {
      thr->join();
    }

  file->subelements.emplace_back(size);

  file->subelements.emplace_back(book);
}

void
CreateCollection::djvuParsing(UDBElement *file,
                              const std::filesystem::path &file_path)
{
  std::fstream f;
  f.open(file_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::osyncstream(std::cout)
          << "CreateCollection::djvuParsing: cannot open file " << file_path
          << std::endl;

      return void();
    }
  f.seekg(0, std::ios_base::end);
  std::fpos pos = f.tellg();
  if(pos <= 0)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::djvuParsing: incorrect file size " << file_path
          << std::endl;

      return void();
    }

  std::string buf;
  buf.resize(static_cast<size_t>(pos));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  std::unique_ptr<std::thread> thr;
  threads_v_mtx->lock();
  auto it_thr = std::find_if(threads_v->begin(), threads_v->end(),
                             [](const std::tuple<unsigned, bool> &el)
                               {
                                 return std::get<1>(el);
                               });
  if(it_thr != threads_v->end())
    {
      std::get<1>(*it_thr) = false;
      thr = std::unique_ptr<std::thread>(new std::thread(
          [this, file, file_path, buf, it_thr]
            {
              bufHash(file, file_path, buf);
              std::lock_guard<std::mutex> lglock(*threads_v_mtx);
              std::get<1>(*it_thr) = true;
              threads_v_var->notify_one();
            }));
#ifdef __linux
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(std::get<0>(*it_thr), &cpu);
      int er = pthread_setaffinity_np(thr->native_handle(), sizeof(cpu_set_t),
                                      &cpu);
      if(er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::djvuParsing: \"" << std::strerror(er)
              << "\"" << std::endl;
        }
#elif defined(_WIN32)
      GROUP_AFFINITY gaf{};
      gaf.Group = std::get<0>(*it_thr) / (sizeof(KAFFINITY) * CHAR_BIT);
      gaf.Mask = (1 << std::get<0>(*it_thr) % (sizeof(KAFFINITY) * CHAR_BIT));
      HANDLE handle = pthread_gethandle(thr->native_handle());
      if(handle != nullptr)
        {
          if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
            {
              std::osyncstream(std::cout)
                  << "CreateCollection::djvuParsing "
                     "SetThreadAffinityMask: \""
                  << std::strerror(GetLastError()) << "\"" << std::endl;
            }
        }
      else
        {
          std::osyncstream(std::cout)
              << "CreateCollection::djvuParsing: handle is null!" << std::endl;
        }
#endif
      threads_v_mtx->unlock();
    }
  else
    {
      threads_v_mtx->unlock();
      bufHash(file, file_path, buf);
    }

  UDBElement book;
  try
    {
      DJVUParser parser(mlbp);
      book = parser.parseBook(buf);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::djvuParsing: \"" << er.what() << "\" "
          << file_path << std::endl;
    }

  auto it = std::find_if(book.subelements.begin(), book.subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::BookTitle;
                           });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::u8string u8str = file_path.stem().u8string();
          it->content = std::string(u8str.begin(), u8str.end());
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str = file_path.stem().u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);
    }

  it = std::find_if(book.subelements.begin(), book.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Date;
                      });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::filesystem::file_time_type cr
              = std::filesystem::last_write_time(file_path);
          auto sctp = std::chrono::time_point_cast<
              std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
          time_t tt = std::chrono::system_clock::to_time_t(sctp);
          it->content = mlbp->timeToDate(tt);
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::Date);
      std::filesystem::file_time_type cr
          = std::filesystem::last_write_time(file_path);
      auto sctp
          = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
      time_t tt = std::chrono::system_clock::to_time_t(sctp);
      el.content = mlbp->timeToDate(tt);
      book.subelements.emplace_back(el);
    }

  if(book.id.empty())
    {
      bid.setId(book, BaseID::Book);
    }

  uint64_t fsz = static_cast<uint64_t>(pos);
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }

  if(thr)
    {
      thr->join();
    }

  file->subelements.emplace_back(size);

  file->subelements.emplace_back(book);
}

void
CreateCollection::odtParsing(UDBElement *file,
                             const std::filesystem::path &file_path)
{
  std::fstream f;
  f.open(file_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::osyncstream(std::cout)
          << "CreateCollection::odtParsing: cannot open file " << file_path
          << std::endl;

      return void();
    }
  f.seekg(0, std::ios_base::end);
  std::fpos pos = f.tellg();
  if(pos <= 0)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::odtParsing: incorrect file size " << file_path
          << std::endl;

      return void();
    }

  std::string buf;
  buf.resize(static_cast<size_t>(pos));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  std::unique_ptr<std::thread> thr;
  threads_v_mtx->lock();
  auto it_thr = std::find_if(threads_v->begin(), threads_v->end(),
                             [](const std::tuple<unsigned, bool> &el)
                               {
                                 return std::get<1>(el);
                               });
  if(it_thr != threads_v->end())
    {
      std::get<1>(*it_thr) = false;
      thr = std::unique_ptr<std::thread>(new std::thread(
          [this, file, file_path, buf, it_thr]
            {
              bufHash(file, file_path, buf);
              std::lock_guard<std::mutex> lglock(*threads_v_mtx);
              std::get<1>(*it_thr) = true;
              threads_v_var->notify_one();
            }));
#ifdef __linux
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(std::get<0>(*it_thr), &cpu);
      int er = pthread_setaffinity_np(thr->native_handle(), sizeof(cpu_set_t),
                                      &cpu);
      if(er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::odtParsing: \"" << std::strerror(er)
              << "\"" << std::endl;
        }
#elif defined(_WIN32)
      GROUP_AFFINITY gaf{};
      gaf.Group = std::get<0>(*it_thr) / (sizeof(KAFFINITY) * CHAR_BIT);
      gaf.Mask = (1 << std::get<0>(*it_thr) % (sizeof(KAFFINITY) * CHAR_BIT));
      HANDLE handle = pthread_gethandle(thr->native_handle());
      if(handle != nullptr)
        {
          if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
            {
              std::osyncstream(std::cout)
                  << "CreateCollection::odtParsing "
                     "SetThreadAffinityMask: \""
                  << std::strerror(GetLastError()) << "\"" << std::endl;
            }
        }
      else
        {
          std::osyncstream(std::cout)
              << "CreateCollection::odtParsing: handle is null!" << std::endl;
        }
#endif
      threads_v_mtx->unlock();
    }
  else
    {
      threads_v_mtx->unlock();
      bufHash(file, file_path, buf);
    }

  UDBElement book;

  try
    {
      ODTParser parser(mlbp);
      book = parser.parseBook(buf);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::odtParsing: \"" << er.what() << "\" "
          << file_path << std::endl;
    }

  auto it = std::find_if(book.subelements.begin(), book.subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::BookTitle;
                           });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::u8string u8str = file_path.stem().u8string();
          it->content = std::string(u8str.begin(), u8str.end());
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str = file_path.stem().u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);
    }

  it = std::find_if(book.subelements.begin(), book.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Date;
                      });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::filesystem::file_time_type cr
              = std::filesystem::last_write_time(file_path);
          auto sctp = std::chrono::time_point_cast<
              std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
          time_t tt = std::chrono::system_clock::to_time_t(sctp);
          it->content = mlbp->timeToDate(tt);
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::Date);
      std::filesystem::file_time_type cr
          = std::filesystem::last_write_time(file_path);
      auto sctp
          = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
      time_t tt = std::chrono::system_clock::to_time_t(sctp);
      el.content = mlbp->timeToDate(tt);
      book.subelements.emplace_back(el);
    }

  if(book.id.empty())
    {
      bid.setId(book, BaseID::Book);
    }

  uint64_t fsz = static_cast<uint64_t>(pos);
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }

  if(thr)
    {
      thr->join();
    }

  file->subelements.emplace_back(size);

  file->subelements.emplace_back(book);
}

void
CreateCollection::txtParsing(UDBElement *file,
                             const std::filesystem::path &file_path)
{
  auto it_hash = std::find_if(
      already_hashed.begin(), already_hashed.end(),
      [file_path](const std::tuple<std::filesystem::path, std::string> &el)
        {
          return file_path == std::get<0>(el);
        });
  if(it_hash != already_hashed.end())
    {
      UDBElement el;
      bid.setId(el, BaseID::FileHash);
      el.content = std::get<1>(*it_hash);
      file->subelements.emplace_back(el);
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::FileHash);
      try
        {
          el.content = fileHash(file_path);
          file->subelements.emplace_back(el);
        }
      catch(std::exception &er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::txtParsing: \"" << er.what() << "\" "
              << file_path << std::endl;
        }
    }

  std::error_code ec;
  uint64_t fsz
      = static_cast<uint64_t>(std::filesystem::file_size(file_path, ec));
  if(ec)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::txtParsing: \"" << ec.message() << "\" "
          << file_path << std::endl;

      return void();
    }
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }
  file->subelements.emplace_back(size);

  TXTParser parser(mlbp);
  UDBElement book = parser.parseBook(file_path);
  file->subelements.emplace_back(book);
}

void
CreateCollection::archiveParsing(UDBElement *file,
                                 const std::filesystem::path &file_path)
{
  std::shared_ptr<ArchiveParser> parser(
      new ArchiveParser(mlbp, threads_v, threads_v_mtx, threads_v_var));
  arch_proc_mtx.lock();
  arch_proc.push_back(parser);
  arch_proc_mtx.unlock();
  try
    {
      file->subelements = parser->parseArchive(file_path);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::archiveParsing: \"" << er.what() << "\" "
          << file_path << std::endl;
    }

  arch_proc_mtx.lock();
  arch_proc.erase(std::remove(arch_proc.begin(), arch_proc.end(), parser),
                  arch_proc.end());
  arch_proc_mtx.unlock();

  if(file->subelements.size() == 0)
    {
      return void();
    }

  auto it_hash = std::find_if(
      already_hashed.begin(), already_hashed.end(),
      [file_path](const std::tuple<std::filesystem::path, std::string> &el)
        {
          return file_path == std::get<0>(el);
        });
  if(it_hash != already_hashed.end())
    {
      UDBElement el;
      bid.setId(el, BaseID::FileHash);
      el.content = std::get<1>(*it_hash);
      file->subelements.emplace_back(el);
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::FileHash);
      try
        {
          el.content = fileHash(file_path);
          file->subelements.emplace_back(el);
        }
      catch(std::exception &er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::archiveParsing: \"" << er.what() << "\" "
              << file_path << std::endl;
        }
    }

  std::error_code ec;
  uint64_t fsz
      = static_cast<uint64_t>(std::filesystem::file_size(file_path, ec));
  if(ec)
    {
      std::osyncstream(std::cout)
          << "CreateCollection::archiveParsing: \"" << ec.message() << "\" "
          << file_path << std::endl;
      return void();
    }
  ByteOrder bo(fsz);
  bo.getLittle(fsz);
  size_t sz_64 = sizeof(fsz);
  char *ptr = reinterpret_cast<char *>(&fsz);
  UDBElement size;
  bid.setId(size, BaseID::FileSize);
  size.content.resize(sz_64);
  for(size_t i = 0; i < sz_64; i++)
    {
      size.content[i] = ptr[i];
    }
  file->subelements.emplace_back(size);
}

std::string
CreateCollection::bufferHash(const std::string &buf)
{
  std::string result;

  gcry_md_hd_t hd;
  gcry_error_t err = gcry_md_open(&hd, GCRY_MD_BLAKE2B_256, 0);
  if(err != 0)
    {
      mlbp->libgcryptErrorHandling(err);
    }
  gcry_md_write(hd, buf.c_str(), buf.size());
  result.resize(gcry_md_get_algo_dlen(GCRY_MD_BLAKE2B_256));
  char *result_buf
      = reinterpret_cast<char *>(gcry_md_read(hd, GCRY_MD_BLAKE2B_256));
  if(result_buf)
    {
      for(size_t i = 0; i < result.size(); i++)
        {
          result[i] = result_buf[i];
        }
    }
  else
    {
      gcry_md_close(hd);
      throw std::runtime_error("CreateCollection::bufferHash: result is null");
    }
  gcry_md_close(hd);

  return result;
}

std::string
CreateCollection::fileHash(const std::filesystem::path &file_path)
{
  std::string result;

  gcry_md_hd_t hd_t;
  gcry_error_t err
      = gcry_md_open(&hd_t, GCRY_MD_BLAKE2B_256, GCRY_MD_FLAG_SECURE);
  if(err != 0)
    {
      mlbp->libgcryptErrorHandling(err);
    }
  std::unique_ptr<gcry_md_handle, std::function<void(gcry_md_handle *)>> hd(
      hd_t,
      [](gcry_md_handle *hd)
        {
          gcry_md_close(hd);
        });
  std::unique_ptr<std::fstream, std::function<void(std::fstream *)>> f(
      new std::fstream,
      [](std::fstream *f)
        {
          if(f->is_open())
            {
              f->close();
            }
          delete f;
        });
  f->open(file_path, std::ios_base::in | std::ios_base::binary);
  if(!f->is_open())
    {
      std::string str = "CreateCollection::fileHash: cannot open file ";
      std::u8string u8str = file_path.u8string();
      str += std::string(u8str.begin(), u8str.end());
      throw std::runtime_error(str);
    }
  f->seekg(0, std::ios_base::end);
  size_t fsz = static_cast<size_t>(f->tellg());
  f->seekg(0, std::ios_base::beg);

  std::string buf;
  size_t buf_sz = 4194304;
  size_t rb = 0;
  size_t diff;
  while(rb < fsz)
    {
      if(cancel.load(std::memory_order_relaxed))
        {
          break;
        }
      diff = fsz - rb;
      if(diff > buf_sz)
        {
          buf.resize(buf_sz);
        }
      else
        {
          buf.resize(diff);
        }
      f->read(buf.data(), buf.size());
      rb += buf.size();
      gcry_md_write(hd.get(), buf.c_str(), buf.size());
    }

  unsigned char *hsh = gcry_md_read(hd.get(), GCRY_MD_BLAKE2B_256);

  unsigned int len = gcry_md_get_algo_dlen(GCRY_MD_BLAKE2B_256);
  result.resize(len);

  unsigned char *ptr = reinterpret_cast<unsigned char *>(result.data());

  for(unsigned int i = 0; i < len; i++)
    {
      ptr[i] = hsh[i];
    }

  return result;
}

void
CreateCollection::bufHash(UDBElement *file,
                          const std::filesystem::path &file_path,
                          const std::string &buf)
{
  auto it_hash = std::find_if(
      already_hashed.begin(), already_hashed.end(),
      [file_path](const std::tuple<std::filesystem::path, std::string> &el)
        {
          return file_path == std::get<0>(el);
        });
  if(it_hash != already_hashed.end())
    {
      UDBElement el;
      bid.setId(el, BaseID::FileHash);
      el.content = std::get<1>(*it_hash);
      file->subelements.emplace_back(el);
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::FileHash);
      try
        {
          el.content = bufferHash(buf);
          file->subelements.emplace_back(el);
        }
      catch(std::exception &er)
        {
          std::osyncstream(std::cout)
              << "CreateCollection::bufHash error: \"" << er.what() << "\" "
              << file_path << std::endl;
        }
    }
}
