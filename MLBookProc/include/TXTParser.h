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
#ifndef TXTPARSER_H
#define TXTPARSER_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <filesystem>

/*!
 * \brief The TXTParser class
 *
 * This class contains methods for txt and md files parsing. In most cases you
 * do not need to use it directly, use CreateCollection and BookInfo instead.
 */
class TXTParser
{
public:
  /*!
   * \brief TXTParser constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  TXTParser(const std::shared_ptr<MLBookProc> &mlbp);

  /*!
   *  Parses file.
   *
   * \param file_path Path to file to be parsed.
   * \return BaseID::Book object.
   */
  UDBElement
  parseBook(const std::filesystem::path &file_path);

  /*!
   * Gets extra info from file.
   *
   * \param file_path Path to file.
   * \return UDBase containing information.
   */
  UDBase
  getBookInfo(const std::filesystem::path &file_path);

private:
  std::shared_ptr<MLBookProc> mlbp;

  BaseID bid;
};

#endif // TXTPARSER_H
