/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <Bases.h>

Bases::Bases()
{
}

Bases::Bases(const std::shared_ptr<MLBookProc> &mlbp,
             const std::shared_ptr<BaseKeeper> &base_keeper,
             const std::shared_ptr<BookmarksKeeper> &bookmarks,
             const std::shared_ptr<NotesKeeper> &notes,
             const std::shared_ptr<GenreBase> &genres_base)
{
  this->mlbp = mlbp;
  this->base_keeper = base_keeper;
  this->bookmarks = bookmarks;
  this->notes = notes;
  this->genres_base = genres_base;
}

Bases::Bases(const Bases &other)
{
  mlbp = other.mlbp;
  base_keeper = other.base_keeper;
  bookmarks = other.bookmarks;
  notes = other.notes;
  genres_base = other.genres_base;
}

Bases::Bases(Bases &&other)
{
  mlbp = std::move(other.mlbp);
  base_keeper = std::move(other.base_keeper);
  bookmarks = std::move(other.bookmarks);
  notes = std::move(other.notes);
  genres_base = std::move(other.genres_base);
}

Bases &
Bases::operator=(const Bases &other)
{
  if(this != &other)
    {
      mlbp = other.mlbp;
      base_keeper = other.base_keeper;
      bookmarks = other.bookmarks;
      notes = other.notes;
      genres_base = other.genres_base;
    }

  return *this;
}

Bases &
Bases::operator=(Bases &&other)
{
  if(this != &other)
    {
      mlbp = std::move(other.mlbp);
      base_keeper = std::move(other.base_keeper);
      bookmarks = std::move(other.bookmarks);
      notes = std::move(other.notes);
      genres_base = std::move(other.genres_base);
    }

  return *this;
}
