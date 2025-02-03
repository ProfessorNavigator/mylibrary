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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <memory>
#include <mutex>
#include <vector>

class BookMarks
{
public:
  BookMarks(const std::shared_ptr<AuxFunc> &af);

  virtual ~BookMarks();

  int
  createBookMark(const BookBaseEntry &bbe);

  std::vector<BookBaseEntry>
  getBookMarks();

  void
  removeBookMark(const BookBaseEntry &bbe);

private:
  void
  loadBookMarks();

  BookBaseEntry
  parse_entry(std::string &buf);

  bool
  saveBookMarks();

  std::string
  form_entry(const BookBaseEntry &bbe);

  std::shared_ptr<AuxFunc> af;

  std::filesystem::path bookmp;

  std::vector<BookBaseEntry> bookmarks;
  std::mutex bookmarksmtx;
};

#endif // BOOKMARKS_H
