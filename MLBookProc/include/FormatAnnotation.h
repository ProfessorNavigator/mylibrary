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
#include <XMLParser.h>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The FormatAnnotation class.
 *
 * This class contains different methods for annotation processing.
 */
class FormatAnnotation : XMLParser
{
public:
  /*!
   * \brief FormatAnnotation constructor.
   * \param af smart pointer to AuxFunc object.
   */
  FormatAnnotation(const std::shared_ptr<AuxFunc> &af);

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
  remove_escape_sequences(std::string &annotation);

  /*!
   * \brief Replaces XML tags.
   *
   * It is highly recommended to call setTagReplacementTable() method before
   * this method call (but it is not compulsory). If tag replacement table is
   * empty, all tags will be just removed.
   * \param annotation UTF-8 annotation content string.
   */
  void
  replace_tags(std::string &annotation);

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
  final_cleaning(std::string &annotation);

  /*!
   * \brief Simply removes all XML tags.
   * \param annotation UTF-8 annotation content string.
   */
  void
  removeAllTags(std::string &annotation);

  /*!
   * \brief Sets tag replacement table.
   *
   * If you need to replace XML tags by your own tags or text, you should call
   * this method before replace_tags() call to set replacement table.
   * \param replacement_table vector containing ReplaceTagItem objects.
   */
  void
  setTagReplacementTable(const std::vector<ReplaceTagItem> &replacement_table);

private:
  void
  replace_html(std::string &annotation, const std::string &sstr,
               const std::string &replacement);

  void
  recursiveReplacement(const std::vector<XMLTag> &tgv,
                       std::string &annotation);

  std::vector<ReplaceTagItem> replacement_table;
};

#endif // FORMATANNOTATION_H
