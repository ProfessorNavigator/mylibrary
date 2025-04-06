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

#ifndef LIBARCHIVE_H
#define LIBARCHIVE_H

#include <ArchEntry.h>
#include <ArchiveFileEntry.h>
#include <ArchiveRemoveEntry.h>
#include <AuxFunc.h>
#include <archive_entry.h>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

/*!
 * \brief The LibArchive class.
 *
 * This class contains various methods for archives processing. Based on
 * <A HREF="https://libarchive.org/">libarchive</A> library.
 */
class LibArchive
{
public:
  /*!
   * \brief LibArchive constructor.
   * \param af smart pointer to AuxFunc object.
   */
  LibArchive(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Unpacks single entry content from zip archive.
   *
   * Access to entry is carried out by its absolute position in zip file. It is
   * recommended to use this method for fast unpacking of single file or
   * directory from zip archive.
   * \warning This method should be used for zip archives only!
   * \param archaddress absolute path to zip archive.
   * \param outfolder absolute path to directory archive entry content to be
   * unpacked to. If directory does not exist, it will be created.
   * \param entry ArchEntry object, obtained by fileNames(),
   * fileNamesStream() or fileinfo() methods.
   * \return Absolute path to unpacked file or directory.
   */
  std::filesystem::path
  unpackByPosition(const std::filesystem::path &archaddress,
                   const std::filesystem::path &outfolder,
                   const ArchEntry &entry);

  /*!
   * \brief Unpacks single entry content from zip archive.
   *
   * If entry is a file, unpacks it and returns file content. If entry is a
   * directory, returns empty string. Access to entry is carried
   * out by its absolute position in zip file. It is recommended to use this
   * method for fast unpacking of single file from zip archive.
   * \warning This method should be used for zip archives only!
   * \param archaddress absolute path to zip archive.
   * \param entry ArchEntry object, obtained by fileNames(),
   * fileNamesStream() or fileinfo() methods.
   * \return Unpacked file content or empty string.
   */
  std::string
  unpackByPositionStr(const std::filesystem::path &archaddress,
                      const ArchEntry &entry);

  /*!
   * \brief Unpacks entry content from archive.
   *
   * This method is suitable for any supported types of archives (see
   * AuxFunc::get_supported_archive_types_unpacking()). However for zip
   * archives it is recommended to use unpackByPosition() and
   * unpackByPositionStr() methods (they are a little bit faster).
   * \param archaddress absolute path to archive.
   * \param outfolder absolute path to directory, entry to be unpacked to. If
   * directory does not exist, it will be created.
   * \param filename file or directory name in archive.
   * \return Absolute path to unpacked file or directory.
   */
  std::filesystem::path
  unpackByFileNameStream(const std::filesystem::path &archaddress,
                         const std::filesystem::path &outfolder,
                         const std::string &filename);

  /*!
   * \brief Unpacks entry content from archive.
   *
   * If entry is a file, unpacks it and returns file content. If entry is a
   * directory, returns empty string. This method is suitable for
   * any supported types of archives (see
   * AuxFunc::get_supported_archive_types_unpacking()). However for zip
   * archives it is recommended to use unpackByPosition() end
   * unpackByPositionStr() methods (they are a little bit faster).
   * \param archaddress absolute path to archive.
   * \param filename file or directory name in archive.
   * \return Unpacked file content or empty string.
   */
  std::string
  unpackByFileNameStreamStr(const std::filesystem::path &archaddress,
                            const std::string &filename);

  /*!
   * \brief Lists all entries in archive file.
   *
   * \warning This method is suitable for zip archives only. For other archive
   * types behavior is undefined.
   * \param filepath absolute path to zip archive.
   * \param filenames vector for results.
   * \return 1 in case of success, -1 in case of error, 0 in case archive file
   * has not been opened.
   */
  int
  fileNames(const std::filesystem::path &filepath,
            std::vector<ArchEntry> &filenames);

  /*!
   * \brief Lists all entries in archive file.
   *
   * This method can be used with all supported archive types (see
   * AuxFunc::get_supported_archive_types_unpacking()). However for zip
   * archives it is recommended to use fileNames() method.
   * \param address absolute path to archive.
   * \param filenames vector of results.
   * \return in case of succes returns ARCHIVE_OK, libarchive error codes will
   * be returned otherwise (see archive.h file for details).
   */
  int
  fileNamesStream(const std::filesystem::path &address,
                  std::vector<ArchEntry> &filenames);

  /*!
   * \brief Returns ArchEntry for particular file or directory in archive.
   *
   * In case of any error ArchEntry filename will be empty.
   * \param address absolute path to archive.
   * \param filename path in archive.
   * \return ArchEntry object.
   */
  ArchEntry
  fileinfo(const std::filesystem::path &address, const std::string &filename);

  /*!
   * \brief Packs file or directory into archive.
   * \param sourcepath absolute path to file or directory to be packed.
   * \param outpath absolute path to resulting archive.
   * \return -100 in case if sourcepath does not exist, -200 in case of error
   * on libarchive object creation, ARCHIVE_OK in case of success, libarchive
   * error code otherwise (see libarchive archive.h file for details).
   */
  int
  libarchive_packing(const std::filesystem::path &sourcepath,
                     const std::filesystem::path &outpath);

  /*!
   * \brief Packs file or directory into archive.
   *
   * Use this method if you need source file or directory to be packed under
   * another name.
   * \param a smart pointer to libarchive object (see libarchive_write_init()).
   * \param sourcepath absolute path to file or directory to be packed.
   * \param rename_source if set to \a true, source name will be replaced for
   * \b new_source_name inside the archive.
   * \param new_source_name new name to be used inside the archive. Should be
   * UTF-8 string.
   * \return -100 in case if sourcepath does not exist, -200 in case of error
   * on libarchive object creation, ARCHIVE_OK in case of success, libarchive
   * error code otherwise (see libarchive archive.h file for details).
   */
  int
  libarchive_packing(const std::shared_ptr<archive> &a,
                     const std::filesystem::path &sourcepath,
                     const bool &rename_source,
                     const std::string &new_source_name);

  /*!
   * \brief Initializes archive objects for removing entries from archive.
   * \param sourcepath absolute path to archive entries to be removed from.
   * \param outpath absolute path to write new archive without removed entries.
   * \return ArchiveRemoveEntry object.
   */
  ArchiveRemoveEntry
  libarchive_remove_init(const std::filesystem::path &sourcepath,
                         const std::filesystem::path &outpath);

  /*!
   * \brief Removes entry from archive.
   * \param rm_e ArchiveRemoveEntry got from libarchive_remove_init().
   * \param to_remove list of entries to be removed.
   * \return ARCHIVE_OK in case of success, libarchive error code otherwise
   * (see libarchive archive.h for details).
   */
  int
  libarchive_remove_entry(ArchiveRemoveEntry rm_e,
                          const std::vector<ArchEntry> &to_remove);

  /*!
   * \brief Prints libarchive error messages.
   * \param a smart pointer to libarchive object.
   * \param message extra text if needed (will be shown before libarchive error
   * text).
   * \param error_number error code.
   */
  void
  libarchive_error(const std::shared_ptr<archive> &a,
                   const std::string &message, const int &error_number);

  /*!
   * \brief Reads archived file to stirng.
   *
   * If entry is not a file, empty string will be returned. In most cases you
   * do not need to call this method directly.
   * \param a pointer to libarchive object.
   * \param entry entry to be read.
   * \return File content.
   */
  std::string
  libarchive_read_entry_str(archive *a, archive_entry *entry);

  /*!
   * \brief Writes data to archive.
   *
   * Writes raw data to archive. In most cases you do not need to call this
   * method directly. Use libarchive_write_directory() and
   * libarchive_write_file() methods instead.
   * \param a pointer to libarchive object.
   * \param data data to be written.
   * \return libarchive error code (see libarchive archive.h for details).
   */
  int
  libarchive_write_data(archive *a, const std::string &data);

  /*!
   * \brief Creates ArchiveFileEntry object.
   *
   * ArchiveFileEntry object is used in libarchive_read_init() and
   * libarchive_read_init_fallback() methods.
   * \param archaddress absolute path to archive to be read.
   * \param position position in archive file to start reading from.
   * \return Smart pointer to ArchiveFileEntry object.
   */
  std::shared_ptr<ArchiveFileEntry>
  createArchFile(const std::filesystem::path &archaddress,
                 const la_int64_t &position = la_int64_t(0));

  /*!
   * \brief Initializes archive reading.
   *
   * This method can return \a nullptr in case of error. If this method
   * failed, you can try to use libarchive_read_init_fallback() instead.
   * \param fl smart pointer to ArchiveFileEntry object (see createArchFile()).
   * \return Smart pointer to libarchive object (can be \a nullptr in case of
   * any error).
   */
  std::shared_ptr<archive>
  libarchive_read_init(std::shared_ptr<ArchiveFileEntry> fl);

  /*!
   * \brief Initializes archive reading.
   *
   * This method can return \a nullptr in case of error.
   * \param fl smart pointer to ArchiveFileEntry object (see createArchFile()).
   * \return Smart pointer to libarchive object (can be \a nullptr in case of
   * any error).
   */
  std::shared_ptr<archive>
  libarchive_read_init_fallback(std::shared_ptr<ArchiveFileEntry> fl);

  /*!
   * \brief Unpacks libarchive entry content.
   *
   * In most cases you do not need to use this method directly. Use
   * unpackByPosition(), unpackByPositionStr(), unpackByFileNameStream(),
   * unpackByFileNameStreamStr() methods instead.
   * \param a pointer to libarchive object.
   * \param entry pointer to libarchive entry object.
   * \param outfolder directory entry content to be unpacked to. If this
   * directory does not exist, it will be created.
   * \return Absolute path to unpacked file or directory.
   */
  std::filesystem::path
  libarchive_read_entry(archive *a, archive_entry *entry,
                        const std::filesystem::path &outfolder);

  /*!
   * \brief Initializes writing to archive.
   *
   * \warning If resulting archive path already exists, it will be overwritten.
   * \param outpath path to resulting archive.
   * \return Smart pointer to libarchive object (can be \a nullptr in case of
   * any error).
   */
  std::shared_ptr<archive>
  libarchive_write_init(const std::filesystem::path &outpath);

  /*!
   * \brief Writes file or directory to archive.
   *
   * \note This method resolves symbolic links and processes them as underlying
   * objects.
   * \param a smart pointer to libarchive object (see libarchive_write_init()).
   * \param source absolute path to file or directory to be packed.
   * \param path_in_arch path of entry in archive (must be relative, example:
   * my_directory/my_file).
   * \return ARCHIVE_OK in case of success, libarchive error code otherwise
   * (see libarchive archive.h for details).
   */
  int
  writeToArchive(std::shared_ptr<archive> a,
                 const std::filesystem::path &source,
                 const std::filesystem::path &path_in_arch);

  /*!
   * \brief Writes directory to archive.
   *
   * In most case you do not need to use this method. Use writeToArchive()
   * instead.
   * \note This method resolves symbolic links and processes them as underlying
   * objects.
   * \param a pointer to libarchive object.
   * \param entry pointer to libarchive entry object.
   * \param path_in_arch path of entry in archive (must be relative, example:
   * "my_directory/my_file").
   * \param source absolute path to directory to be packed.
   * \return ARCHIVE_OK in case of success, libarchive error code otherwise
   * (see libarchive archive.h for details).
   */
  int
  libarchive_write_directory(archive *a, archive_entry *entry,
                             const std::filesystem::path &path_in_arch,
                             const std::filesystem::path &source);

  /*!
   * \brief Writes file to archive.
   *
   * In most case you do not need to use this method. Use writeToArchive()
   * instead.
   * \param a pointer to libarchive object.
   * \param entry pointer to libarchive entry object.
   * \param path_in_arch path of entry in archive (must be relative, example:
   * "my_directory/my_file").
   * \param source absolute path to directory to be packed.
   * \return ARCHIVE_OK in case of success, libarchive error code otherwise
   * (see libarchive archive.h for details).
   */
  int
  libarchive_write_file(archive *a, archive_entry *entry,
                        const std::filesystem::path &path_in_arch,
                        const std::filesystem::path &source);

  /*!
   * \brief Writes raw data from file to archive.
   *
   * In most cases you do not need to use this method. Use writeToArchive()
   * instead.
   * \param a pointer to libarchive object.
   * \param source absolute path to source file.
   * \return libarchive error code (see libarchive archive.h for details).
   */
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

  std::shared_ptr<AuxFunc> af;
};

#endif // LIBARCHIVE_H
