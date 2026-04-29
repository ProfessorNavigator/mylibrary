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
#ifndef BOOKINFO_H
#define BOOKINFO_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>

/*!
 * \brief The BookInfo class
 *
 * This class contains methods for receiving extra information about books.
 */
class BookInfo
{
public:
  /*!
   * \brief BookInfo constructor
   * \param mlbp Smart pointer to MLBookProc object.
   */
  BookInfo(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   * Obtains extra information about book.
   *
   * Result can contain objects of following types: BaseID::CoverPage,
   * BaseID::Annotation, BaseID::Author, BaseID::Keywords, BaseID::Language,
   * BaseID::SourceLanguage, BaseID::Translator, BaseID::SourceBookTitle,
   * BaseID::SourceBookAuthor, BaseID::SourceBookSequence,
   * BaseID::SourceBookGenre, BaseID::SourceBookDate,
   * BaseID::SourceBookKeywords, BaseID::SourceBookLanguage,
   * BaseID::SourceBookSourceLanguage, BaseID::SourceBookTranslator,
   * BaseID::EbookAuthor, BaseID::EbookProgramUsed, BaseID::EbookDate,
   * BaseID::EbookSourceUrl, BaseID::EbookSourceOCR, BaseID::EbookID,
   * BaseID::EbookVersion, BaseID::EbookHistory, BaseID::EbookPublisher,
   * BaseID::PaperBookName, BaseID::PaperBookPublisher, BaseID::PaperBookCity,
   * BaseID::PaperBookYear, BaseID::PaperBookISBN, BaseID::PaperBookSequence,
   * BaseID::CustomInfo, BaseID::DjvuPublisher
   *
   * \param book_search_result UDBElement of BaseID::BookSearchResult type.
   * \return UDBase object containing various information about book.
   */
  UDBase
  getBookInfo(const UDBElement &book_search_result);

  /*!
   * Sets internal <a
   * href="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</a> values. It is
   * needed for correct book cover creation of some book types. This method
   * should be called before getBookInfo().
   *
   * Default values are: 72.0, 72.0.
   *
   * \param horizontal_dpi Horizontal DPI.
   * \param vertical_dpi Vertical DPI.
   */
  void
  setDPI(const double &horizontal_dpi = double(72.0),
         const double &vertical_dpi = double(72.0));

  /*!
   * Returns horizontal DPI.
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
  UDBase
  bookInfo(const std::filesystem::path &p, const UDBElement &path);

  std::shared_ptr<MLBookProc> mlbp;

  double horizontal_dpi = 72.0;
  double vertical_dpi = 72.0;

  BaseID bid;
};

#endif // BOOKINFO_H
