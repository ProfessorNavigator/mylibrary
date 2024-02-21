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

#ifndef INCLUDE_COVERPIXBUF_H_
#define INCLUDE_COVERPIXBUF_H_

#include <BookInfoEntry.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gdkmm/pixbuf.h>
#include <memory>

class CoverPixBuf
{
public:
  CoverPixBuf(const std::shared_ptr<BookInfoEntry> &bie,
	      const int &width, const int &height);
  virtual
  ~CoverPixBuf();

  CoverPixBuf(const std::shared_ptr<BookInfoEntry> &bie);

  CoverPixBuf(const CoverPixBuf &other);

  CoverPixBuf(CoverPixBuf &&other);

  CoverPixBuf&
  operator=(const CoverPixBuf &other);

  CoverPixBuf&
  operator=(CoverPixBuf &&other);

  operator Glib::RefPtr<Gdk::Pixbuf>();

  int
  get_width();

  int
  get_height();

private:
  void
  createBuffer();

  void
  createFromStream(const Glib::RefPtr<Glib::Bytes> &bytes);

  void
  createFromRgba(const bool &alpha);

  std::shared_ptr<BookInfoEntry> bie;

  Glib::RefPtr<Gdk::Pixbuf> buffer;

  int width = -1;
  int height = -1;
};

#endif /* INCLUDE_COVERPIXBUF_H_ */
