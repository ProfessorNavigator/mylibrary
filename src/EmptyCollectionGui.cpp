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

#include <ByteOrder.h>
#include <EmptyCollectionGui.h>
#include <fstream>
#include <giomm-2.68/giomm/file.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <iostream>
#include <libintl.h>

#ifndef ML_GTK_OLD
#include <giomm-2.68/giomm/cancellable.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif

EmptyCollectionGui::EmptyCollectionGui(const std::shared_ptr<AuxFunc> &af,
                                       Gtk::Window *parent_window)
{
  this->af = af;
  this->parent_window = parent_window;
}

void
EmptyCollectionGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Create empty collection"));
  window->set_transient_for(*parent_window);
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
  lab->set_text(gettext("Collection name:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  col_name_ent = Gtk::make_managed<Gtk::Entry>();
  col_name_ent->set_margin(5);
  col_name_ent->set_halign(Gtk::Align::START);
  col_name_ent->set_width_chars(50);
  col_name_ent->set_name("windowEntry");
  grid->attach(*col_name_ent, 0, 1, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Path to books directory:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 2, 2, 1);

  books_path_ent = Gtk::make_managed<Gtk::Entry>();
  books_path_ent->set_margin(5);
  books_path_ent->set_halign(Gtk::Align::FILL);
  books_path_ent->set_name("windowEntry");
  grid->attach(*books_path_ent, 0, 3, 2, 1);

  Gtk::Button *open = Gtk::make_managed<Gtk::Button>();
  open->set_margin(5);
  open->set_halign(Gtk::Align::END);
  open->set_name("operationBut");
  open->set_label(gettext("Open"));
  open->signal_clicked().connect(
      std::bind(&EmptyCollectionGui::open_directory_dialog, this, window,
                books_path_ent));
  grid->attach(*open, 1, 4, 1, 1);

  Gtk::Button *create = Gtk::make_managed<Gtk::Button>();
  create->set_margin(5);
  create->set_halign(Gtk::Align::CENTER);
  create->set_name("applyBut");
  create->set_label(gettext("Create"));
  create->signal_clicked().connect(
      std::bind(&EmptyCollectionGui::check_function, this, window));
  grid->attach(*create, 0, 5, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, 5, 1, 1);

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

void
EmptyCollectionGui::open_directory_dialog(Gtk::Window *win, Gtk::Entry *ent)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  fd->set_title(gettext("Books path"));

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_initial_folder(initial);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

  fd->select_folder(*win,
                    std::bind(&EmptyCollectionGui::open_directory_dialog_slot,
                              this, std::placeholders::_1, fd, ent),
                    cncl);
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *win, gettext("Books path"), Gtk::FileChooser::Action::SELECT_FOLDER,
      true);
  fd->set_application(win->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_current_folder(initial);

  fd->signal_response().connect(
      std::bind(&EmptyCollectionGui::open_directory_dialog_slot, this,
                std::placeholders::_1, fd, ent));

  fd->signal_close_request().connect(
      [fd] {
        std::shared_ptr<Gtk::FileChooserDialog> fdl(fd);
        fdl->set_visible(false);
        return true;
      },
      false);

  fd->present();
#endif
}

#ifndef ML_GTK_OLD
void
EmptyCollectionGui::open_directory_dialog_slot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Entry *ent)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->select_folder_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout << "EmptyCollectionGui::open_directory_dialog_slot error: "
                    << er.what() << std::endl;
        }
    }

  if(fl)
    {
      ent->set_text(Glib::ustring(fl->get_path()));
    }
}
#endif

void
EmptyCollectionGui::error_dialog(Gtk::Window *win, const int &variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
  window->set_title(gettext("Error!"));
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
  lab->set_hexpand(true);
  lab->set_name("windowLabel");
  switch(variant)
    {
    case 1:
      {
        lab->set_text(gettext("Error! Collection name cannot be empty!"));
        break;
      }
    case 2:
      {
        lab->set_text(gettext("Error! Collection already exists!"));
        break;
      }
    case 3:
      {
        lab->set_text(gettext("Error! Books directory not found!"));
        break;
      }
    case 4:
      {
        lab->set_text(
            gettext("Error! Base file has not been opened for writing!"));
        break;
      }
    default:
      {
        delete window;
        return void();
      }
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*close, 0, 1, 1, 1);

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
EmptyCollectionGui::check_function(Gtk::Window *win)
{
  std::string collection_name(col_name_ent->get_text());

  if(collection_name.empty())
    {
      error_dialog(win, 1);
      return void();
    }
  std::filesystem::path col_base_path = af->homePath();
  col_base_path
      /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
  col_base_path /= std::filesystem::u8path(collection_name);
  if(std::filesystem::exists(col_base_path))
    {
      error_dialog(win, 2);
      return void();
    }

  std::filesystem::path books_p
      = std::filesystem::u8path(std::string(books_path_ent->get_text()));
  if(!std::filesystem::exists(books_p))
    {
      error_dialog(win, 3);
      return void();
    }
  creat_collection(win, col_base_path, books_p);
}

void
EmptyCollectionGui::creat_collection(Gtk::Window *win,
                                     std::filesystem::path &col_base_path,
                                     const std::filesystem::path &books_p)
{
  std::filesystem::create_directories(col_base_path);
  col_base_path /= std::filesystem::u8path("base");

  std::fstream f;
  f.open(col_base_path, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      uint16_t val16;
      size_t sz = sizeof(val16);
      std::string buf = books_p.u8string();
      val16 = static_cast<uint16_t>(buf.size());
      ByteOrder bo;
      bo = val16;
      bo.get_little(val16);

      f.write(reinterpret_cast<char *>(&val16), sz);
      f.write(buf.c_str(), buf.size());

      f.close();

      final_dialog(win);

      if(signal_success)
        {
          signal_success(col_base_path.parent_path().filename().u8string());
        }
    }
  else
    {
      error_dialog(win, 4);
    }
}

#ifdef ML_GTK_OLD
void
EmptyCollectionGui::open_directory_dialog_slot(int resp,
                                               Gtk::FileChooserDialog *fd,
                                               Gtk::Entry *ent)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          ent->set_text(Glib::ustring(fl->get_path()));
        }
    }
  fd->close();
}
#endif

void
EmptyCollectionGui::final_dialog(Gtk::Window *win)
{
  win->unset_child();
  win->set_default_size(1, 1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  win->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_valign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Collection has been successfully created."));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
  grid->attach(*close, 0, 1, 1, 1);
}
