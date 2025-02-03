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

#ifndef BOOKBASEENTRY_H
#define BOOKBASEENTRY_H

#include <BookParseEntry.h>
#include <filesystem>

class BookBaseEntry
{
public:
  BookBaseEntry();

  BookBaseEntry(const BookBaseEntry &other);

  BookBaseEntry(BookBaseEntry &&other);

  BookBaseEntry &
  operator=(const BookBaseEntry &other);

  BookBaseEntry &
  operator=(BookBaseEntry &&other);

  bool
  operator==(const BookBaseEntry &other);

  BookBaseEntry(const BookParseEntry &bpe,
                const std::filesystem::path &book_file_path);

  std::filesystem::path file_path;
  BookParseEntry bpe;
};

#endif // BOOKBASEENTRY_H
