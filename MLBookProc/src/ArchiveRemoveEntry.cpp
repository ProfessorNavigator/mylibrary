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

#include <ArchiveRemoveEntry.h>

ArchiveRemoveEntry::ArchiveRemoveEntry()
{
}

ArchiveRemoveEntry::~ArchiveRemoveEntry()
{
  a_read.reset();
  a_write.reset();
}

ArchiveRemoveEntry::ArchiveRemoveEntry(const ArchiveRemoveEntry &other)
{
  a_read = other.a_read;
  a_write = other.a_write;
  fl = other.fl;
}

ArchiveRemoveEntry::ArchiveRemoveEntry(ArchiveRemoveEntry &&other)
{
  a_read = std::move(other.a_read);
  a_write = std::move(other.a_write);
  fl = std::move(other.fl);
}

ArchiveRemoveEntry &
ArchiveRemoveEntry::operator=(const ArchiveRemoveEntry &other)
{
  if(this != &other)
    {
      a_read = other.a_read;
      a_write = other.a_write;
      fl = other.fl;
    }
  return *this;
}

void
ArchiveRemoveEntry::reset()
{
  a_read.reset();
  a_write.reset();
  fl.reset();
}

ArchiveRemoveEntry &
ArchiveRemoveEntry::operator=(ArchiveRemoveEntry &&other)
{
  if(this != &other)
    {
      a_read = std::move(other.a_read);
      a_write = std::move(other.a_write);
      fl = std::move(other.fl);
    }
  return *this;
}
