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

#ifndef INCLUDE_AUXWINDOWS_H_
#define INCLUDE_AUXWINDOWS_H_

#include "MainWindow.h"
class MainWindow;

class AuxWindows
{
public:
  AuxWindows(MainWindow *mw);
  virtual
  ~AuxWindows();
  void
  errorWin(int type, Gtk::Window *par_win);
  void
  bookmarkWindow();
  void
  bookCopyConfirm(Gtk::Window *win, std::mutex *addbmtx, int *stopper);
  void
  aboutProg();
private:
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gio::ListStore<ModelColumns>>
  form_col_view(
      Gtk::ColumnView *sres,
      std::vector<
	  std::tuple<std::string, std::string, std::string, std::string,
	      std::string, std::string>> &bookmark_v,
      std::shared_ptr<
	  std::vector<
	      std::tuple<Glib::RefPtr<ModelColumns>, Gtk::Label*, Gtk::Label*,
		  Gtk::Label*, Gtk::Label*, Gtk::Label*>>> style_v);
  void
  removeBMDialog(
      Gtk::Window *par_win,
      Glib::RefPtr<Gio::ListStore<ModelColumns>> list,
      Glib::RefPtr<Glib::ObjectBase> rem_item,
      std::shared_ptr<
	  std::vector<
	      std::tuple<Glib::RefPtr<ModelColumns>, Gtk::Label*, Gtk::Label*,
		  Gtk::Label*, Gtk::Label*, Gtk::Label*>>> style_v);
  void
  removeBMFunc(Glib::RefPtr<Gio::ListStore<ModelColumns>> list,
	       Glib::RefPtr<Glib::ObjectBase> rem_item);
#endif
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  formBmVector();
  MainWindow *mw;
};

#endif /* INCLUDE_AUXWINDOWS_H_ */
