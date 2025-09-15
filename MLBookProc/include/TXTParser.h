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
#ifndef TXTPARSER_H
#define TXTPARSER_H

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <filesystem>

/*!
 * \brief The TXTParser class
 *
 * This class contains methods for txt files processing. In most cases you do
 * not need to use this class directly. Use CreateCollection, RefreshCollection
 * and BookInfo instead.
 */
class TXTParser
{
public:
  /*!
   * \brief TXTParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  TXTParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Parses txt files
   *
   * This method can be used to obtain information from txt files.
   *
   * \param txt_path absolute path to txt file.
   * \return BookParseEntry object.
   */
  BookParseEntry
  txtParser(const std::filesystem::path &txt_path);

  /*!
   * \brief Gets some extra info from txt files.
   *
   * See also BookInfoEntry.
   *
   * \param txt_path absolute path to txt file.
   * \return Smart pointer to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  txtBookInfo(const std::filesystem::path &txt_path);

private:
  std::shared_ptr<AuxFunc> af;
};

#endif // TXTPARSER_H
