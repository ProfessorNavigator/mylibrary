/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_BOOKINFO_H_
#define INCLUDE_BOOKINFO_H_

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookInfoEntry.h>
#include <ZipFileEntry.h>
#include <memory>
#include <string>

class BookInfo
{
public:
  BookInfo(const std::shared_ptr<AuxFunc> &af);
  virtual
  ~BookInfo();

  std::shared_ptr<BookInfoEntry>
  get_book_info(const BookBaseEntry &bbe);

  void
  set_dpi(const double &h_dpi, const double &v_dpi);

private:
  std::shared_ptr<BookInfoEntry>
  get_from_archive(const BookBaseEntry &bbe, const std::string &ext);

  bool
  compare_func(const ZipFileEntry &ent, const bool &encoding,
	       const std::string &conv_nm, const std::filesystem::path &ch_fbd);

  std::shared_ptr<AuxFunc> af;
  double h_dpi = 72.0;
  double v_dpi = 72.0;
};

#endif /* INCLUDE_BOOKINFO_H_ */
