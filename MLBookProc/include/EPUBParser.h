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

#ifndef EPUBPARSER_H
#define EPUBPARSER_H

#include <ArchEntry.h>
#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <LibArchive.h>
#include <XMLParser.h>
#include <XMLTag.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The EPUBParser class.
 *
 * This class contains various methods for epub books parsing. In most cases
 * you do not need to use this class directly. Use CreateCollection,
 * RefreshCollection and BookInfo instead.
 */
class EPUBParser : public XMLParser, public LibArchive
{
public:
  /*!
   * \brief EPUBParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  EPUBParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Parses epub book.
   * \param filepath absolute path to epub file.
   * \return BookParseEntry object.
   */
  BookParseEntry
  epub_parser(const std::filesystem::path &filepath);

  /*!
   * \brief Returns epub book info and cover.
   * \param filepath absolute path to epub file.
   * \return Smart pointer to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  epub_book_info(const std::filesystem::path &filepath);

private:
  std::string
  epub_get_root_file_address(const std::filesystem::path &filepath,
                             const std::vector<ArchEntry> &filenames);

  BookParseEntry
  epub_parse_root_file(const std::string &root_file_content);

  std::string
  epubTitle(const std::string &root_file_content,
            const std::vector<XMLTag> &tgv);

  std::string
  epubAuthor(const std::string &root_file_content,
             const std::vector<XMLTag> &tgv);

  std::string
  epubGenre(const std::string &root_file_content,
            const std::vector<XMLTag> &tgv);

  std::string
  epubDate(const std::string &root_file_content,
           const std::vector<XMLTag> &tgv);

  std::string
  epub_annotation(const std::string &root_file_content);

  std::string
  epub_cover_address(const std::string &root_file_content);

  void
  epub_language(const std::string &root_file_content, BookInfoEntry &result);

  void
  epub_translator(const std::string &root_file_content, BookInfoEntry &result);

  void
  epub_publisher(const std::string &root_file_content, BookInfoEntry &result);

  void
  epub_identifier(const std::string &root_file_content, BookInfoEntry &result);

  void
  epub_source(const std::string &root_file_content, BookInfoEntry &result);

  std::shared_ptr<AuxFunc> af;
};

#endif // EPUBPARSER_H
