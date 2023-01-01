/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

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

#ifndef INCLUDE_BOOKOPWINDOWS_H_
#define INCLUDE_BOOKOPWINDOWS_H_

#include "MainWindow.h"
class MainWindow;

class BookOpWindows
{
public:
  BookOpWindows(MainWindow *mw);
  virtual
  ~BookOpWindows();
  void
  searchBook(Gtk::ComboBoxText *coll_nm, Gtk::Entry *surname_ent,
	     Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
	     Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent);
  void
  bookRemoveWin(int variant, Gtk::Window *win);
  void
  fileInfo();
  void
  editBook();
  void
  bookSaveRestore(Gtk::Window *win,
		  std::vector<std::tuple<std::string, std::string>> *bookv,
		  int variant);
private:
  MainWindow *mw;
};

#endif /* INCLUDE_BOOKOPWINDOWS_H_ */
