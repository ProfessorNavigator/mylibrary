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

#include <FileParseEntry.h>

FileParseEntry::FileParseEntry()
{
}

FileParseEntry::FileParseEntry(const FileParseEntry &other)
{
  file_rel_path = other.file_rel_path;
  file_hash = other.file_hash;
  books = other.books;
}

FileParseEntry &
FileParseEntry::operator=(const FileParseEntry &other)
{
  if(this != &other)
    {
      file_rel_path = other.file_rel_path;
      file_hash = other.file_hash;
      books = other.books;
    }
  return *this;
}

FileParseEntry::FileParseEntry(FileParseEntry &&other)
{
  file_rel_path = std::move(other.file_rel_path);
  file_hash = std::move(other.file_hash);
  books = std::move(other.books);
}

FileParseEntry &
FileParseEntry::operator=(FileParseEntry &&other)
{
  if(this != &other)
    {
      file_rel_path = std::move(other.file_rel_path);
      file_hash = std::move(other.file_hash);
      books = std::move(other.books);
    }
  return *this;
}
