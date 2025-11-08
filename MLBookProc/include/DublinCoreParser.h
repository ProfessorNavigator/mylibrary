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
#ifndef DUBLINCOREPARSER_H
#define DUBLINCOREPARSER_H

#include <XMLParserCPP.h>

/*!
 * \brief The DublinCoreParser class
 *
 * Auxiliary class. Contains methods for <a
 * href="https://www.dublincore.org/">DublinCore</a> files parsing. This class
 * is used in ODTParser and EPUBParser. You do not need to call this class
 * methods directly.
 */

class DublinCoreParser
{
public:
  /*!
   * \brief DublinCoreParser constructor
   */
  DublinCoreParser();

  /*!
   * \brief DublinCoreParser destructor
   */
  virtual ~DublinCoreParser();

  /*!
   * \brief Gets book title.
   *
   * This method can be used to get title from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Title if any, empty string otherwise.
   */
  std::string
  dcTitle(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book author.
   *
   * This method can be used to get author(s) from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Author if any, empty string otherwise.
   */
  std::string
  dcAuthor(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book genre.
   *
   * This method can be used to get genre(s) from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Gernre if any, empty string otherwise.
   */
  std::string
  dcGenre(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book creation date.
   *
   * This method can be used to get creation date from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return File creation date if any, empty otherwise.
   */
  std::string
  dcDate(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book description.
   *
   * This method can be used to get book description from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Book description if any, empty otherwise.
   */
  std::string
  dcDescription(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book language.
   *
   * This method can be used to get language from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return File language if set, empty otherwise.
   */
  std::string
  dcLanguage(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book translator
   *
   * This method can be used to get translator name from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Translator if any, empty string otherwise.
   */
  std::string
  dcTranslator(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets file publisher.
   *
   * This method can be used to get publisher from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return File publisher if set, empty otherwise.
   */
  std::string
  dcPublisher(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book identifier.
   *
   * This method can be used to get identifier from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Book identifier if any, empty otherwise.
   */
  std::string
  dcIdentifier(const std::vector<XMLElement> &elements);

  /*!
   * \brief Gets book source.
   *
   * This method can be used to get book source from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param elements Vector of XML elements, obtained by
   * XMLParserCPP::parseDocument() method.
   * \return Book source if set, empty otherwise.
   */
  std::string
  dcSource(const std::vector<XMLElement> &elements);

private:
  std::string
  getAuthor1(const std::vector<XMLElement> &elements);

  std::string
  getAuthor2(const std::vector<XMLElement> &elements);

  std::string
  getTranslator1(const std::vector<XMLElement> &elements);

  std::string
  getTranslator2(const std::vector<XMLElement> &elements);

  void
  normalizeString(std::string &str);

  XMLParserCPP *xml_parser;
};

#endif // DUBLINCOREPARSER_H
