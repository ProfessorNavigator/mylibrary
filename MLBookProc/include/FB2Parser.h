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

#ifndef FB2PARSER_H
#define FB2PARSER_H

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <XMLParserCPP.h>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The FB2Parser class.
 *
 * This class contains various methods for fb2 books parsing. In most cases you
 * do not need to use this class directly. Use CreateCollection,
 * RefreshCollection and BookInfo instead.
 */
class FB2Parser
{
public:
  /*!
   * \brief FB2Parser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  FB2Parser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief FB2Parser destructor.
   */
  virtual ~FB2Parser();

  /*!
   * \brief Parses fb2 book.
   *
   * \deprecated This method will be removed in future releases. Use
   * fb2Parser() instead.
   *
   * \note This method can throw std::exception object in case of any errors.
   * \param book fb2 file content.
   * \return BookParseEntry object.
   */
  __attribute__((deprecated)) BookParseEntry
  fb2_parser(const std::string &book);

  /*!
   * \brief Parses fb2 book.
   *
   * \note This method can throw std::exception object in case of any errors.
   * \param book fb2 file content.
   * \return BookParseEntry object.
   */
  BookParseEntry
  fb2Parser(const std::string &book);

  /*!
   * \brief Returns fb2 book info and cover.
   *
   * \deprecated This method will be removed in future releases. Use
   * fb2BookInfo() instead.
   *
   * \param book fb2 books file content.
   * \return Smart poiner to BookInfoEntry object.
   */
  __attribute__((deprecated)) std::shared_ptr<BookInfoEntry>
  fb2_book_info(const std::string &book);

  /*!
   * \brief Returns fb2 book info and cover.
   * \param book fb2 books file content.
   * \return Smart poiner to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  fb2BookInfo(const std::string &book);

private:
  BookParseEntry
  fb2Description();

  std::string
  fb2Author(const std::vector<XMLElement *> &author);

  std::string
  fb2Series(const std::vector<XMLElement *> &sequence);

  std::string
  fb2Genres(const std::vector<XMLElement *> &genres);

  std::string
  fb2Date(const std::vector<XMLElement *> &date);

  void
  fb2AnnotationDecode(std::string &result);

  BookInfoEntry::cover_types
  fb2Cover(std::string &cover);

  void
  fb2CoverGetImage(const std::vector<XMLElement *> &image, std::string &cover,
                   BookInfoEntry::cover_types &result);

  void
  fb2ExtraInfo(BookInfoEntry &result);

  void
  fb2Language(const std::vector<XMLElement *> &lang, std::string &result);

  void
  fb2PublisherInfo(BookInfoEntry &result);

  void
  fb2PublisherInfoString(const std::vector<XMLElement *> &source,
                         std::string &result);

  void
  fb2PublisherInfoString(const std::vector<XMLElement> &source,
                         std::string &result);

  void
  fb2ElectroDocInfo(BookInfoEntry &result);

  std::string
  getBookEncoding();

  std::string
  getBookEncoding(const std::string &book);

  void
  findAnnotationFallback(const std::string &book, BookInfoEntry &result);

  void
  normalizeString(std::string &str);

  std::shared_ptr<AuxFunc> af;

  XMLParserCPP *xml_parser;
  std::vector<XMLElement> book_xml;
  std::vector<XMLElement *> title_info;
};

#endif // FB2PARSER_H
