/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CREATECOLLECTION_H
#define CREATECOLLECTION_H

#include <AuxFunc.h>
#include <FileParseEntry.h>
#include <Hasher.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#ifndef USE_OPENMP
#include <condition_variable>
#endif

class CreateCollection : public Hasher
{
public:
  CreateCollection(const std::shared_ptr<AuxFunc> &af,
                   const std::filesystem::path &collection_path,
                   const std::filesystem::path &books_path,
                   const bool &rar_support, const int &num_threads,
                   std::atomic<bool> *cancel);

  virtual ~CreateCollection();

  void
  createCollection();

  std::function<void()> pulse;

  std::function<void(const double &)> signal_total_bytes;

  std::function<void(const double &progress)> progress;

protected:
  CreateCollection(const std::shared_ptr<AuxFunc> &af, const int &num_threads,
                   std::atomic<bool> *cancel);

  void
  threadRegulator();

  void
  openBaseFile();

  void
  closeBaseFile();

  void
  write_file_to_base(const FileParseEntry &fe);

  std::filesystem::path base_path;
  std::filesystem::path books_path;
  bool rar_support = false;
  std::vector<std::tuple<std::filesystem::path, std::string>> already_hashed;

  std::vector<std::filesystem::path> need_to_parse;

  std::atomic<double> current_bytes;

private:
  void
  fb2_thread(const std::filesystem::path &file_col_path,
             const std::filesystem::path &resolved);

  void
  epub_thread(const std::filesystem::path &file_col_path,
              const std::filesystem::path &resolved);

  void
  pdf_thread(const std::filesystem::path &file_col_path,
             const std::filesystem::path &resolved);

  void
  djvu_thread(const std::filesystem::path &file_col_path,
              const std::filesystem::path &resolved);

  void
  arch_thread(const std::filesystem::path &file_col_path,
              const std::filesystem::path &resolved);

  void
  book_entry_to_file_entry(std::string &file_entry,
                           const std::string &book_entry);

  std::shared_ptr<AuxFunc> af;
  int num_threads = 1;
  std::atomic<bool> *cancel = nullptr;

  std::fstream base_strm;
  std::mutex base_strm_mtx;  

#ifndef USE_OPENMP
  std::mutex newthrmtx;
  std::condition_variable add_thread;
  int run_threads = 0;
#endif
};

#endif // CREATECOLLECTION_H
