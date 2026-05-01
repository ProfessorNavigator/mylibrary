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
#ifndef LIBARCHIVE_H
#define LIBARCHIVE_H

#include <LibArchiveFileData.h>
#include <MLBookProc.h>
#include <archive.h>
#include <filesystem>
#include <istream>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The LibArchive class
 *
 * This class contains methods to work with archives.
 */
class LibArchive
{
public:
  /*!
   * \brief LibArchive constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  LibArchive(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~LibArchive();

  /*!
   * Obtains entries from archive.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param archive_path Path to archive.
   * \param result Vector of obtained entries. First element of tuple - name of
   * object in archive, second - size of unpacked object, third - offset of
   * entry in archive file (this method sets to \a 0 in all cases).
   */
  void
  listFilesInArchive(
      const std::filesystem::path &archive_path,
      std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result);

  /*!
   * Same as listFilesInArchive(), but obtains entiries from archive placed in
   * buffer.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Archive file content.
   * \param result Vector of obtained entries. First element of tuple - name of
   * object in archive, second - size of unpacked object, third - offset of
   * entry in archive file (this method sets to \a 0 in all cases).
   */
  void
  listFilesInArchiveBuffer(
      const std::string &buffer,
      std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result);

  /*!
   * Same as listFilesInArchive(), but designed specially for zip archives.
   * This method can be used to other archives also, but it uses
   * listFilesInArchive() in those cases.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param archive_path Path to archive.
   * \param result Vector of obtained entries. First element of tuple - name of
   * object in archive, second - size of unpacked object, third - offset of
   * entry in archive file (will be set to \a 0 in case of non zip archives).
   */
  void
  listFilesInZip(
      const std::filesystem::path &archive_path,
      std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result);

  /*!
   * Same as listFilesInZip(), but but obtains entiries from archive placed in
   * buffer.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Archive file content.
   * \param result Vector of obtained entries. First element of tuple - name of
   * object in archive, second - size of unpacked object, third - offset of
   * entry in archive file (will be set to \a 0 in case of non zip archives).
   */
  void
  listFilesInZipBuffer(
      const std::string &buffer,
      std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result);

  /*!
   * Unpacks single file from archive to given directory.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param archive_path Path to archive.
   * \param filename Name of file in archive.
   * \param directory Path to directory file should be unpacked to.
   * \param offset Offset of file entry in archive (should be set only in case
   * of zip archives).
   * \return Absolute path to unpacked file.
   */
  std::filesystem::path
  unpackFileToDirectory(const std::filesystem::path &archive_path,
                        const std::string &filename,
                        const std::filesystem::path &directory,
                        const size_t &offset = size_t(0));

  /*!
   * Same as unpackFileToDirectory(), but uses buffered archive.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Archive file content.
   * \param filename Name of file in archive.
   * \param directory Path to directory file should be unpacked to.
   * \param offset Offset of file entry in archive (should be set only in case
   * of zip archives).
   * \return Absolute path to unpacked file.
   */
  std::filesystem::path
  unpackBufferFileToDirectory(const std::string &buffer,
                              const std::string &filename,
                              const std::filesystem::path &directory,
                              const size_t &offset = size_t(0));

  /*!
   * Same as unpackFileToDirectory(), but designed for zip archives specially.
   * This method can be used for other types of archives, but it uses
   * unpackFileToDirectory() in those cases.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param archive_path Path to archive.
   * \param filename Name of file in archive.
   * \param directory Path to directory file should be unpacked to.
   * \return Absolute path to unpacked file.
   */
  std::filesystem::path
  unpackZipFileToDirectory(const std::filesystem::path &archive_path,
                           const std::string &filename,
                           const std::filesystem::path &directory);

  /*!
   * Same as unpackZipFileToDirectory(), but uses buffered archive as source.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Archive file content.
   * \param filename Name of file in archive.
   * \param directory Path to directory file should be unpacked to.
   * \return Absolute path to unpacked file.
   */
  std::filesystem::path
  unpackZipBufferFileToDirectory(const std::string &buffer,
                                 const std::string &filename,
                                 const std::filesystem::path &directory);

  /*!
   * Unpacks file to buffer.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param archive_path Path to archive.
   * \param filename Name of file in archive.
   * \param offset Offset of file entry in archive (should be set only in case
   * of zip archives).
   * \return Buffer containing unpacked file.
   */
  std::string
  unpackFileToBuffer(const std::filesystem::path &archive_path,
                     const std::string &filename,
                     const size_t &offset = size_t(0));

  /*!
   * Same as unpackFileToBuffer(), but uses buffered archive as source.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Archive file content.
   * \param filename Name of file in archive.
   * \param offset Offset of file entry in archive (should be set only in case
   * of zip archives).
   * \return Buffer containing unpacked file.
   */
  std::string
  unpackBufferFileToBuffer(const std::string &buffer,
                           const std::string &filename,
                           const size_t &offset = size_t(0));

  /*!
   * Same as unpackFileToBuffer(), but designed for zip archives specially.
   * This method can be used for other types of archives, but it uses
   * unpackFileToBuffer() in those cases.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param archive_path Path to archive.
   * \param filename Name of file in archive.
   * \return Buffer containing unpacked file.
   */
  std::string
  unpackZipFileToBuffer(const std::filesystem::path &archive_path,
                        const std::string &filename);

  /*!
   * Same as unpackZipFileToBuffer(), but uses buffered archive instead.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Archive file content.
   * \param filename Name of file in archive.
   * \return Buffer containing unpacked file.
   */
  std::string
  unpackZipBufferFileToBuffer(const std::string &buffer,
                              const std::string &filename);

  /*!
   * Writes file to archive.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param source_object File to be written to archive.
   * \param archive_path Path to archive.
   * \param name_in_archive Name of file in archive.
   * \param perms File permissions. In case of std::filesystem::perms::none,
   * source_object permissions will be used.
   * \param overwrite_existing If \a true archive will be overwritten. If \a
   * false archive will be repacked and file will be appended to new archive.
   */
  void
  writeToArchive(const std::filesystem::path &source_object,
                 const std::filesystem::path &archive_path,
                 std::string name_in_archive,
                 const std::filesystem::perms &perms
                 = std::filesystem::perms::none,
                 const bool &overwrite_existing = true);

  /*!
   * Same as writeToArchive(), but uses buffer as source.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param buffer Buffer to be packed to archive.
   * \param archive_path Path to archive.
   * \param name_in_archive Name of file in archive.
   * \param perms File permissions.
   * \param overwrite_existing If \a true archive will be overwritten. If \a
   * false archive will be repacked and file will be appended to new archive.
   */
  void
  writeBufferObjectToArchive(const std::string &buffer,
                             const std::filesystem::path &archive_path,
                             std::string name_in_archive,
                             const std::filesystem::perms &perms,
                             const bool &overwrite_existing);

protected:
  /*!
   * Initializes archive for reading.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param fd Smart pointer to LibArchiveFileData object.
   * \return Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   */
  std::shared_ptr<archive>
  initForReading(const std::shared_ptr<LibArchiveFileData> &fd);

  /*!
   * Initializes archive for writing.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param fd Smart pointer to LibArchiveFileData object.
   * \return Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   */
  std::shared_ptr<archive>
  initForWriting(const std::shared_ptr<LibArchiveFileData> &fd);

  /*!
   * Handles <a href="https://libarchive.org/">libarchive</a> errors.
   *
   * This method throws std::exception in all cases.
   *
   * \param a Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param message Message to be prepended to <a
   * href="https://libarchive.org/">libarchive</a> error message.
   */
  void
  archiveError(const std::shared_ptr<archive> &a, const std::string &message);

  /*!
   * Open callback function for <a
   * href="https://libarchive.org/">libarchive</a>.
   *
   * \param a Pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param client_data Pointer to client data.
   * \return ARCHIVE_OK if the underlying file or data source is successfully
   * opened. If the open fails, returns ARCHIVE_FATAL.
   */
  static int
  openCallBack(archive *a, void *client_data);

  /*!
   * Read callback function for <a
   * href="https://libarchive.org/">libarchive</a>.
   *
   * \param a Pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param client_data Pointer to client data.
   * \param buffer Pointer to buffer.
   * \return Number of actually read bytes.
   */
  static la_ssize_t
  readCallBack(archive *a, void *client_data, const void **buffer);

  /*!
   * Skip callback function for <a
   * href="https://libarchive.org/">libarchive</a>.
   *
   * \param a Pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param client_data Pointer to client data.
   * \param request Number of bytes to skip.
   * \return Number of actually skipped bytes.
   */
  static la_int64_t
  skipCallback(archive *a, void *client_data, la_int64_t request);

  /*!
   * Close callback function for <a
   * href="https://libarchive.org/">libarchive</a>.
   *
   * \param a Pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param client_data Pointer to client data.
   * \return ARCHIVE_OK in case of success, ARCHIVE_FATAL otherwise.
   */
  static int
  closeCallback(archive *a, void *client_data);

  /*!
   * Seek callback function for <a
   * href="https://libarchive.org/">libarchive</a>.
   *
   * \param a Pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param client_data Pointer to client data.
   * \param offset Requested offset.
   * \param whence The possibilities for the third argument to `fseek`.
   * \return  Actual position in file or ARCHIVE_FATAL in case of errors.
   */
  static la_int64_t
  seekCallback(archive *a, void *client_data, la_int64_t offset, int whence);

  /*!
   * Write callback function for <a
   * href="https://libarchive.org/">libarchive</a>.
   *
   * \param a Pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param client_data Pointer to client data.
   * \param buffer Buffer to be written.
   * \param length Size of given buffer.
   * \return Number of actually written bytes.
   */
  static la_ssize_t
  writeCallback(archive *a, void *client_data, const void *buffer,
                size_t length);

  /*!
   * Unpacks file to directory.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param fd Smart pointer to LibArchiveFileData object.
   * \param filename File to be unpacked.
   * \param directory Directory file to be unpacked to.
   * \return Absolute path to unpacked file.
   */
  std::filesystem::path
  unpackToDirectory(std::shared_ptr<LibArchiveFileData> fd,
                    const std::string &filename,
                    const std::filesystem::path &directory);

  /*!
   * Unpacks file to buffer.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param fd Smart pointer to LibArchiveFileData object.
   * \param filename File to be unpacked.
   * \param result Buffer file to be unpacked to.
   */
  void
  unpackToBuffer(std::shared_ptr<LibArchiveFileData> fd,
                 const std::string &filename, std::string &result);

  /*!
   * Unpacks single entry to directory.
   *
   * \param a Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param e Smart pointer to `archive_entry` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param file_path Path entry should be unpacked to.
   */
  void
  unpackEntryToDirectory(std::shared_ptr<archive> a,
                         std::shared_ptr<archive_entry> e,
                         const std::filesystem::path &file_path);

  /*!
   * Unpacks single entry to buffer.
   *
   * \param a Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param e Smart pointer to `archive_entry` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \return Buffer containing result (empty if entry is not a file).
   */
  std::string
  unpackEntryToBuffer(std::shared_ptr<archive> a,
                      std::shared_ptr<archive_entry> e);

  /*!
   * Writes file to archive.
   *
   * \param a Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param path Path to file to be packed.
   * \param name_in_archive Name of file in archive.
   * \param perms File permissions.
   */
  void
  writeFile(std::shared_ptr<archive> a, const std::filesystem::path &path,
            std::string name_in_archive, const std::filesystem::perms &perms);

  /*!
   * Writes buffer to archive.
   *
   * \param a Smart pointer to `archive` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param e Smart pointer to `archive_entry` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \param buf Buffer to be packed.
   */
  void
  writeBufferToArchive(std::shared_ptr<archive> a,
                       std::shared_ptr<archive_entry> e,
                       const std::string &buf);

  /*!
   * Returns permissions read from entry.
   *
   * \param e Smart pointer to `archive_entry` object (see <a
   * href="https://libarchive.org/">libarchive</a> documentation for details).
   * \return
   */
  std::filesystem::perms
  getPermissionsFromEntry(const std::shared_ptr<archive_entry> &e);

  /*!
   * Smart pointer to MLBookProc object.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::shared_ptr<MLBookProc> mlbp;

private:
  void
  getCentralDirectory(std::shared_ptr<std::istream> f, const uint64_t &fsz,
                      std::string &result);

  std::tuple<uint64_t, uint64_t>
  parseEOCDRecord(std::shared_ptr<std::istream> f, const uint64_t &fsz);

  std::tuple<uint64_t, uint64_t>
  parseEOCDRecordZip64(std::shared_ptr<std::istream> f, const uint64_t &fsz,
                       const uint64_t &eocd_record_size);

  void
  parseCentralDirectory(
      const std::string &central_directory,
      std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result);

  void
  parseExtraField(const std::string &extra, uint64_t &compressed_sz,
                  uint64_t &offset, const int &mask);

  std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
  symlinkWriteResolver(const std::filesystem::path &relative,
                       const std::filesystem::path &symlink);

  bool
  getUTFfilename(const std::string &extra_field, const std::string &crc_sum,
                 std::string &filename);

  std::string
  crc32Sum(unsigned char *buf, size_t len);

  int ZIP64_UNCOMPRESSED = 1;
  int ZIP64_COMPRESSED = 2;
  int ZIP64_OFFSET = 4;
};

#endif // LIBARCHIVE_H
