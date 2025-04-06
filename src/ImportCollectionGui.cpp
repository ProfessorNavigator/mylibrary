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

#include <ByteOrder.h>
#include <ImportCollectionGui.h>
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

ImportCollectionGui::ImportCollectionGui(const std::shared_ptr<AuxFunc> &af,
                                         Gtk::Window *parent_window)
{
  this->af = af;
  this->parent_window = parent_window;
}

void
ImportCollectionGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Import collection base"));
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
  lab->set_text(gettext("New collection name:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  col_name = Gtk::make_managed<Gtk::Entry>();
  col_name->set_margin(5);
  col_name->set_halign(Gtk::Align::START);
  col_name->set_width_chars(50);
  col_name->set_name("windowEntry");
  grid->attach(*col_name, 0, 1, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Path to base to be imported:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 2, 2, 1);

  base_path = Gtk::make_managed<Gtk::Entry>();
  base_path->set_margin(5);
  base_path->set_hexpand(true);
  base_path->set_name("windowEntry");
  grid->attach(*base_path, 0, 3, 2, 1);

  Gtk::Button *open = Gtk::make_managed<Gtk::Button>();
  open->set_margin(5);
  open->set_halign(Gtk::Align::END);
  open->set_name("operationBut");
  open->set_label(gettext("Open"));
  open->signal_clicked().connect(std::bind(
      &ImportCollectionGui::open_file_dialog, this, window, base_path, 1));
  grid->attach(*open, 1, 4, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Path to books directory:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 5, 2, 1);

  books_path = Gtk::make_managed<Gtk::Entry>();
  books_path->set_margin(5);
  books_path->set_hexpand(true);
  books_path->set_name("windowEntry");
  grid->attach(*books_path, 0, 6, 2, 1);

  open = Gtk::make_managed<Gtk::Button>();
  open->set_margin(5);
  open->set_halign(Gtk::Align::END);
  open->set_name("operationBut");
  open->set_label(gettext("Open"));
  open->signal_clicked().connect(std::bind(
      &ImportCollectionGui::open_file_dialog, this, window, books_path, 2));
  grid->attach(*open, 1, 7, 1, 1);

  Gtk::Button *import = Gtk::make_managed<Gtk::Button>();
  import->set_margin(5);
  import->set_halign(Gtk::Align::CENTER);
  import->set_name("applyBut");
  import->set_label(gettext("Import"));
  import->signal_clicked().connect(
      std::bind(&ImportCollectionGui::check_function, this, window));
  grid->attach(*import, 0, 8, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, 8, 1, 1);

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
ImportCollectionGui::open_file_dialog(Gtk::Window *win, Gtk::Entry *ent,
                                      const int &variant)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  switch(variant)
    {
    case 1:
      {
        fd->set_title(gettext("Open base"));
        break;
      }
    case 2:
      {
        fd->set_title(gettext("Books path"));
        break;
      }
    default:
      break;
    }

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());

  fd->set_initial_folder(initial);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

  switch(variant)
    {
    case 1:
      {
        fd->open(*win,
                 std::bind(&ImportCollectionGui::open_file_dialog_slot_base,
                           this, std::placeholders::_1, fd, ent),
                 cncl);
        break;
      }
    case 2:
      {
        fd->select_folder(
            *win,
            std::bind(&ImportCollectionGui::open_file_dialog_slot_books, this,
                      std::placeholders::_1, fd, ent),
            cncl);
        break;
      }
    default:
      break;
    }
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fd;
  switch(variant)
    {
    case 1:
      {
        fd = new Gtk::FileChooserDialog(*win, gettext("Open base"),
                                        Gtk::FileChooser::Action::OPEN, true);
        break;
      }
    case 2:
      {
        fd = new Gtk::FileChooserDialog(
            *win, gettext("Books path"),
            Gtk::FileChooser::Action::SELECT_FOLDER, true);
        break;
      }
    default:
      return void();
    }

  fd->set_application(win->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_current_folder(initial);

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  switch(variant)
    {
    case 1:
      {
        but = fd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);
        fd->signal_response().connect(
            std::bind(&ImportCollectionGui::open_file_dialog_slot_base, this,
                      std::placeholders::_1, fd, ent));
        break;
      }
    case 2:
      {
        but = fd->add_button(gettext("Select"), Gtk::ResponseType::ACCEPT);
        fd->signal_response().connect(
            std::bind(&ImportCollectionGui::open_file_dialog_slot_books, this,
                      std::placeholders::_1, fd, ent));
        break;
      }
    default:
      {
        delete fd;
        return void();
      }
    }
  but->set_margin(5);
  but->set_name("applyBut");

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
ImportCollectionGui::open_file_dialog_slot_base(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Entry *ent)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->open_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout
              << "ImportCollectionGui::open_file_dialog_slot_base error: "
              << er.what() << std::endl;
        }
    }

  if(fl)
    {
      ent->set_text(Glib::ustring(fl->get_path()));
    }
}

void
ImportCollectionGui::open_file_dialog_slot_books(
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
          std::cout
              << "ImportCollectionGui::open_file_dialog_slot_books error: "
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
ImportCollectionGui::error_dialog(Gtk::Window *win, const int &variant)
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
        lab->set_text(gettext("Error! Base file not found!"));
        break;
      }
    case 4:
      {
        lab->set_text(gettext("Error! Books directory not found!"));
        break;
      }
    case 5:
      {
        lab->set_text(
            gettext("Error! Base file has not been opened for reading!"));
        break;
      }
    case 6:
      {
        lab->set_text(
            gettext("Error! Base file has not been opened for writing!"));
        break;
      }
    case 7:
      {
        lab->set_text(gettext("Error! Bad base file!"));
        break;
      }
    case 8:
      {
        lab->set_text(gettext("Error! Bad base file (2)!"));
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
ImportCollectionGui::check_function(Gtk::Window *win)
{
  std::string collection_name(col_name->get_text());

  if(collection_name.empty())
    {
      error_dialog(win, 1);
      return void();
    }
  std::filesystem::path col_base_path = af->homePath();
  col_base_path /= std::filesystem::u8path(".local")
                   / std::filesystem::u8path("share")
                   / std::filesystem::u8path("MyLibrary")
                   / std::filesystem::u8path("Collections");
  col_base_path /= std::filesystem::u8path(collection_name);
  if(std::filesystem::exists(col_base_path))
    {
      error_dialog(win, 2);
      return void();
    }

  std::filesystem::path source_base_path
      = std::filesystem::u8path(std::string(base_path->get_text()));
  if(!std::filesystem::exists(source_base_path))
    {
      error_dialog(win, 3);
      return void();
    }

  std::filesystem::path books_p
      = std::filesystem::u8path(std::string(books_path->get_text()));
  if(!std::filesystem::exists(books_p))
    {
      error_dialog(win, 4);
      return void();
    }

  import_collection(win, col_base_path, source_base_path, books_p);
}

void
ImportCollectionGui::import_collection(Gtk::Window *win,
                                       std::filesystem::path &col_base_path,
                                       const std::filesystem::path &base_p,
                                       const std::filesystem::path &books_p)
{
  bool success = false;
  std::filesystem::create_directories(col_base_path);
  col_base_path /= std::filesystem::u8path("base");

  std::fstream f;
  f.open(base_p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t fsz;
      f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);

      std::fstream out;
      out.open(col_base_path, std::ios_base::out | std::ios_base::binary);

      if(out.is_open())
        {
          uint16_t val16;
          size_t sz = sizeof(val16);

          if(fsz < sz)
            {
              error_dialog(win, 7);
              out.close();
              f.close();
              std::filesystem::remove_all(col_base_path.parent_path());
              return void();
            }

          f.read(reinterpret_cast<char *>(&val16), sz);
          ByteOrder bo;
          bo.set_little(val16);
          val16 = bo;

          if(fsz < sz + static_cast<size_t>(val16))
            {
              error_dialog(win, 8);
              out.close();
              f.close();
              std::filesystem::remove_all(col_base_path.parent_path());
              return void();
            }

          f.seekg(val16, std::ios_base::cur);

          std::string buf = books_p.u8string();
          if(buf.size() > 0)
            {
              std::string sstr("\\");
              std::string::size_type n = 0;
              for(;;)
                {
                  n = buf.find(sstr, n);
                  if(n != std::string::npos)
                    {
                      buf.erase(n, sstr.size());
                      buf.insert(n, "/");
                    }
                  else
                    {
                      break;
                    }
                }
            }
          val16 = static_cast<uint16_t>(buf.size());
          bo = val16;
          bo.get_little(val16);
          out.write(reinterpret_cast<char *>(&val16), sz);

          out.write(buf.c_str(), buf.size());

          buf.clear();

          buf.resize(fsz - static_cast<size_t>(f.tellg()));
          f.read(buf.data(), buf.size());

          out.write(buf.c_str(), buf.size());

          out.close();
          success = true;
        }
      else
        {
          error_dialog(win, 6);
        }
      f.close();
    }
  else
    {
      error_dialog(win, 5);
    }

  if(success)
    {
      final_dialog(win);
      if(signal_success)
        {
          signal_success(col_base_path.parent_path().filename().u8string());
        }
    }
}

#ifdef ML_GTK_OLD
void
ImportCollectionGui::open_file_dialog_slot_base(int resp,
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

void
ImportCollectionGui::open_file_dialog_slot_books(int resp,
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
ImportCollectionGui::final_dialog(Gtk::Window *win)
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
  lab->set_text(gettext("Collection has been successfully imported."));
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
