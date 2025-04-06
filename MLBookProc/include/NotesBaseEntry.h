/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef NOTESBASEENTRY_H
#define NOTESBASEENTRY_H

#include <filesystem>

/*!
 * \brief The NotesBaseEntry class.
 *
 * Auxiliary class containing note info. In most cases you do not need to
 * create NotesBaseEntry object yourself (see NotesKeeper).
 */
class NotesBaseEntry
{
public:
  /*!
   * \brief NotesBaseEntry constructor.
   */
  NotesBaseEntry();

  /*!
   * \brief NotesBaseEntry constructor.
   * \param collection_name collection name.
   * \param book_file_full_path absolute path to book file.
   * \param book_path relative path to book in file (in case of archive, empty
   * otherwise).
   */
  NotesBaseEntry(const std::string &collection_name,
                 const std::filesystem::path &book_file_full_path,
                 const std::string &book_path);

  /*!
   * \brief NotesBaseEntry copy constructor.
   */
  NotesBaseEntry(const NotesBaseEntry &other);

  /*!
   * \brief NotesBaseEntry move constructor.
   */
  NotesBaseEntry(NotesBaseEntry &&other);

  /*!
   * \brief operator =
   */
  NotesBaseEntry &
  operator=(const NotesBaseEntry &other);

  /*!
   * \brief operator =
   */
  NotesBaseEntry &
  operator=(NotesBaseEntry &&other);

  /*!
   * \brief operator ==
   */
  bool
  operator==(const NotesBaseEntry &other) const;

  /*!
   * \brief Collection name.
   */
  std::string collection_name;

  /*!
   * \brief Absolute path to book file.
   */
  std::filesystem::path book_file_full_path;

  /*!
   * \brief Relative path to book in file (in case of archive, empty
   * otherwise).
   */
  std::string book_path;

  /*!
   * \brief Absolute path to note file.
   */
  std::filesystem::path note_file_full_path;
};

#endif // NOTESBASEENTRY_H
