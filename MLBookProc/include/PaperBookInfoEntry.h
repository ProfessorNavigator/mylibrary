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

#ifndef PAPERBOOKINFOENTRY_H
#define PAPERBOOKINFOENTRY_H

#include <string>

/*!
 * \brief The PaperBookInfoEntry class.
 *
 * Auxiliary class containing some information about paper book source (if
 * any).
 */
class PaperBookInfoEntry
{
public:
  /*!
   * \brief PaperBookInfoEntry constructor.
   */
  PaperBookInfoEntry();

  /*!
   * \brief PaperBookInfoEntry copy constructor.
   */
  PaperBookInfoEntry(const PaperBookInfoEntry &other);

  /*!
   * \brief PaperBookInfoEntry move constructor.
   */
  PaperBookInfoEntry(PaperBookInfoEntry &&other);

  /*!
   * \brief operator =
   */
  PaperBookInfoEntry &
  operator=(const PaperBookInfoEntry &other);

  /*!
   * \brief operator =
   */
  PaperBookInfoEntry &
  operator=(PaperBookInfoEntry &&other);

  /*!
   * \brief If paper book info is available, will be set to \a true.
   */
  bool available = false;

  /*!
   * \brief Paper book name.
   */
  std::string book_name;

  /*!
   * \brief Paper book publisher.
   */
  std::string publisher;

  /*!
   * \brief City where paper book was published.
   */
  std::string city;

  /*!
   * \brief Year of paper book publishing.
   */
  std::string year;

  /*!
   * \brief <A HREF="https://en.wikipedia.org/wiki/ISBN">ISBN</A> of paper
   * book.
   */
  std::string isbn;
};

#endif // PAPERBOOKINFOENTRY_H
