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
#ifndef BOOKMARKSKEEPER_H
#define BOOKMARKSKEEPER_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <filesystem>
#include <shared_mutex>

/*!
 * \brief The BookmarksKeeper class
 *
 * This class contains bookmarks database and methods to work with it.
 * See also <a href="https://github.com/ProfessorNavigator/libudb">LibUDB</a>
 * documentation.
 */
class BookmarksKeeper : public UDBase
{
public:
  /*!
   * \brief BookmarksKeeper constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  BookmarksKeeper(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~BookmarksKeeper();

  /*!
   * Loads bookmarks database to memory.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param bookmarks_base_path Path to bookmarks database.
   */
  void
  loadBookmarksBase(const std::filesystem::path &bookmarks_base_path);

  /*!
   * Adds book to bookmarks.
   *
   * \param book_search_result UDBElement of BaseID::BookSearchResult type.
   */
  void
  addToBookmarks(const UDBElement &book_search_result);

  /*!
   * Removes bookmark from database.
   *
   * \param bookmark UDBElement of BaseID::BookMark type.
   */
  void
  removeBookmark(const UDBElement &bookmark);

  /*!
   * Returns pointer to internal mutex used to block bookmarks database calls.
   *
   * \return Pointer to std::shared_mutex object.
   */
  std::shared_mutex *
  getRawMutex();

private:
  void
  loadLegacyBase();

  void
  parseLegacyEntry(const std::string &entry);

  std::shared_ptr<MLBookProc> mlbp;

  std::filesystem::path base_path;

  BaseID bid;

  std::shared_mutex base_mtx;
};

#endif // BOOKMARKSKEEPER_H
