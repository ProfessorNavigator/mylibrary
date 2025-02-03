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

#ifndef ZIPFILEENTRY_H
#define ZIPFILEENTRY_H

#include <cstdint>
#include <string>

class ZipFileEntry
{
public:
  ZipFileEntry();

  ZipFileEntry(const ZipFileEntry &other);

  ZipFileEntry &
  operator=(const ZipFileEntry &other);

  ZipFileEntry(ZipFileEntry &&other);

  ZipFileEntry &
  operator=(ZipFileEntry &&other);

  uint64_t size = 0;
  uint64_t compressed_size = 0;
  int64_t position = -1;
  std::string filename;
};

#endif // ZIPFILEENTRY_H
