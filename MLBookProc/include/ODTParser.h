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
#ifndef ODTPARSER_H
#define ODTPARSER_H

#include <BaseID.h>
#include <DublinCoreParser.h>
#include <LibArchive.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <XMLParserCPP.h>

/*!
 * \brief The ODTParser class
 *
 * This class contains methods for odt files parsing. In most cases you do not
 * need to use it directly. Use CreateCollection and BookInfo instead.
 */
class ODTParser
{
public:
  /*!
   * \brief ODTParser constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  ODTParser(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~ODTParser();

  /*!
   * Parses odt file.
   *
   * \param book_content File content.
   * \return BaseID::Book object.
   */
  UDBElement
  parseBook(const std::string &book_content);

  /*!
   * Gets extra information from odt file.
   *
   * \param book_content File content.
   * \return UDBase containing found information.
   */
  UDBase
  getBookInfo(const std::string &book_content);

private:
  void
  normalizeString(std::string &str);

  std::shared_ptr<MLBookProc> mlbp;

  LibArchive *la;
  DublinCoreParser *dc_parser;
  XMLParserCPP *xml_parser;

  BaseID bid;
};

#endif // ODTPARSER_H
