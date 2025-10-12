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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <memory>
#include <mutex>
#include <vector>

/*!
 * \brief The BookMarks class.
 *
 * This class keeps and operates collection bookmarks. Path to bookmarks base
 * is "~/.local/share/MyLibrary/BookMarks/bookmarks".
 */
class BookMarks
{
public:
  /*!
   * \brief BookMarks constructor.
   * \param af smart pointer to AuxFunc object.
   */
  BookMarks(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief BookMarks destructor.
   */
  virtual ~BookMarks();

  /*!
   * \brief Creates bookmark.
   *
   * If bookmark already exists, returns 0, in case of success returns 1,
   * otherwise returns -1.
   *
   * \param col_name collection name book came from.
   * \param bbe BookBaseEntry got from BaseKeeper::searchBook().
   * \return Error code.
   */
  int
  createBookMark(const std::string &col_name, const BookBaseEntry &bbe);

  /*!
   * \brief Returns bookmarks.
   * \return Vector of bookmarks tuples. First element of tuple is collection
   * name, book came from. Second element is book entry.
   */
  std::vector<std::tuple<std::string, BookBaseEntry>>
  getBookMarks();

  /*!
   * \brief Removes bookmark.
   *
   * \param col_name collection name.
   * \param bbe book entry to be removed.
   */
  void
  removeBookMark(const std::string &col_name, const BookBaseEntry &bbe);

private:
  void
  loadBookMarks();

  std::tuple<std::string, BookBaseEntry>
  parse_entry(const std::string &buf);

  bool
  saveBookMarks();

  std::string
  form_entry(const std::string &col_name, const BookBaseEntry &bbe);

  std::shared_ptr<AuxFunc> af;

  std::filesystem::path bookmp;

  std::vector<std::tuple<std::string, BookBaseEntry>> bookmarks;
  std::mutex bookmarksmtx;
};

#endif // BOOKMARKS_H
