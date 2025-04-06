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

#ifndef ARCHIVEREMOVEENTRY_H
#define ARCHIVEREMOVEENTRY_H

#include <ArchiveFileEntry.h>
#include <archive.h>
#include <memory>

/*!
 * \brief The ArchiveRemoveEntry class.
 *
 * Auxiliary class for LibArchive, to be used in case of removing files from
 * archives. In most cases you do not need to use it directly.
 */
class ArchiveRemoveEntry
{
public:
  /*!
   * \brief ArchiveRemoveEntry constructor.
   */
  ArchiveRemoveEntry();

  /*!
   * \brief ArchiveRemoveEntry destructor.
   */
  virtual ~ArchiveRemoveEntry();

  /*!
   * \brief ArchiveRemoveEntry copy constructor.
   */
  ArchiveRemoveEntry(const ArchiveRemoveEntry &other);

  /*!
   * \brief ArchiveRemoveEntry move constructor.
   */
  ArchiveRemoveEntry(ArchiveRemoveEntry &&other);

  /*!
   * \brief operator =
   */
  ArchiveRemoveEntry &
  operator=(const ArchiveRemoveEntry &other);

  /*!
   * \brief operator =
   */
  ArchiveRemoveEntry &
  operator=(ArchiveRemoveEntry &&other);

  /*!
   * \brief libarchive object, file to be removed from.
   */
  std::shared_ptr<archive> a_read;

  /*!
   * \brief libarchive object for new archive.
   */
  std::shared_ptr<archive> a_write;

  /*!
   * \brief ArchiveFileEntry object, file to be removed from.
   */
  std::shared_ptr<ArchiveFileEntry> fl;
};

#endif // ARCHIVEREMOVEENTRY_H
