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
#ifndef BASES_H
#define BASES_H

#include <BaseKeeper.h>
#include <BookmarksKeeper.h>
#include <GenreBase.h>
#include <MLBookProc.h>
#include <NotesKeeper.h>

/*!
 * \brief The Bases class
 *
 * This class intended for keeping smart pointers to different databases, which
 * can be useful in plugins.
 */
class Bases
{
public:
  Bases();

  /*!
   * \brief Bases constructor.
   * \param mlbp Smart pointer to MLBookProc object (see MLBookProc
   * documentation).
   * \param base_keeper Smart pointer to BaseKeeper (see MLBookProc
   * documentation).
   * \param bookmarks Smart pointer to BookmarksKeeper object (see MLBookProc
   * documentation).
   * \param notes Smart poinet to NotesKeeper object (see MLBookProc
   * documentation).
   * \param genres_base Smart pointer to GenreBase object.
   */
  Bases(const std::shared_ptr<MLBookProc> &mlbp,
        const std::shared_ptr<BaseKeeper> &base_keeper,
        const std::shared_ptr<BookmarksKeeper> &bookmarks,
        const std::shared_ptr<NotesKeeper> &notes,
        const std::shared_ptr<GenreBase> &genres_base);

  /*!
   * \brief Copy constructor.
   *
   * \param other Bases object to be copied.
   */
  Bases(const Bases &other);

  /*!
   * \brief Move constructor
   *
   * \param other Bases object to be moved.
   */
  Bases(Bases &&other);

  /*!
   * \brief operator =
   * \param other Bases object to be copied.
   * \return Reference to this Bases object.
   */
  Bases &
  operator=(const Bases &other);

  /*!
   * \brief operator =
   * \param other Bases object to be moved.
   * \return Reference to this Bases object.
   */
  Bases &
  operator=(Bases &&other);

  /*!
   * \brief Smart pointer to MLBookProc object (see MLBookProc documentation).
   */
  std::shared_ptr<MLBookProc> mlbp;

  /*!
   * \brief Smart pointer to BaseKeeper (see MLBookProc documentation).
   */
  std::shared_ptr<BaseKeeper> base_keeper;

  /*!
   * \brief Smart pointer to BookmarksKeeper object (see MLBookProc
   * documentation).
   */
  std::shared_ptr<BookmarksKeeper> bookmarks;

  /*!
   * \brief Smart poinet to NotesKeeper object (see MLBookProc documentation).
   */
  std::shared_ptr<NotesKeeper> notes;

  /*!
   * \brief Smart pointer to GenreBase object.
   */
  std::shared_ptr<GenreBase> genres_base;
};

#endif // BASES_H
