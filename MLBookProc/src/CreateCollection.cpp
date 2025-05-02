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

#include <ARCHParser.h>
#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <CreateCollection.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <MLException.h>
#include <PDFParser.h>
#include <algorithm>
#include <cstring>
#include <iostream>

#ifndef USE_OPENMP
#include <thread>
#endif

CreateCollection::CreateCollection(
    const std::shared_ptr<AuxFunc> &af,
    const std::filesystem::path &collection_path,
    const std::filesystem::path &books_path, const bool &rar_support,
    const int &num_threads)
    : Hasher(af)
{
  this->af = af;
  base_path = collection_path;
  base_path /= std::filesystem::u8path("base");
  this->books_path = books_path;
  this->rar_support = rar_support;
  if(num_threads > 0)
    {
      this->num_threads = num_threads;
    }
  else
    {
      this->num_threads = 1;
    }
#ifndef USE_OPENMP
  current_bytes.store(0.0);
  stop_all_signal = [this] {
    archp_obj_mtx.lock();
    for(auto it = archp_obj.begin(); it != archp_obj.end(); it++)
      {
        reinterpret_cast<ARCHParser *>(*it)->stopAll();
      }
    archp_obj_mtx.unlock();
  };
#else
  omp_init_lock(&archp_obj_mtx);
  stop_all_signal = [this] {
    omp_set_lock(&archp_obj_mtx);
    for(auto it = archp_obj.begin(); it != archp_obj.end(); it++)
      {
        reinterpret_cast<ARCHParser *>(*it)->stopAll();
      }
    omp_unset_lock(&archp_obj_mtx);
  };
#endif
}

CreateCollection::~CreateCollection()
{
#ifdef USE_OPENMP
  omp_destroy_lock(&archp_obj_mtx);
#endif
  if(base_strm.is_open())
    {
      base_strm.close();
    }
}

CreateCollection::CreateCollection(const std::shared_ptr<AuxFunc> &af,
                                   const int &num_threads)
    : Hasher(af)
{
  this->af = af;
  if(num_threads > 0)
    {
      this->num_threads = num_threads;
    }
#ifndef USE_OPENMP
  current_bytes.store(0.0);
  stop_all_signal = [this] {
    archp_obj_mtx.lock();
    for(auto it = archp_obj.begin(); it != archp_obj.end(); it++)
      {
        reinterpret_cast<ARCHParser *>(*it)->stopAll();
      }
    archp_obj_mtx.unlock();
  };
#else
  omp_init_lock(&archp_obj_mtx);
  stop_all_signal = [this] {
    omp_set_lock(&archp_obj_mtx);
    for(auto it = archp_obj.begin(); it != archp_obj.end(); it++)
      {
        reinterpret_cast<ARCHParser *>(*it)->stopAll();
      }
    omp_unset_lock(&archp_obj_mtx);
  };
#endif
}

void
CreateCollection::createCollection()
{
  if(!std::filesystem::exists(books_path))
    {
      return void();
    }
#pragma omp parallel
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      for(auto &dirit : std::filesystem::recursive_directory_iterator(
              books_path,
              std::filesystem::directory_options::follow_directory_symlink))
        {
          if(pulse)
            {
              pulse();
            }
          std::filesystem::path p = dirit.path();
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
                  need_to_parse.emplace_back(p);
                }
            }
        }
      omp_fulfill_event(event);
    }
#pragma omp taskwait
    if(!rar_support)
      {
        std::string sstr = ".rar";
        need_to_parse.erase(
            std::remove_if(need_to_parse.begin(), need_to_parse.end(),
                           [sstr, this](std::filesystem::path &el) {
                             std::string ext = el.extension().u8string();
                             ext = af->stringToLower(ext);
                             return ext == sstr;
                           }),
            need_to_parse.end());
      }

    if(signal_total_bytes)
      {
        double tmp = 0.0;
        for(auto it = need_to_parse.begin(); it != need_to_parse.end(); it++)
          {
            std::error_code ec;
            uintmax_t sz
                = static_cast<double>(std::filesystem::file_size(*it, ec));

            if(ec)
              {
                std::cout << "CreateCollection::createCollection: "
                          << ec.message() << std::endl;
              }
            else
              {
                tmp += static_cast<double>(sz);
              }
          }
        signal_total_bytes(tmp);
      }
    threadRegulator();
  }
}

void
CreateCollection::threadRegulator()
{
  if(need_to_parse.size() > 0)
    {
      openBaseFile();
    }
  else
    {
      return void();
    }

#ifndef USE_OPENMP
  run_threads = 0;
  for(auto it = need_to_parse.begin(); it != need_to_parse.end(); it++)
    {
      if(cancel.load())
        {
          break;
        }
      std::unique_lock<std::mutex> run_thr_lock(run_threads_mtx);
      run_threads_var.wait(run_thr_lock, [this] {
        if(num_threads > 1)
          {
            return run_threads < num_threads - 1;
          }
        else
          {
            return run_threads < 1;
          }
      });
      run_threads++;
      std::filesystem::path p = *it;
      std::thread thr([this, p] {
        std::cout << "Start parsing: " << p << std::endl;
        threadFunc(p);
        std::lock_guard<std::mutex> lglock(run_threads_mtx);
        std::cout << "Parsing finished: " << p << std::endl;
        run_threads--;
        run_threads_var.notify_one();
      });
      thr.detach();
    }

  std::unique_lock<std::mutex> run_thr_lock(run_threads_mtx);
  run_threads_var.wait(run_thr_lock, [this] {
    return run_threads <= 0;
  });
#else
  omp_set_num_threads(num_threads);
  omp_set_dynamic(true);
  int lvls = omp_get_max_active_levels();
  omp_set_max_active_levels(omp_get_supported_active_levels());
#pragma omp parallel
  {
#pragma omp for
    for(auto it = need_to_parse.begin(); it != need_to_parse.end(); it++)
      {
        bool cncl;
#pragma omp atomic read
        cncl = cancel;
        if(cncl)
          {
#pragma omp cancel for
            continue;
          }
#pragma omp critical
        {
          std::cout << "Start parsing: " << *it << std::endl;
        }
        threadFunc(*it);
#pragma omp critical
        {
          std::cout << "Parsing finished: " << *it << std::endl;
        }
      }
  }
  omp_set_max_active_levels(lvls);
  omp_set_dynamic(false);
#endif
  if(base_strm.is_open())
    {
      base_strm.close();
    }
}

void
CreateCollection::fb2_thread(const std::filesystem::path &file_col_path,
                             const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  std::error_code ec;
  uintmax_t fsz = std::filesystem::file_size(filepath, ec);
  if(ec)
    {
      throw MLException("CreateCollection::fb2_thread: " + ec.message());
    }
  if(fsz == 0)
    {
      throw MLException("CreateCollection::fb2_thread: file size is zero");
    }

  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string book;
      book.resize(static_cast<std::string::size_type>(fsz));
      f.read(book.data(), book.size());
      f.close();

      FB2Parser fb2(af);
      BookParseEntry be = fb2.fb2_parser(book);
      if(be.book_name.empty())
        {
          be.book_name = filepath.stem().u8string();
        }

      FileParseEntry fe;
      fe.file_rel_path
          = file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(
          already_hashed.begin(), already_hashed.end(),
          [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
            return std::get<0>(el) == file_col_path;
          });
      if(ithsh != already_hashed.end())
        {
          fe.file_hash = std::get<1>(*ithsh);
        }
      else
        {
          fe.file_hash = buf_hashing(book);
        }
      fe.books.emplace_back(be);
      write_file_to_base(fe);
    }
}

void
CreateCollection::openBaseFile()
{
  std::filesystem::create_directories(base_path.parent_path());
  if(!base_strm.is_open())
    {
      base_strm.open(base_path, std::ios_base::out | std::ios_base::app
                                    | std::ios_base::binary);
    }
  if(!base_strm.is_open())
    {
      throw MLException(
          "CreateCollection::openBaseFile: cannot open base file");
    }
  else
    {
      uintmax_t fsz;
      base_strm.seekg(0, std::ios_base::end);
      fsz = base_strm.tellg();
      if(fsz == 0)
        {
          base_strm.seekg(0, std::ios_base::beg);
          std::string col_path = books_path.u8string();
          if(col_path.size() > 0)
            {
              std::string::size_type n = 0;
              std::string sstr = "\\";
              for(;;)
                {
                  n = col_path.find(sstr, n);
                  if(n != std::string::npos)
                    {
                      col_path.erase(n, sstr.size());
                      col_path.insert(n, "/");
                    }
                  else
                    {
                      break;
                    }
                }
            }
          uint16_t sz = static_cast<uint16_t>(col_path.size());
          ByteOrder bo;
          bo = sz;
          bo.get_little(sz);
          base_strm.write(reinterpret_cast<char *>(&sz), sizeof(sz));
          base_strm.write(col_path.c_str(), col_path.size());
        }
    }
}

void
CreateCollection::closeBaseFile()
{
  if(base_strm.is_open())
    {
      base_strm.close();
    }
}

void
CreateCollection::book_entry_to_file_entry(std::string &file_entry,
                                           const std::string &book_entry)
{
  uint16_t val16 = static_cast<uint16_t>(book_entry.size());
  ByteOrder bo(val16);
  bo.get_little(val16);
  size_t sz = file_entry.size();
  file_entry.resize(sz + sizeof(val16));
  std::memcpy(&file_entry[sz], &val16, sizeof(val16));

  std::copy(book_entry.begin(), book_entry.end(),
            std::back_inserter(file_entry));
}

void
CreateCollection::epub_thread(const std::filesystem::path &file_col_path,
                              const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  FileParseEntry fe;

  auto ithsh = std::find_if(
      already_hashed.begin(), already_hashed.end(),
      [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
        return std::get<0>(el) == file_col_path;
      });
  if(ithsh != already_hashed.end())
    {
      fe.file_hash = std::get<1>(*ithsh);
    }
  else
    {
#ifndef USE_OPENMP
      std::unique_lock<std::mutex> rthr_lock(run_threads_mtx);
      run_threads_var.wait(rthr_lock, [this] {
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
      rthr_lock.unlock();

      std::shared_ptr<int> thr_finish(&run_threads, [this](int *) {
        std::lock_guard<std::mutex> lglock(run_threads_mtx);
        run_threads--;
        run_threads_var.notify_one();
      });
#endif
      fe.file_hash = file_hashing(filepath);
    }
#ifndef USE_OPENMP
  if(!cancel.load())
#else
  bool cncl;
#pragma atomic read
  cncl = cancel;
  if(!cncl)
#endif
    {
      fe.file_rel_path
          = file_col_path.lexically_proximate(books_path).u8string();

      EPUBParser epub(af);
      BookParseEntry be = epub.epub_parser(filepath);
      if(be.book_name.empty())
        {
          be.book_name = filepath.stem().u8string();
        }
      fe.books.emplace_back(be);

      write_file_to_base(fe);
    }
}

void
CreateCollection::pdf_thread(const std::filesystem::path &file_col_path,
                             const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  std::error_code ec;
  uintmax_t fsz = std::filesystem::file_size(filepath, ec);
  if(ec)
    {
      throw MLException("CreateCollection::pdf_thread: " + ec.message());
    }
  if(fsz == 0)
    {
      throw MLException("CreateCollection::pdf_thread: file size is zero");
    }

  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string book;
      book.resize(fsz);
      f.read(book.data(), book.size());
      f.close();

      PDFParser pdf(af);
      BookParseEntry be = pdf.pdf_parser(book);
      if(be.book_name.empty())
        {
          be.book_name = filepath.stem().u8string();
        }

      FileParseEntry fe;
      fe.file_rel_path
          = file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(
          already_hashed.begin(), already_hashed.end(),
          [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
            return std::get<0>(el) == file_col_path;
          });
      if(ithsh != already_hashed.end())
        {
          fe.file_hash = std::get<1>(*ithsh);
        }
      else
        {
          fe.file_hash = buf_hashing(book);
        }

      fe.books.emplace_back(be);
      write_file_to_base(fe);
    }
}

void
CreateCollection::arch_thread(const std::filesystem::path &file_col_path,
                              const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  ARCHParser arp(af, rar_support);
#ifndef USE_OPENMP
  archp_obj_mtx.lock();
  archp_obj.push_back(&arp);
  archp_obj_mtx.unlock();

  std::function<bool()> waiter = [this] {
    if(num_threads > 1)
      {
        return run_threads < num_threads;
      }
    else
      {
        return run_threads < 2;
      }
  };

  std::function<void(int *)> deleter = [this](int *) {
    std::lock_guard<std::mutex> lglock(run_threads_mtx);
    run_threads--;
    run_threads_var.notify_one();
  };

  std::unique_lock<std::mutex> rthr_lock(run_threads_mtx);
  run_threads_var.wait(rthr_lock, waiter);
  run_threads++;
  rthr_lock.unlock();
  std::shared_ptr<int> thr_finish(&run_threads, deleter);
#else
  omp_set_lock(&archp_obj_mtx);
  archp_obj.push_back(&arp);
  omp_unset_lock(&archp_obj_mtx);
#endif
  FileParseEntry fe;
  fe.books = arp.arch_parser(filepath);
#ifndef USE_OPENMP
  thr_finish.reset();

  if(fe.books.size() > 0 && !cancel.load())
    {
      fe.file_rel_path
          = file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(
          already_hashed.begin(), already_hashed.end(),
          [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
            return std::get<0>(el) == file_col_path;
          });
      if(ithsh != already_hashed.end())
        {
          fe.file_hash = std::get<1>(*ithsh);
        }
      else
        {
          rthr_lock.lock();
          run_threads_var.wait(rthr_lock, waiter);
          run_threads++;
          rthr_lock.unlock();
          thr_finish = std::shared_ptr<int>(&run_threads, deleter);
          fe.file_hash = file_hashing(filepath);
          thr_finish.reset();
        }
      if(!cancel.load())
        {
          write_file_to_base(fe);
        }
    }

  archp_obj_mtx.lock();
  archp_obj.erase(std::remove(archp_obj.begin(), archp_obj.end(), &arp),
                  archp_obj.end());
  archp_obj_mtx.unlock();
#else
  bool cncl;
#pragma omp atomic read
  cncl = cancel;

  if(fe.books.size() > 0 && !cncl)
    {
      fe.file_rel_path
          = file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(
          already_hashed.begin(), already_hashed.end(),
          [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
            return std::get<0>(el) == file_col_path;
          });
      if(ithsh != already_hashed.end())
        {
          fe.file_hash = std::get<1>(*ithsh);
        }
      else
        {
          fe.file_hash = file_hashing(filepath);
        }
      if(!cncl)
        {
          write_file_to_base(fe);
        }
    }
  omp_set_lock(&archp_obj_mtx);
  archp_obj.erase(std::remove(archp_obj.begin(), archp_obj.end(), &arp),
                  archp_obj.end());
  omp_unset_lock(&archp_obj_mtx);
#endif
}

void
CreateCollection::write_file_to_base(const FileParseEntry &fe)
{
  std::string file_entry;
  // File address entry
  std::string sep_cor = fe.file_rel_path;
  if(sep_cor.size() > 0)
    {

      std::string sstr = "\\";
      std::string::size_type n = 0;
      for(;;)
        {
          n = sep_cor.find(sstr, n);
          if(n != std::string::npos)
            {
              sep_cor.erase(n, sstr.size());
              sep_cor.insert(n, "/");
            }
          else
            {
              break;
            }
        }
    }
  uint16_t val16 = static_cast<uint16_t>(sep_cor.size());
  ByteOrder bo;
  bo = val16;
  bo.get_little(val16);
  file_entry.resize(sizeof(val16));
  std::memcpy(&file_entry[0], &val16, sizeof(val16));

  std::copy(sep_cor.begin(), sep_cor.end(), std::back_inserter(file_entry));

  // Book hash
  book_entry_to_file_entry(file_entry, fe.file_hash);
  uint64_t val64;
  size_t sz;
  for(auto it = fe.books.begin(); it != fe.books.end(); it++)
    {
      BookParseEntry be = *it;
      // Book entry size
      sz = be.book_author.size() + be.book_date.size() + be.book_genre.size()
           + be.book_name.size() + be.book_path.size() + be.book_series.size();
      sz += 12; // size of size fields

      val64 = static_cast<uint64_t>(sz);
      bo = val64;
      bo.get_little(val64);
      sz = file_entry.size();
      file_entry.resize(sz + sizeof(val64));
      std::memcpy(&file_entry[sz], &val64, sizeof(val64));

      // Book entries
      book_entry_to_file_entry(file_entry, be.book_path);
      book_entry_to_file_entry(file_entry, be.book_author);
      book_entry_to_file_entry(file_entry, be.book_name);
      book_entry_to_file_entry(file_entry, be.book_series);
      book_entry_to_file_entry(file_entry, be.book_genre);
      book_entry_to_file_entry(file_entry, be.book_date);
    }
  val64 = static_cast<uint64_t>(file_entry.size());
  bo = val64;
  bo.get_little(val64);

#ifndef USE_OPENMP
  base_strm_mtx.lock();
  base_strm.write(reinterpret_cast<char *>(&val64), sizeof(val64));
  base_strm.write(file_entry.c_str(), file_entry.size());
  base_strm_mtx.unlock();
#else
#pragma omp critical
  {
    base_strm.write(reinterpret_cast<char *>(&val64), sizeof(val64));
    base_strm.write(file_entry.c_str(), file_entry.size());
  }
#endif
}

void
CreateCollection::threadFunc(const std::filesystem::path &need_to_parse)
{
  std::filesystem::path p = need_to_parse;
  std::filesystem::path resolved;
  std::string ext;
  if(std::filesystem::symlink_status(p).type()
     == std::filesystem::file_type::symlink)
    {
      resolved = std::filesystem::read_symlink(p);
      ext = af->get_extension(resolved);
    }
  else
    {
      ext = af->get_extension(p);
    }
  ext = af->stringToLower(ext);

  if(ext == ".fb2")
    {
      try
        {
          fb2_thread(p, resolved);
        }
      catch(MLException &er)
        {
          std::cout << er.what() << std::endl;
        }
      if(progress)
        {
          std::error_code ec;
          uintmax_t sz = std::filesystem::file_size(p, ec);
#ifndef USE_OPENMP
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator fb2: "
                        << ec.message() << std::endl;
            }
          else
            {
              current_bytes.store(current_bytes.load()
                                  + static_cast<double>(sz));
            }
          progress(current_bytes.load());
#else
          double cb_val;
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator fb2: "
                        << ec.message() << std::endl;
#pragma omp atomic read
              cb_val = current_bytes;
            }
          else
            {
#pragma omp atomic capture
              {
                current_bytes += static_cast<double>(sz);
                cb_val = current_bytes;
              }
            }
          progress(cb_val);
#endif
        }
    }
  else if(ext == ".epub")
    {
      try
        {
          epub_thread(p, resolved);
        }
      catch(MLException &er)
        {
          std::cout << er.what() << std::endl;
        }
      if(progress)
        {
          std::error_code ec;
          uintmax_t sz = std::filesystem::file_size(p, ec);
#ifndef USE_OPENMP
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator epub: "
                        << ec.message() << std::endl;
            }
          else
            {
              current_bytes.store(current_bytes.load()
                                  + static_cast<double>(sz));
            }
          progress(current_bytes.load());
#else
          double cb_val;
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator epub: "
                        << ec.message() << std::endl;
#pragma omp atomic read
              cb_val = current_bytes;
            }
          else
            {
#pragma omp atomic capture
              {
                current_bytes += static_cast<double>(sz);
                cb_val = current_bytes;
              }
            }
          progress(cb_val);
#endif
        }
    }
  else if(ext == ".pdf")
    {
      try
        {
          pdf_thread(p, resolved);
        }
      catch(MLException &er)
        {
          std::cout << er.what() << std::endl;
        }
      if(progress)
        {
          std::error_code ec;
          uintmax_t sz = std::filesystem::file_size(p, ec);
#ifndef USE_OPENMP
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator pdf: "
                        << ec.message() << std::endl;
            }
          else
            {
              current_bytes.store(current_bytes.load()
                                  + static_cast<double>(sz));
            }
          progress(current_bytes.load());
#else
          double cb_val;
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator pdf: "
                        << ec.message() << std::endl;
#pragma omp atomic read
              cb_val = current_bytes;
            }
          else
            {
#pragma omp atomic capture
              {
                current_bytes += static_cast<double>(sz);
                cb_val = current_bytes;
              }
            }
          progress(cb_val);
#endif
        }
    }
  else if(ext == ".djvu")
    {
      try
        {
          djvu_thread(p, resolved);
        }
      catch(MLException &er)
        {
          std::cout << er.what() << std::endl;
        }
      if(progress)
        {
          std::error_code ec;
          uintmax_t sz = std::filesystem::file_size(p, ec);
#ifndef USE_OPENMP
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator djvu: "
                        << ec.message() << std::endl;
            }
          else
            {
              current_bytes.store(current_bytes.load()
                                  + static_cast<double>(sz));
            }
          progress(current_bytes.load());
#else
          double cb_val;
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator djvu: "
                        << ec.message() << std::endl;
#pragma omp atomic read
              cb_val = current_bytes;
            }
          else
            {
#pragma omp atomic capture
              {
                current_bytes += static_cast<double>(sz);
                cb_val = current_bytes;
              }
            }
          progress(cb_val);
#endif
        }
    }
  else
    {
      try
        {
          arch_thread(p, resolved);
        }
      catch(MLException &er)
        {
          std::cout << er.what() << std::endl;
        }
      if(progress)
        {
          std::error_code ec;
          uintmax_t sz = std::filesystem::file_size(p, ec);
#ifndef USE_OPENMP
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator archive: "
                        << ec.message() << std::endl;
            }
          else
            {
              current_bytes.store(current_bytes.load()
                                  + static_cast<double>(sz));
            }
          progress(current_bytes.load());
#else
          double cb_val;
          if(ec)
            {
              std::cout << "CreateCollection::threadRegulator archive: "
                        << ec.message() << std::endl;
#pragma omp atomic read
              cb_val = current_bytes;
            }
          else
            {
#pragma omp atomic capture
              {
                current_bytes += static_cast<double>(sz);
                cb_val = current_bytes;
              }
            }
          progress(cb_val);
#endif
        }
    }
}

void
CreateCollection::djvu_thread(const std::filesystem::path &file_col_path,
                              const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  DJVUParser djvu(af);
#ifndef USE_OPENMP
  BookParseEntry be = djvu.djvu_parser(filepath);

  FileParseEntry fe;
  fe.file_rel_path = file_col_path.lexically_proximate(books_path).u8string();
  auto ithsh = std::find_if(
      already_hashed.begin(), already_hashed.end(),
      [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
        return std::get<0>(el) == file_col_path;
      });
  if(ithsh != already_hashed.end())
    {
      fe.file_hash = std::get<1>(*ithsh);
    }
  else
    {
      std::unique_lock<std::mutex> rthr_lock(run_threads_mtx);
      run_threads_var.wait(rthr_lock, [this] {
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
      rthr_lock.unlock();

      std::shared_ptr<int> thr_finish(&run_threads, [this](int *) {
        std::lock_guard<std::mutex> lglock(run_threads_mtx);
        run_threads--;
        run_threads_var.notify_one();
      });
      fe.file_hash = file_hashing(filepath);
    }
  if(!cancel.load())
    {
      fe.books.emplace_back(be);
      write_file_to_base(fe);
    }
#else
  BookParseEntry be = djvu.djvu_parser(filepath);
  FileParseEntry fe;
  fe.file_rel_path = file_col_path.lexically_proximate(books_path).u8string();
  auto ithsh = std::find_if(
      already_hashed.begin(), already_hashed.end(),
      [file_col_path](std::tuple<std::filesystem::path, std::string> &el) {
        return std::get<0>(el) == file_col_path;
      });
  if(ithsh != already_hashed.end())
    {
      fe.file_hash = std::get<1>(*ithsh);
    }
  else
    {
      fe.file_hash = file_hashing(filepath);
    }
  bool cncl;
#pragma omp atomic read
  cncl = cancel;
  if(!cncl)
    {
      fe.books.emplace_back(be);
      write_file_to_base(fe);
    }
#endif
}
