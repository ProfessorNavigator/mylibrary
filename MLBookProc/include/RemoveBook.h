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

#ifndef REMOVEBOOK_H
#define REMOVEBOOK_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <SelfRemovingPath.h>
#include <filesystem>
#include <memory>
#include <string>

/*!
 * \brief The RemoveBook class.
 *
 * This class contains methods to carry out book removing from collection.
 */
class RemoveBook
{
public:
  /*!
   * \brief RemoveBook constructor.
   * \param af smart pointer to AuxFunc object.
   * \param bbe BookBaseEntry containing book info.
   * \param col_name collection name.
   * \param bookmarks BookMarks object.
   */
  RemoveBook(const std::shared_ptr<AuxFunc> &af, const BookBaseEntry &bbe,
             const std::string &col_name,
             const std::shared_ptr<BookMarks> &bookmarks);

  /*!
   * \brief Removes book.
   *
   * \note This method can throw MLException in case of errors.
   */
  void
  removeBook();

private:
  void
  archiveRemove(const std::filesystem::path &archive_path,
                const std::string &book_path,
                const std::filesystem::path &out_d);

  std::shared_ptr<AuxFunc> af;
  BookBaseEntry bbe;
  std::string col_name;
  std::shared_ptr<BookMarks> bookmarks;

  std::vector<std::string> supported_archives;
};

#endif // REMOVEBOOK_H
