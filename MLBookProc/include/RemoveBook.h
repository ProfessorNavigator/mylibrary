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
#ifndef REMOVEBOOK_H
#define REMOVEBOOK_H

#include <BaseID.h>
#include <LibArchive.h>
#include <UDBElement.h>
#include <functional>

/*!
 * \brief The RemoveBook class
 *
 * This class contains methods for removing books from collections.
 */
class RemoveBook : public LibArchive
{
public:
  /*!
   * \brief RemoveBook constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  RemoveBook(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   * Removes book from collection database and filesystem.
   *
   * \note This method can throw std::exception in case of errors.
   * \warning If book is packed in rar archive, whole archive will be removed.
   *
   * \param base_path Path to collection database file.
   * \param book_search_result BookID::BookSearchResult object.
   */
  void
  removeBook(const std::filesystem::path &base_path,
             const UDBElement &book_search_result);

  /*!
   * If book was packed in archive, archive file will be reparsed after
   * removing. This callback idicates parsing progress if set.
   */
  std::function<void(double processed, double total)> signal_parsing_progress;

private:
  size_t
  removeFromArchive(const UDBElement &path,
                    const std::filesystem::path &archive_path);

  BaseID bid;
};

#endif // REMOVEBOOK_H
