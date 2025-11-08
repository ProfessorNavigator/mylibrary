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
#include <XMLElementAttribute.h>

XMLElementAttribute::XMLElementAttribute()
{
}

XMLElementAttribute::XMLElementAttribute(const XMLElementAttribute &other)
{
  attribute_id = other.attribute_id;
  attribute_value = other.attribute_value;
}

XMLElementAttribute::XMLElementAttribute(XMLElementAttribute &&other)
{
  attribute_id = std::move(other.attribute_id);
  attribute_value = std::move(other.attribute_value);
}

XMLElementAttribute &
XMLElementAttribute::operator=(const XMLElementAttribute &other)
{
  if(this != &other)
    {
      attribute_id = other.attribute_id;
      attribute_value = other.attribute_value;
    }
  return *this;
}
