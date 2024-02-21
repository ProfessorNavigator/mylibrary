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

#ifndef INCLUDE_FULLSIZECOVER_H_
#define INCLUDE_FULLSIZECOVER_H_

#include <BookInfoEntry.h>
#include <cairomm-1.16/cairomm/context.h>
#include <cairomm-1.16/cairomm/refptr.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gdkmm/pixbuf.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

class FullSizeCover
{
public:
  FullSizeCover(const std::shared_ptr<BookInfoEntry> &bie,
		Gtk::Window *parent_window);
  virtual
  ~FullSizeCover();

  void
  createWindow();

private:
  void
  calculateSizes();

  void
  cover_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
	     int height);

  std::shared_ptr<BookInfoEntry> bie;
  Gtk::Window *parent_window = nullptr;

  Glib::RefPtr<Gdk::Pixbuf> cover_buf;

  int width = 0;
  int height = 0;
};

#endif /* INCLUDE_FULLSIZECOVER_H_ */
