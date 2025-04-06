/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SEARCHRESULTSHOW_H
#define SEARCHRESULTSHOW_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <GenreGroup.h>
#include <SearchResultModelItem.h>
#include <SearchResultModelItemAuth.h>
#include <SearchResultModelItemFL.h>
#include <giomm-2.68/giomm/liststore.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/filterlistmodel.h>
#include <gtkmm-4.0/gtkmm/listitem.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>
#include <gtkmm-4.0/gtkmm/stringfilter.h>
#include <memory>
#include <string>
#include <vector>

class SearchResultShow
{
public:
  SearchResultShow(const std::shared_ptr<AuxFunc> &af,
                   Gtk::ColumnView *search_res);

  virtual ~SearchResultShow();

  void
  clearSearchResult();

  void
  searchResultShow(const std::vector<BookBaseEntry> &result);

  void
  searchResultShow(const std::vector<FileParseEntry> &result);

  void
  searchResultShow(const std::vector<std::string> &result);

  void
  select_item(const Glib::RefPtr<SearchResultModelItem> &item);

  void
  select_item(const Glib::RefPtr<SearchResultModelItemFL> &item);

  void
  select_item(const Glib::RefPtr<SearchResultModelItemAuth> &item);

  Glib::RefPtr<SearchResultModelItem>
  get_selected_item();

  Glib::RefPtr<SearchResultModelItemFL>
  get_selected_item_file();

  Glib::RefPtr<SearchResultModelItemAuth>
  get_selected_item_auth();

  void
  removeItem(const Glib::RefPtr<SearchResultModelItem> &item);

  void
  removeBook(const Glib::RefPtr<SearchResultModelItem> &item);

  Glib::RefPtr<Gio::ListStore<SearchResultModelItem>>
  get_model();

  void
  filterFiles(const Glib::ustring &filter_val);

  void
  filterAuth(const Glib::ustring &filter_val);

  void
  filterBooks(const Glib::ustring &filter_val, const guint &variant);

private:
  Glib::RefPtr<Gtk::SignalListItemFactory>
  createFactory(const int &variant);

  void
  formAuthorColumn();

  void
  formBookColumn();

  void
  formSeriesColumn();

  void
  formGenreColumn();

  void
  formDateColumn();

  void
  formFilesColumn();

  void
  formAuthColumn();

  void
  itemSetup(const Glib::RefPtr<Gtk::ListItem> &list_item);

  void
  itemBind(const Glib::RefPtr<Gtk::ListItem> &list_item, const int &variant);

  Glib::ustring
  translate_genres(const std::string &genres);

  void
  append_genre(Glib::ustring &result, std::string &genre);

  Glib::ustring
  expression_slot(const Glib::RefPtr<Glib::ObjectBase> &list_item,
                  const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::ColumnView *search_res = nullptr;

  Glib::Dispatcher *disp_adjust;

  std::vector<GenreGroup> genre_list;

  Glib::RefPtr<SearchResultModelItem> selected_item;
  Glib::RefPtr<SearchResultModelItemFL> selected_item_file;
  Glib::RefPtr<SearchResultModelItemAuth> selected_item_auth;

  Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> model;
  Glib::RefPtr<Gio::ListStore<SearchResultModelItemFL>> model_files;
  Glib::RefPtr<Gio::ListStore<SearchResultModelItemAuth>> model_auth;

  Glib::RefPtr<Gtk::StringFilter> str_filter;

  Glib::RefPtr<Gtk::FilterListModel> filter_model;

  Glib::RefPtr<Gtk::StringFilter> author_filter;
  Glib::RefPtr<Gtk::StringFilter> book_filter;
  Glib::RefPtr<Gtk::StringFilter> series_filter;
  Glib::RefPtr<Gtk::StringFilter> genre_filter;
  Glib::RefPtr<Gtk::StringFilter> date_filter;
};

#endif // SEARCHRESULTSHOW_H
