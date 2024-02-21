/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_ARCHIVEFILEENTRY_H_
#define INCLUDE_ARCHIVEFILEENTRY_H_

#include <archive_entry.h>
#include <filesystem>
#include <fstream>

class ArchiveFileEntry
{
public:
  ArchiveFileEntry();
  virtual
  ~ArchiveFileEntry();

  std::fstream file;
  std::filesystem::path file_path;
  la_ssize_t buf_sz = 1048576;
  la_ssize_t read_bytes = 0;
  la_ssize_t file_size = 0;
  char *read_buf = nullptr;
};

#endif /* INCLUDE_ARCHIVEFILEENTRY_H_ */
