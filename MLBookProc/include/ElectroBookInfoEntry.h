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

#ifndef ELECTROBOOKINFOENTRY_H
#define ELECTROBOOKINFOENTRY_H

#include <string>

/*!
 * \brief The ElectroBookInfoEntry class.
 *
 * Auxiliary class containing various technical info about digital book file.
 */
class ElectroBookInfoEntry
{
public:
  /*!
   * \brief ElectroBookInfoEntry constructor.
   */
  ElectroBookInfoEntry();

  /*!
   * \brief ElectroBookInfoEntry copy constructor.
   */
  ElectroBookInfoEntry(const ElectroBookInfoEntry &other);

  /*!
   * \brief ElectroBookInfoEntry move constructor.
   */
  ElectroBookInfoEntry(ElectroBookInfoEntry &&other);

  /*!
   * \brief operator =
   */
  ElectroBookInfoEntry &
  operator=(const ElectroBookInfoEntry &other);

  /*!
   * \brief operator =
   */
  ElectroBookInfoEntry &
  operator=(ElectroBookInfoEntry &&other);

  /*!
   * \brief Indicates if any information is available.
   */
  bool available = false;

  /*!
   * \brief Author of file.
   */
  std::string author;

  /*!
   * \brief Program used to create file.
   */
  std::string program_used;

  /*!
   * \brief Date of file creation.
   */
  std::string date;

  /*!
   * \brief Source URL if this document is a conversion of some other (online)
   * document.
   */
  std::string src_url;

  /*!
   * \brief Author of the original (online) document, if this is a conversion.
   */
  std::string src_ocr;

  /*!
   * \brief Unique file identifier.
   */
  std::string id;

  /*!
   * \brief File version.
   */
  std::string version;

  /*!
   * \brief History of file changes.
   */
  std::string history;

  /*!
   * \brief File publisher.
   */
  std::string publisher;
};

#endif // ELECTROBOOKINFOENTRY_H
