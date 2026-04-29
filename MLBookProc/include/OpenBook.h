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
#ifndef OPENBOOK_H
#define OPENBOOK_H

#include <BaseID.h>
#include <LibArchive.h>
#include <MLBookProc.h>
#include <UDBElement.h>
#include <filesystem>
#include <functional>

/*!
 * \brief The OpenBook class
 *
 * This class contains methods for books opening.
 */
class OpenBook
{
public:
  /*!
   * \brief OpenBook constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  OpenBook(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~OpenBook();

  /*!
   * Opens given book. If book is in archive, unpacks book to \a
   * unpacking_directory.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param book_search_result BaseID::BookSearchResult object, containing book
   * to be opened.
   * \param unpacking_directory Path to directory book to be unpacked to.
   * Attantion! If this directory exists, all its content will be removed.
   * \param open_call_back This function will be called at the end of operation
   * with path to book as parameter.
   */
  void
  openBook(const UDBElement &book_search_result,
           const std::filesystem::path &unpacking_directory,
           std::function<void(const std::filesystem::path &)> open_call_back);

private:
  void
  openBook(const std::filesystem::path &p,
           std::function<void(const std::filesystem::path &)> open_call_back,
           const UDBElement &path);

  std::shared_ptr<MLBookProc> mlbp;

  LibArchive *la = nullptr;

  std::vector<std::string> supported_archives;

  std::filesystem::path unpack_dir;

  int call_count = 0;

  BaseID bid;
};

#endif // OPENBOOK_H
