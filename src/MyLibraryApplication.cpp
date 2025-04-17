/*
 * Copyright (C) 2022-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <MyLibraryApplication.h>

#ifndef USE_OPENMP
MyLibraryApplication::MyLibraryApplication(const std::shared_ptr<AuxFunc> &af) :
    Gtk::Application("ru.mail.bobilev_yury.MyLibrary")
{
  this->af = af;
}
#else
MyLibraryApplication::MyLibraryApplication(const std::shared_ptr<AuxFunc> &af)
    : Gtk::Application("ru.mail.bobilev_yury.MyLibrary.omp")
{
  this->af = af;
}
#endif

MyLibraryApplication::~MyLibraryApplication()
{
  delete mw;
}

Glib::RefPtr<MyLibraryApplication>
MyLibraryApplication::create(const std::shared_ptr<AuxFunc> &af)
{
  return Glib::make_refptr_for_instance<MyLibraryApplication>(
      new MyLibraryApplication(af));
}

MainWindow*
MyLibraryApplication::create_appwindow()
{
  mw = new MainWindow(af);
  add_window(*mw);
  mw->signal_hide().connect([this]
  {
    std::vector<Gtk::Window*> wv;
    wv = get_windows();
    for(size_t i = 0; i < wv.size(); i++)
      {
	Gtk::Window *win = wv[i];
        if(win != mw)
          {
	    win->set_visible(false);
	    delete win;
	  }
      }
  });

  return mw;
}

void
MyLibraryApplication::on_activate()
{
  std::vector<Gtk::Window*> winv;
  winv = get_windows();
  if(winv.size() == 0)
    {
      auto appwin = create_appwindow();
      appwin->present();
    }
}
