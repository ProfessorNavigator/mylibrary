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

#ifndef XMLTAG_H
#define XMLTAG_H

#include <string>
#include <vector>

class XMLTag
{
public:
  XMLTag();

  XMLTag(const XMLTag &other);

  XMLTag &
  operator=(const XMLTag &other);

  XMLTag(XMLTag &&other);

  XMLTag &
  operator=(XMLTag &&other);

  bool
  hasContent() const;

  std::string element;
  std::string tag_id;
  std::string::size_type content_start;
  std::string::size_type content_end;
  std::vector<XMLTag> tag_list;
};

#endif // XMLTAG_H
