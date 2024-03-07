/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_GENREGROUP_H_
#define INCLUDE_GENREGROUP_H_

#include <Genre.h>
#include <string>
#include <vector>

class GenreGroup
{
public:
  GenreGroup();
  virtual
  ~GenreGroup();

  GenreGroup(const GenreGroup &other);

  GenreGroup(GenreGroup &&other);

  GenreGroup&
  operator=(const GenreGroup &other);

  GenreGroup&
  operator=(GenreGroup &&other);

  std::string group_code;
  std::string group_name;
  std::vector<Genre> genres;
};

#endif /* INCLUDE_GENREGROUP_H_ */