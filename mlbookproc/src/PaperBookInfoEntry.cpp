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

#include <PaperBookInfoEntry.h>

PaperBookInfoEntry::PaperBookInfoEntry()
{
}

PaperBookInfoEntry::PaperBookInfoEntry(const PaperBookInfoEntry &other)
{
  available = other.available;
  book_name = other.book_name;
  publisher = other.publisher;
  city = other.city;
  year = other.year;
  isbn = other.isbn;
}

PaperBookInfoEntry::PaperBookInfoEntry(PaperBookInfoEntry &&other)
{
  available = std::move(other.available);
  book_name = std::move(other.book_name);
  publisher = std::move(other.publisher);
  city = std::move(other.city);
  year = std::move(other.year);
  isbn = std::move(other.isbn);
}

PaperBookInfoEntry &
PaperBookInfoEntry::operator=(const PaperBookInfoEntry &other)
{
  if(this != &other)
    {
      available = other.available;
      book_name = other.book_name;
      publisher = other.publisher;
      city = other.city;
      year = other.year;
      isbn = other.isbn;
    }
  return *this;
}

PaperBookInfoEntry &
PaperBookInfoEntry::operator=(PaperBookInfoEntry &&other)
{
  if(this != &other)
    {
      available = std::move(other.available);
      book_name = std::move(other.book_name);
      publisher = std::move(other.publisher);
      city = std::move(other.city);
      year = std::move(other.year);
      isbn = std::move(other.isbn);
    }
  return *this;
}
