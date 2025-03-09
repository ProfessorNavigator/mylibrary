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

#ifndef BOOKPARSEENTRY_H
#define BOOKPARSEENTRY_H

#include <string>

class BookParseEntry
{
public:
  BookParseEntry();

  BookParseEntry(const BookParseEntry &other);

  BookParseEntry(BookParseEntry &&other);

  BookParseEntry &
  operator=(const BookParseEntry &other);

  BookParseEntry &
  operator=(BookParseEntry &&other);

  bool
  operator==(const BookParseEntry &other);

  std::string book_path;
  std::string book_author;
  std::string book_name;
  std::string book_series;
  std::string book_genre;
  std::string book_date;
};

#endif // BOOKPARSEENTRY_H
