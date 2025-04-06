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

#include <BookParseEntry.h>
#include <CopyBookGui.h>
#include <MLException.h>
#include <OpenBook.h>
#include <SelfRemovingPath.h>
#include <filesystem>
#include <functional>
#include <glibmm-2.68/glibmm/miscutils.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <iostream>
#include <libintl.h>
#include <string>

#ifndef ML_GTK_OLD
#include <giomm-2.68/giomm/cancellable.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif

CopyBookGui::CopyBookGui(const std::shared_ptr<AuxFunc> &af,
                         Gtk::Window *parent_window, const BookBaseEntry &bbe)
{
  this->af = af;
  this->parent_window = parent_window;
  this->bbe = bbe;
}

void
CopyBookGui::createWindow()
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_title(gettext("Save as..."));
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(Glib::get_home_dir());
  fd->set_initial_folder(initial);

  std::string ext = bbe.file_path.extension().u8string();
  if(ext != ".fb2" && ext != ".epub" && ext != ".pdf" && ext != ".djvu")
    {
      std::string bp = bbe.bpe.book_path;
      std::string::size_type n;
      std::string sstr = "\n";
      for(;;)
        {
          n = bp.find(sstr);
          if(n != std::string::npos)
            {
              bp.erase(0, n + sstr.size());
            }
          else
            {
              if(!bp.empty())
                {
                  ext = std::filesystem::u8path(bp).extension().u8string();
                }
              break;
            }
        }
    }
  std::string filename;
  if(!bbe.bpe.book_author.empty())
    {
      filename = bbe.bpe.book_author + "-" + bbe.bpe.book_name + ext;
    }
  else
    {
      filename = bbe.bpe.book_name + ext;
    }
  fd->set_initial_name(filename);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fd->save(*parent_window,
           std::bind(&CopyBookGui::save_slot, this, std::placeholders::_1, fd),
           cncl);
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fd
      = new Gtk::FileChooserDialog(*parent_window, gettext("Save as..."),
                                   Gtk::FileChooser::Action::SAVE, true);
  fd->set_application(parent_window->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(Glib::get_home_dir());
  fd->set_current_folder(initial);

  std::string ext = bbe.file_path.extension().u8string();
  if(ext != ".fb2" && ext != ".epub" && ext != ".pdf" && ext != ".djvu")
    {
      std::string bp = bbe.bpe.book_path;
      std::string::size_type n;
      std::string sstr = "\n";
      for(;;)
        {
          n = bp.find(sstr);
          if(n != std::string::npos)
            {
              bp.erase(0, n + sstr.size());
            }
          else
            {
              if(!bp.empty())
                {
                  ext = std::filesystem::u8path(bp).extension().u8string();
                }
              break;
            }
        }
    }
  std::string filename;
  if(!bbe.bpe.book_author.empty())
    {
      filename = bbe.bpe.book_author + "-" + bbe.bpe.book_name + ext;
    }
  else
    {
      filename = bbe.bpe.book_name + ext;
    }
  fd->set_current_name(Glib::ustring(filename));

  fd->signal_response().connect(
      std::bind(&CopyBookGui::save_slot, this, std::placeholders::_1, fd));

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
CopyBookGui::save_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
                       const Glib::RefPtr<Gtk::FileDialog> &fd)
{
  std::shared_ptr<CopyBookGui> cbg;
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->save_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout << "CopyBookGui::save_slot error: " << er.what()
                    << std::endl;
        }
      cbg = std::shared_ptr<CopyBookGui>(this);
    }
  if(fl)
    {
      copy_func(fl);
    }
}
#endif

void
CopyBookGui::copy_func(const Glib::RefPtr<Gio::File> &fl)
{
  std::filesystem::path tmp = af->temp_path();
  tmp /= std::filesystem::u8path(af->randomFileName());
  std::filesystem::create_directories(tmp);
  SelfRemovingPath srp(tmp);
  std::filesystem::path out = std::filesystem::u8path(fl->get_path());
  std::shared_ptr<OpenBook> ob = std::make_shared<OpenBook>(af);
  Glib::ustring result = gettext("Book successfully saved!");
  try
    {
      ob->open_book(bbe, false, tmp, false,
                    std::bind(&AuxFunc::copy_book_callback, af.get(),
                              std::placeholders::_1, out));
    }
  catch(MLException &er)
    {
      std::cout << er.what() << std::endl;
      result.clear();
      result = gettext("Error!");
      result = result + "\n" + er.what();
    }
  result_dialog(result);
}

#ifdef ML_GTK_OLD
void
CopyBookGui::save_slot(int resp, Gtk::FileChooserDialog *fd)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          copy_func(fl);
        }
    }

  fd->close();
}
#endif

void
CopyBookGui::result_dialog(const Glib::ustring &text)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
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
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  lab->set_text(text);
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*close, 0, 1, 1, 1);

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
