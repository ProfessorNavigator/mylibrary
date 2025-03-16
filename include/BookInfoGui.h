/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef BOOKINFOGUI_H
#define BOOKINFOGUI_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookInfo.h>
#include <BookInfoEntry.h>
#include <GenreGroup.h>
#include <giomm-2.68/giomm/menu.h>
#include <gtkmm-4.0/gdkmm/pixbuf.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/popovermenu.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>
#include <vector>

class BookInfoGui
{
public:
  BookInfoGui(const std::shared_ptr<AuxFunc> &af, Gtk::Window *parent_window);

  virtual ~BookInfoGui();

  BookInfoGui(const std::shared_ptr<AuxFunc> &af, Gtk::Window *parent_window,
              const std::shared_ptr<BookInfoEntry> &bie);

  void
  creatWindow(const BookBaseEntry &bbe);

private:
  Glib::ustring
  translate_genre(const std::string &genre_str);

  Glib::ustring
  translate_genre_func(std::string &genre,
                       const std::vector<GenreGroup> &genre_list);

  void
  formBookSection(const BookBaseEntry &bbe, Gtk::Grid *grid, int &row_num);

  void
  formPaperBookInfoSection(const BookBaseEntry &bbe, Gtk::Grid *grid,
                           int &row_num);

  void
  formEectordocInfoSection(const BookBaseEntry &bbe, Gtk::Grid *grid,
                           int &row_num);

  void
  formFileSection(const BookBaseEntry &bbe, Gtk::Grid *grid, int &row_num);

  Gdk::Rectangle
  screen_size();

  int
  cover_width(Gtk::ScrolledWindow *scrl);

  void
  cover_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

  Glib::RefPtr<Gio::Menu>
  cover_menu();

  void
  show_cover_popup_menu(int num, double x, double y,
                        Gtk::PopoverMenu *pop_menu);

  void
  cover_operations_action_group(Gtk::Window *win);

  void
  cover_full_size(Gtk::Window *win);

  void
  save_cover(Gtk::Window *win);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;

  BookInfo *bi = nullptr;

  std::shared_ptr<BookInfoEntry> bie;
  Glib::RefPtr<Gdk::Pixbuf> cover_buf;
};

#endif // BOOKINFOGUI_H
