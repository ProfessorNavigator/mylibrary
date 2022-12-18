/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of MyLibrary.
 MyLibrary is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 MyLibrary is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with MyLibrary. If not,
 see <https://www.gnu.org/licenses/>.
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
  void
  collectionOpFunc(Gtk::ComboBoxText *cmb, Gtk::Window *win, int variant);
  void
  collectionCreate();
  void
  collectionCreateFunc(Gtk::Entry *coll_ent, Gtk::Entry *path_ent,
		       Gtk::Window *par_win);
  void
  openDialogCC(Gtk::Window *window, Gtk::Entry *path_ent, int variant);
  void
  collectionRefresh(Gtk::ComboBoxText *cmb, Gtk::Window *win1,
		    Gtk::Window *win2);
  void
  importCollection();
  void
  importCollectionFunc(Gtk::Window *window, Gtk::Entry *coll_nm_ent,
		       Gtk::Entry *coll_path_ent, Gtk::Entry *book_path_ent);
  void
  exportCollection();
  void
  exportCollectionFunc(Gtk::ComboBoxText *cmb, Gtk::Entry *exp_path_ent,
		       Gtk::Window *win);
private:
  MainWindow *mw;
};

#endif /* INCLUDE_COLLECTIONOPWINDOWS_H_ */
