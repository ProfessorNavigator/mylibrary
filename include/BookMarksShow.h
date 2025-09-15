/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef BOOKMARKSSHOW_H
#define BOOKMARKSSHOW_H

#include <AuxFunc.h>
#include <BookMarksModelItem.h>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/filterlistmodel.h>
#include <gtkmm-4.0/gtkmm/listitem.h>
#include <gtkmm-4.0/gtkmm/stringfilter.h>

class BookMarksShow
{
public:
  BookMarksShow(const std::shared_ptr<AuxFunc> &af, Gtk::ColumnView *bm_view);

  void
  showBookMarks(
      const std::vector<std::tuple<std::string, BookBaseEntry>> &bm_v);

  void
  selectItem(const Glib::RefPtr<BookMarksModelItem> &item);

  Glib::RefPtr<BookMarksModelItem>
  getSelectedItem();

  void
  removeItem(const Glib::RefPtr<BookMarksModelItem> &item);

  void
  filterBookmarks(const Glib::ustring &filter, const guint &variant);

  void
  setWidth();

private:
  Glib::RefPtr<Gtk::ColumnViewColumn>
  collectionColumn();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  authorColumn();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  bookColumn();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  seriesColumn();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  genreColumn();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  dateColumn();

  void
  slotSetup(const Glib::RefPtr<Gtk::ListItem> &item);

  void
  slotBind(const Glib::RefPtr<Gtk::ListItem> &item, const int &variant);

  void
  slotUnbind(const Glib::RefPtr<Gtk::ListItem> &item);

  Glib::ustring
  translate_genres(const std::string &genres);

  void
  append_genre(Glib::ustring &result, std::string &genre);

  Glib::ustring
  expressionSlot(const Glib::RefPtr<Glib::ObjectBase> &item,
                 const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::ColumnView *bm_view;

  std::vector<GenreGroup> genre_list;

  Glib::RefPtr<Gio::ListStore<BookMarksModelItem>> model;

  Glib::RefPtr<BookMarksModelItem> selected_item;

  Glib::RefPtr<Gtk::StringFilter> coll_filter;
  Glib::RefPtr<Gtk::StringFilter> auth_filter;
  Glib::RefPtr<Gtk::StringFilter> book_filter;
  Glib::RefPtr<Gtk::StringFilter> series_filter;
  Glib::RefPtr<Gtk::StringFilter> genre_filter;
  Glib::RefPtr<Gtk::StringFilter> date_filter;

  Glib::RefPtr<Gtk::FilterListModel> filter_model;
};

#endif // BOOKMARKSSHOW_H
