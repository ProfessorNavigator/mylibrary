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
#ifndef XMLTEXTENCODING_H
#define XMLTEXTENCODING_H

#include <string>
#include <vector>

/*!
 * \brief The XMLTextEncoding class
 *
 * This class contains methods for text encodings processing. Some of these
 * methods are suitable not only for XML documents, but also for any text
 * strings.
 */
class XMLTextEncoding
{
public:
  /*!
   * \brief Detects XML document encoding.
   *
   * This method tries to detect encoding from the XML header. If operation was
   * not successfull, tries to detect encoding by <A
   * HREF="https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucsdet_8h.html">ICU
   * library</A> methods.
   * \param document String, containing XML document.
   * \return Text code page. Empty in case of any errors.
   */
  static std::string
  detectDocumentEncoding(const std::string &document);

  /*!
   * \brief Detects text string encoding.
   * \param str String text encoding to be detected in.
   * \param filter If \a true, algorithm ignores html and xml tags.
   * \return Vector with all found encoding names. First element in vector is
   * most suitable result (see <A
   * HREF="https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucsdet_8h.html#a50b60117e8300e81db1671393a785b2a">ICU</A>
   * documentation for details).
   */
  static std::vector<std::string>
  detectStringEncoding(const std::string &str,
                       const bool &filter = bool(false));

  /*!
   * \brief Converts string from one encoding to another.
   * \param source String to be converted.
   * \param result Result of conversion (emty in case of any errors).
   * \param source_code_page Code page of \b source string (if empty, default
   * system encoding is used).
   * \param result_code_page Desired code page of \b resylt string (if empty,
   * resylt will use default system encoding).
   */
  static void
  convertToEncoding(const std::string &source, std::string &result,
                    const std::string &source_code_page,
                    const std::string &result_code_page);
};

#endif // XMLTEXTENCODING_H
