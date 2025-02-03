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
#include <CreateCollectionGui.h>
#include <giomm/cancellable.h>
#include <giomm/file.h>
#include <glibmm/signalproxy.h>
#include <glibmm/ustring.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>

#ifndef ML_GTK_OLD
#include <gtkmm/error.h>
#endif
#include <filesystem>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <iostream>
#include <libintl.h>
#include <locale>
#include <sigc++/connection.h>
#include <sstream>
#include <thread>

CreateCollectionGui::CreateCollectionGui(const std::shared_ptr<AuxFunc> &af,
                                         Gtk::Window *main_window)
{
  this->af = af;
  this->main_window = main_window;
}

void
CreateCollectionGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Create collection"));
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
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Collection name:"));
  grid->attach(*lab, 0, row, 2, 1);
  row++;

  collection_name = Gtk::make_managed<Gtk::Entry>();
  collection_name->set_margin(5);
  collection_name->set_halign(Gtk::Align::FILL);
  collection_name->set_hexpand(true);
  collection_name->set_width_chars(30);
  grid->attach(*collection_name, 0, row, 2, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Path to books directory:"));
  grid->attach(*lab, 0, row, 2, 1);
  row++;

  book_path = Gtk::make_managed<Gtk::Entry>();
  book_path->set_margin(5);
  book_path->set_halign(Gtk::Align::FILL);
  book_path->set_hexpand(true);
  book_path->set_width_chars(50);
  grid->attach(*book_path, 0, row, 2, 1);
  row++;

  Gtk::Button *book_path_but = Gtk::make_managed<Gtk::Button>();
  book_path_but->set_margin(5);
  book_path_but->set_halign(Gtk::Align::END);
  book_path_but->set_label(gettext("Open"));
  book_path_but->set_name("operationBut");
  book_path_but->signal_clicked().connect(
      std::bind(&CreateCollectionGui::bookPathDialog, this, window));
  grid->attach(*book_path_but, 1, row, 1, 1);
  row++;

  Gtk::Grid *thread_grid = Gtk::make_managed<Gtk::Grid>();
  thread_grid->set_halign(Gtk::Align::FILL);
  thread_grid->set_valign(Gtk::Align::FILL);
  thread_grid->set_expand(true);
  grid->attach(*thread_grid, 0, row, 2, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  strm << std::thread::hardware_concurrency();
  lab->set_text(gettext("Thread number (recommended max value: ")
                + Glib::ustring(strm.str()) + ")");
  thread_grid->attach(*lab, 0, 0, 1, 1);

  thread_num = Gtk::make_managed<Gtk::Entry>();
  thread_num->set_margin(5);
  thread_num->set_halign(Gtk::Align::START);
  thread_num->set_max_width_chars(2);
  thread_num->set_width_chars(2);
  thread_num->set_alignment(Gtk::Align::CENTER);
  thread_num->set_text("1");
  thread_grid->attach(*thread_num, 1, 0, 1, 1);

  disable_rar = Gtk::make_managed<Gtk::CheckButton>();
  disable_rar->set_margin(5);
  disable_rar->set_halign(Gtk::Align::START);
  disable_rar->set_active(false);
  disable_rar->set_label(gettext("Disable rar archives support"));
  grid->attach(*disable_rar, 0, row, 2, 1);
  row++;

  Gtk::Button *create = Gtk::make_managed<Gtk::Button>();
  create->set_margin(5);
  create->set_halign(Gtk::Align::CENTER);
  create->set_label(gettext("Create"));
  create->set_name("applyBut");
  create->signal_clicked().connect(
      std::bind(&CreateCollectionGui::checkInput, this, window));
  grid->attach(*create, 0, row, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_label(gettext("Cancel"));
  cancel->set_name("cancelBut");
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, row, 1, 1);

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
CreateCollectionGui::bookPathDialog(Gtk::Window *win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_title(gettext("Books path"));
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_initial_folder(initial);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fd->select_folder(*win,
                    std::bind(&CreateCollectionGui::bookPathDialogSlot, this,
                              std::placeholders::_1, fd),
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
      std::bind(&CreateCollectionGui::bookPathDialogSlot, this,
                std::placeholders::_1, fd));

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
CreateCollectionGui::bookPathDialogSlot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->select_folder_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::Code::FAILED)
        {
          std::cout << "CreateCollectionGui::bookPathDialogSlot error: "
                    << er.what() << std::endl;
        }
    }
  if(fl)
    {
      book_path->set_text(fl->get_path());
    }
}
#endif

void
CreateCollectionGui::checkInput(Gtk::Window *win)
{
  std::string filename(collection_name->get_text());
  if(filename.empty())
    {
      errorDialog(win, 1);
      return void();
    }

  std::filesystem::path collection_path = af->homePath();
  collection_path
      /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
  collection_path /= std::filesystem::u8path(filename);
  if(std::filesystem::exists(collection_path))
    {
      errorDialog(win, 2);
      return void();
    }

  filename = std::string(book_path->get_text());
  std::filesystem::path bookpath = std::filesystem::u8path(filename);
  if(!std::filesystem::exists(bookpath))
    {
      errorDialog(win, 3);
      return void();
    }
  CollectionCrProcessGui *progr = new CollectionCrProcessGui(
      af, main_window, collection_path, bookpath, !disable_rar->get_active(),
      std::string(thread_num->get_text()));
  progr->add_new_collection = add_new_collection;
  progr->createWindow(1);
  win->close();
}

#ifdef ML_GTK_OLD
void
CreateCollectionGui::bookPathDialogSlot(int resp, Gtk::FileChooserDialog *fd)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          book_path->set_text(fl->get_path());
        }
    }
  fd->close();
}
#endif

void
CreateCollectionGui::errorDialog(Gtk::Window *win, const int &variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
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
  switch(variant)
    {
    case 1:
      {
        lab->set_text(gettext("Collection name cannot be empty!"));
        break;
      }
    case 2:
      {
        lab->set_text(gettext("Collection already exists!"));
        break;
      }
    case 3:
      {
        lab->set_text(gettext("Books path does not exist!"));
        break;
      }
    default:
      break;
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
