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

#include <SearchProcessGui.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <libintl.h>
#include <memory>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#endif

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
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this, lab, cancel] {
    bk->stopSearch();
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
                               std::shared_ptr<AuxFunc> af, const int &variant)
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

  int row = 0;

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Reading base..."));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  Gtk::ProgressBar *prog = nullptr;
  if(variant == 2)
    {
      prog = Gtk::make_managed<Gtk::ProgressBar>();
      prog->set_margin(5);
      prog->set_show_text(true);
      prog->set_name("progressBars");
      prog->set_visible(false);
      grid->attach(*prog, 0, row, 1, 1);
      row++;
    }

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this, lab, cancel, prog] {
    bk->stopSearch();
    cancel->set_visible(false);
    if(prog)
      {
        prog->set_visible(false);
      }
    lab->set_text(gettext("Reading interrupting..."));
  });
  grid->attach(*cancel, 0, row, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<SearchProcessGui> gui(this);
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  switch(variant)
    {
    case 1:
      {
        copyFiles(window, collection_name, af);
        break;
      }
    case 2:
      {
        showAuthors(window, prog, lab, collection_name);
        break;
      }
    }
  window->present();
}

void
SearchProcessGui::createWindow(const std::vector<NotesBaseEntry> &notes)
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
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this, lab, cancel] {
    bk->stopSearch();
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

  showBooksWithNotes(window, notes);
}

void
SearchProcessGui::startSearch(Gtk::Window *win, const BookBaseEntry &search)
{
  Glib::Dispatcher *search_finished = new Glib::Dispatcher;

  search_finished->connect([this, win, search_finished] {
    std::unique_ptr<Glib::Dispatcher> disp(search_finished);
    if(search_result_show)
      {
        search_result_show(search_result);
      }
    win->close();
  });

#ifndef USE_OPENMP
  std::thread thr([this, search, search_finished] {
    search_result = bk->searchBook(search);
    search_finished->emit();
  });
  thr.detach();
#else
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      search_result = bk->searchBook(search);
      search_finished->emit();
      omp_fulfill_event(event);
    }
  }
#endif
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

#ifndef USE_OPENMP
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
#else
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      files = bk->get_base_vector();
      std::filesystem::path book_p = bk->get_books_path(collection_name, af);
      for(auto it = files.begin(); it != files.end(); it++)
        {
          std::filesystem::path p = book_p;
          p /= std::filesystem::u8path(it->file_rel_path);
          it->file_rel_path = p.u8string();
        }
      copy_proc_finished->emit();
      omp_fulfill_event(event);
    }
  }
#endif
}

void
SearchProcessGui::showAuthors(Gtk::Window *win, Gtk::ProgressBar *prog,
                              Gtk::Label *lab,
                              const std::string &collection_name)
{
  double *pr = new double(0.0);
  double *sz = new double(1.0);
#ifdef USE_OPENMP
  omp_lock_t *prog_mtx = new omp_lock_t;
  omp_init_lock(prog_mtx);
#else
  std::mutex *prog_mtx = new std::mutex;
#endif

  Glib::Dispatcher *progr_disp = new Glib::Dispatcher;
  progr_disp->connect([pr, sz, prog_mtx, prog] {
#ifdef USE_OPENMP
    omp_set_lock(prog_mtx);
    double frac = (*pr) / (*sz);
    omp_unset_lock(prog_mtx);
    if(!prog->get_visible())
      {
        prog->set_visible(true);
      }
    prog->set_fraction(frac);
#else
    prog_mtx->lock();
    double frac = (*pr) / (*sz);
    prog_mtx->unlock();
    if(!prog->get_visible())
      {
        prog->set_visible(true);
      }
    prog->set_fraction(frac);
#endif
  });

  Glib::Dispatcher *show_res = new Glib::Dispatcher;
  show_res->connect([show_res, win, this] {
    std::unique_ptr<Glib::Dispatcher> disp(show_res);
    if(search_result_authors)
      {
        search_result_authors(authors);
      }
    win->close();
  });

  Glib::Dispatcher *search_finished = new Glib::Dispatcher;
  search_finished->connect(
      [search_finished, progr_disp, pr, sz, prog_mtx, prog, lab, show_res] {
        std::unique_ptr<Glib::Dispatcher> disp(search_finished);
        prog->set_visible(false);
        lab->set_text(gettext("Sorting..."));
        delete progr_disp;
        delete pr;
        delete sz;
#ifdef USE_OPENMP
        omp_destroy_lock(prog_mtx);
#pragma omp masked
        {
          omp_event_handle_t event;
#pragma omp task detach(event)
          {
            show_res->emit();
            omp_fulfill_event(event);
          }
        }
#else
        std::thread thr([show_res] {
          show_res->emit();
        });
        thr.detach();
#endif
        delete prog_mtx;
      });

#ifndef USE_OPENMP
  bk->auth_show_progr = [pr, sz, prog_mtx, progr_disp](const double &progr,
                                                       const double &size) {
    prog_mtx->lock();
    *pr = progr;
    *sz = size;
    prog_mtx->unlock();
    progr_disp->emit();
  };
  std::thread thr([this, search_finished] {
    authors = bk->collectionAuthors();
    bk->auth_show_progr = nullptr;
    search_finished->emit();
  });
  thr.detach();
#else
  bk->auth_show_progr = [pr, sz, prog_mtx, progr_disp](const double &progr,
                                                       const double &size) {
    omp_set_lock(prog_mtx);
    *pr = progr;
    *sz = size;
    omp_unset_lock(prog_mtx);
    progr_disp->emit();
  };
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      authors = bk->collectionAuthors();
      search_finished->emit();
      bk->auth_show_progr = nullptr;
      omp_fulfill_event(event);
    }
  }
#endif
}

void
SearchProcessGui::showBooksWithNotes(Gtk::Window *win,
                                     const std::vector<NotesBaseEntry> &notes)
{
  Glib::Dispatcher *search_finished = new Glib::Dispatcher;

  search_finished->connect([this, win, search_finished] {
    std::unique_ptr<Glib::Dispatcher> disp(search_finished);
    if(search_result_show)
      {
        search_result_show(search_result);
      }
    win->close();
  });

#ifndef USE_OPENMP
  std::thread thr([this, notes, search_finished] {
    search_result = bk->booksWithNotes(notes);
    search_finished->emit();
  });
  thr.detach();
#else
#pragma omp parallel
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      search_result = bk->booksWithNotes(notes);
      search_finished->emit();
      omp_fulfill_event(event);
    }
  }
#endif
}
