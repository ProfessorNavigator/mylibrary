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

#ifndef GENREGROUP_H
#define GENREGROUP_H

#include <Genre.h>
#include <string>
#include <vector>

/*!
 * \brief The GenreGroup class.
 *
 * Auxiliary class containing genre group (see also
 * AuxFunc::get_genre_list()).
 */
class GenreGroup
{
public:
  /*!
   * \brief GenreGroup constructor.
   */
  GenreGroup();

  /*!
   * \brief GenreGroup copy constructor.
   */
  GenreGroup(const GenreGroup &other);

  /*!
   * \brief GenreGroup move constructor.
   */
  GenreGroup(GenreGroup &&other);

  /*!
   * \brief operator =
   */
  GenreGroup &
  operator=(const GenreGroup &other);

  /*!
   * \brief operator =
   */
  GenreGroup &
  operator=(GenreGroup &&other);

  /*!
   * \brief Genre group code name.
   */
  std::string group_code;

  /*!
   * \brief Translated human-readable genre group name.
   *
   * If translation is not available, English genre group name will be set.
   * For translations see "<share_path>/MLBookProc/genre_groups.csv"
   */
  std::string group_name;

  /*!
   * \brief List of group genres (see Genre).
   */
  std::vector<Genre> genres;
};

#endif // GENREGROUP_H
