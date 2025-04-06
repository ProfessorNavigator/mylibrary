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

#include <RemoveCollectionGui.h>
#include <filesystem>
#include <glibmm-2.68/glibmm/main.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <libintl.h>

RemoveCollectionGui::RemoveCollectionGui(
    const std::shared_ptr<AuxFunc> &af, Gtk::Window *main_window,
    const std::shared_ptr<NotesKeeper> &notes)
{
  this->af = af;
  this->main_window = main_window;
  this->notes = notes;
  reserve_notes_path = af->homePath();
  reserve_notes_path /= std::filesystem::u8path(af->randomFileName());
  while(std::filesystem::exists(reserve_notes_path))
    {
      reserve_notes_path = af->homePath();
      reserve_notes_path /= std::filesystem::u8path(af->randomFileName());
    }
}

void
RemoveCollectionGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Collection removing"));
  window->set_transient_for(*main_window);
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
  lab->set_text(gettext("Collection"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 2, 1);
  row++;

  collection_name = Gtk::make_managed<Gtk::DropDown>();
  collection_name->set_margin(5);
  collection_name->set_halign(Gtk::Align::CENTER);
  collection_name->set_name("comboBox");
  grid->attach(*collection_name, 0, row, 2, 1);
  row++;

  Glib::RefPtr<Gtk::StringList> model = formCollectionsModel();
  collection_name->set_model(model);
  collection_name->set_enable_search(true);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_selectable(true);
  lab->set_text(Glib::ustring(gettext("Reserve copies path: "))
                + reserve_notes_path.u8string());
  lab->set_visible(false);

  Gtk::CheckButton *reserve_notes_chb = Gtk::make_managed<Gtk::CheckButton>();
  reserve_notes_chb->set_margin(5);
  reserve_notes_chb->set_halign(Gtk::Align::START);
  reserve_notes_chb->set_active(false);
  reserve_notes_chb->set_name("windowLabel");
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
  grid->attach(*reserve_notes_chb, 0, row, 2, 1);
  row++;

  grid->attach(*lab, 0, row, 2, 1);
  row++;

  Gtk::Button *remove = Gtk::make_managed<Gtk::Button>();
  remove->set_margin(5);
  remove->set_halign(Gtk::Align::CENTER);
  remove->set_label(gettext("Remove"));
  remove->set_name("removeBut");
  remove->signal_clicked().connect(
      std::bind(&RemoveCollectionGui::confirmationDialog, this, window));
  grid->attach(*remove, 0, row, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, row, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::shared_ptr<Gtk::Window> win(window);
        delete this;
        return true;
      },
      false);

  window->present();
}

Glib::RefPtr<Gtk::StringList>
RemoveCollectionGui::formCollectionsModel()
{
  Glib::RefPtr<Gtk::StringList> list
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::filesystem::path col_path = af->homePath();
  col_path /= std::filesystem::u8path(".local")
              / std::filesystem::u8path("share")
              / std::filesystem::u8path("MyLibrary")
              / std::filesystem::u8path("Collections");
  if(std::filesystem::exists(col_path))
    {
      for(auto &dirit : std::filesystem::directory_iterator(col_path))
        {
          std::filesystem::path p = dirit.path();
          if(std::filesystem::is_directory(p))
            {
              list->append(p.filename().u8string());
            }
        }
    }

  return list;
}

void
RemoveCollectionGui::confirmationDialog(Gtk::Window *win)
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

  Glib::RefPtr<Gtk::StringList> list
      = std::dynamic_pointer_cast<Gtk::StringList>(
          collection_name->get_model());
  guint sel = collection_name->get_selected();
  if(sel != 0xffffffff)
    {
      std::string filename(list->get_string(sel));

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_expand(true);
      lab->set_text(Glib::ustring(gettext("Collection for removing:")) + " "
                    + Glib::ustring(filename));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, 0, 2, 1);

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin_top(10);
      lab->set_margin_bottom(5);
      lab->set_margin_start(5);
      lab->set_margin_end(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_text(gettext("Continue?"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, 1, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
      yes->set_margin(5);
      yes->set_halign(Gtk::Align::CENTER);
      yes->set_expand(true);
      yes->set_label(gettext("Yes"));
      yes->set_name("removeBut");
      yes->signal_clicked().connect([window, win, filename, this] {
        successDialog(win, filename);
        window->close();
      });
      grid->attach(*yes, 0, 2, 1, 1);

      Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
      no->set_margin(5);
      no->set_halign(Gtk::Align::CENTER);
      no->set_expand(true);
      no->set_label(gettext("No"));
      no->set_name("cancelBut");
      no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
      grid->attach(*no, 1, 2, 1, 1);
    }
  else
    {
      delete window;
      return void();
    }

  window->signal_close_request().connect(
      [window] {
        std::shared_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

void
RemoveCollectionGui::successDialog(Gtk::Window *win,
                                   const std::string &filename)
{
  std::filesystem::path p = af->homePath();
  p /= std::filesystem::u8path(".local") / std::filesystem::u8path("share")
       / std::filesystem::u8path("MyLibrary")
       / std::filesystem::u8path("Collections");
  p /= std::filesystem::u8path(filename);
  std::filesystem::remove_all(p);

  notes->removeCollection(filename, reserve_notes_path, reserve_notes);

  if(collection_removed)
    {
      collection_removed(filename);
    }

  win->unset_child();
  win->set_default_size(1, 1);

  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  win->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Collection has been removed."));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_label(gettext("Close"));
  close->set_name("operationBut");
  close->signal_clicked().connect([win] {
    win->close();
  });
  grid->attach(*close, 0, 1, 1, 1);
}
