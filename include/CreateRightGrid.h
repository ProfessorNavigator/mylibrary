/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_CREATERIGHTGRID_H_
#define INCLUDE_CREATERIGHTGRID_H_

#ifndef ML_GTK_OLD
#include <gtkmm.h>
#include "ModelColumns.h"

struct style_item
{
  Glib::RefPtr<ModelColumns>item;
  Gtk::Label *authors = nullptr;
  Gtk::Label *book = nullptr;
  Gtk::Label *series = nullptr;
  Gtk::Label *genre = nullptr;
  Gtk::Label *date = nullptr;
};
#endif

#include "MainWindow.h"

class MainWindow;

class CreateRightGrid
{
public:
  CreateRightGrid(MainWindow *mw);
  virtual
  ~CreateRightGrid();
  Gtk::Grid*
  formRightGrid();
  void
  searchResultShow();
#ifndef ML_GTK_OLD
  std::string
  genre_str(std::string src);
#endif
#ifdef ML_GTK_OLD
  void
  searchResultShow(int variant, Gtk::Window *win);
#endif
private:
#ifdef ML_GTK_OLD
  Gtk::Grid*
  formPopoverGrid(Gtk::TreeView *sres, Gtk::Popover *book_popover);
#endif
#ifndef ML_GTK_OLD
  Gtk::Grid*
  formPopoverGrid(Gtk::ColumnView *sres, Gtk::Popover *book_popover);
  void
  rowActivated(guint rownum, Gtk::ColumnView *sres);
#endif
  MainWindow *mw = nullptr;
};

#endif /* INCLUDE_CREATERIGHTGRID_H_ */
