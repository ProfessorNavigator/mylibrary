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

#ifndef PDFPARSER_H
#define PDFPARSER_H

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <memory>
#include <string>

class PDFParser
{
public:
  PDFParser(const std::shared_ptr<AuxFunc> &af);

  BookParseEntry
  pdf_parser(const std::string &file);

  std::shared_ptr<BookInfoEntry>
  pdf_annotation_n_cover(const std::string &file, const double &x_dpi,
                         const double &y_dpi);

private:
  std::shared_ptr<AuxFunc> af;
};

#endif // PDFPARSER_H
