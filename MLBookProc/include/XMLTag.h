/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef XMLTAG_H
#define XMLTAG_H

#include <string>
#include <vector>

/*!
 * \brief The XMLTag class.
 *
 * Auxiliary class for XMLParser. Contains tag start element content, tag name,
 * index of tag content first byte in <A
 * HREF="https://www.w3schools.com/xml/default.asp">XML</A> document and index
 * of content last byte. Also contains list of tags were found in tag content.
 */
class XMLTag
{
public:
  /*!
   * \brief XMLTag constructor.
   */
  XMLTag();

  /*!
   * \brief XMLTag copy constructor.
   */
  XMLTag(const XMLTag &other);

  /*!
   * \brief operator =
   */
  XMLTag &
  operator=(const XMLTag &other);

  /*!
   * \brief XMLTag move constructor.
   */
  XMLTag(XMLTag &&other);

  /*!
   * \brief operator =
   */
  XMLTag &
  operator=(XMLTag &&other);

  /*!
   * \brief Checks if tag has content.
   *
   * This method returns \a true, if \b content_start and \b content_end are
   * not equal to std::string::npos.
   * \return \a true if tag has content.
   */
  bool
  hasContent() const;

  /*!
   * \brief Tag start element content.
   *
   * Tag start element content including opening "<" and closing ">" symbols.
   */
  std::string element;

  /*!
   * \brief Tag name.
   */
  std::string tag_id;

  /*!
   * \brief Index of first byte of tag content.
   *
   * Index of first byte of tag content in <A
   * HREF="https://www.w3schools.com/xml/default.asp">XML</A> document. Start
   * of tag can be found by \b element size subtration from \b content_start.
   * If \b content_start value is equal to std::string::npos, it indicates
   * error on tag reading (even if tag does not have any content).
   */
  std::string::size_type content_start;

  /*!
   * \brief Index of last byte of tag content.
   *
   * Index of last byte of tag content in <A
   * HREF="https://www.w3schools.com/xml/default.asp">XML</A> document. Can be
   * equal to std::string::npos, if tag does not have content.
   */
  std::string::size_type content_end;

  /*!
   * \brief List of <A
   * HREF="https://www.w3schools.com/xml/default.asp">XML</A> tags, found in
   * tag content (if any).
   */
  std::vector<XMLTag> tag_list;
};

#endif // XMLTAG_H
