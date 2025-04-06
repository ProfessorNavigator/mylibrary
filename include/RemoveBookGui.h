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

#ifndef REMOVEBOOKGUI_H
#define REMOVEBOOKGUI_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <NotesKeeper.h>
#include <functional>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>

class RemoveBookGui
{
public:
  RemoveBookGui(const std::shared_ptr<AuxFunc> &af, Gtk::Window *parent_window,
                const BookBaseEntry &bbe, const std::string &col_name,
                const std::shared_ptr<BookMarks> &bookmarks,
                const std::shared_ptr<NotesKeeper> &notes);

  void
  createWindow();

  std::function<void(const BookBaseEntry &bbe)> remove_callback;

private:
  void
  removeBookFunc(Gtk::Window *win);

  void
  removeFinished(Gtk::Window *win);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;
  BookBaseEntry bbe;
  std::string col_name;
  std::shared_ptr<BookMarks> bookmarks;
  std::shared_ptr<NotesKeeper> notes;

  std::shared_ptr<Glib::Dispatcher> remove_callback_disp;
  int remove_result = 0;

  bool notes_reserve = false;
  std::filesystem::path notes_reserve_path;
};

#endif // REMOVEBOOKGUI_H
