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

#ifndef PAPERBOOKINFOENTRY_H
#define PAPERBOOKINFOENTRY_H

#include <string>

class PaperBookInfoEntry
{
public:
  PaperBookInfoEntry();

  PaperBookInfoEntry(const PaperBookInfoEntry &other);

  PaperBookInfoEntry(PaperBookInfoEntry &&other);

  PaperBookInfoEntry &
  operator=(const PaperBookInfoEntry &other);

  PaperBookInfoEntry &
  operator=(PaperBookInfoEntry &&other);

  bool available = false;
  std::string book_name;
  std::string publisher;
  std::string city;
  std::string year;
  std::string isbn;
};

#endif // PAPERBOOKINFOENTRY_H
