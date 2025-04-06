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
#ifndef NOTESGUI_H
#define NOTESGUI_H

#include <BookBaseEntry.h>
#include <NotesKeeper.h>
#include <gtkmm-4.0/gtkmm/textview.h>
#include <gtkmm-4.0/gtkmm/window.h>

class NotesGui
{
public:
  NotesGui(Gtk::Window *parent_window,
           const std::shared_ptr<NotesKeeper> notes);

  void
  creatWindow(const std::string &collection_name, const BookBaseEntry &bbe);

private:
  Gdk::Rectangle
  screen_size();

  void
  confirmationDialog();

  void
  closeDialog();

  Gtk::Window *parent_window;
  std::shared_ptr<NotesKeeper> notes;

  Gtk::Window *notes_window;
  NotesBaseEntry nbe;

  std::string note_buffer;

  Gtk::TextView *note_txt;
};

#endif // NOTESGUI_H
