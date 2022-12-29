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

#ifndef INCLUDE_MAINWINDOW_H_
#define INCLUDE_MAINWINDOW_H_

#include <gtkmm.h>
#include <iostream>
#include <libintl.h>
#include <vector>
#include <tuple>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include "AuxFunc.h"
#include "CreateCollection.h"
#include "SearchBook.h"
#include "AnnotationCover.h"
#include "RefreshCollection.h"
#include "CreateLeftGrid.h"
#include "CreateRightGrid.h"
#include "CollectionOpWindows.h"
#include "BookOpWindows.h"
#include "AuxWindows.h"

class MainWindow : public Gtk::ApplicationWindow
{
  friend class CreateLeftGrid;
  friend class CreateRightGrid;
  friend class CollectionOpWindows;
  friend class BookOpWindows;
  friend class AuxWindows;
public:
  MainWindow();
  virtual
  ~MainWindow();
private:
  void
  mainWindow();
  void
  collectionCreate();
  void
  collectionOpFunc(Gtk::ComboBoxText *cmb, Gtk::Window *win,
		   Gtk::CheckButton *rem_empty_ch, int variant);
  void
  collectionOp(int variant);
  void
  collectionRefresh(Gtk::ComboBoxText *cmb, Gtk::CheckButton *rem_empty_ch,
		    Gtk::Window *win1, Gtk::Window *win2);
  void
  collectionCreateFunc(Gtk::Entry *coll_ent, Gtk::Entry *path_ent,
		       Gtk::Entry *thr_ent, Gtk::Window *par_win);
  void
  creationPulseWin(Gtk::Window *window);
  void
  openDialogCC(Gtk::Window *window, Gtk::Entry *path_ent, int variant);
  void
  errorWin(int type, Gtk::Window *par_win, Glib::Dispatcher *disp);
  void
  formCollCombo(Gtk::ComboBoxText *combo);
  void
  rowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);
  void
  drawCover(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
  void
  openBook(int variant);
  void
  copyTo();
  void
  saveDialog(std::filesystem::path filepath, bool archive);
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
  void
  bookAddWin(Gtk::Window *win, Gtk::Entry *book_path_ent,
	     Gtk::Entry *book_nm_ent);
  void
  bookAddWinFunc(Gtk::Window *win, Gtk::CheckButton *ch_pack);
  void
  bookCopyConfirm(Gtk::Window *win, std::mutex *addbmtx, int *stopper);
  void
  createBookmark();
  void
  bookmarkWindow();
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
  void
  aboutProg();
  bool
  closeFunc();

  Gtk::ProgressBar *coll_cr_prog = nullptr;
  int coll_cr_cancel = 0;
  Glib::Dispatcher *search_compl_disp = nullptr;
  int search_cancel = 0;
  int coll_refresh_cancel = 0;
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> bookmark_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book

  std::string cover_image = "";
  std::string cover_image_path = "";
  Glib::RefPtr<Gtk::CssProvider> css_provider;
  std::vector<
      std::tuple<std::string, std::vector<std::tuple<std::string, std::string>>>> *genrev =
      nullptr;
  std::string prev_search_nm;
protected:
  void
  searchBook(Gtk::ComboBoxText *coll_nm, Gtk::Entry *surname_ent,
	     Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
	     Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent);
  Gdk::Rectangle
  screenRes();
  void
  readCollection(Gtk::ComboBoxText *collect_box);
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> search_result_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> base_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  std::string active_genre = "nill";
  std::vector<Gtk::Expander*> expv;
  Gtk::TreeView *bm_tv = nullptr;
  std::mutex *searchmtx = nullptr;
};

#endif /* INCLUDE_MAINWINDOW_H_ */
