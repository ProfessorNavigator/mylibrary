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

#ifndef FULLSIZECOVER_H
#define FULLSIZECOVER_H

#include <BookInfoEntry.h>
#include <gtkmm-4.0/gdkmm/pixbuf.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

class FullSizeCover
{
public:
  FullSizeCover(const std::shared_ptr<BookInfoEntry> &bie,
                Gtk::Window *parent_window);

  void
  createWindow();

private:
  void
  calculateSizes();

  void
  cover_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

  std::shared_ptr<BookInfoEntry> bie;
  Gtk::Window *parent_window = nullptr;

  Glib::RefPtr<Gdk::Pixbuf> cover_buf;

  int width = 0;
  int height = 0;
};

#endif // FULLSIZECOVER_H
