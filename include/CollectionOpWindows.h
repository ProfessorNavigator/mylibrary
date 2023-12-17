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

#ifndef INCLUDE_COLLECTIONOPWINDOWS_H_
#define INCLUDE_COLLECTIONOPWINDOWS_H_

#include "MainWindow.h"
class MainWindow;

class CollectionOpWindows
{
public:
  CollectionOpWindows(MainWindow *mw);
  virtual
  ~CollectionOpWindows();
  void
  collectionOp(int variant);
#ifdef ML_GTK_OLD
  void
  collectionOpFunc(Gtk::ComboBoxText *cmb, Gtk::Window *win,
		   Gtk::CheckButton *rem_empty_ch,
		   Gtk::CheckButton *fast_refresh, int variant);
  void
  collectionRefresh(Gtk::ComboBoxText *cmb, Gtk::CheckButton *rem_empty_ch,
  		    Gtk::CheckButton *fast_refresh, Gtk::Window *win);
  void
  exportCollectionFunc(Gtk::ComboBoxText *cmb, Gtk::Entry *exp_path_ent,
  		       Gtk::Window *win);
  void
  bookPathSelInCol(Gtk::ComboBoxText *cmb, Gtk::Entry *book_nm_ent,
  		   Gtk::Window *win);
#endif
#ifndef ML_GTK_OLD
  void
  collectionOpFunc(Gtk::DropDown *cmb, Gtk::Window *win,
		   Gtk::CheckButton *rem_empty_ch,
		   Gtk::CheckButton *fast_refresh, int variant);
  void
  collectionRefresh(Gtk::DropDown *cmb, Gtk::CheckButton *rem_empty_ch,
		    Gtk::CheckButton *fast_refresh, Gtk::Window *win);
  void
  exportCollectionFunc(Gtk::DropDown *cmb, Gtk::Entry *exp_path_ent,
		       Gtk::Window *win);
  void
  bookPathSelInCol(Gtk::DropDown *cmb, Gtk::Entry *book_nm_ent,
		   Gtk::Window *win);
#endif
  void
  collectionCreate();
  void
  collectionCreateFunc(Gtk::Entry *coll_ent, Gtk::Entry *path_ent,
		       Gtk::Entry *thr_ent, Gtk::Window *par_win);
  void
  openDialogCC(Gtk::Window *win, Gtk::Entry *path_ent, int variant);
  void
  importCollection();
  void
  importCollectionFunc(Gtk::Window *window, Gtk::Entry *coll_nm_ent,
		       Gtk::Entry *coll_path_ent, Gtk::Entry *book_path_ent);
  void
  exportCollection();
private:
  MainWindow *mw;
};

#endif /* INCLUDE_COLLECTIONOPWINDOWS_H_ */
