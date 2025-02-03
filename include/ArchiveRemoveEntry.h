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

#ifndef ARCHIVEREMOVEENTRY_H
#define ARCHIVEREMOVEENTRY_H

#include <ArchiveFileEntry.h>
#include <archive.h>
#include <memory>

class ArchiveRemoveEntry
{
public:
  ArchiveRemoveEntry();

  virtual ~ArchiveRemoveEntry();

  ArchiveRemoveEntry(const ArchiveRemoveEntry &other);

  ArchiveRemoveEntry(ArchiveRemoveEntry &&other);

  ArchiveRemoveEntry &
  operator=(const ArchiveRemoveEntry &other);

  ArchiveRemoveEntry &
  operator=(ArchiveRemoveEntry &&other);

  std::shared_ptr<archive> a_read;
  std::shared_ptr<archive> a_write;
  std::shared_ptr<ArchiveFileEntry> fl;
};

#endif // ARCHIVEREMOVEENTRY_H
