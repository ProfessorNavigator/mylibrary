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

#ifndef BOOKPARSEENTRY_H
#define BOOKPARSEENTRY_H

#include <string>

/*!
 * \brief The BookParseEntry class.
 *
 * Auxiliary class keeping relative path to book in file (in case of archive,
 * empty otherwise), book author(s), book name, book series, book genre(s),
 * date of book creation (if available).
 */
class BookParseEntry
{
public:
  /*!
   * \brief BookParseEntry constructor.
   */
  BookParseEntry();

  /*!
   * \brief BookParseEntry copy constructor.
   */
  BookParseEntry(const BookParseEntry &other);

  /*!
   * \brief BookParseEntry move constructor.
   */
  BookParseEntry(BookParseEntry &&other);

  /*!
   * \brief operator =
   */
  BookParseEntry &
  operator=(const BookParseEntry &other);

  /*!
   * \brief operator =
   */
  BookParseEntry &
  operator=(BookParseEntry &&other);

  /*!
   * \brief operator ==
   */
  bool
  operator==(const BookParseEntry &other);

  /*!
   * \brief Path to book in file (in case of archive, empty
   * otherwise).
   *
   * In case of "archive inside archive" situation "\n" (ASCII new line) symbol
   * used as separator. It means that path to book inside archive looks like
   * "<archive_one>\n<archive_two>\n<archive_three>\n<book_file>" or
   * "<archive_one>\n<book_file>".
   */
  std::string book_path;

  /*!
   * \brief Book author (if any, empty otherwise).
   */
  std::string book_author;

  /*!
   * \brief Book name.
   *
   * Must not be empty. If book name cannot be obtained from book metadata,
   * book file name will be used.
   */
  std::string book_name;

  /*!
   * \brief Book series (if any, empty otherwise).
   */
  std::string book_series;

  /*!
   * \brief Book genres(s) (if any, empty otherwise).
   *
   * List of genres separated by ", " sequence.
   */
  std::string book_genre;

  /*!
   * \brief Book creation date (if available in metadata, empty otherwise).
   */
  std::string book_date;
};

#endif // BOOKPARSEENTRY_H
