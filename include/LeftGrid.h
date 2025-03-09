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

#ifndef LEFTGRID_H
#define LEFTGRID_H

#include <AuxFunc.h>
#include <BaseKeeper.h>
#include <BookBaseEntry.h>
#include <Genre.h>
#include <NotesKeeper.h>
#include <functional>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/menubutton.h>
#include <gtkmm-4.0/gtkmm/popover.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>
#include <vector>

class LeftGrid
{
public:
  LeftGrid(const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
           const std::shared_ptr<NotesKeeper> &notes);

  virtual ~LeftGrid();

  Gtk::Grid *
  createGrid();

  Gtk::DropDown *
  get_collection_select();

  void
  clear_all_search_fields();

  void
  add_new_collection(const std::string &col_name);

  void
  clearCollectionBase();

  bool
  reloadCollection(const std::string &col_name);

  void
  searchAuth(const std::string &auth);

  void
  reloadCollectionList();

  std::function<void()> clear_search_result;

  std::function<void(const std::vector<BookBaseEntry> &result)>
      search_result_show;

  std::function<void(const std::vector<FileParseEntry> &result)>
      search_result_show_files;

  std::function<void(const std::vector<std::string> &result)>
      search_result_authors;

private:
  Glib::RefPtr<Gtk::StringList>
  createCollectionsList();

  void
  formAuthorSection(Gtk::Grid *grid, int &row);

  void
  formBookSection(Gtk::Grid *grid, int &row);

  void
  formSearchSection(Gtk::Grid *grid, int &row);

  void
  setGenrePopover(Gtk::MenuButton *genre_button);

  void
  setActiveCollection();

  void
  saveActiveCollection();

  void
  loadCollection(const guint &sel);

  void
  searchBook();

  void
  showCollectionFiles();

  void
  showCollectionAuthors();

  void
  showBooksWithNotes();

  Gtk::Grid *
  formGenreExpanderGrid(const std::vector<Genre> &genre, Gtk::Popover *pop);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;
  std::shared_ptr<NotesKeeper> notes;

  Gtk::DropDown *collection_select = nullptr;

  Gtk::Entry *surname = nullptr;
  Gtk::Entry *name = nullptr;
  Gtk::Entry *sec_name = nullptr;
  Gtk::Entry *book_name = nullptr;
  Gtk::Entry *series = nullptr;
  Gtk::MenuButton *genre_button = nullptr;
  BaseKeeper *base_keeper = nullptr;

  Genre selected_genre;
};

#endif // LEFTGRID_H
