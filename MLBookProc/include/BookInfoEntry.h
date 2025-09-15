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

#ifndef BOOKINFOENTRY_H
#define BOOKINFOENTRY_H

#include <ElectroBookInfoEntry.h>
#include <PaperBookInfoEntry.h>
#include <string>

/*!
 * \brief The BookInfoEntry class.
 *
 * Auxiliary class keeping various extra information about book (see
 * BookInfo).
 */
class BookInfoEntry
{
public:
  /*!
   * \brief BookInfoEntry constructor.
   */
  BookInfoEntry();

  /*!
   * \brief BookInfoEntry destructor.
   */
  virtual ~BookInfoEntry();

  /*!
   * \brief BookInfoEntry copy constructor.
   */
  BookInfoEntry(const BookInfoEntry &other);

  /*!
   * \brief BookInfoEntry move constructor.
   */
  BookInfoEntry(BookInfoEntry &&other);

  /*!
   * \brief operator =
   */
  BookInfoEntry &
  operator=(const BookInfoEntry &other);

  /*!
   * \brief operator =
   */
  BookInfoEntry &
  operator=(BookInfoEntry &&other);

  /*!
   * \brief Book annotation.
   */
  std::string annotation;

  /*!
   * \brief Book cover image.
   */
  std::string cover;

  /*!
   * \brief The cover types enumerator.
   */
  enum cover_types
  {
    base64, /*!< base64 image*/
    file,   /*!< various image files formats, like JPG*/
    rgb,    /*!< RGB image*/
    rgba,   /*!< RGBA image*/
    bgra,   /*!< BGRA image*/
    text,   /*!< Not image, just raw text*/
    error   /*!< no image, default value*/
  };

  /*!
   * \brief Type of image.
   */
  cover_types cover_type = cover_types::error;

  /*!
   * \brief Book language.
   */
  std::string language;

  /*!
   * \brief Language of book source.
   */
  std::string src_language;

  /*!
   * \brief Translator.
   */
  std::string translator;

  /*!
   * \brief Pointer to PaperBookInfoEntry.
   */
  PaperBookInfoEntry *paper = nullptr;

  /*!
   * \brief Various technical information about book file. See
   * ElectroBookInfoEntry.
   */
  ElectroBookInfoEntry *electro = nullptr;

  /*!
   * \brief Number of bytes per row for RGB and RGBA images.
   */
  int bytes_per_row = 0;
};

#endif // BOOKINFOENTRY_H
