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

#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <AuxFunc.h>
#include <MLException.h>
#include <XMLTag.h>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The XMLParser class.
 *
 * This class containes various methods to process <A
 * HREF="https://www.w3schools.com/xml/default.asp">XML</A> documents.
 */
class XMLParser
{
public:
  /*!
   * \brief XMLParser constructor.
   * \param af smart pointer to AuxFunc constructor.
   */
  XMLParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Returns all tags with particular name.
   *
   * This method uses listAllTags() and searchTag() under hood.
   *
   * \note This method can throw MLException in case of errors.
   * \param book <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document content.
   * \param tag_id requested tag name.
   * \return Vector of found tags.
   */
  std::vector<XMLTag>
  get_tag(const std::string &book, const std::string &tag_id);

  /*!
   * \brief Returns <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document encoding.
   *
   * Tries to get <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document header and read encoding from it.
   * \param book <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document content.
   * \return Returns <A
   * HREF="https://www.w3schools.com/xml/default.asp">XML</A> document encoding
   * if it was found.
   */
  std::string
  get_book_encoding(const std::string &book);

  /*!
   * \brief Returns <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * tag attribute if it was found.
   * \param element tag start element (see XMLTag::element).
   * \param attr_name requested attribute name.
   * \return Returns attribute content if it was found.
   */
  std::string
  get_element_attribute(const std::string &element,
                        const std::string &attr_name);

  /*!
   * \brief Parses <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document.
   *
   * Returns all found tags.
   * \note This method can throw MLException in case of errors.
   * \param book <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document content.
   * \return Vector of found tags.
   */
  std::vector<XMLTag>
  listAllTags(const std::string &book);

  /*!
   * \brief Searches tag in tag list.
   *
   * Searches tags in tag list by given tag name.
   * \param list tag list search to be carried out in.
   * \param tag_id tag name for tags to be searched.
   * \param result vector for results (can be not empty, this methods appends
   * found tags to existing results).
   */
  void
  searchTag(const std::vector<XMLTag> &list, const std::string &tag_id,
            std::vector<XMLTag> &result);

  /*!
   * \brief Replaces symbols encoded by "&..." sequences.
   *
   * Replaces symbols encoded by "&..." sequences for actual values (see <A
   * HREF="https://www.w3schools.com/xml/xml_syntax.asp">this</A> for details).
   * \param book <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document content symbols to be replaced in.
   */
  void
  htmlSymbolsReplacement(std::string &book);

  /*!
   * \brief Removes all tag elements from <A
   * HREF="https://www.w3schools.com/xml/default.asp">XML</A> document.
   *
   * Simply removes all found <A
   * HREF="https://www.w3schools.com/xml/default.asp">XML</A> tag elements and
   * leaves only document content.
   * \param book <A HREF="https://www.w3schools.com/xml/default.asp">XML</A>
   * document content.
   */
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
             const std::string::size_type &book_end, XMLTag &tag,
             std::string::size_type &tag_end);

  void
  tagId(XMLTag &tag);

  std::shared_ptr<AuxFunc> af;
};

#endif // XMLPARSER_H
