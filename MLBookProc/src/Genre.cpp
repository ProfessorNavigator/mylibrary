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

#include <Genre.h>

Genre::Genre()
{
}

Genre::Genre(const Genre &other)
{
  genre_code = other.genre_code;
  genre_name = other.genre_name;
}

Genre &
Genre::operator=(const Genre &other)
{
  if(this != &other)
    {
      genre_code = other.genre_code;
      genre_name = other.genre_name;
    }
  return *this;
}

Genre::Genre(Genre &&other)
{
  genre_code = std::move(other.genre_code);
  genre_name = std::move(other.genre_name);
}

Genre &
Genre::operator=(Genre &&other)
{
  if(this != &other)
    {
      genre_code = std::move(other.genre_code);
      genre_name = std::move(other.genre_name);
    }
  return *this;
}
