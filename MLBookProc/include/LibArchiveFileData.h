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
#ifndef LIBARCHIVEFILEDATA_H
#define LIBARCHIVEFILEDATA_H

#include <filesystem>
#include <istream>
#include <memory>

/*!
 * \brief The LibArchiveFileData class
 *
 * Auxiliary class for LibArchive.
 */
class LibArchiveFileData
{
public:
  LibArchiveFileData();

  virtual ~LibArchiveFileData();

  /*!
   * Smart pointer to file or buffer stream.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::shared_ptr<std::iostream> f;

  /*!
   * File or buffer size.
   *
   * \warning Do not set or modify this object yourself.
   */
  size_t file_size;

  /*!
   * File or buffer stream open mode. Should be set manually.
   */
  std::ios_base::openmode open_mode;

  /*!
   * Path to file to be opened. If empty, #source_buffer will be used instead.
   * Set it in case of need.
   */
  std::filesystem::path path;

  /*!
   * Buffer to be opened in stream. Set it in case of need.
   */
  std::string source_buffer;

  /*!
   * Pointer to inner buffer.
   *
   * \warning Do not set or modify this object yourself.
   */
  char *buffer;

  /*!
   * Size of #buffer.
   *
   * \warning Do not set or modify this object yourself.
   */
  size_t buffer_size;

  /*!
   * Byte number buffer or file reading should be started from. Default value
   * is \a 0.
   *
   * Set it in case of need.
   */
  size_t start_offset;
};

#endif // LIBARCHIVEFILEDATA_H
