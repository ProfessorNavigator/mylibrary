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
#ifndef FB2PARSER_H
#define FB2PARSER_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <XMLParserCPP.h>

/*!
 * \brief The FB2Parser class
 *
 * This class contains methods for fb2 files parsing. In most cases you do not
 * call them directly. Use CreateCollection and BookInfo instead.
 */
class FB2Parser
{
public:
  /*!
   * \brief FB2Parser constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  FB2Parser(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~FB2Parser();

  /*!
   * Parses fb2 book.
   *
   * \param file_content fb2 file content.
   * \return BaseID::Book object.
   */
  UDBElement
  parseBook(const std::string &file_content);

  /*!
   * Obtains extra information about book (see BookInfo).
   *
   * \param book_content fb2 file content.
   * \return UDBase object containing found information.
   */
  UDBase
  getBookInfo(const std::string &book_content);

private:
  UDBElement
  fb2GetInfoForBase(const std::vector<XMLElement> &book_xml);

  std::vector<UDBElement>
  fb2Author(const std::vector<XMLElement *> &author);

  void
  fb2AuthorBookInfo(const std::vector<XMLElement *> &author,
                    std::vector<UDBElement> &result);

  void
  fb2Series(const std::vector<XMLElement *> &sequence,
            std::vector<UDBElement> &book_subelements);

  void
  fb2Annotation(const std::vector<XMLElement *> &annotation,
                std::string &result);

  void
  fb2Cover(const std::vector<XMLElement> &book_xml, UDBElement &result);

  void
  fb2CoverGetImage(const std::vector<XMLElement> &book_xml,
                   const std::vector<XMLElement *> &image, UDBElement &result);

  void
  getResult(const std::vector<XMLElement *> &elements,
            std::vector<UDBElement> &result, const BaseID::ID &element_id);

  void
  normalizeString(std::string &str);

  std::shared_ptr<MLBookProc> mlbp;

  XMLParserCPP *xml_parser;

  BaseID bid;
};

#endif // FB2PARSER_H
