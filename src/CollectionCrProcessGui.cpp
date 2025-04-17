/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <CollectionCrProcessGui.h>
#include <MLException.h>
#include <RefreshCollection.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <iostream>
#include <libintl.h>
#include <locale>
#include <sstream>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#endif

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
  process_name->set_name("windowLabel");
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
  cancel->signal_clicked().connect([this] {
    if(stop_ops)
      {
        stop_ops();
      }
    cancel->hide();
    creation_progress->hide();
    process_name->set_text(gettext("Interruption..."));
  });
  grid->attach(*cancel, 0, 2, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<Gtk::Window> win(window);
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
        std::unique_ptr<CollectionCrProcessGui> ccpg(this);
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
                                              rar_support, thr_num);

  stop_ops = [cc] {
    cc->cancelAll();
  };

  pulse_disp = new Glib::Dispatcher;
  pulse_disp->connect([this] {
    creation_progress->pulse();
  });
  cc->pulse = [this] {
    pulse_disp->emit();
  };

  total_files_disp = new Glib::Dispatcher;
  total_files_disp->connect([this] {
    process_name->set_text(gettext("Collection creation progress:"));
    creation_progress->set_show_text(true);
    creation_progress->set_fraction(0.0);
  });
  cc->signal_total_bytes = [this](const double &tot) {
    total_bytes = tot;
    if(total_bytes == 0.0)
      {
        total_bytes = 1.0;
      }
    total_files_disp->emit();
  };

  progress_disp = new Glib::Dispatcher;
  progress_disp->connect([this] {
    creation_progress->set_fraction(progress_count / total_bytes);
  });
  cc->progress = [this](const double &prog) {
    if(prog > progress_count.load())
      {
        progress_count.store(prog);
      }
    progress_disp->emit();
  };

  creation_finished_disp = new Glib::Dispatcher;
  creation_finished_disp->connect([win, this] {
    finishInfo(win, 1);
  });

  new_collection_name_disp = new Glib::Dispatcher;
  new_collection_name_disp->connect([this] {
    if(add_new_collection)
      {
        add_new_collection(collection_path.filename().u8string());
      }
  });

#ifdef USE_OPENMP
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      try
        {
          cc->createCollection();
          new_collection_name_disp->emit();
        }
      catch(MLException &e)
        {
          std::cout << e.what() << std::endl;
        }
      creation_finished_disp->emit();
      delete cc;
      omp_fulfill_event(event);
    }
  }
#else
  std::thread thr([cc, this] {
    try
      {
        cc->createCollection();
        new_collection_name_disp->emit();
      }
    catch(MLException &e)
      {
        std::cout << e.what() << std::endl;
      }
    creation_finished_disp->emit();
    delete cc;
  });
  thr.detach();
#endif
}

void
CollectionCrProcessGui::createProcessRefresh(Gtk::Window *win)
{
  progress_count.store(0.0);
  if(thr_num <= 0)
    {
      thr_num = 1;
    }

  RefreshCollection *rfr
      = new RefreshCollection(af, coll_name, thr_num, remove_empty,
                              fast_refresh, refresh_bookmarkse, bookmarks);
  rfr->set_rar_support(rar_support);

  stop_ops = [rfr] {
    rfr->cancelAll();
  };
  pulse_disp = new Glib::Dispatcher;
  pulse_disp->connect([this] {
    creation_progress->pulse();
  });
  rfr->pulse = [this] {
    pulse_disp->emit();
  };

  total_bytes_to_hash_disp = new Glib::Dispatcher;
  total_bytes_to_hash_disp->connect([this] {
    process_name->set_text(gettext("Collection hashing progress:"));
    creation_progress->set_show_text(true);
    creation_progress->set_fraction(0.0);
  });

  rfr->total_bytes_to_hash = [this](const double &tot) {
    if(tot == 0.0)
      {
        total_bytes_to_hash = 1.0;
      }
    else
      {
        total_bytes_to_hash = tot;
      }
    total_bytes_to_hash_disp->emit();
  };

  bytes_hashed_disp = new Glib::Dispatcher;
  bytes_hashed_disp->connect([this] {
    creation_progress->set_fraction(bytes_hashed / total_bytes_to_hash);
  });

  rfr->bytes_hashed = [this](const double &hashed) {
    bytes_hashed = hashed;
    bytes_hashed_disp->emit();
  };

  total_files_disp = new Glib::Dispatcher;
  total_files_disp->connect([this] {
    process_name->set_text(gettext("Collection refreshing progress:"));
    creation_progress->set_show_text(true);
    creation_progress->set_fraction(0.0);
  });

  rfr->signal_total_bytes = [this](const double &tot) {
    total_bytes = tot;
    if(total_bytes == 0.0)
      {
        total_bytes = 1.0;
      }
    total_files_disp->emit();
  };

  progress_disp = new Glib::Dispatcher;
  progress_disp->connect([this] {
    creation_progress->set_fraction(progress_count / total_bytes);
  });
  rfr->progress = [this](const double &prog) {
    if(prog > progress_count.load())
      {
        progress_count.store(prog);
      }
    progress_disp->emit();
  };

  creation_finished_disp = new Glib::Dispatcher;
  creation_finished_disp->connect([win, this] {
    finishInfo(win, 2);
    if(collection_refreshed)
      {
        collection_refreshed(coll_name);
      }
  });
#ifdef USE_OPENMP
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      try
        {
          rfr->refreshCollection();
        }
      catch(MLException &e)
        {
          std::cout << e.what() << std::endl;
        }
      creation_finished_disp->emit();
      omp_fulfill_event(event);
    }
  }
#else
  std::thread thr([rfr, this] {
    try
      {
        rfr->refreshCollection();
      }
    catch(MLException &e)
      {
        std::cout << e.what() << std::endl;
      }
    creation_finished_disp->emit();
  });
  thr.detach();
#endif
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
  lab->set_name("windowLabel");
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
