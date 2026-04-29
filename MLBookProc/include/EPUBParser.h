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
#ifndef EPUBPARSER_H
#define EPUBPARSER_H

#include <BaseID.h>
#include <DublinCoreParser.h>
#include <LibArchive.h>
#include <MLBookProc.h>
#include <UDBase.h>

/*!
 * \brief The EPUBParser class
 *
 * This class contains methods for epub files parsing. In most cases you do not
 * need to use them directly. Use CreateCollection and BookInfo instead.
 */
class EPUBParser
{
public:
  /*!
   * \brief EPUBParser constructor.
   * \param mlbp Smart pointer to MLBookProcObject.
   */
  EPUBParser(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~EPUBParser();

  /*!
   * Parses epub book.
   *
   * \param book_content epub file content.
   * \return BaseID::Book object.
   */
  UDBElement
  parseBook(const std::string &book_content);

  /*!
   * Obtains extra inforamtion about book (see BookInfo).
   *
   * \param book_content epub file content.
   * \return UDBase object containing found information.
   */
  UDBase
  getBookInfo(const std::string &book_content);

private:
  std::string
  epubGetRootFileAddress(
      const std::string &book_content,
      const std::vector<std::tuple<std::string, uint64_t, uint64_t>>
          &filenames);

  void
  epubParseRootFile(const std::string &root_file_content, UDBElement &result);

  std::string
  epubCoverAddress1(const std::vector<XMLElement> &root_file_content);

  std::string
  epubCoverAddress2(
      const std::vector<XMLElement> &root_file_content,
      const std::string &root_file_path, const std::string &book_content,
      const std::vector<std::tuple<std::string, uint64_t, uint64_t>>
          &filenames);

  bool
  imageSearchFunction1(const XMLElement &el);

  bool
  imageSearchFunction2(const XMLElement &el);

  void
  normalizeString(std::string &str);

  std::shared_ptr<MLBookProc> mlbp;

  DublinCoreParser *dc_parser;
  XMLParserCPP *xml_parser;
  LibArchive *arch;

  BaseID bid;
};

#endif // EPUBPARSER_H
