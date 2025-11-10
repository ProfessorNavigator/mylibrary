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
#ifndef XMLPARSERCPP_H
#define XMLPARSERCPP_H

#include <XMLElement.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/*!
 * \mainpage XMLParserCPP
 *
 *  \b XMLParserCPP is a library for XML files parsing.
 *
 * To start add  cmake package XMLParserCPP to your project.
 *
 * \code{.unparsed}
 * find_package(XMLParserCPP)
 * if(XMLParserCPP_FOUND)
 *  target_link_libraries(myproject XMLParserCPP::XMLParserCPP)
 * endif()
 * \endcode
 *
 * Further reading: XMLParserCPP, XMLAlgorithms.
 */

/*!
 * \brief The XMLParserCPP class
 *
 * XMLParserCPP contains methods for XML file parsing.
 */
class XMLParserCPP
{
public:
  /*!
   * \brief XMLParserCPP constructor.
   */
  XMLParserCPP();

  /*!
   * \brief Parses XML document.
   *
   * This method parses XML document and returns vector of found XML elements.
   *
   * \note This method can throw std::exception in case of any errors.
   *
   * \param xml_document XML document content.
   * \return Vector of found XML elements.
   */
  std::vector<XMLElement>
  parseDocument(const std::string &xml_document);

private:
  void
  parseElementAttribute(const std::string &document, size_t &position,
                        std::shared_ptr<XMLElement> element);

  void
  parseProgramControlElement(const std::string &document, size_t &position,
                             std::shared_ptr<XMLElement> element);

  void
  parseSpecialElement(const std::string &document, size_t &position,
                      std::shared_ptr<XMLElement> element);

  void
  parseElementContent(const std::string &document, size_t &position,
                      std::shared_ptr<XMLElement> element);

  void
  replaceXMLEntities(std::vector<XMLElement> &elements);

  void
  replacementFunc(std::string &str);

  std::vector<std::tuple<std::string, std::string>> replacement;
};

#endif // XMLPARSERCPP_H
