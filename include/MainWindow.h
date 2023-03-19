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
#ifndef ML_GTK_OLD
#include "ModelColumns.h"
#endif

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
  creationPulseWin(Gtk::Window *window, int *cncl);
  void
  formCollCombo(Gtk::ComboBoxText *combo);
#ifdef ML_GTK_OLD
  void
  rowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);
#endif
  void
  drawCover(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
  void
  bookAddWin(Gtk::Window *win, Gtk::Entry *book_path_ent,
	     Gtk::Entry *book_nm_ent);
  void
  bookAddWinFunc(Gtk::Window *win, Gtk::ComboBoxText *ext);
#ifdef ML_GTK_OLD
  void
  createBookmark();
#endif
  bool
  closeFunc();
  Gdk::Rectangle
  screenRes();
  void
  readCollection(Gtk::ComboBoxText *collect_box);

  Gtk::ProgressBar *coll_cr_prog = nullptr;
  std::string cover_image = "";
  std::string cover_image_path = "";
  std::vector<
      std::tuple<std::string, std::vector<std::tuple<std::string, std::string>>>> *genrev =
      nullptr;
  std::string prev_search_nm;
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> search_result_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> base_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  std::string active_genre = "nill";
  std::vector<Gtk::Expander*> expv;
#ifdef ML_GTK_OLD
  Gtk::TreeView *bm_tv = nullptr;
  std::vector<
        std::tuple<std::string, std::string, std::string, std::string,
  	  std::string, std::string>> bookmark_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
#endif
  std::mutex *searchmtx = nullptr;
#ifndef ML_GTK_OLD
  std::vector<Glib::RefPtr<Gtk::ColumnViewColumn>> search_res_col;
  guint *ms_sel_book = nullptr;
  Glib::RefPtr<ModelColumns> ms_sel_item;
  guint *bm_sel_book = nullptr;
  Glib::RefPtr<Gio::ListStore<ModelColumns>> list_sr;
  std::vector<
      std::tuple<Glib::RefPtr<ModelColumns>, Gtk::Label*, Gtk::Label*,
	  Gtk::Label*, Gtk::Label*, Gtk::Label*>> ms_style_v;
#endif
};

#endif /* INCLUDE_MAINWINDOW_H_ */
