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
#ifndef INPXLOADER_H
#define INPXLOADER_H

#include <BaseID.h>
#include <LibArchive.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <archive_entry.h>
#include <filesystem>

/*!
 * \brief The InpxLoader class
 *
 * This class methods converts inpx file to BaseKeeper database. In most cases
 * you do not need to use this class directly. Use BaseKeeper instead.
 */
class InpxLoader : public LibArchive
{
public:
  /*!
   * \brief InpxLoader constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  InpxLoader(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   * Creates datdbase recognized by BaseKeeper from inpx file.
   *
   * \param books_directory Path to directory containing books.
   * \param inpx_file_path Path to inpx file.
   * \return
   */
  UDBase
  loadInpxCollection(const std::filesystem::path &books_directory,
                     const std::filesystem::path &inpx_file_path);

private:
  void
  parseEntry(std::shared_ptr<archive> a, std::shared_ptr<archive_entry> e,
             const std::filesystem::path &books_directory);

  void
  parseInpFile(const std::string &buf,
               const std::filesystem::path &result_path);

  UDBElement
  parseBookEntry(const std::string &buf);

  std::vector<UDBElement>
  parseAuthors(const std::string &buf);

  UDBElement
  parseAuthor(const std::string &buf);

  std::vector<UDBElement>
  parseGenres(const std::string &buf);

  std::vector<std::filesystem::path> books_directory_files;

  UDBase result;

  BaseID bid;
};

#endif // INPXLOADER_H
