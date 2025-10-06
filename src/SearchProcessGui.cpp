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
#include <thread>

SearchProcessGui::SearchProcessGui(BaseKeeper *bk, Gtk::Window *main_window)
{
  this->bk = bk;
  this->main_window = main_window;
}

void
SearchProcessGui::createWindow(const BookBaseEntry &search,
                               const double &coef_coincedence)
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

  startSearch(window, search, coef_coincedence);
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

  Gtk::Label *operation_name_lab = Gtk::make_managed<Gtk::Label>();
  operation_name_lab->set_margin(5);
  operation_name_lab->set_halign(Gtk::Align::CENTER);
  operation_name_lab->set_expand(true);
  operation_name_lab->set_text(gettext("Reading base..."));
  operation_name_lab->set_name("windowLabel");
  grid->attach(*operation_name_lab, 0, row, 1, 1);
  row++;

  AuthShowStruct s_struct;
  s_struct.operation_name_lab = operation_name_lab;

  if(variant == 2)
    {
#ifdef USE_GPUOFFLOADING
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_expand(true);
      lab->set_use_markup(true);
      lab->set_markup(Glib::ustring("<i>") + gettext("CPU progress") + "</i>");
      lab->set_name("windowLabel");
      lab->set_visible(false);
      grid->attach(*lab, 0, row, 1, 1);
      row++;
      s_struct.cpu_label = lab;
#endif
      Gtk::Label *progr_val = Gtk::make_managed<Gtk::Label>();
      progr_val->set_margin(5);
      progr_val->set_halign(Gtk::Align::CENTER);
      progr_val->set_name("windowLabel");
      progr_val->set_visible(false);
      grid->attach(*progr_val, 0, row, 1, 1);
      row++;
      s_struct.cpu_progr_val = progr_val;

      Gtk::ProgressBar *prog = Gtk::make_managed<Gtk::ProgressBar>();
      prog->set_margin(5);
      prog->set_show_text(false);
      prog->set_name("progressBars");
      prog->set_visible(false);
      grid->attach(*prog, 0, row, 1, 1);
      row++;
      s_struct.cpu_progr_bar = prog;
#ifdef USE_GPUOFFLOADING
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_expand(true);
      lab->set_use_markup(true);
      lab->set_markup(Glib::ustring("<i>") + gettext("GPU progress") + "</i>");
      lab->set_name("windowLabel");
      lab->set_visible(false);
      grid->attach(*lab, 0, row, 1, 1);
      row++;
      s_struct.gpu_label = lab;

      progr_val = Gtk::make_managed<Gtk::Label>();
      progr_val->set_margin(5);
      progr_val->set_halign(Gtk::Align::CENTER);
      progr_val->set_name("windowLabel");
      progr_val->set_visible(false);
      grid->attach(*progr_val, 0, row, 1, 1);
      row++;
      s_struct.gpu_progr_val = progr_val;

      prog = Gtk::make_managed<Gtk::ProgressBar>();
      prog->set_margin(5);
      prog->set_show_text(false);
      prog->set_name("progressBars");
      prog->set_visible(false);
      grid->attach(*prog, 0, row, 1, 1);
      row++;
      s_struct.gpu_progr_bar = prog;
#endif
    }

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect([this, cancel, s_struct] {
    bk->stopSearch();
    cancel->set_visible(false);
    if(s_struct.cpu_label)
      {
        s_struct.cpu_label->set_visible(false);
      }
    if(s_struct.cpu_progr_bar)
      {
        s_struct.cpu_progr_bar->set_visible(false);
      }
    if(s_struct.cpu_progr_val)
      {
        s_struct.cpu_progr_val->set_visible(false);
      }
    if(s_struct.gpu_label)
      {
        s_struct.gpu_label->set_visible(false);
      }
    if(s_struct.gpu_progr_bar)
      {
        s_struct.gpu_progr_bar->set_visible(false);
      }
    if(s_struct.gpu_progr_val)
      {
        s_struct.gpu_progr_val->set_visible(false);
      }
    s_struct.operation_name_lab->set_text(gettext("Reading interrupting..."));
    Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
    while(mc->pending())
      {
        mc->iteration(true);
      }
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
        showAuthors(window, s_struct, collection_name);
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
SearchProcessGui::startSearch(Gtk::Window *win, const BookBaseEntry &search,
                              const double &coef_coincedence)
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

  std::thread thr([this, search, search_finished, coef_coincedence] {
    search_result = bk->searchBook(search, coef_coincedence);
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
#ifdef USE_OPENMP
#pragma omp parallel
#pragma omp for
    for(auto it = files.begin(); it != files.end(); it++)
      {
        std::filesystem::path p = book_p;
        p /= std::filesystem::u8path(it->file_rel_path);
        it->file_rel_path = p.u8string();
      }
#else
    for(auto it = files.begin(); it != files.end(); it++)
      {
        std::filesystem::path p = book_p;
        p /= std::filesystem::u8path(it->file_rel_path);
        it->file_rel_path = p.u8string();
      }
#endif
    copy_proc_finished->emit();
  });
  thr.detach();
}

void
SearchProcessGui::showAuthors(Gtk::Window *win, AuthShowStruct &s_struct,
                              const std::string &collection_name)
{
  double *progr_cpu = new double(0.0);
  double *progr_cpu_sz = new double(1.0);
  std::mutex *prog_mtx = new std::mutex;
#ifndef USE_GPUOFFLOADING
  Glib::Dispatcher *progr_disp = new Glib::Dispatcher;
  std::shared_ptr<std::stringstream> strm(new std::stringstream);
  progr_disp->connect([progr_cpu, progr_cpu_sz, prog_mtx, s_struct, strm] {
    prog_mtx->lock();
    double frac = (*progr_cpu) / (*progr_cpu_sz);
    prog_mtx->unlock();
    strm->clear();
    strm->str("");
    *strm << std::fixed << std::setprecision(2) << frac * 100.0;
    s_struct.cpu_progr_val->set_visible(true);
    s_struct.cpu_progr_bar->set_visible(true);
    s_struct.cpu_progr_val->set_text(Glib::ustring(strm->str()) + "%");
    s_struct.cpu_progr_bar->set_fraction(frac);
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
  search_finished->connect([search_finished, progr_cpu, progr_cpu_sz, prog_mtx,
                            s_struct, show_res, progr_disp] {
    std::unique_ptr<Glib::Dispatcher> disp(search_finished);
    s_struct.cpu_progr_bar->set_visible(false);
    s_struct.cpu_progr_val->set_visible(false);
    s_struct.operation_name_lab->set_text(gettext("Sorting..."));
    delete progr_disp;
    delete progr_cpu;
    delete progr_cpu_sz;

    std::thread thr([show_res] {
      show_res->emit();
    });
    thr.detach();
    delete prog_mtx;
  });

  bk->auth_cpu_show_progr = [progr_cpu, progr_cpu_sz, prog_mtx, progr_disp](
                                const double &progr, const double &size) {
    prog_mtx->lock();
    *progr_cpu = progr;
    *progr_cpu_sz = size;
    prog_mtx->unlock();
    progr_disp->emit();
  };
  std::thread thr([this, search_finished] {
    authors = bk->collectionAuthors();
    bk->auth_cpu_show_progr = nullptr;
    search_finished->emit();
  });
  thr.detach();
#else
  double *progr_gpu = new double(0.0);
  double *progr_gpu_sz = new double(1.0);
  std::mutex *progr_gpu_mtx = new std::mutex;

  Glib::Dispatcher *progr_disp_cpu = new Glib::Dispatcher;
  std::shared_ptr<std::stringstream> strm(new std::stringstream);
  progr_disp_cpu->connect([progr_cpu, progr_cpu_sz, prog_mtx, s_struct, strm] {
    prog_mtx->lock();
    double frac = (*progr_cpu) / (*progr_cpu_sz);
    prog_mtx->unlock();
    strm->clear();
    strm->str("");
    *strm << std::fixed << std::setprecision(2) << frac * 100.0;
    s_struct.cpu_label->set_visible(true);
    s_struct.cpu_progr_val->set_visible(true);
    s_struct.cpu_progr_bar->set_visible(true);
    s_struct.cpu_progr_val->set_text(Glib::ustring(strm->str()) + "%");
    s_struct.cpu_progr_bar->set_fraction(frac);
  });

  Glib::Dispatcher *progr_disp_gpu = new Glib::Dispatcher;
  std::shared_ptr<std::stringstream> strm_gpu(new std::stringstream);
  progr_disp_gpu->connect(
      [progr_gpu, progr_gpu_sz, progr_gpu_mtx, s_struct, strm_gpu] {
        progr_gpu_mtx->lock();
        double frac = (*progr_gpu) / (*progr_gpu_sz);
        progr_gpu_mtx->unlock();
        strm_gpu->clear();
        strm_gpu->str("");
        *strm_gpu << std::fixed << std::setprecision(2) << frac * 100.0;
        s_struct.gpu_label->set_visible(true);
        s_struct.gpu_progr_val->set_visible(true);
        s_struct.gpu_progr_bar->set_visible(true);
        s_struct.gpu_progr_val->set_text(Glib::ustring(strm_gpu->str()) + "%");
        s_struct.gpu_progr_bar->set_fraction(frac);
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

  Glib::Dispatcher *result_creating_disp = new Glib::Dispatcher;
  result_creating_disp->connect([s_struct] {
    s_struct.operation_name_lab->set_text(gettext("Collecting results..."));
    s_struct.cpu_label->set_visible(false);
    s_struct.cpu_progr_val->set_visible(false);
    s_struct.cpu_progr_bar->set_visible(false);
    s_struct.gpu_label->set_visible(false);
    s_struct.gpu_progr_bar->set_visible(false);
    s_struct.gpu_progr_val->set_visible(false);
  });

  Glib::Dispatcher *search_finished = new Glib::Dispatcher;
  search_finished->connect([search_finished, progr_cpu, progr_cpu_sz, prog_mtx,
                            progr_gpu, progr_gpu_sz, progr_gpu_mtx, s_struct,
                            show_res, progr_disp_gpu, progr_disp_cpu,
                            result_creating_disp] {
    std::unique_ptr<Glib::Dispatcher> disp(search_finished);
    s_struct.cpu_label->set_visible(false);
    s_struct.cpu_progr_bar->set_visible(false);
    s_struct.cpu_progr_val->set_visible(false);
    s_struct.gpu_label->set_visible(false);
    s_struct.gpu_progr_bar->set_visible(false);
    s_struct.gpu_progr_val->set_visible(false);
    s_struct.operation_name_lab->set_text(gettext("Sorting..."));
    delete progr_disp_gpu;
    delete progr_disp_cpu;
    delete progr_cpu;
    delete progr_cpu_sz;
    delete progr_gpu;
    delete progr_gpu_sz;

    delete result_creating_disp;

    std::thread thr([show_res] {
      show_res->emit();
    });
    thr.detach();
    delete prog_mtx;
    delete progr_gpu_mtx;
  });

  bk->auth_cpu_show_progr
      = [progr_cpu, progr_cpu_sz, prog_mtx, progr_disp_cpu,
         result_creating_disp](const double &progr, const double &size) {
          prog_mtx->lock();
          *progr_cpu = progr;
          *progr_cpu_sz = size;
          prog_mtx->unlock();
          progr_disp_cpu->emit();
        };

  bk->auth_gpu_show_progr
      = [progr_gpu, progr_gpu_sz, progr_gpu_mtx,
         progr_disp_gpu](const double &progr, const double &size) {
          progr_gpu_mtx->lock();
          *progr_gpu = progr;
          *progr_gpu_sz = size;
          progr_gpu_mtx->unlock();
          progr_disp_gpu->emit();
        };

  bk->auth_collecting_results = [result_creating_disp] {
    result_creating_disp->emit();
  };

  std::thread thr([this, search_finished] {
    authors = bk->collectionAuthors();
    bk->auth_cpu_show_progr = nullptr;
    bk->auth_gpu_show_progr = nullptr;
    bk->auth_collecting_results = nullptr;
    search_finished->emit();
  });
  thr.detach();
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

  std::thread thr([this, notes, search_finished] {
    search_result = bk->booksWithNotes(notes);
    search_finished->emit();
  });
  thr.detach();
}
