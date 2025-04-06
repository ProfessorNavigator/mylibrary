/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <ArchEntry.h>

ArchEntry::ArchEntry()
{
}

ArchEntry::ArchEntry(const ArchEntry &other)
{
  size = other.size;
  compressed_size = other.compressed_size;
  position = other.position;
  filename = other.filename;
}

ArchEntry &
ArchEntry::operator=(const ArchEntry &other)
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

ArchEntry::ArchEntry(ArchEntry &&other)
{
  size = other.size;
  compressed_size = other.compressed_size;
  position = other.position;
  filename = std::move(other.filename);
}

ArchEntry &
ArchEntry::operator=(ArchEntry &&other)
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
