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
#ifndef MLBOOKPROC_H
#define MLBOOKPROC_H

#include <DJVUContext.h>
#include <filesystem>
#include <gcrypt.h>
#include <gpg-error.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/*!
 * \mainpage MLBookProc
 *
 *  \b MLBookProc is a library for managing `.fb2`, `.epub`,
 * `.pdf`, `.djvu` e-books and `.odt`, `.txt` and `.md` files collections. It
 * can also works with same formats packed in zip, 7z, jar, cpio, iso, tar,
 * tar.gz, tar.bz2, tar.xz, rar archives (rar archives are available only for
 * reading) itself or packed in same types of  archives with `.fbd` files (any
 * files, not only books).
 * \b MLBookProc creates own database and does not change files content, names
 * or location.
 *
 * To start add  cmake package MLBookProc to your project.
 *
 * \code{.unparsed}
 * find_package(MLBookProc REQUIRED)
 * find_package(XMLParserCPP REQUIRED)
 * target_link_libraries(my_target MLBookProc::MLBookProc)
 * target_link_libraries(my_target XMLParserCPP::XMLParserCPP)
 * \endcode
 *
 * Then create MLBookProc object. Further reading: CreateCollection,
 * RefreshCollection, BaseKeeper, BookmarksKeeper, NotesKeeper, BookInfo,
 * OpenBook.
 */

/*!
 * \brief The MLBookProc class
 *
 * This class contains various auxiliary methods. It should be created before
 * any other MLBookProc library objects.
 *
 * \warning Only one MLBookProc object per application is allowed, otherwise
 * behavior is undefined.
 */
class MLBookProc
{
public:
  MLBookProc(const MLBookProc &) = delete;
  MLBookProc(MLBookProc &&) = delete;

  MLBookProc &
  operator=(const MLBookProc &)
      = delete;

  MLBookProc &
  operator=(MLBookProc &&)
      = delete;

  virtual ~MLBookProc();

  /*!
   * Creates MLBookProc object and retunrs smart pointer to it.
   * \return Smart pointer to MLBookProc object.
   */
  static std::shared_ptr<MLBookProc>
  create();

  /*!
   * Converts all letters in the string to lowercase letters.
   *
   * \param str UTF-8 string to be converted.
   * \return UTF-8 lowercase string.
   */
  std::string
  stringToLower(const std::string &str);

  /*!
   * Returns string, containing file extensinon (looks like `.ext`).
   * \param p Path to file, extension of which should be obtained.
   * \return UTF-8 string, containing extension.
   */
  std::string
  getExtension(const std::filesystem::path &p);

  /*!
   * Returns string, containing file extensinon (looks like `.ext`).
   * \param file_name Path to file, extension of which should be obtained.
   * \return UTF-8 string, containing extension.
   */
  std::string
  getExtension(const std::string &file_name);

  /*!
   * Returns vector, containing supported file formats (UTF-8 strings, looks
   * like `fbd` or `zip`).
   * \return Vector of UTF-8 strings.
   */
  std::vector<std::string>
  getSupportedFileTypes();

  /*!
   * Returns vector, containing archive formats (UTF-8 strings, looks
   * like `zip` or `tar.gz`) for which packing is supported.
   * \return Vector of UTF-8 strings.
   */
  std::vector<std::string>
  getSupportedArchivesTypesUnpacking();

  /*!
   * Returns vector, containing archive formats (UTF-8 strings, looks
   * like `zip` or `tar.gz`) for which unpacking is supported.
   * \return Vector of UTF-8 strings.
   */
  std::vector<std::string>
  getSupportedArchivesTypesPacking();

  /*!
   * Checks if given file is supported.
   *
   * \param filename Path to file or file name.
   * \return \a true if file is supported, \a false otherwise.
   */
  bool
  ifSupportedFile(const std::string &filename);

  /*!
   * Checks if given file is supported.
   *
   * \param p Path to file or file name.
   * \return \a true if file is supported, \a false otherwise.
   */
  bool
  ifSupportedFile(const std::filesystem::path &p);

  /*!
   * Auxiliary method for <a
   * href="https://www.gnupg.org/software/libgcrypt/index.html">libgcrypt</a>
   * error handling. It throws std::exception in all cases.
   * \param err <a
   * href="https://www.gnupg.org/software/libgcrypt/index.html">libgcrypt</a>
   * error type.
   */
  void
  libgcryptErrorHandling(const gcry_error_t &err);

  /*!
   * Returns random number. Supported types: `int64_t`, `int32_t`, `int16_t`,
   * `int8_t`, `uint64_t`, `uint32_t`, `uint16_t`, `uint8_t`.
   * \return
   */
  template <typename T>
  T
  randomNumber();

  /*!
   * Returns absolute path to system temporary directory.
   *
   * \return Absolute path to system temporary directory.
   */
  std::filesystem::path
  tempDirPath();

  /*!
   * Returns random file name.
   *
   * \return Random file name.
   */
  std::filesystem::path
  randomFileName();

  /*!
   * Converts `time_t` object to date string representation (looks like
   * `01.01.2026`).
   *
   * \param tt `time_t` object to be converted.
   * \return UTF-8 string, containing date.
   */
  std::string
  timeToDate(const time_t &tt);

  /*!
   * Creates DJVUContext object.
   *
   * \return Smart pointer to DJVUContext object.
   */
  std::shared_ptr<DJVUContext>
  getDJVUContext();

private:
  MLBookProc();

  void
  activation();

  static void
  djvuMessageCallback(ddjvu_context_t *context, void *closure);

  std::vector<std::string> supported_types;

  std::weak_ptr<DJVUContext> djvu_contex;
  std::mutex djvu_context_mtx;
};

#endif // MLBOOKPROC_H
