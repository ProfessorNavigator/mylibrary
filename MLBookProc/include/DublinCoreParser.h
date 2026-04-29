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

#include <BaseID.h>
#include <UDBElement.h>
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

  virtual ~DublinCoreParser();

  /*!
   * Obtains book title.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::BookTitle objects.
   */
  std::vector<UDBElement>
  dcTitle(const std::vector<XMLElement> &elements);

  /*!
   * Obtains authors.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::Author objects.
   */
  std::vector<UDBElement>
  dcAuthor(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book genres.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::Genre objects.
   */
  std::vector<UDBElement>
  dcGenre(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book creation date.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::Date objects.
   */
  std::vector<UDBElement>
  dcDate(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book description.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Book description if any, empty otherwise.
   */
  std::string
  dcDescription(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book language.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::Language objects.
   */
  std::vector<UDBElement>
  dcLanguage(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book translators.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::Translator objects.
   */
  std::vector<UDBElement>
  dcTranslator(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book publisher.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::EbookPublisher objects.
   */
  std::vector<UDBElement>
  dcPublisher(const std::vector<XMLElement> &elements);

  /*!
   * Obtains book ID.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::EbookID objects.
   */
  std::vector<UDBElement>
  dcIdentifier(const std::vector<XMLElement> &elements);

  /*!
   * Obtains source book info.
   *
   * \param elements Parsed <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \return Vector of BaseID::SourceBookDublinCore objects.
   */
  std::vector<UDBElement>
  dcSource(const std::vector<XMLElement> &elements);

private:
  std::vector<UDBElement>
  getAuthor1(const std::vector<XMLElement> &elements);

  std::vector<UDBElement>
  getAuthor2(const std::vector<XMLElement> &elements);

  std::vector<UDBElement>
  getTranslator1(const std::vector<XMLElement> &elements);

  std::vector<UDBElement>
  getTranslator2(const std::vector<XMLElement> &elements);

  void
  normalizeString(std::string &str);

  XMLParserCPP *xml_parser;

  BaseID bid;
};

#endif // DUBLINCOREPARSER_H
