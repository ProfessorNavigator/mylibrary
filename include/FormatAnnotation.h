/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FORMATANNOTATION_H
#define FORMATANNOTATION_H

#include <AuxFunc.h>
#include <XMLParser.h>
#include <memory>
#include <string>
#include <vector>

class FormatAnnotation : XMLParser
{
public:
  FormatAnnotation(const std::shared_ptr<AuxFunc> &af);

  void
  remove_escape_sequences(std::string &annotation);

  void
  replace_tags(std::string &annotation);

  void
  final_cleaning(std::string &annotation);

  void
  replace_html_symbols(std::string &annotation);

private:
  struct replace_tag
  {
    std::string tag_to_replace;
    std::string begin_replacement;
    std::string end_replacement;
  };

  void
  formReplacementTable();

  void
  tag_replacement_process(const std::string &tag, std::string &annotation,
                          std::string::size_type &n);

  void
  replace_html(std::string &annotation, const std::string &sstr,
               const std::string &replacement);

  std::vector<replace_tag> replacement_table;
};

#endif // FORMATANNOTATION_H
