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
#ifndef XMLELEMENTATTRIBUTE_H
#define XMLELEMENTATTRIBUTE_H

#include <string>

/*!
 * \brief The XMLElementAttribute class
 *
 * This class contains XML element attribute.
 */
class XMLElementAttribute
{
public:
  /*!
   * \brief XMLElementAttribute constructor.
   */
  XMLElementAttribute();

  /*!
   * \brief XMLElementAttribute copy constructor.
   * \param other XMLElementAttribute to be copied.
   */
  XMLElementAttribute(const XMLElementAttribute &other);

  /*!
   * \brief XMLElementAttribute move constructor.
   * \param other XMLElementAttribute to be moved.
   */
  XMLElementAttribute(XMLElementAttribute &&other);

  /*!
   * \brief operator =
   * \param other XMLElementAttribute to be copied.
   * \return XMLElementAttribute
   */
  XMLElementAttribute &
  operator=(const XMLElementAttribute &other);

  /*!
   * \brief attribute_id Attribute name.
   */
  std::string attribute_id;

  /*!
   * \brief attribute_value Attribute value.
   */
  std::string attribute_value;
};

#endif // XMLELEMENTATTRIBUTE_H
