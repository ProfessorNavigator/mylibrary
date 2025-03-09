/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef NOTESKEEPER_H
#define NOTESKEEPER_H

#include <AuxFunc.h>
#include <NotesBaseEntry.h>
#include <mutex>

class NotesKeeper
{
public:
  NotesKeeper(const std::shared_ptr<AuxFunc> &af);

  virtual ~NotesKeeper();

  void
  loadBase();

  void
  editNote(const NotesBaseEntry &nbe, const std::string &note);

  NotesBaseEntry
  getNote(const std::string &collection_name,
          const std::filesystem::path &book_file_full_path,
          const std::string &book_path);

  void
  removeNotes(const NotesBaseEntry &nbe,
              const std::filesystem::path &reserve_directory,
              const bool &make_reserve);

  void
  removeCollection(const std::string &collection_name,
                   const std::filesystem::path &reserve_directory,
                   const bool &make_reserve);

  void
  refreshCollection(const std::string &collection_name,
                    const std::filesystem::path &reserve_directory,
                    const bool &make_reserve);

  std::vector<NotesBaseEntry>
  getNotesForCollection(const std::string &collection_name);

private:
  void
  parseRawBase(const std::string &raw_base);

  void
  parseEntry(const std::string &entry);

  void
  saveBase();

  std::shared_ptr<AuxFunc> af;
  std::filesystem::path base_directory_path;

  std::vector<NotesBaseEntry> base;
  std::mutex base_mtx;
};

#endif // NOTESKEEPER_H
