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
#include <XMLElement.h>

XMLElement::XMLElement()
{
}

XMLElement::XMLElement(const XMLElement &other)
{
  element_name = other.element_name;
  element_attributes = other.element_attributes;
  elements = other.elements;
  content = other.content;
  element_type = other.element_type;
  empty = other.empty;
}

XMLElement::XMLElement(XMLElement &&other)
{
  element_name = std::move(other.element_name);
  element_attributes = std::move(other.element_attributes);
  elements = std::move(other.elements);
  content = std::move(other.content);
  element_type = std::move(other.element_type);
  empty = std::move(other.empty);
}

XMLElement &
XMLElement::operator=(const XMLElement &other)
{
  if(this != &other)
    {
      element_name = other.element_name;
      element_attributes = other.element_attributes;
      elements = other.elements;
      content = other.content;
      element_type = other.element_type;
      empty = other.empty;
    }
  return *this;
}
