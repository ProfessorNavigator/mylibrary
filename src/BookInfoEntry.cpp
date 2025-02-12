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

#include <BookInfoEntry.h>

BookInfoEntry::BookInfoEntry()
{
  paper = new PaperBookInfoEntry;
  electro = new ElectroBookInfoEntry;
}

BookInfoEntry::~BookInfoEntry()
{
  delete paper;
  delete electro;
}

BookInfoEntry::BookInfoEntry(BookInfoEntry &&other)
{  
  annotation = std::move(other.annotation);
  cover = std::move(other.cover);
  cover_type = std::move(other.cover_type);
  language = std::move(other.language);
  src_language = std::move(other.src_language);
  translator = std::move(other.translator);
  paper = other.paper;
  other.paper = nullptr;
  electro = other.electro;
  other.electro = nullptr;
  bytes_per_row = other.bytes_per_row;
}

BookInfoEntry::BookInfoEntry(const BookInfoEntry &other)
{
  annotation = other.annotation;
  cover = other.cover;
  cover_type = other.cover_type;
  language = other.language;
  src_language = other.src_language;
  translator = other.translator;
  paper = other.paper;
  electro = other.electro;
  bytes_per_row = other.bytes_per_row;
}

BookInfoEntry &
BookInfoEntry::operator=(const BookInfoEntry &other)
{
  if(this != &other)
    {
      annotation = other.annotation;
      cover = other.cover;
      cover_type = other.cover_type;
      language = other.language;
      src_language = other.src_language;
      translator = other.translator;
      paper = other.paper;
      electro = other.electro;
      bytes_per_row = other.bytes_per_row;
    }
  return *this;
}

BookInfoEntry &
BookInfoEntry::operator=(BookInfoEntry &&other)
{
  if(this != &other)
    {
      annotation = std::move(other.annotation);
      cover = std::move(other.cover);
      cover_type = std::move(other.cover_type);
      language = std::move(other.language);
      src_language = std::move(other.src_language);
      translator = std::move(other.translator);
      paper = other.paper;
      other.paper = nullptr;
      electro = other.electro;
      other.electro = nullptr;
      bytes_per_row = other.bytes_per_row;
    }
  return *this;
}
