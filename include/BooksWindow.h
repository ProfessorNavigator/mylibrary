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
#ifndef BOOKSWINDOW_H
#define BOOKSWINDOW_H

#include <AuxFunc.h>
#include <BookMarks.h>
#include <NotesKeeper.h>
#include <RightGrid.h>
#include <functional>
#include <gtkmm-4.0/gtkmm/window.h>

class BooksWindow : public Gtk::Window
{
public:
  BooksWindow(Gtk::Window *parent_window, const std::shared_ptr<AuxFunc> &af,
              const std::string &collection_name,
              const std::shared_ptr<BookMarks> &bookmarks,
              const std::shared_ptr<NotesKeeper> &notes);
  virtual ~BooksWindow();

  void
  createWindow(const std::vector<BookBaseEntry> &result);

private:
  Gtk::Window *parent_window;
  std::shared_ptr<AuxFunc> af;
  std::string collection_name;
  std::shared_ptr<BookMarks> bookmarks;
  std::shared_ptr<NotesKeeper> notes;

  RightGrid *rg = nullptr;
};

#endif // BOOKSWINDOW_H
