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

#ifndef EDITBOOKGUI_H
#define EDITBOOKGUI_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <EditBookGenreModelItem.h>
#include <Genre.h>
#include <GenreGroup.h>
#include <functional>
#include <giomm-2.68/giomm/liststore.h>
#include <giomm-2.68/giomm/menu.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <gtkmm-4.0/gtkmm/columnviewcolumn.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/listitem.h>
#include <gtkmm-4.0/gtkmm/menubutton.h>
#include <gtkmm-4.0/gtkmm/popovermenu.h>
#include <gtkmm-4.0/gtkmm/singleselection.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class EditBookGui
{
public:
  EditBookGui(const std::shared_ptr<AuxFunc> &af, Gtk::Window *parent_window,
              const std::shared_ptr<BookMarks> &bookmarks,
              const std::string &collection_name, const BookBaseEntry &bbe);

  virtual ~EditBookGui();

  void
  createWindow();

  std::function<void(const BookBaseEntry &bbe_old,
                     const BookBaseEntry &bbe_new,
                     const std::string &collecton_name)>
      successfully_edited_signal;

private:
  void
  form_window_grid(Gtk::Window *window);

  Glib::RefPtr<Gio::ListStore<EditBookGenreModelItem>>
  create_genre_model();

  Glib::RefPtr<EditBookGenreModelItem>
  create_item(std::string &genre_code);

  Glib::RefPtr<Gtk::ColumnViewColumn>
  genre_col_name();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  genre_col_code();

  void
  slot_genre_setup(const Glib::RefPtr<Gtk::ListItem> &list_item);

  void
  slot_genre_bind(const Glib::RefPtr<Gtk::ListItem> &list_item,
                  const int &variant);

  void
  select_genre(guint pos);

  void
  setGenrePopover(Gtk::MenuButton *genre_button);

  Gtk::Grid *
  formGenreExpanderGrid(const std::string &group_name,
                        const std::vector<Genre> &genre);

  void
  slot_genre_select(int numclc, double x, double y, const Genre &g,
                    const std::string &group_name);

  void
  remove_genre();

  void
  create_genre_action_group(Gtk::Window *win);

  Glib::RefPtr<Gio::Menu>
  create_genre_menu();

  void
  show_genre_menu(int numclck, double x, double y, Gtk::PopoverMenu *menu,
                  const Glib::RefPtr<Gtk::SingleSelection> &selection);

  void
  confirmationDialog(Gtk::Window *win);

  void
  wait_window(Gtk::Window *win);

  void
  edit_book(Gtk::Window *win);

  void
  finish_dialog(Gtk::Window *win, const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;
  std::shared_ptr<BookMarks> bookmarks;
  std::string collection_name;
  BookBaseEntry bbe;

  std::vector<GenreGroup> genre_list;

  Glib::RefPtr<Gio::ListStore<EditBookGenreModelItem>> genre_model;

  Glib::RefPtr<EditBookGenreModelItem> selected_genre;

  Gtk::Popover *genre_popover = nullptr;

  Glib::Dispatcher *restore_disp = nullptr;

  std::shared_ptr<std::thread> refresh_thr;

  Gtk::Entry *book_ent = nullptr;
  Gtk::Entry *author_ent = nullptr;
  Gtk::Entry *series_ent = nullptr;
  Gtk::Entry *date_ent = nullptr;
};

#endif // EDITBOOKGUI_H
