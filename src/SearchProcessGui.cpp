/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <glibmm/dispatcher.h>
#include <glibmm/signalproxy.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <libintl.h>
#include <sigc++/connection.h>
#include <SearchProcessGui.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <thread>

SearchProcessGui::SearchProcessGui(BaseKeeper *bk,
				   Gtk::Window *main_window)
{
  this->bk = bk;
  this->main_window = main_window;
}

SearchProcessGui::~SearchProcessGui()
{

}

void
SearchProcessGui::createWindow(const BookBaseEntry &search)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Search"));
  window->set_transient_for(*main_window);
  window->set_modal(true);
  window->set_deletable(false);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(true);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Search in progress..."));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this, lab, cancel]
  {
    this->bk->stopSearch();
    cancel->set_visible(false);
    lab->set_text(gettext("Search interrupting..."));
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  window->signal_close_request().connect([window, this]
  {
    std::shared_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    delete this;
    return true;
  },
					 false);

  window->present();

  startSearch(window, search);
}

void
SearchProcessGui::startSearch(Gtk::Window *win,
			      const BookBaseEntry &search)
{
  Glib::Dispatcher *search_finished = new Glib::Dispatcher;

  search_finished->connect([this, win, search_finished]
  {
    std::shared_ptr<Glib::Dispatcher> disp(search_finished);
    if(this->search_result_show)
      {
	this->search_result_show(this->search_result);
      }
    win->close();
  });

  std::thread *thr = new std::thread([this, search, search_finished]
  {
    this->search_result = this->bk->searchBook(search);
    search_finished->emit();
  });
  thr->detach();
  delete thr;
}
