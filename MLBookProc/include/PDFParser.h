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

#ifndef PDFPARSER_H
#define PDFPARSER_H

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <memory>
#include <string>

/*!
 * \brief The PDFParser class.
 *
 * This class contains methods for pdf book parsing, annotations and covers
 * obtaining. In most cases you do not need to use this class directly. Use
 * CreateCollection, RefreshCollection and BookInfo instead.
 */
class PDFParser
{
public:
  /*!
   * \brief PDFParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  PDFParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Parses pdf file.
   * \param file pdf file content.
   * \return BookParseEntry object.
   */
  BookParseEntry
  pdf_parser(const std::string &file);

  /*!
   * \brief Returns pdf book annotation and cover.
   * \param file pdf file content.
   * \param x_dpi horizontal <A
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A>.
   * \param y_dpi vertical
   * <A HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A>.
   * \return Smart pointer to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  pdf_annotation_n_cover(const std::string &file, const double &x_dpi,
                         const double &y_dpi);

private:
  std::shared_ptr<AuxFunc> af;
};

#endif // PDFPARSER_H
