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

#include <ZipFileEntry.h>

ZipFileEntry::ZipFileEntry()
{
}

ZipFileEntry::ZipFileEntry(const ZipFileEntry &other)
{
  size = other.size;
  compressed_size = other.compressed_size;
  position = other.position;
  filename = other.filename;
}

ZipFileEntry &
ZipFileEntry::operator=(const ZipFileEntry &other)
{
  if(this != &other)
    {
      size = other.size;
      compressed_size = other.compressed_size;
      position = other.position;
      filename = other.filename;
    }
  return *this;
}

ZipFileEntry::ZipFileEntry(ZipFileEntry &&other)
{
  size = other.size;
  compressed_size = other.compressed_size;
  position = other.position;
  filename = std::move(other.filename);
}

ZipFileEntry &
ZipFileEntry::operator=(ZipFileEntry &&other)
{
  if(this != &other)
    {
      size = other.size;
      compressed_size = other.compressed_size;
      position = other.position;
      filename = std::move(other.filename);
    }
  return *this;
}
