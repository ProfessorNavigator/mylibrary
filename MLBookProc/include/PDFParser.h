/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>

/*!
 * \brief The PDFParser class
 *
 * This class contains methods for pdf files parsing. In most cases you do not
 * need to use it directly. Usd CreateCollection and BookInfo instead.
 */
class PDFParser
{
public:
  /*!
   * \brief PDFParser constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  PDFParser(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   * Parses pdf file.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param book_content File content.
   * \return BaseID::Book object.
   */
  UDBElement
  parseBook(const std::string &book_content);

  /*!
   * Gets extra information from file.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param book_content File content.
   * \return UDBase object containing found information.
   */
  UDBase
  getBookInfo(const std::string &book_content);

  /*!
   * Sets DPI (see BookInfo::setDPI).
   *
   * \param horizontal_dpi Horizontal DPI.
   * \param vertical_dpi Vertical DPI.
   */
  void
  setDPI(const double &horizontal_dpi = double(72.0),
         const double &vertical_dpi = double(72.0));

  /*!
   * Returns horizontal DPI.
   *
   * \return Horizontal DPI.
   */
  double
  getHorizontalDPI();

  /*!
   * Returns vertical DPI.
   *
   * \return Vertical DPI.
   */
  double
  getVerticalDPI();

private:
  void
  normalizeString(std::string &str);

  std::shared_ptr<MLBookProc> mlbp;

  double horizontal_dpi = 72.0;
  double vertical_dpi = 72.0;

  BaseID bid;
};

#endif // PDFPARSER_H
