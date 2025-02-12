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

#ifndef BOOKMARKSGUI_H
#define BOOKMARKSGUI_H

#include <AuxFunc.h>
#include <BookMarks.h>
#include <BookMarksShow.h>
#include <OpenBook.h>
#include <giomm-2.68/giomm/menu.h>
#include <glib-2.0/glib/gtypes.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/popovermenu.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

class BookMarksGui
{
public:
  BookMarksGui(const std::shared_ptr<AuxFunc> &af,
               const std::shared_ptr<BookMarks> &bookmarks,
               Gtk::Window *main_window);

  virtual ~BookMarksGui();

  void
  createWindow();  

private:
  void
  loadWindowSizes();

  void
  setWindowSizesByMonitor();

  void
  saveWindowSizes(Gtk::Window *win);

  void
  slot_row_activated(guint pos);

  void
  creat_bookmarks_action_group(Gtk::Window *win);

  void
  confirmationDialog(Gtk::Window *win);

  Glib::RefPtr<Gio::Menu>
  bookmark_menu();

  void
  show_popup_menu(int num, double x, double y, Gtk::PopoverMenu *pop_menu);

  // TODO remove legacy in future releases
  void
  legacyWarning(Gtk::Window *win);

  std::shared_ptr<AuxFunc> af;
  std::shared_ptr<BookMarks> bookmarks;
  Gtk::Window *main_window = nullptr;

  int window_height = 0;
  int window_width = 0;

  BookMarksShow *bms = nullptr;

  Gtk::ColumnView *book_marks = nullptr;
  OpenBook *open_book = nullptr;
};

#endif // BOOKMARKSGUI_H
