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
#ifndef NOTESKEEPER_H
#define NOTESKEEPER_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <shared_mutex>

/*!
 * \brief The NotesKeeper class
 *
 * This class keeps notes databse and provides methods to work with it.
 */
class NotesKeeper : public UDBase
{
public:
  /*!
   * \brief NotesKeeper constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  NotesKeeper(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   * Loads notes database to memory.
   *
   * \param base_path Path to notes database file.
   */
  void
  loadNotesBase(const std::filesystem::path &base_path);

  /*!
   * Search notes for given book.
   *
   * \param book_search_result BaseID::BookSearchResult object.
   * \return UDBase object containing BaseID::BookNote objects (if any).
   */
  UDBase
  searchNotes(const UDBElement &book_search_result);

  /*!
   * Creates or edits notes for given book. If \a note_text is empty, removes
   * notes for given book.
   *
   * \param book_search_result BookID::BookSearchResult object.
   * \param note_txt Note text.
   */
  void
  addNote(const UDBElement &book_search_result, const std::string &note_txt);

  /*!
   * Removes note.
   *
   * \param note BaseID::BookNote object to be removed from database.
   */
  void
  removeNote(const UDBElement &note);

private:
  void
  loadLegacyBase(const std::filesystem::path &base_path);

  void
  parseLegacyEntry(const std::string &entry);

  std::shared_ptr<MLBookProc> mlbp;

  std::filesystem::path base_path;

  BaseID bid;

  std::shared_mutex base_mtx;
};
#endif // NOTESKEEPER_H
