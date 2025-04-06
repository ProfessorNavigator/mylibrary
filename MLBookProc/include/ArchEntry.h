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

#ifndef ZIPFILEENTRY_H
#define ZIPFILEENTRY_H

#include <cstdint>
#include <string>

/*!
 * \brief The ArchEntry class
 *
 * Auxiliary class for LibArchive. Contains some technical info about
 * compressed files and directories. In most cases you do not need to create
 * AcrhEntry objects yourself.
 */
class ArchEntry
{
public:
  /*!
   * \brief ArchEntry constructor.
   */
  ArchEntry();

  /*!
   * \brief ArchEntry copy constructor.
   */
  ArchEntry(const ArchEntry &other);  

  /*!
   * \brief ArchEntry move constructor.
   */
  ArchEntry(ArchEntry &&other);

  /*!
   * \brief operator =
   */
  ArchEntry &
  operator=(const ArchEntry &other);

  /*!
   * \brief operator =
   */
  ArchEntry &
  operator=(ArchEntry &&other);

  /*!
   * \brief Size of unpacked entry object (if available, 0 otherwise).
   */
  uint64_t size = 0;

  /*!
   * \brief Size of compressed object (if available, 0 otherwise).
   */
  uint64_t compressed_size = 0;

  /*!
   * \brief Position of entry in archive file (if available, -1 otherwise).
   */
  int64_t position = -1;

  /*!
   * \brief Path to file or directory in archive.
   */
  std::string filename;
};

#endif // ZIPFILEENTRY_H
