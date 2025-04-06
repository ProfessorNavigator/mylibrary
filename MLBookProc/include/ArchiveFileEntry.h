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

#ifndef ARCHIVEFILEENTRY_H
#define ARCHIVEFILEENTRY_H

#include <archive_entry.h>
#include <filesystem>
#include <fstream>

/*!
 * \brief The ArchiveFileEntry class.
 *
 * Auxiliary class for LibArchive methods. In most cases you do not need to
 * create it directly, use LibArchive::createArchFile() method instead.
 */
class ArchiveFileEntry
{
public:
  /*!
   * \brief ArchiveFileEntry constructor.
   */
  ArchiveFileEntry();

  /*!
   * \brief ArchiveFileEntry destructor.
   */
  virtual ~ArchiveFileEntry();

  /*!
   * \brief Archive file stream
   */
  std::fstream file;

  /*!
   * \brief Archive file absolute path
   */
  std::filesystem::path file_path;

  /*!
   * \brief Size of buffer to be used in file stream
   */
  la_ssize_t buf_sz = 1048576;

  /*!
   * \brief Number of bites already read from archive file.
   */
  la_ssize_t read_bytes = 0;

  /*!
   * \brief Archive file size
   */
  la_ssize_t file_size = 0;

  /*!
   * \brief Buffer to be used in file stream
   */
  char *read_buf = nullptr;
};

#endif // ARCHIVEFILEENTRY_H
