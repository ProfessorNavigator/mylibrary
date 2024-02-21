/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_SEARCHRESULTSHOW_H_
#define INCLUDE_SEARCHRESULTSHOW_H_

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <giomm-2.68/giomm/liststore.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <glibmm-2.68/glibmm/ustring.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/listitem.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>
#include <GenreGroup.h>
#include <SearchResultModelItem.h>
#include <memory>
#include <string>
#include <vector>

class SearchResultShow
{
public:
  SearchResultShow(const std::shared_ptr<AuxFunc> &af,
		   Gtk::ColumnView *search_res);
  virtual
  ~SearchResultShow();

  void
  clearSearchResult();

  void
  searchResultShow(const std::vector<BookBaseEntry> &result);

  void
  select_item(const Glib::RefPtr<SearchResultModelItem> &item);

  Glib::RefPtr<SearchResultModelItem>
  get_selected_item();

  void
  removeItem(const Glib::RefPtr<SearchResultModelItem> &item);

  void
  removeBook(const Glib::RefPtr<SearchResultModelItem> &item);

  Glib::RefPtr<Gio::ListStore<SearchResultModelItem>>
  get_model();

private:
  Glib::RefPtr<Gtk::SignalListItemFactory>
  createFactory(const int &variant);

  void
  formAuthorColumn(
      const Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> &model);

  void
  formBookColumn(
      const Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> &model);

  void
  formSeriesColumn(
      const Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> &model);

  void
  formGenreColumn(
      const Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> &model);

  void
  formDateColumn(
      const Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> &model);

  void
  itemSetup(const Glib::RefPtr<Gtk::ListItem> &list_item);

  void
  itemBind(const Glib::RefPtr<Gtk::ListItem> &list_item,
	   const int &variant);

  Glib::ustring
  translate_genres(const std::string &genres);

  void
  append_genre(Glib::ustring &result, std::string &genre);

  Glib::ustring
  expression_slot(const Glib::RefPtr<Glib::ObjectBase> &list_item,
		  const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::ColumnView *search_res = nullptr;

  std::vector<GenreGroup> genre_list;

  Glib::RefPtr<SearchResultModelItem> selected_item;

  Glib::RefPtr<Gio::ListStore<SearchResultModelItem>> model;
};

#endif /* INCLUDE_SEARCHRESULTSHOW_H_ */
