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

#ifndef ADDBOOK_H
#define ADDBOOK_H

#include <AuxFunc.h>
#include <BookMarks.h>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

class AddBook
{
public:
  AddBook(const std::shared_ptr<AuxFunc> &af,
          const std::string &collection_name, const bool &remove_sources,
          const std::shared_ptr<BookMarks> &bookmarks);

  void
  simple_add(const std::vector<
             std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  void
  simple_add_dir(const std::vector<std::tuple<std::filesystem::path,
                                              std::filesystem::path>> &books);

  void
  overwrite_archive(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  void
  overwrite_archive_dir(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  void
  add_to_existing_archive(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  void
  add_to_existing_archive_dir(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  static std::vector<std::string>
  archive_filenames(const std::filesystem::path &archive_path);

private:
  void
  remove_src(const std::vector<
             std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  std::shared_ptr<AuxFunc> af;
  std::string collection_name;
  bool remove_sources = false;
  std::shared_ptr<BookMarks> bookmarks;
};

#endif // ADDBOOK_H
