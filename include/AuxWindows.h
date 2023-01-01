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

#ifndef INCLUDE_AUXWINDOWS_H_
#define INCLUDE_AUXWINDOWS_H_

#include "MainWindow.h"
class MainWindow;

class AuxWindows
{
public:
  AuxWindows(MainWindow *mw);
  virtual
  ~AuxWindows();
  void
  errorWin(int type, Gtk::Window *par_win, Glib::Dispatcher *disp);
  void
  bookmarkWindow();
  void
  bookCopyConfirm(Gtk::Window *win, std::mutex *addbmtx, int *stopper);
  void
  aboutProg();
private:
  MainWindow *mw;
};

#endif /* INCLUDE_AUXWINDOWS_H_ */
