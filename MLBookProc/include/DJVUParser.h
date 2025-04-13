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

#ifndef DJVUPARSER_H
#define DJVUPARSER_H

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <filesystem>
#include <libdjvu/ddjvuapi.h>
#include <memory>

/*!
 * \brief The DJVUParser class.
 *
 * This class contains various methods for djvu books processing. In most cases
 * you do not need to use this class directly. Use CreateCollection,
 * RefreshCollection and BookInfo instead.
 */
class DJVUParser
{
public:
  /*!
   * \brief DJVUParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  DJVUParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Parses djvu book.
   * \param filepath absolute path to djvu book.
   * \return BookParseEntry object.
   */
  BookParseEntry
  djvu_parser(const std::filesystem::path &filepath);

  /*!
   * \brief Returns book info and book cover.
   * \param filepath absolute path to djvu book.
   * \return Smart pointer to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  djvu_book_info(const std::filesystem::path &filepath);

private:
  bool
  handleDJVUmsgs(const std::shared_ptr<ddjvu_context_t> &ctx,
                 const std::shared_ptr<ddjvu_document_t> &doc,
                 const bool &wait = bool(true));

  void
  getTag(const std::string &exp, const std::string &tag, std::string &line);

  std::shared_ptr<AuxFunc> af;
};

#endif // DJVUPARSER_H
