/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <SearchProcessGui.h>
#include <glibmm/dispatcher.h>
#include <glibmm/signalproxy.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <libintl.h>
#include <memory>
#include <sigc++/connection.h>
#include <thread>

SearchProcessGui::SearchProcessGui(BaseKeeper *bk, Gtk::Window *main_window)
{
  this->bk = bk;
  this->main_window = main_window;
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
  cancel->signal_clicked().connect([this, lab, cancel] {
    this->bk->stopSearch();
    cancel->set_visible(false);
    lab->set_text(gettext("Search interrupting..."));
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<SearchProcessGui> gui(this);
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();

  startSearch(window, search);
}

void
SearchProcessGui::createWindow(const std::string &collection_name,
                               std::shared_ptr<AuxFunc> af)
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
  lab->set_text(gettext("Reading base..."));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this, lab, cancel] {
    this->bk->stopSearch();
    cancel->set_visible(false);
    lab->set_text(gettext("Reading interrupting..."));
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<SearchProcessGui> gui(this);
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();

  copyFiles(window, collection_name, af);
}

void
SearchProcessGui::startSearch(Gtk::Window *win, const BookBaseEntry &search)
{
  Glib::Dispatcher *search_finished = new Glib::Dispatcher;

  search_finished->connect([this, win, search_finished] {
    std::unique_ptr<Glib::Dispatcher> disp(search_finished);
    if(search_result_show)
      {
        search_result_show(this->search_result);
      }
    win->close();
  });

  std::thread thr([this, search, search_finished] {
    search_result = bk->searchBook(search);
    search_finished->emit();
  });
  thr.detach();
}

void
SearchProcessGui::copyFiles(Gtk::Window *win,
                            const std::string &collection_name,
                            std::shared_ptr<AuxFunc> af)
{
  Glib::Dispatcher *copy_proc_finished = new Glib::Dispatcher;
  copy_proc_finished->connect([this, copy_proc_finished, win] {
    std::unique_ptr<Glib::Dispatcher> disp(copy_proc_finished);
    if(search_result_file)
      {
        search_result_file(files);
      }
    win->close();
  });

  std::thread thr([this, collection_name, af, copy_proc_finished] {
    files = bk->get_base_vector();
    std::filesystem::path book_p = bk->get_books_path(collection_name, af);
    for(auto it = files.begin(); it != files.end(); it++)
      {
        std::filesystem::path p = book_p;
        p /= std::filesystem::u8path(it->file_rel_path);
        it->file_rel_path = p.u8string();
      }
    copy_proc_finished->emit();
  });
  thr.detach();
}
