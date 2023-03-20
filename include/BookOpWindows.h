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
#ifndef ML_GTK_OLD
#include "ModelColumns.h"
#include "ModelBoxes.h"
#endif

class MainWindow;

class BookOpWindows
{
public:
  BookOpWindows(MainWindow *mw);
  virtual
  ~BookOpWindows();
#ifdef ML_GTK_OLD
  void
  searchBook(Gtk::ComboBoxText *coll_nm, Gtk::Entry *surname_ent,
	     Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
  	     Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent);
  void
  bookRemoveWin(int variant, Gtk::Window *win, Gtk::TreeView *sres);
  void
  bookRemoveVar1(Gtk::MessageDialog *mess);
  void
  copyTo(Gtk::TreeView *sres, int variant, Gtk::Window *win);
  void
  bookSaveRestore(Gtk::Window *win,
  		  std::vector<std::tuple<std::string, std::string>> *bookv,
  		  int variant);
  void
  bookRemoveVar2(Gtk::Window *win);
  void
  openBook(int variant);
#endif
#ifndef ML_GTK_OLD
  void
  searchBook(Gtk::DropDown *coll_nm, Gtk::Entry *surname_ent,
	     Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
	     Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent);
  void
  bookRemoveWin(Gtk::Window *win);
  void
  bookRemoveVar1(Glib::RefPtr<Gtk::AlertDialog> mess);
  void
  createBookmark();
  void
  copyTo(Gtk::ColumnView *sres, int variant, Gtk::Window *win);
  void
  bookSaveRestore(Gtk::Window *win,
		  std::vector<std::tuple<std::string, std::string>> *bookv,
		  int variant, Glib::RefPtr<ModelColumns> mod_col);
  void
  openBook();
  void
  openBookFunc(Glib::ustring input_path);
  void
  copyToFunc(Glib::ustring input_path, Gtk::Window *win);
#endif
  void
  fileInfo();
  void
  editBook();
  void
  saveDialog(std::filesystem::path filepath, bool archive, Gtk::Window *win);
private:
  MainWindow *mw;
};

#endif /* INCLUDE_BOOKOPWINDOWS_H_ */
