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
#ifndef XMLELEMENT_H
#define XMLELEMENT_H

#include <XMLElementAttribute.h>
#include <string>
#include <vector>

/*!
 * \brief The XMLElement class
 *
 * This class contains different XML element parameters.
 */
class XMLElement
{
public:
  /*!
   * \brief XMLElement constructor.
   */
  XMLElement();

  /*!
   * \brief XMLElement copy constructor.
   * \param other XMLElement to be copied.
   */
  XMLElement(const XMLElement &other);

  /*!
   * \brief XMLElement move constructor.
   * \param other XMLElement to be moved.
   */
  XMLElement(XMLElement &&other);

  /*!
   * \brief operator =
   * \param other XMLElement to be copied.
   * \return XMLElement.
   */
  XMLElement &
  operator=(const XMLElement &other);

  /*!
   * \brief XML element name.
   */
  std::string element_name;

  /*!
   * \brief Vector of XML element attributes.
   */
  std::vector<XMLElementAttribute> element_attributes;

  /*!
   * \brief Vector of XML element subelements.
   */
  std::vector<XMLElement> elements;

  /*!
   * \brief XML element content if any.
   */
  std::string content;

  /*!
   * \brief XML elements types enumerator.
   */
  enum Type
  {
    /*! \"Header\" XML elements like
     * \code{.unparsed}<?xml version="1.0" encoding="UTF-8"?>\endcode
     */
    ProgramControlElement,

    /*! Char data elements \code{.unparsed}<![CDATA[]]>\endcode*/
    CharData,
    /*! XML comments \code{.unparsed}<!-- Comment -->\endcode*/
    Comment,
    /*! Special XML elements like \code{.unparsed}<!DOCTYPE html>\endcode*/
    SpecialElement,
    /*! \"Ordinary\" XML elements like \code{.unparsed}<element>\endcode*/
    OrdinaryElement,
    /*! Indicates XMLElement containing XML element content (element_name is
       empty string)*/
    ElementContent
  };

  /*!
   * \brief XMLElement type.
   */
  Type element_type = Type::OrdinaryElement;

  /*!
   * \brief If \a true indicates XML elements without content.
   *
   * Example:
   * \code{.unsorted}<empty-line/>\endcode
   */
  bool empty = false;
};

#endif // XMLELEMENT_H
