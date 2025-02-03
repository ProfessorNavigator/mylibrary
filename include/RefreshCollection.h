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

#ifndef REFRESHCOLLECTION_H
#define REFRESHCOLLECTION_H

#include <AuxFunc.h>
#include <BaseKeeper.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <CreateCollection.h>
#include <FileParseEntry.h>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class RefreshCollection : public CreateCollection
{
public:
  RefreshCollection(const std::shared_ptr<AuxFunc> &af,
                    const std::string &collection_name, const int &num_threads,
                    std::atomic<bool> *cancel, const bool &remove_empty,
                    const bool &fast_refresh, const bool &refresh_bookmarks,
                    const std::shared_ptr<BookMarks> &bookmarks);

  std::function<void(const double &total_hash)> total_bytes_to_hash;

  std::function<void(const double &hashed)> bytes_hashed;

  void
  refreshCollection();

  void
  refreshFile(const BookBaseEntry &bbe);

  bool
  editBook(const BookBaseEntry &bbe_old, const BookBaseEntry &bbe_new);

  bool
  refreshBook(const BookBaseEntry &bbe);

  void
  set_rar_support(const bool &rar_support);

private:
  std::filesystem::path
  get_base_path(const std::string &collection_name);

  std::filesystem::path
  get_books_path();

  void
  compaire_vectors(std::vector<FileParseEntry> &base,
                   std::vector<std::filesystem::path> &books_files);

  bool
  compare_function1(const std::filesystem::path &book_path,
                    const FileParseEntry &ent);

  bool
  compare_function2(const FileParseEntry &ent,
                    const std::filesystem::path &book_path);

  void
  check_hashes(std::vector<FileParseEntry> *base,
               std::vector<std::filesystem::path> *books_files);

  void
  hash_thread(const std::filesystem::path &file_to_hash,
              std::vector<FileParseEntry> *base);

  void
  refreshBookMarks(const std::shared_ptr<BaseKeeper> &bk);

  std::shared_ptr<AuxFunc> af;
  int num_threads = 1;
  std::atomic<bool> *cancel = nullptr;
  bool remove_empty = false;
  bool fast_refresh = false;
  bool refresh_bookmarks = false;
  std::shared_ptr<BookMarks> bookmarks;

  std::atomic<uintmax_t> bytes_summ;

  std::mutex already_hashedmtx;
  std::mutex need_to_parsemtx;
  std::mutex basemtx;

#ifndef USE_OPENMP
  std::mutex newthrmtx;
  std::condition_variable continue_hashing;
  int run_threads = 0;
#endif
};

#endif // REFRESHCOLLECTION_H
