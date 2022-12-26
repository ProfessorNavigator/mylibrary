/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

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

#include "MyLibraryApplication.h"

MyLibraryApplication::MyLibraryApplication() :
    Gtk::Application("ru.mail.bobilev_yury.MyLibrary")
{
  // TODO Auto-generated constructor stub
}

MyLibraryApplication::~MyLibraryApplication()
{
  // TODO Auto-generated destructor stub
}

Glib::RefPtr<MyLibraryApplication>
MyLibraryApplication::create()
{
  return Glib::make_refptr_for_instance<MyLibraryApplication>(
      new MyLibraryApplication());
}

MainWindow*
MyLibraryApplication::create_appwindow()
{
  MainWindow *mw = new MainWindow;
  this->add_window(*mw);
  mw->signal_hide().connect([mw, this]
  {
    std::vector<Gtk::Window*> wv;
    wv = this->get_windows();
    for(size_t i = 0; i < wv.size(); i++)
      {
	Gtk::Window *win = wv[i];
	if(win != mw)
	  {
	    win->hide();
	    delete win;
	  }
      }
    Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
    while(mc->pending())
      {
	mc->iteration(true);
      }
    delete mw;
  });

  return mw;
}

void
MyLibraryApplication::on_activate()
{
  std::vector<Gtk::Window*> winv;
  winv = this->get_windows();
  if(winv.size() == 0)
    {
      auto appwin = create_appwindow();
      appwin->show();
    }
}
