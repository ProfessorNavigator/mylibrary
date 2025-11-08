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
#ifndef REPLACETAGITEM_H
#define REPLACETAGITEM_H

#include <string>

/*!
 * \brief The ReplaceTagItem class.
 *
 * Auxiliary class for FormatAnnotation (see
 * FormatAnnotation::setTagReplacementTable()).
 */
class ReplaceTagItem
{
public:
  /*!
   * \brief ReplaceTagItem constructor.
   */
  ReplaceTagItem();

  /*!
   * \brief ReplaceTagItem copy constructor.
   */
  ReplaceTagItem(const ReplaceTagItem &other);

  /*!
   * \brief ReplaceTagItem move constructor.
   */
  ReplaceTagItem(ReplaceTagItem &&other);

  /*!
   * \brief operator =
   */
  ReplaceTagItem &
  operator=(const ReplaceTagItem &other);

  /*!
   * \brief operator =
   */
  ReplaceTagItem &
  operator=(ReplaceTagItem &&other);

  /*!
   * \brief Id of tag to be replaced (see XMLElement::element_name).
   */
  std::string tag_to_replace;

  /*!
   * \brief Replacement for start tag element.
   */
  std::string begin_replacement;

  /*!
   * \brief Replacement for end tag element.
   */
  std::string end_replacement;
};

#endif // REPLACETAGITEM_H
