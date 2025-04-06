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

#ifndef GENRE_H
#define GENRE_H

#include <string>

/*!
 * \brief The Genre class.
 *
 * Auxiliary class containing translated genre element (see also
 * AuxFunc::get_genre_list() and GenreGroup).
 */
class Genre
{
public:
  /*!
   * \brief Genre constructor.
   */
  Genre();

  /*!
   * \brief Genre copy constructor.
   */
  Genre(const Genre &other);

  /*!
   * \brief Genre move constructor.
   */
  Genre(Genre &&other);

  /*!
   * \brief operator =
   */
  Genre &
  operator=(const Genre &other);

  /*!
   * \brief operator =
   */
  Genre &
  operator=(Genre &&other);

  /*!
   * \brief fb2 genre code.
   *
   * This code is usually valid for epub books also.
   */
  std::string genre_code;

  /*!
   * \brief Translated human-readable genre name.
   *
   * If translation is not available, English genre name will be set.
   * For translations see "<share_path>/MLBookProc/genres.csv"
   */
  std::string genre_name;
};

#endif // GENRE_H
