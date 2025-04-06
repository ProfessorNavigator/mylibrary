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

#ifndef FILEPARSEENTRY_H
#define FILEPARSEENTRY_H

#include <BookParseEntry.h>
#include <string>
#include <vector>

/*!
 * \brief The FileParseEntry class.
 *
 * Auxiliary class, used in CreateCollection, RefreshCollection and BaseKeeper.
 * Contains collection file info.
 */
class FileParseEntry
{
public:
  /*!
   * \brief FileParseEntry constructor.
   */
  FileParseEntry();

  /*!
   * \brief FileParseEntry copy constructor.
   */
  FileParseEntry(const FileParseEntry &other);

  /*!
   * \brief FileParseEntry move constructor.
   */
  FileParseEntry(FileParseEntry &&other);

  /*!
   * \brief operator =
   */
  FileParseEntry &
  operator=(const FileParseEntry &other);

  /*!
   * \brief operator =
   */
  FileParseEntry &
  operator=(FileParseEntry &&other);

  /*!
   * \brief Relative path to file in collection.
   *
   * Path is relative to collection directory path.
   */
  std::string file_rel_path;

  /*!
   * \brief File hash sum.
   *
   * Blake-256 algorithm currently used.
   */
  std::string file_hash;

  /*!
   * \brief Contains books info.
   *
   * Vector containing information about books in file.
   */
  std::vector<BookParseEntry> books;
};

#endif // FILEPARSEENTRY_H
