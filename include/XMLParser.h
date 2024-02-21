/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_XMLPARSER_H_
#define INCLUDE_XMLPARSER_H_

#include <AuxFunc.h>
#include <XMLTag.h>
#include <XMLTagId.h>
#include <memory>
#include <string>
#include <vector>

class XMLParser
{
public:
  XMLParser(const std::shared_ptr<AuxFunc> &af);
  virtual
  ~XMLParser();

  std::vector<XMLTag>
  get_tag(const std::string &book, const std::string &tag_id);

  std::vector<XMLTag>
  get_tag(const std::string &book, const std::string &tag_id,
	  const bool &content_decode);

  std::string
  get_book_encoding(const std::string &book);

  std::string
  get_element_attribute(const std::string &element,
			const std::string &attr_name);

  XMLTagId
  get_tag_id(const std::string &tag);

private:
  std::shared_ptr<AuxFunc> af;
};

#endif /* INCLUDE_XMLPARSER_H_ */
