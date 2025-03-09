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

#include <XMLTag.h>

XMLTag::XMLTag()
{
  content_start = std::string::npos;
  content_end = std::string::npos;
}

XMLTag::XMLTag(const XMLTag &other)
{
  element = other.element;
  tag_id = other.tag_id;
  tag_list = other.tag_list;
  content_start = other.content_start;
  content_end = other.content_end;
}

XMLTag &
XMLTag::operator=(const XMLTag &other)
{
  if(this != &other)
    {
      element = other.element;
      tag_id = other.tag_id;
      tag_list = other.tag_list;
      content_start = other.content_start;
      content_end = other.content_end;
    }
  return *this;
}

bool
XMLTag::hasContent() const
{
  if(content_start != std::string::npos && content_end != std::string::npos)
    {
      return true;
    }
  else
    {
      return false;
    }
}

XMLTag::XMLTag(XMLTag &&other)
{
  element = std::move(other.element);
  tag_id = std::move(other.tag_id);
  tag_list = std::move(other.tag_list);
  content_start = std::move(other.content_start);
  content_end = std::move(other.content_end);
}

XMLTag &
XMLTag::operator=(XMLTag &&other)
{
  if(this != &other)
    {
      element = std::move(other.element);
      tag_id = std::move(other.tag_id);
      content_start = std::move(other.content_start);
      content_end = std::move(other.content_end);
    }
  return *this;
}
