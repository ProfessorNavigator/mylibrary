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

#ifndef LIBARCHIVE_H
#define LIBARCHIVE_H

#include <ArchiveFileEntry.h>
#include <ArchiveRemoveEntry.h>
#include <ZipFileEntry.h>
#include <archive_entry.h>
#include <filesystem>
#include <memory>
#include <stddef.h>
#include <string>
#include <tuple>
#include <vector>

class LibArchive
{
public:
  LibArchive();

  std::filesystem::path
  unpackByPosition(const std::filesystem::path &archaddress,
                   const std::filesystem::path &outfolder,
                   const ZipFileEntry &entry);

  std::string
  unpackByPosition(const std::filesystem::path &archaddress,
                   const ZipFileEntry &entry);

  std::filesystem::path
  unpackByFileNameStream(const std::filesystem::path &archaddress,
                         const std::filesystem::path &outfolder,
                         const std::string &filename);

  std::string
  unpackByFileNameStreamStr(const std::filesystem::path &archaddress,
                            const std::string &filename);

  int
  fileNames(const std::filesystem::path &filepath,
            std::vector<ZipFileEntry> &filenames);

  int
  fileNamesStream(const std::filesystem::path &address,
                  std::vector<ZipFileEntry> &filenames);

  ZipFileEntry
  fileinfo(const std::filesystem::path &address, const std::string &filename);

  int
  libarchive_packing(const std::filesystem::path &sourcepath,
                     const std::filesystem::path &outpath);

  int
  libarchive_packing(const std::shared_ptr<archive> &a,
                     const std::filesystem::path &sourcepath,
                     const bool &rename_source,
                     const std::string &new_source_name);

  ArchiveRemoveEntry
  libarchive_remove_init(const std::filesystem::path &sourcepath,
                         const std::filesystem::path &outpath);

  void
  libarchive_error(const std::shared_ptr<archive> &a,
                   const std::string &message, const int &error_number);

  std::string
  libarchive_read_entry_str(archive *a, archive_entry *entry);

  int
  libarchive_write_data(archive *a, const std::string &data);

  std::shared_ptr<ArchiveFileEntry>
  createArchFile(const std::filesystem::path &archaddress,
                 const la_int64_t &position);

  std::shared_ptr<archive>
  libarchive_read_init(std::shared_ptr<ArchiveFileEntry> fl);

  std::shared_ptr<archive>
  libarchive_read_init_fallback(std::shared_ptr<ArchiveFileEntry> fl);

  std::filesystem::path
  libarchive_read_entry(archive *a, archive_entry *entry,
                        const std::filesystem::path &outfolder);

  std::shared_ptr<archive>
  libarchive_write_init(const std::filesystem::path &outpath);

  int
  libarchive_write_directory(archive *a, archive_entry *entry,
                             const std::filesystem::path &path_in_arch,
                             const std::filesystem::path &source);

  int
  libarchive_write_file(archive *a, archive_entry *entry,
                        const std::filesystem::path &path_in_arch,
                        const std::filesystem::path &source);

  int
  libarchive_write_data_from_file(archive *a,
                                  const std::filesystem::path &source);

private:
  static int
  libarchive_open_callback(archive *a, void *data);

  static la_ssize_t
  libarchive_read_callback(archive *a, void *data, const void **buffer);

  static la_int64_t
  libarchive_skip_callback(archive *a, void *data, la_int64_t request);

  static la_int64_t
  libarchive_seek_callback(archive *a, void *data, la_int64_t offset,
                           int whence);

  static int
  libarchive_close_callback(archive *a, void *data);

  int
  write_func(archive *a, const std::filesystem::path &source,
             const std::filesystem::path &path_in_arch);

  std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
  dir_symlink_resolver(const std::filesystem::path &source,
                       const std::filesystem::path &append_to);

  static la_ssize_t
  libarchive_write_callback(archive *a, void *data, const void *buffer,
                            size_t length);

  static int
  libarchive_free_callback(archive *a, void *data);

  static int
  libarchive_open_callback_write(archive *a, void *data);
};

#endif // LIBARCHIVE_H
