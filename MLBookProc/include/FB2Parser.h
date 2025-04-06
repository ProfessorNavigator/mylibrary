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
#include <XMLParser.h>
#include <XMLTag.h>
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
class FB2Parser : public XMLParser
{
public:
  /*!
   * \brief FB2Parser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  FB2Parser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Parses fb2 book.
   *
   * \note This method can throw MLException object in case of any errors.
   * \param book fb2 file content.
   * \return BookParseEntry object.
   */
  BookParseEntry
  fb2_parser(const std::string &book);

  /*!
   * \brief Returns fb2 book info and cover.
   * \param book fb2 books file content.
   * \return Smart poiner to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  fb2_book_info(const std::string &book);

private:
  BookParseEntry
  fb2_description(const std::string &book);

  std::string
  fb2_author(const std::string &book, const std::vector<XMLTag> &author);

  std::string
  fb2_series(const std::vector<XMLTag> &sequence);

  std::string
  fb2_genres(const std::string &book, const std::vector<XMLTag> &genres);

  void
  fb2_annotation_decode(const std::string &book, std::string &result);

  void
  fb2_cover(const std::string &book, std::string &cover);

  void
  fb2_extra_info(const std::string &book, BookInfoEntry &result);

  void
  fb2_publisher_info(const std::string &book, BookInfoEntry &result);

  void
  fb2_electro_doc_info(const std::string &book, BookInfoEntry &result);

  void
  fb2_elctor_publisher(const std::string &book, const std::vector<XMLTag> &tgv,
                       BookInfoEntry &result);

  void
  recursiveGetTags(const std::string &book, const std::vector<XMLTag> &tgv,
                   std::string &result);

  std::shared_ptr<AuxFunc> af;
};

#endif // FB2PARSER_H
