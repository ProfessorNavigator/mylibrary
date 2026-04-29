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
#ifndef DJVUPARSER_H
#define DJVUPARSER_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <libdjvu/ddjvuapi.h>

/*!
 * \brief The DJVUParser class
 *
 * This class contains methods for djvu files parsing. In most cases you do not
 * need to use it directly. Use CreateCollection and BookInfo instead.
 */
class DJVUParser
{
public:
  /*!
   * \brief DJVUParser constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  DJVUParser(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   * Parses djvu file.
   *
   * \param book_content Content of djvu file.
   * \return BaseID::Book object.
   */
  UDBElement
  parseBook(const std::string &book_content);

  /*!
   * Obtains extra information from djvu file.
   *
   * \param book_content Content of djvu file.
   * \return UDBase object containing informatiton (see BookInfo).
   */
  UDBase
  getBookInfo(const std::string &book_content);

private:
  bool
  setBookContentToStream(const std::shared_ptr<DJVUContext> &ctx,
                         const std::shared_ptr<ddjvu_document_t> &doc,
                         const std::string &book_content);

  bool
  waitDocumentInfo(const std::shared_ptr<DJVUContext> &ctx,
                   const std::shared_ptr<ddjvu_document_t> &doc);

  std::shared_ptr<ddjvu_page_t>
  getFirstPage(const std::shared_ptr<DJVUContext> &ctx,
               const std::shared_ptr<ddjvu_document_t> &doc);

  void
  getTag(const std::string &exp, const std::string &tag, std::string &line);

  std::shared_ptr<MLBookProc> mlbp;

  BaseID bid;
};

#endif // DJVUPARSER_H
