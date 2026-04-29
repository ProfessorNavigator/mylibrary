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
#ifndef GENREBASE_H
#define GENREBASE_H

#include <QString>
#include <UDBase.h>
#include <shared_mutex>

/*!
 * \brief The GenreBase class
 *
 * This class intended for keeping database of supported genre codes and their
 * translations.
 */
class GenreBase : public UDBase
{
public:
  GenreBase();

  virtual ~GenreBase();

  /*!
   * Returns transladed genre name.
   *
   * \param genre_code Genre code to be translated.
   * \return QString object containing genre translation or genre code, if it
   * was not found in base.
   */
  QString
  getTranslationByGenreCode(const std::string &genre_code);

private:
  std::string
  loadFile(const QString &path);

  void
  createBase();

  int
  getCsvColumnNumber(const std::string &source);

  void
  loadGroups(const std::string &g_groups, const int &g_groups_column);

  void
  loadGenres(const std::string &genres, const int &genres_column);

  std::shared_mutex base_mtx;
};

#endif // GENREBASE_H
