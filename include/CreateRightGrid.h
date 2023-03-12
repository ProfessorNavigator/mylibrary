/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of MyLibrary.
 MyLibrary is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 MyLibrary is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with MyLibrary. If not,
 see <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_CREATERIGHTGRID_H_
#define INCLUDE_CREATERIGHTGRID_H_

#include "MainWindow.h"

class MainWindow;

class CreateRightGrid
{
public:
  CreateRightGrid(MainWindow *mw);
  virtual
  ~CreateRightGrid();
  Gtk::Grid*
  formRightGrid();
  void
  searchResultShow(int variant);
private:
  Gtk::Grid*
  formPopoverGrid(Gtk::TreeView *sres, Gtk::Popover *book_popover);
  MainWindow *mw;
};

#endif /* INCLUDE_CREATERIGHTGRID_H_ */
