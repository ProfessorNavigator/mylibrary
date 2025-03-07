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

#include <CollectionCrProcessGui.h>
#include <RefreshCollectionGui.h>
#include <filesystem>
#include <functional>
#include <glib/gtypes.h>
#include <glibmm/signalproxy.h>
#include <glibmm/ustring.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/enums.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <libintl.h>
#include <locale>
#include <sigc++/connection.h>
#include <sstream>
#include <string>
#include <thread>

RefreshCollectionGui::RefreshCollectionGui(
    const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
    const std::shared_ptr<BookMarks> &bookmarks)
{
  this->af = af;
  this->main_window = main_window;
  this->bookmarks = bookmarks;
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
  strm << std::thread::hardware_concurrency();
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

  Gtk::Grid *action_group_grid = Gtk::make_managed<Gtk::Grid>();
  action_group_grid->set_halign(Gtk::Align::FILL);
  action_group_grid->set_valign(Gtk::Align::FILL);
  action_group_grid->set_expand(true);
  action_group_grid->set_column_homogeneous(true);
  grid->attach(*action_group_grid, 0, 7, 2, 1);

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
  collections /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
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
      ccpg->collection_refreshed = collection_refreshed;
      ccpg->createWindow(2);
      win->unset_transient_for();
      parent_window->close();
      win->close();
    }
}
