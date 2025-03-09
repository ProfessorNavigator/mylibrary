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

#ifndef REMOVEBOOK_H
#define REMOVEBOOK_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <LibArchive.h>
#include <SelfRemovingPath.h>
#include <filesystem>
#include <memory>
#include <string>

class RemoveBook : public LibArchive
{
public:
  RemoveBook(const std::shared_ptr<AuxFunc> &af, const BookBaseEntry &bbe,
             const std::string &col_name,
             const std::shared_ptr<BookMarks> &bookmarks);

  void
  removeBook();

private:
  std::filesystem::path
  archive_remove(const SelfRemovingPath &out_dir);

  std::shared_ptr<AuxFunc> af;
  BookBaseEntry bbe;
  std::string col_name;
  std::shared_ptr<BookMarks> bookmarks;

  SelfRemovingPath keep_path;
};

#endif // REMOVEBOOK_H
