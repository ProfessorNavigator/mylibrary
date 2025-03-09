/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

class FB2Parser : public XMLParser
{
public:
  FB2Parser(const std::shared_ptr<AuxFunc> &af);

  BookParseEntry
  fb2_parser(const std::string &book);

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
