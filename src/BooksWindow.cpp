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

#include <BooksWindow.h>
#include <libintl.h>

BooksWindow::BooksWindow(Gtk::Window *parent_window,
                         const std::shared_ptr<AuxFunc> &af,
                         const std::string &collection_name,
                         const std::shared_ptr<BookMarks> &bookmarks,
                         const std::shared_ptr<NotesKeeper> &notes)
{
  this->parent_window = parent_window;
  this->af = af;
  this->collection_name = collection_name;
  this->bookmarks = bookmarks;
  this->notes = notes;
  rg = new RightGrid(af, this, bookmarks, notes,
                     RightGrid::ShowVariant::SeparateWindow);
}

BooksWindow::~BooksWindow()
{
  delete rg;
}

void
BooksWindow::createWindow(const std::vector<BookBaseEntry> &result)
{
  this->set_application(parent_window->get_application());
  this->set_title(gettext("Books"));
  this->set_transient_for(*parent_window);
  this->set_name("MLwindow");

  rg->get_current_collection_name = [this] {
    return collection_name;
  };

  Gtk::Grid *grid = rg->createGrid();
  this->set_child(*grid);

  rg->searchResultShow(result, RightGrid::ShowVariant::MainWindow);
}
