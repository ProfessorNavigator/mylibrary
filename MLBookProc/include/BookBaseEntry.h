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

#ifndef BOOKBASEENTRY_H
#define BOOKBASEENTRY_H

#include <BookParseEntry.h>
#include <filesystem>

/*!
 * \brief The BookBaseEntry class.
 *
 * Auxiliary class, used to keep collections databases seacrh requests and
 * search results.
 */
class BookBaseEntry
{
public:
  /*!
   * \brief BookBaseEntry constructor.
   */
  BookBaseEntry();

  /*!
   * \brief BookBaseEntry copy constructor.
   */
  BookBaseEntry(const BookBaseEntry &other);

  /*!
   * \brief BookBaseEntry move constructor.
   */
  BookBaseEntry(BookBaseEntry &&other);

  /*!
   * \brief operator =
   */
  BookBaseEntry &
  operator=(const BookBaseEntry &other);

  /*!
   * \brief operator =
   */
  BookBaseEntry &
  operator=(BookBaseEntry &&other);

  /*!
   * \brief operator ==
   */
  bool
  operator==(const BookBaseEntry &other);

  /*!
   * \brief BookBaseEntry constructor.
   * \param bpe BookParseEntry object.
   * \param book_file_path absolute path to books file or archive.
   */
  BookBaseEntry(const BookParseEntry &bpe,
                const std::filesystem::path &book_file_path);

  /*!
   * \brief Absolute path to book file or archive.
   */
  std::filesystem::path file_path;

  /*!
   * \brief BookParseEntry object.
   */
  BookParseEntry bpe;
};

#endif // BOOKBASEENTRY_H
