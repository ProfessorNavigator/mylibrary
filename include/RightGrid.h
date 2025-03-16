/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef RIGHTGRID_H
#define RIGHTGRID_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookInfo.h>
#include <BookInfoEntry.h>
#include <BookMarks.h>
#include <FormatAnnotation.h>
#include <NotesKeeper.h>
#include <OpenBook.h>
#include <SearchResultModelItem.h>
#include <SearchResultShow.h>
#include <functional>
#include <giomm-2.68/giomm/menu.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/drawingarea.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/menubutton.h>
#include <gtkmm-4.0/gtkmm/popovermenu.h>
#include <gtkmm-4.0/gtkmm/textbuffer.h>
#include <gtkmm-4.0/gtkmm/textview.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>
#include <vector>

class RightGrid
{
public:
  RightGrid(const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
            const std::shared_ptr<BookMarks> &bookmarks,
            const std::shared_ptr<NotesKeeper> &notes);

  virtual ~RightGrid();

  Gtk::Grid *
  createGrid();

  Gtk::ColumnView *
  get_search_result();

  void
  clearSearchResult();

  void
  search_result_show(const std::vector<BookBaseEntry> &result);

  void
  search_result_show_files(const std::vector<FileParseEntry> &result);

  void
  searchResultShowAuthors(const std::vector<std::string> &result);

  std::function<std::string()> get_current_collection_name;

  std::function<void(const std::string &col_name)> reload_collection_base;

  std::function<void(const std::string &auth)> search_books_callback;

private:
  void
  slot_row_activated(guint pos);

  void
  set_annotation_n_cover(const Glib::RefPtr<SearchResultModelItem> &item);

  void
  annotation_parse_http(Glib::RefPtr<Gtk::TextBuffer> &tb);

  void
  cover_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

  void
  get_dpi();

  void
  book_operations_action_group();

  void
  book_menu(Glib::RefPtr<Gio::Menu> &result);

  void
  files_menu(Glib::RefPtr<Gio::Menu> &result);

  void
  auth_menu(Glib::RefPtr<Gio::Menu> &result);

  void
  show_popup_menu(int num, double x, double y, Gtk::PopoverMenu *pop_menu);

  Glib::RefPtr<Gio::Menu>
  cover_menu();

  void
  show_cover_popup_menu(int num, double x, double y,
                        Gtk::PopoverMenu *pop_menu);

  void
  cover_operations_action_group();

  void
  cover_full_size();

  void
  save_cover();

  void
  book_remove_action();

  void
  bookNotesAction();

  void
  bookmarks_save_result_dialog(const int &variant);

  void
  edit_book_success_slot(const BookBaseEntry &bbe_old,
                         const BookBaseEntry &bbe_new,
                         const std::string &col_name);

  void
  open_book_action();

  void
  transfer_book_action();

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;
  std::shared_ptr<BookMarks> bookmarks;
  std::shared_ptr<NotesKeeper> notes;

  Gtk::ColumnView *search_res = nullptr;
  SearchResultShow *srs = nullptr;
  BookInfo *bi = nullptr;
  Gtk::TextView *annotation = nullptr;
  FormatAnnotation *formatter = nullptr;
  Gtk::DrawingArea *cover_area = nullptr;
  OpenBook *open_book = nullptr;

  std::string current_collection;

  std::shared_ptr<BookInfoEntry> bie;

  Glib::RefPtr<Gio::Menu> menu_sr;
  Gtk::DropDown *filter_selection;

  Gtk::MenuButton *book_ops;
};

#endif // RIGHTGRID_H
