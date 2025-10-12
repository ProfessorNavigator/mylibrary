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

#ifndef RIGHTGRID_H
#define RIGHTGRID_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookInfo.h>
#include <BookInfoEntry.h>
#include <BookMarks.h>
#include <CoverPixBuf.h>
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
  enum ShowVariant
  {
    MainWindow,
    SeparateWindow
  };

  RightGrid(const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
            const std::shared_ptr<BookMarks> &bookmarks,
            const std::shared_ptr<NotesKeeper> &notes,
            const ShowVariant &sv = ShowVariant::MainWindow);

  virtual ~RightGrid();

  Gtk::Grid *
  createGrid();

  Gtk::ColumnView *
  getSearchResult();

  void
  clearSearchResult();

  void
  searchResultShow(const std::vector<BookBaseEntry> &result,
                   const ShowVariant &sv);

  void
  searchResultShowFiles(const std::vector<FileParseEntry> &result);

  void
  searchResultShowAuthors(const std::vector<std::string> &result);

  std::function<std::string()> get_current_collection_name;

  std::function<void(const std::string &col_name)> reload_collection_base;

  std::function<void(const std::string &auth)> search_books_callback;

  std::function<void(const std::string &auth)>
      search_books_callback_separate_window;

private:
  void
  slotRowActivated(guint pos);

  void
  setAnnotationNCover(const Glib::RefPtr<SearchResultModelItem> &item);

  void
  annotationParseHttp(Glib::RefPtr<Gtk::TextBuffer> &tb);

  void
  coverDraw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

  void
  getDpi();

  void
  bookOperationsActionGroup();

  void
  bookMenu(Glib::RefPtr<Gio::Menu> &result);

  void
  filesMenu(Glib::RefPtr<Gio::Menu> &result);

  void
  authMenu(Glib::RefPtr<Gio::Menu> &result);

  void
  showPopupMenu(int num, double x, double y, Gtk::PopoverMenu *pop_menu);

  Glib::RefPtr<Gio::Menu>
  coverMenu();

  void
  showCoverPopupMenu(int num, double x, double y, Gtk::PopoverMenu *pop_menu);

  void
  coverOperationsActionGroup();

  void
  coverFullSize();

  void
  saveCover();

  void
  bookRemoveAction();

  void
  bookNotesAction();

  void
  bookmarksSaveResultDialog(const int &variant);

  void
  editBookSuccessSlot(const BookBaseEntry &bbe_old,
                      const BookBaseEntry &bbe_new,
                      const std::string &col_name);

  void
  openBookAction();

  void
  transferBookAction();

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;
  std::shared_ptr<BookMarks> bookmarks;
  std::shared_ptr<NotesKeeper> notes;
  ShowVariant show_variant;

  Gtk::ColumnView *search_res = nullptr;
  SearchResultShow *srs = nullptr;
  BookInfo *bi = nullptr;
  Gtk::TextView *annotation = nullptr;
  FormatAnnotation *formatter = nullptr;
  Gtk::DrawingArea *cover_area = nullptr;
  OpenBook *open_book = nullptr;

  std::string current_collection;

  std::shared_ptr<BookInfoEntry> bie;
  CoverPixBuf cover_buf;

  Glib::RefPtr<Gio::Menu> menu_sr;
  Gtk::DropDown *filter_selection;

  Gtk::MenuButton *book_ops;

  Gtk::Grid *created_grid = nullptr;
};

#endif // RIGHTGRID_H
