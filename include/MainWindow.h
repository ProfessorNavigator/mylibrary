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

class CreateLeftGrid;

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

#ifdef ML_GTK_OLD
  void
  rowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);
#endif

  void
  drawCover(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

  void
  bookAddWin(Gtk::Window *win, Gtk::Entry *book_path_ent,
	     Gtk::Entry *book_nm_ent);

#ifndef ML_GTK_OLD
  void
  bookAddWinFunc(Gtk::Window *win, Gtk::DropDown *ext);

  void
  readCollection(Gtk::DropDown *collect_box);
#endif

#ifdef ML_GTK_OLD
  void
  createBookmark();

  void
  bookAddWinFunc(Gtk::Window *win, Gtk::ComboBoxText *ext);

  void
  readCollection(Gtk::ComboBoxText *collect_box);
#endif

  bool
  closeFunc();

  Gdk::Rectangle
  screenRes();

  Gtk::ProgressBar *coll_cr_prog = nullptr;
  cover_image cover_struct;
  std::vector<genres> *genrev = nullptr;
  std::string prev_search_nm;
  std::vector<book_item> search_result_v;
  std::vector<book_item> base_v;
  std::string active_genre = "nill";
  std::vector<Gtk::Expander*> expv;
#ifdef ML_GTK_OLD
  Gtk::TreeView *bm_tv = nullptr;
  std::vector<book_item> bookmark_v;
#endif
  std::mutex *searchmtx = nullptr;
#ifndef ML_GTK_OLD
  std::vector<Glib::RefPtr<Gtk::ColumnViewColumn>> search_res_col;
  guint *ms_sel_book = nullptr;
  Glib::RefPtr<ModelColumns> ms_sel_item;
  guint *bm_sel_book = nullptr;
  Gtk::ColumnView *bm_col_view = nullptr;
  Glib::RefPtr<Gio::ListStore<ModelColumns>> list_sr;
  std::vector<style_item> ms_style_v;
#endif
};

#endif /* INCLUDE_MAINWINDOW_H_ */
