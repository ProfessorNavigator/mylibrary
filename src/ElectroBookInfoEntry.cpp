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

#include <ElectroBookInfoEntry.h>

ElectroBookInfoEntry::ElectroBookInfoEntry()
{
}

ElectroBookInfoEntry::ElectroBookInfoEntry(const ElectroBookInfoEntry &other)
{
  available = other.available;
  author = other.author;
  program_used = other.program_used;
  date = other.date;
  src_url = other.src_url;
  src_ocr = other.src_ocr;
  id = other.id;
  version = other.version;
  history = other.version;
  publisher = other.publisher;
}

ElectroBookInfoEntry::ElectroBookInfoEntry(ElectroBookInfoEntry &&other)
{
  available = other.available;
  author = other.author;
  program_used = other.program_used;
  date = other.date;
  src_url = other.src_url;
  src_ocr = other.src_ocr;
  id = other.id;
  version = other.version;
  history = other.version;
  publisher = other.publisher;
}

ElectroBookInfoEntry &
ElectroBookInfoEntry::operator=(const ElectroBookInfoEntry &other)
{
  if(this != &other)
    {
      available = other.available;
      author = other.author;
      program_used = other.program_used;
      date = other.date;
      src_url = other.src_url;
      src_ocr = other.src_ocr;
      id = other.id;
      version = other.version;
      history = other.version;
      publisher = other.publisher;
    }
  return *this;
}

ElectroBookInfoEntry &
ElectroBookInfoEntry::operator=(ElectroBookInfoEntry &&other)
{
  if(this != &other)
    {
      available = other.available;
      author = other.author;
      program_used = other.program_used;
      date = other.date;
      src_url = other.src_url;
      src_ocr = other.src_ocr;
      id = other.id;
      version = other.version;
      history = other.version;
      publisher = other.publisher;
    }
  return *this;
}
