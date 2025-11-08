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

#ifndef OPENBOOK_H
#define OPENBOOK_H

#include <ArchEntry.h>
#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <SelfRemovingPath.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The OpenBook class.
 *
 * This class contains methods for books "opening".
 */
class OpenBook
{
public:
  /*!
   * \brief OpenBook constructor.
   * \param af smart pointer to AuxFunc object.
   */
  OpenBook(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Opens book.
   *
   * If book is in archive, unpacks book and returns absolute path to unpacked
   * file. Otherwise returns absolute path to book file.
   *
   * If \b copy is set to \a true and \b copy_path is not empty, creates
   * directory on \b copy_path and copies book to it.
   *
   * If \b find_fbd is set to \a true, will try to find and open fbd file
   * instead of book.
   *
   * If \b open_callback is not \a nullptr, calls it.
   *
   * \note This method can throw std::exception in case of errors.
   * \param bbe BookBaseEntry object.
   * \param copy if set to \a true, copy of book file will be created.
   * \param copy_path absolute path to directory book to be copied to.
   * \param find_fbd if set to \a true, this method will try to find and open
   * fbd file instead of book.
   * \param open_callback method to be called at the end of all operations. \b
   * path argument is an absolute path to method work result.
   * \return Absolute path to book to be opened.
   */
  std::filesystem::path
  open_book(
      const BookBaseEntry &bbe, const bool &copy,
      const std::filesystem::path &copy_path, const bool &find_fbd,
      std::function<void(const std::filesystem::path &path)> open_callback);

private:
  std::filesystem::path
  open_archive(const BookBaseEntry &bbe, const std::string &ext,
               const std::filesystem::path &copy_path, const bool &find_fbd);

  void
  correct_separators(std::vector<ArchEntry> &files);

  bool
  compare_func(const ArchEntry &ent, const bool &encoding,
               const std::string &conv_nm,
               const std::filesystem::path &ch_fbd);

  std::shared_ptr<AuxFunc> af;

  SelfRemovingPath reading;
};

#endif // OPENBOOK_H
