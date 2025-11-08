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
#ifndef XMLALGORITHMS_H
#define XMLALGORITHMS_H

#include <XMLElement.h>

/*!
 * \brief The XMLAlgorithms class
 *
 * XMLAlgorithms class contains various methods for XMLElement processing.
 */
class XMLAlgorithms
{
public:
  /*!
   * \brief XMLAlgorithms constructor.
   */
  XMLAlgorithms();

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches for XML elements of a given name within vector of
   * XMLElement provided. Search is carried out in subelements as well (see
   * XMLElement documentation).
   *
   * \warning Resulting pointers can become invalid if any changes are made to
   * the source elements vector.
   *
   * \param elements Vector of XMLElement search to be carried out on.
   * \param element_name Desired XML element name.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement> &elements,
                const std::string &element_name,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches XML elements of given type within vector of
   * XMLElement provided. Search is carried out in subelements as well (see
   * XMLElement documentation).
   *
   * \warning Resulting pointers can become invalid if any changes are made to
   * the source elements vector.
   *
   * \param elements Vector of XMLElement search to be carried out on.
   * \param element_type Desired XML element type.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement> &elements,
                const XMLElement::Type &element_type,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches XML elements of given name containing attribute of
   * given ID within vector of XMLElement provided. Search is carried out in
   * subelements as well (see XMLElement documentation).
   *
   * \warning Resulting pointers can become invalid if any changes are made to
   * the source elements vector.
   *
   * \param elements Vector of XMLElement search to be carried out on.
   * \param element_name Desired XML element name.
   * \param attribute_id Desired XML element attribute name.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement> &elements,
                const std::string &element_name,
                const std::string &attribute_id,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches XML elements of given name containing attribute of
   * given ID set to given value within vector of XMLElement provided. Search
   * is carried out in subelements as well (see XMLElement documentation).
   *
   * \warning Resulting pointers can become invalid if any changes are made to
   * the source elements vector.
   *
   * \param elements Vector of XMLElement search to be carried out on.
   * \param element_name Desired XML element name.
   * \param attribute_id Desired attribute name.
   * \param attribute_value Desired attribute value.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement> &elements,
                const std::string &element_name,
                const std::string &attribute_id,
                const std::string &attribute_value,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches for XML elements of a given name within vector of
   * pointers to XMLElement provided. Search is carried out in subelements as
   * well (see XMLElement documentation).
   *
   * \param elements Vector of pointers to XMLElement search to be carried
   * out on.
   * \param element_name Desired XML element name.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement *> &elements,
                const std::string &element_name,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches XML elements of given type within vector of poiters
   * to XMLElement provided. Search is carried out in subelements as well (see
   * XMLElement documentation).
   *
   * \param elements Vector of pointers to XMLElement search to be carried
   * out on.
   * \param element_type Desired XML element type.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement *> &elements,
                const XMLElement::Type &element_type,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches XML elements of given name containing attribute of
   * given ID within vector of pointers to XMLElement provided. Search is
   * carried out in subelements as well (see XMLElement documentation).
   *
   * \param elements Vector of pointers to XMLElement search to be carried
   * out on.
   * \param element_name Desired XML element name.
   * \param attribute_id Desired XML element attribute name.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement *> &elements,
                const std::string &element_name,
                const std::string &attribute_id,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Searches for XML elements.
   *
   * This method searches XML elements of given name containing attribute of
   * given ID set to given value within vector of pointers to XMLElement
   * provided. Search is carried out in subelements as well (see XMLElement
   * documentation).
   *
   * \param elements Vector of pointers to XMLElement search to be carried
   * out on.
   * \param element_name Desired XML element name.
   * \param attribute_id Desired XML element attribute name.
   * \param attribute_value Desired XML element attribute value.
   * \param result Vector of pointers to XMLElement search results to be
   * appended to.
   */
  static void
  searchElement(const std::vector<XMLElement *> &elements,
                const std::string &element_name,
                const std::string &attribute_id,
                const std::string &attribute_value,
                std::vector<XMLElement *> &result);

  /*!
   * \brief Writes XML elements to given string.
   * \param elements Vector of XML elements to be written.
   * \param result String XML elements are to be written to.
   */
  static void
  writeXML(const std::vector<XMLElement> &elements, std::string &result);

  /*!
   * \brief Writes XML elements to given string.
   * \param elements Vector of  pointers to XMLElement to be written.
   * \param result String XML elements are to be written to.
   */
  static void
  writeXML(const std::vector<XMLElement *> &elements, std::string &result);

private:
  void
  writeXMLRecursive(const std::vector<XMLElement> &elements,
                    std::string &result, size_t &tab_count);

  void
  writeXMLRecursive(const std::vector<XMLElement *> &elements,
                    std::string &result, size_t &tab_count);

  void
  copyAndReplaceProhibitedSymbols(const std::string &source,
                                  std::string &result);
};

#endif // XMLALGORITHMS_H
