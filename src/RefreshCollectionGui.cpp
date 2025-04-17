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
#include <RefreshCollectionGui.h>
#include <filesystem>
#include <functional>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <libintl.h>
#include <sstream>
#include <string>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#endif

RefreshCollectionGui::RefreshCollectionGui(
    const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
    const std::shared_ptr<BookMarks> &bookmarks,
    const std::shared_ptr<NotesKeeper> &notes)
{
  this->af = af;
  this->main_window = main_window;
  this->bookmarks = bookmarks;
  this->notes = notes;

  reserve_notes_directory = af->homePath();
  reserve_notes_directory /= std::filesystem::u8path(af->randomFileName());
  while(std::filesystem::exists(reserve_notes_directory))
    {
      reserve_notes_directory = af->homePath();
      reserve_notes_directory /= std::filesystem::u8path(af->randomFileName());
    }
}

void
RefreshCollectionGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Refresh collection"));
  window->set_transient_for(*main_window);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_text(gettext("Collection to refresh:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  Glib::RefPtr<Gtk::StringList> col_list = createCollectionsList();

  collection = Gtk::make_managed<Gtk::DropDown>();
  collection->set_margin(5);
  collection->set_halign(Gtk::Align::CENTER);
  collection->set_model(col_list);
  collection->set_name("comboBox");
  if(col_list->get_n_items() > 0)
    {
      collection->set_selected(0);
    }
  collection->set_enable_search(true);
  grid->attach(*collection, 0, 1, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  std::stringstream strm;
  strm.imbue(std::locale("C"));
#ifndef USE_OPENMP
  strm << std::thread::hardware_concurrency();
#else
  strm << omp_get_num_procs();
#endif
  lab->set_text(gettext("Thread number (recommended max value: ")
                + Glib::ustring(strm.str()) + ")");
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 2, 1, 1);

  num_threads = Gtk::make_managed<Gtk::Entry>();
  num_threads->set_margin(5);
  num_threads->set_halign(Gtk::Align::START);
  num_threads->set_max_length(2);
  num_threads->set_max_width_chars(2);
  num_threads->set_alignment(Gtk::Align::CENTER);
  num_threads->set_text("1");
  num_threads->set_name("windowEntry");
  grid->attach(*num_threads, 1, 2, 1, 1);

  clean_empty = Gtk::make_managed<Gtk::CheckButton>();
  clean_empty->set_margin(5);
  clean_empty->set_halign(Gtk::Align::START);
  clean_empty->set_active(false);
  clean_empty->set_label(
      gettext("Remove empty files and directories from collection"));
  clean_empty->set_name("windowLabel");
  grid->attach(*clean_empty, 0, 3, 2, 1);

  fast_refreshing = Gtk::make_managed<Gtk::CheckButton>();
  fast_refreshing->set_margin(5);
  fast_refreshing->set_halign(Gtk::Align::START);
  fast_refreshing->set_active(false);
  fast_refreshing->set_label(gettext("Fast refreshing (without hashing)"));
  fast_refreshing->set_name("windowLabel");
  grid->attach(*fast_refreshing, 0, 4, 2, 1);

  refresh_bookmarks = Gtk::make_managed<Gtk::CheckButton>();
  refresh_bookmarks->set_margin(5);
  refresh_bookmarks->set_halign(Gtk::Align::START);
  refresh_bookmarks->set_active(false);
  refresh_bookmarks->set_label(gettext("Remove absent books from bookmarks"));
  refresh_bookmarks->set_name("windowLabel");
  grid->attach(*refresh_bookmarks, 0, 5, 2, 1);

  disable_rar = Gtk::make_managed<Gtk::CheckButton>();
  disable_rar->set_margin(5);
  disable_rar->set_halign(Gtk::Align::START);
  disable_rar->set_active(false);
  disable_rar->set_label(gettext("Disable rar archives support"));
  disable_rar->set_name("windowLabel");
  grid->attach(*disable_rar, 0, 6, 2, 1);

  Gtk::Grid *reserve_notes_grid = Gtk::make_managed<Gtk::Grid>();
  reserve_notes_grid->set_halign(Gtk::Align::FILL);
  reserve_notes_grid->set_valign(Gtk::Align::FILL);
  grid->attach(*reserve_notes_grid, 0, 7, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_visible(false);
  lab->set_text(Glib::ustring(gettext("Notes reserve path: "))
                + reserve_notes_directory.u8string());

  Gtk::CheckButton *reserve_notes_chb = Gtk::make_managed<Gtk::CheckButton>();
  reserve_notes_chb->set_margin(5);
  reserve_notes_chb->set_halign(Gtk::Align::START);
  reserve_notes_chb->set_name("windowLabel");
  reserve_notes_chb->set_active(false);
  reserve_notes_chb->set_label(gettext("Make reserve copies of notes"));
  reserve_notes_chb->signal_toggled().connect([this, lab, reserve_notes_chb] {
    reserve_notes = reserve_notes_chb->get_active();
    if(reserve_notes)
      {
        lab->set_visible(true);
      }
    else
      {
        lab->set_visible(false);
      }
  });
  reserve_notes_grid->attach(*reserve_notes_chb, 0, 0, 1, 1);

  reserve_notes_grid->attach(*lab, 0, 1, 1, 1);

  Gtk::Grid *action_group_grid = Gtk::make_managed<Gtk::Grid>();
  action_group_grid->set_halign(Gtk::Align::FILL);
  action_group_grid->set_valign(Gtk::Align::FILL);
  action_group_grid->set_expand(true);
  action_group_grid->set_column_homogeneous(true);
  grid->attach(*action_group_grid, 0, 8, 2, 1);

  Gtk::Button *refresh = Gtk::make_managed<Gtk::Button>();
  refresh->set_margin(5);
  refresh->set_halign(Gtk::Align::CENTER);
  refresh->set_label(gettext("Refresh"));
  refresh->set_name("removeBut");
  refresh->signal_clicked().connect(
      std::bind(&RefreshCollectionGui::confirmationDialog, this, window));
  action_group_grid->attach(*refresh, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  action_group_grid->attach(*cancel, 1, 0, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);

  window->present();
}

Glib::RefPtr<Gtk::StringList>
RefreshCollectionGui::createCollectionsList()
{
  Glib::RefPtr<Gtk::StringList> list
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::filesystem::path collections = af->homePath();
  collections /= std::filesystem::u8path(".local")
                 / std::filesystem::u8path("share")
                 / std::filesystem::u8path("MyLibrary")
                 / std::filesystem::u8path("Collections");
  if(std::filesystem::exists(collections))
    {
      for(auto &dirit : std::filesystem::directory_iterator(collections))
        {
          std::filesystem::path p = dirit.path();
          if(std::filesystem::is_directory(p))
            {
              list->append(Glib::ustring(p.filename().u8string()));
            }
        }
    }

  return list;
}

void
RefreshCollectionGui::confirmationDialog(Gtk::Window *win)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
  window->set_title(gettext("Confirmation"));
  window->set_transient_for(*win);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Are you sure?"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_margin(5);
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_label(gettext("Yes"));
  yes->set_name("removeBut");
  yes->signal_clicked().connect(
      std::bind(&RefreshCollectionGui::refreshCollection, this, window, win));
  grid->attach(*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_margin(5);
  no->set_halign(Gtk::Align::CENTER);
  no->set_label(gettext("No"));
  no->set_name("cancelBut");
  no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*no, 1, 1, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

void
RefreshCollectionGui::refreshCollection(Gtk::Window *win,
                                        Gtk::Window *parent_window)
{
  Glib::RefPtr<Gtk::StringList> list
      = std::dynamic_pointer_cast<Gtk::StringList>(collection->get_model());
  guint sel = collection->get_selected();
  if(sel != GTK_INVALID_LIST_POSITION && list)
    {
      std::string coll_name(list->get_string(sel));
      std::string num_threads(this->num_threads->get_text());

      CollectionCrProcessGui *ccpg = new CollectionCrProcessGui(
          af, main_window, coll_name, num_threads, clean_empty->get_active(),
          fast_refreshing->get_active(), refresh_bookmarks->get_active(),
          !disable_rar->get_active(), bookmarks);
      std::filesystem::path rsrv = reserve_notes_directory;
      bool rn = reserve_notes;
      std::shared_ptr<NotesKeeper> nk = notes;
      std::function<void(const std::string &)> sign = collection_refreshed;
      ccpg->collection_refreshed
          = [rsrv, rn, nk, sign](const std::string &coll_name) {
              nk->refreshCollection(coll_name, rsrv, rn);
              sign(coll_name);
            };
      ccpg->createWindow(2);
      win->unset_transient_for();
      parent_window->close();
      win->close();
    }
}
