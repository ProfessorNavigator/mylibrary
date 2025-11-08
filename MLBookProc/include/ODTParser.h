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
#ifndef ODTPARSER_H
#define ODTPARSER_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookInfoEntry.h>
#include <DublinCoreParser.h>
#include <LibArchive.h>
#include <XMLParserCPP.h>

/*!
 * \brief The ODTParser class
 *
 * This class contains methods for odt files processing. In most cases you do
 * not need to use this class directly. Use CreateCollection, RefreshCollection
 * and BookInfo instead.
 */
class ODTParser : public LibArchive
{
public:
  /*!
   * \brief ODTParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  ODTParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief ODTParser destructor.
   */
  virtual ~ODTParser();

  /*!
   * \brief Parses odt files.
   *
   * This method can be used to obtain information from odt files.
   *
   * \note This method can throw std::exception in case of some errors.
   *
   * \param odt_path absolute path to odt file.
   * \return BookParseEntry object.
   */
  BookParseEntry
  odtParser(const std::filesystem::path &odt_path);

  /*!
   * \brief Gets some extra info from odt files.
   *
   * This method can be used to obtain odt file cover and some other info (see
   * BookInfoEntry) if such info is avaliable.
   *
   * \note This method can throw std::exception in case of some errors.
   * \param odt_path absolute path to odt file.
   * \return Smart pointer to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  odtBookInfo(const std::filesystem::path &odt_path);

private:
  std::shared_ptr<AuxFunc> af;

  DublinCoreParser *dc;
  XMLParserCPP *xml_parser;
};

#endif // ODTPARSER_H
