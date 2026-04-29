/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <LibArchiveFileData.h>

LibArchiveFileData::LibArchiveFileData()
{  
  file_size = 0;
  buffer_size = 4194304;
  buffer = new char[buffer_size];
  start_offset = 0;
}

LibArchiveFileData::~LibArchiveFileData()
{  
  delete[] buffer;
}
