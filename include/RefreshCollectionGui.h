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

#ifndef REFRESHCOLLECTIONGUI_H
#define REFRESHCOLLECTIONGUI_H

#include <AuxFunc.h>
#include <BookMarks.h>
#include <NotesKeeper.h>
#include <functional>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>

class RefreshCollectionGui
{
public:
  RefreshCollectionGui(const std::shared_ptr<AuxFunc> &af,
                       Gtk::Window *main_window,
                       const std::shared_ptr<BookMarks> &bookmarks,
                       const std::shared_ptr<NotesKeeper> &notes);

  void
  createWindow();

  std::function<void(const std::string &collection_name)> collection_refreshed;

private:
  Glib::RefPtr<Gtk::StringList>
  createCollectionsList();

  void
  confirmationDialog(Gtk::Window *win);

  void
  refreshCollection(Gtk::Window *win, Gtk::Window *parent_window);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;
  std::shared_ptr<BookMarks> bookmarks;
  std::shared_ptr<NotesKeeper> notes;

  Gtk::DropDown *collection = nullptr;
  Gtk::Entry *num_threads = nullptr;
  Gtk::CheckButton *clean_empty = nullptr;
  Gtk::CheckButton *fast_refreshing = nullptr;
  Gtk::CheckButton *refresh_bookmarks = nullptr;
  Gtk::CheckButton *disable_rar = nullptr;

  bool reserve_notes = false;
  std::filesystem::path reserve_notes_directory;
};

#endif // REFRESHCOLLECTIONGUI_H
