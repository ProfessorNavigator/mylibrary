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

#ifndef INCLUDE_FILEPARSEENTRY_H_
#define INCLUDE_FILEPARSEENTRY_H_

#include <BookParseEntry.h>
#include <string>
#include <vector>

class FileParseEntry
{
public:
  FileParseEntry();
  virtual
  ~FileParseEntry();

  FileParseEntry(const FileParseEntry &other);

  FileParseEntry(FileParseEntry &&other);

  FileParseEntry&
  operator=(const FileParseEntry &other);

  FileParseEntry&
  operator=(FileParseEntry &&other);

  std::string file_rel_path;
  std::string file_hash;
  std::vector<BookParseEntry> books;
};

#endif /* INCLUDE_FILEPARSEENTRY_H_ */
