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

#include <CollectionCrProcessGui.h>
#include <glibmm/signalproxy.h>
#include <gtkmm/application.h>
#include <gtkmm/enums.h>
#include <gtkmm/grid.h>
#include <gtkmm/object.h>
#include <libintl.h>
#include <MLException.h>
#include <RefreshCollection.h>
#include <sigc++/connection.h>
#include <iostream>
#include <locale>
#include <sstream>
#include <thread>

CollectionCrProcessGui::CollectionCrProcessGui(
    const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
    const std::filesystem::path &collection_path,
    const std::filesystem::path &books_path, const bool &rar_support,
    const std::string &num_thr)
{
  this->af = af;
  this->main_window = main_window;
  this->collection_path = collection_path;
  this->books_path = books_path;
  this->rar_support = rar_support;
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  strm.str(num_thr);
  strm >> thr_num;
  cancel_proc.store(false);
}

CollectionCrProcessGui::~CollectionCrProcessGui()
{
  delete pulse_disp;
  delete creation_finished_disp;
  delete new_collection_name_disp;
  delete total_files_disp;
  delete progress_disp;
  delete total_bytes_to_hash_disp;
  delete bytes_hashed_disp;
}

CollectionCrProcessGui::CollectionCrProcessGui(
    const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
    const std::string &coll_name, const std::string &num_thr,
    const bool &remove_empty, const bool &fast, const bool &refresh_bookmarks,
    const bool &rar_support, const std::shared_ptr<BookMarks> &bookmarks)
{
  this->af = af;
  this->main_window = main_window;
  this->coll_name = coll_name;
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  strm.str(num_thr);
  strm >> thr_num;
  cancel_proc.store(false);
  this->remove_empty = remove_empty;
  fast_refresh = fast;
  this->refresh_bookmarkse = refresh_bookmarks;
  this->rar_support = rar_support;
  this->bookmarks = bookmarks;
}

void
CollectionCrProcessGui::createWindow(const int &variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  switch(variant)
    {
    case 1:
      {
	window->set_title(gettext("Collection creation progress"));
	break;
      }
    case 2:
      {
	window->set_title(gettext("Collection refreshing progress"));
	break;
      }
    default:
      break;
    }
  window->set_transient_for(*main_window);
  if(variant == 2)
    {
      window->set_modal(true);
    }
  window->set_deletable(false);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  process_name = Gtk::make_managed<Gtk::Label>();
  process_name->set_margin(5);
  process_name->set_halign(Gtk::Align::CENTER);
  process_name->set_expand(true);
  process_name->set_text(gettext("Collecting files..."));
  grid->attach(*process_name, 0, 0, 1, 1);

  creation_progress = Gtk::make_managed<Gtk::ProgressBar>();
  creation_progress->set_margin(5);
  creation_progress->set_halign(Gtk::Align::CENTER);
  creation_progress->set_name("progressBars");
  grid->attach(*creation_progress, 0, 1, 1, 1);

  cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this]
  {
    this->cancel_proc.store(true);
    this->cancel->hide();
    this->creation_progress->hide();
    this->process_name->set_text(gettext("Interruption..."));
  });
  grid->attach(*cancel, 0, 2, 1, 1);

  window->signal_close_request().connect([window, this]
  {
    std::shared_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    delete this;
    return true;
  },
					 false);

  window->present();

  switch(variant)
    {
    case 1:
      {
	createProcessCreation(window);
	break;
      }
    case 2:
      {
	createProcessRefresh(window);
	break;
      }
    default:
      {
	delete window;
	std::shared_ptr<CollectionCrProcessGui> ccpg(this);
	break;
      }
    }
}

void
CollectionCrProcessGui::createProcessCreation(Gtk::Window *win)
{
  progress_count.store(0.0);
  if(thr_num <= 0)
    {
      thr_num = 1;
    }

  CreateCollection *cc = new CreateCollection(af, collection_path, books_path,
					      rar_support, thr_num,
					      &cancel_proc);

  pulse_disp = new Glib::Dispatcher;
  pulse_disp->connect([this]
  {
    this->creation_progress->pulse();
  });
  cc->pulse = [this]
  {
    this->pulse_disp->emit();
  };

  total_files_disp = new Glib::Dispatcher;
  total_files_disp->connect([this]
  {
    this->process_name->set_text(gettext("Collection creation progress:"));
    this->creation_progress->set_show_text(true);
    this->creation_progress->set_fraction(0.0);
  });
  cc->total_file_number = [this]
  (const double &tot)
    {
      this->total_files = tot;
      if(this->total_files == 0.0)
	{
	  this->total_files = 1.0;
	}
      this->total_files_disp->emit();
    };

  progress_disp = new Glib::Dispatcher;
  progress_disp->connect(
      [this]
      {
	this->creation_progress->set_fraction(
	    this->progress_count / this->total_files);
      });
  cc->progress = [this]
  (const double &prog)
    {
      if(prog > this->progress_count.load())
	{
	  this->progress_count.store(prog);
	}
      this->progress_disp->emit();
    };

  creation_finished_disp = new Glib::Dispatcher;
  creation_finished_disp->connect([win, this]
  {
    this->finishInfo(win, 1);
  });

  new_collection_name_disp = new Glib::Dispatcher;
  new_collection_name_disp->connect([this]
  {
    if(this->add_new_collection)
      {
	this->add_new_collection(this->collection_path.filename().u8string());
      }
  });

  std::thread *thr = new std::thread([cc, this]
  {
    try
      {
	cc->createCollection();
	this->new_collection_name_disp->emit();
      }
    catch(MLException &e)
      {
	std::cout << e.what() << std::endl;
      }
    this->creation_finished_disp->emit();
    delete cc;
  });
  thr->detach();
  delete thr;
}

void
CollectionCrProcessGui::createProcessRefresh(Gtk::Window *win)
{
  progress_count.store(0.0);
  if(thr_num <= 0)
    {
      thr_num = 1;
    }

  RefreshCollection *rfr = new RefreshCollection(af, coll_name, thr_num,
						 &cancel_proc, remove_empty,
						 fast_refresh,
						 refresh_bookmarkse, bookmarks);
  rfr->set_rar_support(rar_support);
  pulse_disp = new Glib::Dispatcher;
  pulse_disp->connect([this]
  {
    this->creation_progress->pulse();
  });
  rfr->pulse = [this]
  {
    this->pulse_disp->emit();
  };

  total_bytes_to_hash_disp = new Glib::Dispatcher;
  total_bytes_to_hash_disp->connect([this]
  {
    this->process_name->set_text(gettext("Collection hashing progress:"));
    this->creation_progress->set_show_text(true);
    this->creation_progress->set_fraction(0.0);
  });

  rfr->total_bytes_to_hash = [this]
  (const double &tot)
    {
      if(tot == 0.0)
	{
	  this->total_bytes_to_hash = 1.0;
	}
      else
	{
	  this->total_bytes_to_hash = tot;
	}
      this->total_bytes_to_hash_disp->emit();
    };

  bytes_hashed_disp = new Glib::Dispatcher;
  bytes_hashed_disp->connect(
      [this]
      {
	this->creation_progress->set_fraction(
	    this->bytes_hashed / this->total_bytes_to_hash);
      });

  rfr->bytes_hashed = [this]
  (const double &hashed)
    {
      this->bytes_hashed = hashed;
      this->bytes_hashed_disp->emit();
    };

  total_files_disp = new Glib::Dispatcher;
  total_files_disp->connect([this]
  {
    this->process_name->set_text(gettext("Collection refreshing progress:"));
    this->creation_progress->set_show_text(true);
    this->creation_progress->set_fraction(0.0);
  });

  rfr->total_file_number = [this]
  (const double &tot)
    {
      this->total_files = tot;
      if(this->total_files == 0.0)
	{
	  this->total_files = 1.0;
	}
      this->total_files_disp->emit();
    };

  progress_disp = new Glib::Dispatcher;
  progress_disp->connect(
      [this]
      {
	this->creation_progress->set_fraction(
	    this->progress_count / this->total_files);
      });
  rfr->progress = [this]
  (const double &prog)
    {
      if(prog > this->progress_count.load())
	{
	  this->progress_count.store(prog);
	}
      this->progress_disp->emit();
    };

  creation_finished_disp = new Glib::Dispatcher;
  creation_finished_disp->connect([win, this]
  {
    this->finishInfo(win, 2);
    if(this->collection_refreshed)
      {
	this->collection_refreshed(this->coll_name);
      }
  });

  std::thread *thr = new std::thread([rfr, this]
  {
    try
      {
	rfr->refreshCollection();
      }
    catch(MLException &e)
      {
	std::cout << e.what() << std::endl;
      }
    this->creation_finished_disp->emit();
  });
  thr->detach();
  delete thr;
}

void
CollectionCrProcessGui::finishInfo(Gtk::Window *win, const int &variant)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(true);
  switch(variant)
    {
    case 1:
      {
	win->set_title(gettext("Collection created"));
	break;
      }
    case 2:
      {
	win->set_title(gettext("Collection refreshed"));
	break;
      }
    default:
      break;
    }

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  win->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  switch(variant)
    {
    case 1:
      {
	lab->set_text(gettext("Collection creation process finished."));
	break;
      }
    case 2:
      {
	lab->set_text(gettext("Collection refresh process finished."));
	break;
      }
    default:
      break;
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_label(gettext("Close"));
  close->set_name("operationBut");
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
  grid->attach(*close, 0, 1, 1, 1);
}
