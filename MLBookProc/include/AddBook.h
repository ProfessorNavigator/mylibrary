/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef ADDBOOK_H
#define ADDBOOK_H

#include <AuxFunc.h>
#include <BookMarks.h>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

/*!
 * \brief The AddBook class.
 *
 * AddBook contains various methods to add books to collections.
 */
class AddBook
{
public:
  /*!
   * \brief AddBook constructor.
   *
   * \param af AuxFunc object.
   * \param collection_name name of collection book to be added to.
   * \param remove_sources if \a true, \b MLBookProc will remove source file
   * after book has been added.
   * \param bookmarks BookMarks object.
   */
  AddBook(const std::shared_ptr<AuxFunc> &af,
          const std::string &collection_name, const bool &remove_sources,
          const std::shared_ptr<BookMarks> &bookmarks);

  /*!
   * \brief Adds single book files.
   *
   * Adds books, listed in vector, to collection and parse them. User must set
   * both paths manually.
   * \warning Second tuples paths must not be outside collection
   * books directory.
   * \param books vector, containing absolute paths to source files (first
   * tuple element) and absolute paths of books files in collection (second
   * tuple element).
   */
  void
  simple_add(const std::vector<
             std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  /*!
   * \brief Adds directories containing book files to collection.
   *
   * Same as simple_add(), but adds directories containing books to collection.
   * \warning Second tuples paths must not be outside collection books
   * directory.
   * \param books vector, containing absolute paths to source directories
   * (first tuple element) and absolute paths of directories in collection
   * (second tuple element).
   */
  void
  simple_add_dir(const std::vector<std::tuple<std::filesystem::path,
                                              std::filesystem::path>> &books);

  /*!
   * \brief Removes archive from collection and replaces it by new one.
   *
   * Removes archive from collection (if it exists) and creates new archive
   * with the same name, containing books from books vector.
   *
   * \param archive_path absolute path to archive file in collection.
   * \param books vector, containing absolute paths to source books files
   * (first tuple element) and relative paths to books files in archive (second
   * tuple element).
   */
  void
  overwrite_archive(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  /*!
   * \brief Removes archive from collection and replaces it by new one.
   *
   * Same as overwrite_archive(), but creates new archive, containing
   * directories from books vector.
   * \param archive_path absolute path to archive file in collection.
   * \param books vector, containing absolute paths to source directories
   * (first tuple element) and relative paths to directories in archive (second
   * tuple element).
   */
  void
  overwrite_archive_dir(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  /*!
   * \brief Adds books to existing archive.
   *
   * Unpacks existing archive and packs it again with old and new books inside.
   * \param archive_path absolute path to archive in collection.
   * \param books vector, containing absolute paths to source books files
   * (first tuple element) and relative paths to books in archive (second tuple
   * element).
   */
  void
  add_to_existing_archive(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  /*!
   * \brief Adds books to existing archive.
   *
   * Same as add_to_existing_archive(), but adds directories to existing
   * archive.
   * \param archive_path absolute path to archive in collection.
   * \param books vector, containing absolute paths to source directories
   * (first tuple element) and relative paths to directories in archive (second
   * tuple element).
   */
  void
  add_to_existing_archive_dir(
      const std::filesystem::path &archive_path,
      const std::vector<
          std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  /*!
   * \brief Lists all files in archive.
   *
   * \param archive_path absolute path to archive.
   * \param af smart pointer to AuxFunc object.
   * \return Vector containing relative paths of archive files.
   */
  static std::vector<std::string>
  archive_filenames(const std::filesystem::path &archive_path,
                    const std::shared_ptr<AuxFunc> &af);

private:
  void
  remove_src(const std::vector<
             std::tuple<std::filesystem::path, std::filesystem::path>> &books);

  std::shared_ptr<AuxFunc> af;
  std::string collection_name;
  bool remove_sources = false;
  std::shared_ptr<BookMarks> bookmarks;
};

#endif // ADDBOOK_H
