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

#ifndef FORMATANNOTATION_H
#define FORMATANNOTATION_H

#include <AuxFunc.h>
#include <ReplaceTagItem.h>
#include <XMLParserCPP.h>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The FormatAnnotation class.
 *
 * This class contains different methods for annotation processing.
 */
class FormatAnnotation
{
public:
  /*!
   * \brief FormatAnnotation constructor.
   * \param af smart pointer to AuxFunc object.
   */
  FormatAnnotation(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief FormatAnnotation destructor.
   */
  virtual ~FormatAnnotation();

  /*!
   * \brief Removes escape sequences from annotation.
   *
   * Removes <A
   * HREF="https://en.cppreference.com/w/cpp/language/ascii">ASCII</A> symbols
   * 0 to 31 (inclusive) from annotation. Only exception is symbol 9
   * (horizontal tab). It will be replaced by 32 (space). Also removes extra
   * spaces from annotation beginning.
   *
   * \deprecated This method will be removed in future releases. Use
   * removeEscapeSequences() instead.
   *
   * \param annotation UTF-8 annotation content string.
   */
  __attribute__((deprecated)) void
  remove_escape_sequences(std::string &annotation);

  /*!
   * \brief Removes escape sequences from annotation.
   *
   * Removes <A
   * HREF="https://en.cppreference.com/w/cpp/language/ascii">ASCII</A> symbols
   * 0 to 31 (inclusive) from annotation. Only exception is symbol 9
   * (horizontal tab). It will be replaced by 32 (space). Also removes extra
   * spaces from annotation beginning.
   * \param annotation UTF-8 annotation content string.
   */
  void
  removeEscapeSequences(std::string &annotation);

  /*!
   * \brief Replaces XML tags.
   *
   * It is highly recommended to call setTagReplacementTable() method before
   * this method call (but it is not compulsory). If tag replacement table is
   * empty, all tags will be just removed.
   *
   * \deprecated This method will be removed in future releases. Use
   * replaceTags() instead.
   *
   * \param annotation UTF-8 annotation content string.
   */
  __attribute__((deprecated)) void
  replace_tags(std::string &annotation);

  /*!
   * \brief Replaces XML tags.
   *
   * It is highly recommended to call setTagReplacementTable() method before
   * this method call (but it is not compulsory). If tag replacement table is
   * empty, all tags will be just removed. In case of errors, this method does
   * nothing.
   * \param annotation UTF-8 annotation content string.
   */
  void
  replaceTags(std::string &annotation);

  /*!
   * \brief Cleans some sequences from annotation.
   *
   * Replaces "three spaces" by "two spaces" at the annotation beginning,
   * removes "\n" from annotation beginning, removes 0 - 32 <A
   * HREF="https://en.cppreference.com/w/cpp/language/ascii">ASCII</A> symbols
   * from the annotation end. Also replaces "\n\n\n" sequences by "\n\n" in
   * whole annotation.
   *
   * \deprecated This method will be removed in future releases. Use
   * finalCleaning() instead.
   *
   * \param annotation UTF-8 annotation content string.
   */
  __attribute__((deprecated)) void
  final_cleaning(std::string &annotation);

  /*!
   * \brief Cleans some sequences from annotation.
   *
   * Replaces "three spaces" by "two spaces" at the annotation beginning,
   * removes "\n" from annotation beginning, removes 0 - 32 <A
   * HREF="https://en.cppreference.com/w/cpp/language/ascii">ASCII</A> symbols
   * from the annotation end. Also replaces "\n\n\n" sequences by "\n\n" in
   * whole annotation.
   * \param annotation UTF-8 annotation content string.
   */
  void
  finalCleaning(std::string &annotation);

  /*!
   * \brief Simply removes all XML tags.
   *
   * In case of errors, this method does nothing.
   *
   * \param annotation UTF-8 annotation content string.
   */
  void
  removeAllTags(std::string &annotation);

  /*!
   * \brief Sets tag replacement table.
   *
   * If you need to replace XML tags by your own tags or text, you should call
   * this method before replaceTags() call to set replacement table.
   * \param replacement_table vector containing ReplaceTagItem objects.
   * \param symbols_replacement Vector of tuples. First element of tuple is
   * string to be replaced in tag content, second element is replacement
   * string.
   */
  void
  setTagReplacementTable(
      const std::vector<ReplaceTagItem> &replacement_table,
      const std::vector<std::tuple<std::string, std::string>>
          &symbols_replacement);

private:
  void
  recursiveReplacement(const std::vector<XMLElement> &elements,
                       std::string &annotation);

  void
  recursiveTagRemoving(const std::vector<XMLElement> &elements,
                       std::string &annotation);

  std::string
  replaceSymbols(const std::string &source);

  XMLParserCPP *xml_parser;

  std::vector<ReplaceTagItem> replacement_table;

  std::vector<std::tuple<std::string, std::string>> symbols_replacement;

  std::shared_ptr<AuxFunc> af;
};

#endif // FORMATANNOTATION_H
