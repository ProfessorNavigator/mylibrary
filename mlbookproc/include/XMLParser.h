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

#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <AuxFunc.h>
#include <MLException.h>
#include <XMLTag.h>
#include <memory>
#include <string>
#include <vector>

class XMLParser
{
public:
  XMLParser(const std::shared_ptr<AuxFunc> &af);

  std::vector<XMLTag>
  get_tag(const std::string &book, const std::string &tag_id);

  std::string
  get_book_encoding(const std::string &book);

  std::string
  get_element_attribute(const std::string &element,
                        const std::string &attr_name);

  std::vector<XMLTag>
  listAllTags(const std::string &book);

  void
  searchTag(const std::vector<XMLTag> &list, const std::string &tag_id,
            std::vector<XMLTag> &result);

  void
  htmlSymbolsReplacement(std::string &book);

  void
  removeAllTags(std::string &book);

private:
  enum tag_type
  {
    single,
    has_content,
    end_tag,
    spec_tag
  };

  std::string
  tagElement(const std::string &book, const std::string::size_type &start,
             std::string::size_type &end, tag_type &tg_type);

  void
  tagContent(const std::string &book, const std::string::size_type &start,
             XMLTag &tag, std::string::size_type &tag_end);

  void
  tagId(XMLTag &tag);

  std::shared_ptr<AuxFunc> af;
};

#endif // XMLPARSER_H
