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

#ifndef BOOKINFO_H
#define BOOKINFO_H

#include <ArchEntry.h>
#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookInfoEntry.h>
#include <memory>
#include <string>

/*!
 * \brief The BookInfo class.
 *
 * This class contains methods to get extra information (like annotation,
 * cover, source paper book info, etc) from books.
 */
class BookInfo
{
public:
  /*!
   * \brief BookInfo constructor.
   * \param af smart pointer to AuxFunc object.
   */
  BookInfo(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Retruns information about book.
   *
   * See also set_dpi().
   *
   * \deprecated This method is deprecated and will be removed infuture
   * releases. Use getBookInfo() instead.
   *
   * \param bbe search result, returned by BaseKeeper::searchBook() method.
   * \return Smart pointer to BookInfoEntry object containing various
   * information about book.
   */
  __attribute__((deprecated)) std::shared_ptr<BookInfoEntry>
  get_book_info(const BookBaseEntry &bbe);

  /*!
   * \brief Retruns information about book.
   *
   * See also setDpi().
   * \param bbe search result, returned by BaseKeeper::searchBook() method.
   * \return Smart pointer to BookInfoEntry object containing various
   * information about book.
   */
  std::shared_ptr<BookInfoEntry>
  getBookInfo(const BookBaseEntry &bbe);

  /*!
   * \brief Sets DPI.
   *
   * This method should be called before get_book_info(). It sets <A
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A> to display
   * books cover correctly. Default values are 72.0 and 72.0. It is not
   * compulsory to call this method, but it is highly recommended.
   *
   * \param h_dpi horizontal
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A>.
   * \param v_dpi vertical
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A>.
   */
  __attribute__((deprecated)) void
  set_dpi(const double &h_dpi, const double &v_dpi);

  /*!
   * \brief Sets DPI.
   *
   * This method should be called before getBookInfo(). It sets <A
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A> to display
   * books cover correctly. Default values are 72.0 and 72.0. It is not
   * compulsory to call this method, but it is highly recommended.
   *
   * \param h_dpi horizontal
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A>.
   * \param v_dpi vertical
   * HREF="https://en.wikipedia.org/wiki/Dots_per_inch">DPI</A>.
   */
  void
  setDpi(const double &h_dpi, const double &v_dpi);

private:
  std::shared_ptr<BookInfoEntry>
  getFromArchive(const BookBaseEntry &bbe, const std::string &ext);

  bool
  compareFunc(const ArchEntry &ent, const bool &encoding,
              const std::string &conv_nm, const std::filesystem::path &ch_fbd);

  std::shared_ptr<AuxFunc> af;
  double h_dpi = 72.0;
  double v_dpi = 72.0;
};

#endif // BOOKINFO_H
