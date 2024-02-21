/*
 * Copyright (C) 2022-2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_MYLIBRARYAPPLICATION_H_
#define INCLUDE_MYLIBRARYAPPLICATION_H_

#include <gtkmm.h>
#include "MainWindow.h"

class MyLibraryApplication : public Gtk::Application
{
public:
  virtual
  ~MyLibraryApplication();

  static Glib::RefPtr<MyLibraryApplication>
  create(const std::shared_ptr<AuxFunc> &af);

protected:
  MyLibraryApplication(const std::shared_ptr<AuxFunc> &af);

  void
  on_activate() override;

private:
  MainWindow*
  create_appwindow();

  MainWindow *mw = nullptr;

  std::shared_ptr<AuxFunc> af;
};

#endif /* INCLUDE_MYLIBRARYAPPLICATION_H_ */
