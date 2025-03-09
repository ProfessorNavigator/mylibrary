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
#ifndef NOTESBASEENTRY_H
#define NOTESBASEENTRY_H

#include <filesystem>

class NotesBaseEntry
{
public:
  NotesBaseEntry();

  NotesBaseEntry(const std::string &collection_name,
                 const std::filesystem::path &book_file_full_path,
                 const std::string &book_path);

  NotesBaseEntry(const NotesBaseEntry &other);

  NotesBaseEntry(NotesBaseEntry &&other);

  NotesBaseEntry &
  operator=(const NotesBaseEntry &other);

  NotesBaseEntry &
  operator=(NotesBaseEntry &&other);

  bool
  operator==(const NotesBaseEntry &other) const;

  std::string collection_name;
  std::filesystem::path book_file_full_path;
  std::string book_path;
  std::filesystem::path note_file_full_path;
};

#endif // NOTESBASEENTRY_H
