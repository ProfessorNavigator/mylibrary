/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <GenreGroup.h>

GenreGroup::GenreGroup()
{
}

GenreGroup::GenreGroup(const GenreGroup &other)
{
  group_code = other.group_code;
  group_name = other.group_name;
  genres = other.genres;
}

GenreGroup &
GenreGroup::operator=(const GenreGroup &other)
{
  if(this != &other)
    {
      group_code = other.group_code;
      group_name = other.group_name;
      genres = other.genres;
    }
  return *this;
}

GenreGroup::GenreGroup(GenreGroup &&other)
{
  group_code = other.group_code;
  group_name = other.group_name;
  genres = other.genres;
}

GenreGroup &
GenreGroup::operator=(GenreGroup &&other)
{
  if(this != &other)
    {
      group_code = other.group_code;
      group_name = other.group_name;
      genres = other.genres;
    }
  return *this;
}
