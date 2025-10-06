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

#ifndef AUXFUNC_H
#define AUXFUNC_H

#include <Genre.h>
#include <GenreGroup.h>
#include <MLException.h>
#include <atomic>
#include <filesystem>
#include <gcrypt.h>
#include <libdjvu/ddjvuapi.h>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <mutex>
#endif

/*!
 * \mainpage MLBookProc
 *
 *  \b MLBookProc is a library for managing `.fb2`, `.epub`,
 * `.pdf`, `.djvu` e-books, `.odt`, `.txt` and `.md` files collections. It can
 * also works with same formats packed in zip, 7z, jar, cpio, iso, tar, tar.gz,
 * tar.bz2, tar.xz, rar archives (rar archives are available only for reading)
 * itself or packed in same types of  archives with `.fbd` files (any files,
 * not only books).
 * \b MLBookProc creates own database and does not change files content, names
 * or location.
 *
 * To start add  cmake package MLBookProc to your project.
 *
 * \code{.unparsed}
 * find_package(MLBookProc)
 * if(MLBookProc_FOUND)
 *  target_link_libraries(myproject MLBookProc::mlbookproc)
 * endif()
 * \endcode
 *
 * \note MLBookProc sets USE_OPENMP build variable in case of OpenMP usage. If
 * USE_OPENMP is set, it is highly recommended to use only OpenMP for
 * multithreading in your application.
 *
 * Then create AuxFunc object. Further reading: CreateCollection,
 * RefreshCollection, BaseKeeper, BookMarks, NotesKeeper, BookInfo, OpenBook.
 */

/*!
 * \brief The AuxFunc class.
 *
 * AuxFunc class contains various useful auxiliary methods. AuxFunc object must
 * be created (see create()) before using of any \b MLBookProc methods or
 * classes. create() method should be called only once per program. Also it is
 * strongly recommended to call get_activated() method immediately after
 * AuxFunc object creation.
 */
class AuxFunc
{
public:
  /*!
   * \brief AuxFunc destructor.
   */
  virtual ~AuxFunc();

  /*!
   * \brief Converts string to UTF-8 string.
   *
   * \param input string to be converted.
   * \param conv_name input string encoding name (see
   * <A
   * HREF="https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucnv_8h.html#abe52185c0f4c3e001f0df1f17b08f0bc">icu
   * documentation</A> for details).
   * \return UTF-8 encoded string or empty string in case of any error.
   */
  std::string
  to_utf_8(const std::string &input, const char *conv_name);

  /*!
   * \brief Converts UTF-8 string to string in system default encoding.
   * \param input UTF-8 string to be converted.
   * \return String in system encoding or empty string in case of any error.
   */
  std::string
  utf8_to_system(const std::string &input);

  /*!
   * \brief Converts UTF-8 string to string in chosen encoding.
   * \param input string to be converted.
   * \param conv_name output string encoding name (see
   * <A
   * HREF="https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucnv_8h.html#abe52185c0f4c3e001f0df1f17b08f0bc">icu
   * documentation</A> for details).
   * \return String in chosen encoding or empty string in case of any error.
   */
  std::string
  utf_8_to(const std::string &input, const char *conv_name);

  /*!
   * \brief Returns converter name.
   * \param num converter number (see <A
   * HREF="https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucnv_8h.html#aac7a88b0c9cb9ce01a0801574b9fd820">icu
   * documentation</A> for details).
   * \return Pointer to string, containing converter name.
   */
  const char *
  get_converter_by_number(const int32_t &num);

  /*!
   * \brief Tries to detect string encoding.
   *
   * \param buf string which encoding is to be detected.
   * \return String containing encoding name.
   */
  std::string
  detect_encoding(const std::string &buf);

  /*!
   * \brief Returns user home directory path.
   * \return Absolute path to home directory.
   */
  std::filesystem::path
  homePath();

  /*!
   * \brief Returns absolute path to program executable file.
   * \return Absolute path to executable file.
   */
  std::filesystem::path
  get_selfpath();

  /*!
   * \brief Returns absolute path to system temporary directory.
   * \return Absolute path to system temporary directory.
   */
  std::filesystem::path
  temp_path();

  /*!
   * \brief Returns absolute path to \a share directory, used by \b MLBookProc.
   *
   * Result path is calculating as path relative to program executable
   * file path ('absolute_path_to_executable_file/../share').
   *
   * \return Absolute path to \a share directory, used by \b MLBookProc.
   */
  std::filesystem::path
  share_path();

  /*!
   * \brief Returns translated genre groups and genres names.
   *
   * Resulting genre groups and genres will be in default system language, if
   * translations are available, or in English.
   *
   * \return Vector of GenreGroup objects.
   */
  std::vector<GenreGroup>
  get_genre_list();

  /*!
   * \brief Auxiliary method to reinterpret libgcrypt errors as strings.
   *
   * In most cases you do not need to call this method directly.
   *
   * \param err libgcrypt error code (see <A
   * HREF="https://gnupg.org/documentation/manuals/gcrypt/Error-Values.html#Error-Values">libgcrypt</A>
   * documentation for details).
   * \return Human-readable string explaining error code.
   */
  std::string
  libgcrypt_error_handling(const gcry_error_t &err);

  /*!
   * \brief Converts given string to hex format.
   *
   * Each char element will be converted to two hexidecimal digits.
   *
   * \param source string to be converted.
   * \return String in hex format.
   */
  std::string
  to_hex(const std::string &source);

  /*!
   * \brief Converts all letters of the string to lowercase letters.
   * \param line UTF-8 encoded string to be converted to lowercase.
   * \return Lowercase string.
   */
  std::string
  stringToLower(const std::string &line);

  /*!
   * \brief Returns random string.
   * \return Random string (it will look like "<random_hex_numbe>MLBookProc").
   */
  std::string
  randomFileName();

  /*!
   * \brief Converts time_t value to calendar date.
   * \param tt time_t value.
   * \return Date string (it will look like
   * "<day_number>.<month_number>.<year>")
   */
  std::string
  time_t_to_date(const time_t &tt);

  /*!
   * \brief Checks if given file is supported by \b MLBookProc.
   *
   * 'Supported types' check is carried out by file extension.
   *
   * \param ch_p absolute path to file.
   * \return \a true if file is supported, \a false otherwise.
   */
  bool
  if_supported_type(const std::filesystem::path &ch_p);

  /*!
   * \brief Checks if given archive is supported by \b MLBookProc.
   *
   * Same as if_supported_type(), but check is carried out only for archives
   * (if given file is not archive \a false will be returned).
   *
   * \param ch_p absolute path to archive (may not exist).
   * \return \a true if archive is supported, \a false otherwise.
   */
  bool
  ifSupportedArchiveUnpackaingType(const std::filesystem::path &ch_p);

  /*!
   * \brief Checks if given archive is supported by \b MLBookProc for packing.
   *
   * Same as ifSupportedArchiveUnpackaingType(), but checks if given file is
   * supported for packing
   *
   * \param ch_p absolute path to archive (may not exist).
   * \return \a true if archive is supported, \a false otherwise.
   */
  bool
  ifSupportedArchivePackingType(const std::filesystem::path &ch_p);

  /*!
   * \brief Converst 'html' symbols to UTF-8 characters.
   *
   * Replaces "&#<unicode_number>;" symbols by UTF-8 characters.
   *
   * \param input string to be converted.
   */
  void
  html_to_utf8(std::string &input);

  /*!
   * \brief Opens given file in default system application.
   * \param path absolute path to file to be opened.
   */
  void
  open_book_callback(const std::filesystem::path &path);

  /*!
   * \brief Replaces out file by source file.
   *
   * This method acts like std::filesystem::copy. It was introduced due to
   * MinGW bug (MinGW ignores
   * std::filesystem::copy_options::overwrite_existing).
   *
   * \param source file to be copied.
   * \param out file to be replaced.
   */
  void
  copy_book_callback(const std::filesystem::path &source,
                     const std::filesystem::path &out);

  /*!
   * \brief Returns supported file types.
   * \return Vector of supported files extensions without beginning dot (looks
   * like "fb2").
   */
  std::vector<std::string>
  get_supported_types();

  /*!
   * \brief Same as get_supported_types(), but returns only archives types,
   * available for packing.
   * \return Vector of supported archives types.
   */
  std::vector<std::string>
  get_supported_archive_types_packing();

  /*!
   * \brief Same as get_supported_types(), but returns only archives types,
   * available for unpacking.
   * \return Vector of supported archives types.
   */
  std::vector<std::string>
  get_supported_archive_types_unpacking();

  /*!
   * \brief Returns file extesion.
   * \param p absolute path to file.
   * \return File extension with beginning dot (looks like ".tar.gz").
   */
  std::string
  get_extension(const std::filesystem::path &p);

  /*!
   * \brief Returns number of available converters.
   *
   * See <A
   * HREF="https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucnv_8h.html#ab759c0b6fc64dfb067a81bdf2f2a9d6b">icu
   * documentation</A> for details.
   *
   * \return Signed 32-bit integer number of available convertrs.
   */
  int32_t
  get_charset_conv_quantity();

  /*!
   * \brief Checks if depencies have been successfully activated.
   *
   * \return \a true if all dependencies have been successfully activated, \a
   * false otherwise.
   */
  bool
  get_activated();

#ifdef USE_OPENMP
  /*!
   * \brief 'Find' method for C++ standard containers.
   *
   * This method acts like <A
   * HREF="https://en.cppreference.com/w/cpp/algorithm/find">std::find</A>, but
   * uses <A HREF="https://www.openmp.org/">OpenMP</A> multithreading for
   * acceleration.
   *
   * \param start start iterator.
   * \param end end iterator.
   * \param val value to be found.
   * \return Iterator of found value or end iterator.
   */
  template <class InputIt,
            class T = typename std::iterator_traits<InputIt>::value_type>
  static InputIt
  parallelFind(InputIt start, InputIt end, const T &val)
  {
    InputIt res = end;
    const T *val_ptr = &val;
#pragma omp parallel
    {
#pragma omp for
      for(InputIt i = start; i != end; i++)
        {
          if(*i == *val_ptr)
            {
#pragma omp critical
              {
                if(i < res)
                  {
                    res = i;
                  }
              }
#pragma omp cancel for
            }
        }
    }

    return res;
  }

  /*!
   * \brief 'Find' method for C++ standard containers.
   *
   * This method acts like <A
   * HREF="https://en.cppreference.com/w/cpp/algorithm/find">std::find_if</A>,
   * but uses <A HREF="https://www.openmp.org/">OpenMP</A> multithreading for
   * acceleration.
   *
   * \param start start iterator.
   * \param end end iterator.
   * \param predicate method to be used to check values.
   * \return Iterator of found value or end iterator.
   */
  template <class InputIt, class UnaryPred>
  static InputIt
  parallelFindIf(InputIt start, InputIt end, UnaryPred predicate)
  {
    InputIt res = end;
#pragma omp parallel
    {
#pragma omp for
      for(InputIt i = start; i != end; i++)
        {
          if(predicate(*i))
            {
#pragma omp critical
              {
                if(i < res)
                  {
                    res = i;
                  }
              }
#pragma omp cancel for
            }
        }
    }

    return res;
  }

  /*!
   * \brief "Remove" method for C++ containers.
   *
   * This method acts like <A
   * HREF="https://en.cppreference.com/w/cpp/algorithm/remove">std::remove</A>,
   * but uses <A HREF="https://www.openmp.org/">OpenMP</A> multithreading for
   * acceleration.
   *
   * \param start start iterator.
   * \param end end iterator.
   * \param val value to be removed.
   * \return Past-the-end iterator for the new range of values (if this is not
   * end, then it points to an unspecified value, and so do iterators to any
   * values between this iterator and end).
   */
  template <class InputIt,
            class T = typename std::iterator_traits<InputIt>::value_type>
  static InputIt
  parallelRemove(InputIt start, InputIt end, const T &val)
  {
    start = parallelFind(start, end, val);
    if(start != end)
      {
        T *s = start.base();
        T *e = end.base();
#pragma omp parallel
#pragma omp for ordered
        for(T *i = s + 1; i != e; i++)
          {
            if((*i) != val)
              {
#pragma omp ordered
                {
                  *s = std::move((*i));
                  s++;
                }
              }
          }
        start = InputIt(s);
      }
    return start;
  }

  /*!
   * \brief 'Remove' method for C++ containers.
   *
   * This method acts like <A
   * HREF="https://en.cppreference.com/w/cpp/algorithm/remove">std::remove_if</A>,
   * but uses <A HREF="https://www.openmp.org/">OpenMP</A> multithreading for
   * acceleration.
   *
   * \param start start iterator.
   * \param end end iterator.
   * \param predicate method to be used to check values.
   * \return Past-the-end iterator for the new range of values (if this is not
   * end, then it points to an unspecified value, and so do iterators to any
   * values between this iterator and end).
   */
  template <class InputIt, class UnaryPred,
            class T = typename std::iterator_traits<InputIt>::value_type>
  static InputIt
  parallelRemoveIf(InputIt start, InputIt end, UnaryPred predicate)
  {
    start = parallelFindIf(start, end, predicate);
    if(start != end)
      {
        T *s = start.base();
        T *e = end.base();
#pragma omp parallel
#pragma omp for ordered
        for(T *i = s + 1; i != e; i++)
          {
            if(!predicate((*i)))
              {
#pragma omp ordered
                {
                  *s = std::move((*i));
                  s++;
                }
              }
          }
        start = InputIt(s);
      }
    return start;
  }
#endif

  /*!
   * \brief Creats AuxFunc object
   *
   * This method must be called before using of any \b MLBookProc classes or
   * methods.
   * \warning This method should be called only once per program.
   * \return Smart pointer to AuxFunc object.
   */
  static std::shared_ptr<AuxFunc>
  create();

  // TODO correct docs
  /*!
   * \brief Returns smart pointer to djvu context object.
   *
   * \return Smart pointer to djvu context object.
   */
  std::tuple<std::shared_ptr<ddjvu_context_t>, std::shared_ptr<int>>
  getDJVUContext();

#ifdef USE_GPUOFFLOADING
  // TODO docs
  void
  setCpuGpuBalance(const double &balance_authors,
                   const double &balance_search);

  // TODO docs
  double
  getCpuGpuBalanceAuthors();

  // TODO docs
  double
  getCpuGpuBalanceSearch();
#endif

private:
  AuxFunc();

  std::vector<std::tuple<std::string, Genre>>
  read_genres(const bool &wrong_loc, const std::string &locname);

  std::vector<GenreGroup>
  read_genre_groups(const bool &wrong_loc, const std::string &locname);

  static void
  djvuMessageCallback(ddjvu_context_t *context, void *closure);

  bool activated = true;

  std::mt19937_64 *rng;
  std::uniform_int_distribution<uint64_t> *dist;

  std::weak_ptr<ddjvu_context_t> djvu_context;
#ifdef USE_OPENMP
  omp_lock_t djvu_context_mtx;
#else
  std::mutex djvu_context_mtx;
#endif
  std::vector<std::weak_ptr<int>> djvu_pipes;

#ifdef USE_GPUOFFLOADING
  std::atomic<double> cpu_gpu_balance_authors = 0.95;
  std::atomic<double> cpu_gpu_balance_search = 0.5;
#endif
};

#endif // AUXFUNC_H
