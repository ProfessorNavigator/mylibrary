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

#include <XMLTagId.h>

XMLTagId::XMLTagId()
{

}

XMLTagId::~XMLTagId()
{

}

XMLTagId::XMLTagId(const XMLTagId &other)
{
  if(this != &other)
    {
      end_tag = other.end_tag;
      single_tag = other.single_tag;
      tag_id = other.tag_id;
    }
}

XMLTagId::XMLTagId(XMLTagId &&other)
{
  if(this != &other)
    {
      end_tag = other.end_tag;
      single_tag = other.single_tag;
      tag_id = other.tag_id;
    }
}

XMLTagId&
XMLTagId::operator=(const XMLTagId &other)
{
  if(this != &other)
    {
      end_tag = other.end_tag;
      single_tag = other.single_tag;
      tag_id = other.tag_id;
    }
  return *this;
}

XMLTagId&
XMLTagId::operator=(XMLTagId &&other)
{
  if(this != &other)
    {
      end_tag = other.end_tag;
      single_tag = other.single_tag;
      tag_id = other.tag_id;
    }
  return *this;
}

