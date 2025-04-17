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

#include <CoverPixBuf.h>
#include <SaveCover.h>
#include <filesystem>
#include <functional>
#include <glibmm-2.68/glibmm/miscutils.h>
#include <gtkmm-4.0/gdkmm/pixbuf.h>
#include <gtkmm-4.0/gdkmm/pixbufformat.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <iostream>
#include <libintl.h>
#include <vector>

#ifndef ML_GTK_OLD
#include <giomm-2.68/giomm/cancellable.h>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif

SaveCover::SaveCover(const std::shared_ptr<BookInfoEntry> &bie,
                     Gtk::Window *parent_window)
{
  this->bie = bie;
  this->parent_window = parent_window;
}

void
SaveCover::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Cover saving"));
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
  lab->set_expand(true);
  lab->set_use_markup(true);
  lab->set_markup("<b>" + Glib::ustring(gettext("Format")) + "</b>");
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  Glib::RefPtr<Gtk::StringList> format_list = create_model();

  format = Gtk::make_managed<Gtk::DropDown>();
  format->set_margin(5);
  format->set_halign(Gtk::Align::CENTER);
  format->set_name("comboBox");
  format->set_model(format_list);
  if(format_list->get_n_items() > 0)
    {
      format->set_selected(0);
    }
  grid->attach(*format, 0, 1, 2, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_margin(5);
  save->set_halign(Gtk::Align::CENTER);
  save->set_name("applyBut");
  save->set_label(gettext("Save"));
  save->signal_clicked().connect(
      std::bind(&SaveCover::save_dialog, this, window));
  grid->attach(*save, 0, 5, 1, 1);

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
SaveCover::save_dialog(Gtk::Window *win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_title(gettext("Cover saving path"));
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(Glib::get_home_dir());
  fd->set_initial_folder(initial);

  Glib::ustring name;
  guint pos = format->get_selected();
  if(pos != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> model
          = std::dynamic_pointer_cast<Gtk::StringList>(format->get_model());
      if(model)
        {
          name = "Cover." + model->get_string(pos);

          Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
          filter->add_suffix(model->get_string(pos));
          fd->set_default_filter(filter);

          Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> f_list
              = Gio::ListStore<Gtk::FileFilter>::create();
          f_list->append(filter);
          fd->set_filters(f_list);
        }
    }
  fd->set_initial_name(name);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fd->save(*win,
           std::bind(&SaveCover::save_dialog_result, this,
                     std::placeholders::_1, fd, win),
           cncl);
#else
  Gtk::FileChooserDialog *fd
      = new Gtk::FileChooserDialog(*win, gettext("Cover saving path"),
                                   Gtk::FileChooser::Action::SAVE, true);
  fd->set_application(win->get_application());
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

  Glib::ustring name;
  guint pos = format->get_selected();
  if(pos != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> model
          = std::dynamic_pointer_cast<Gtk::StringList>(format->get_model());
      if(model)
        {
          name = "Cover." + model->get_string(pos);
        }
    }
  fd->set_current_name(name);

  fd->signal_response().connect(std::bind(&SaveCover::save_dialog_result, this,
                                          std::placeholders::_1, fd, win));

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
SaveCover::save_dialog_result(const Glib::RefPtr<Gio::AsyncResult> &result,
                              const Glib::RefPtr<Gtk::FileDialog> &fd,
                              Gtk::Window *win)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->save_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::Code::FAILED)
        {
          std::cout << "SaveCover::save_dialog_result error: " << er.what()
                    << std::endl;
        }
    }
  if(fl)
    {
      saveFunc(win, fl);
    }
}
#endif

Glib::RefPtr<Gtk::StringList>
SaveCover::create_model()
{
  Glib::RefPtr<Gtk::StringList> result
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::vector<Gdk::PixbufFormat> formats = Gdk::Pixbuf::get_formats();

  for(auto it = formats.begin(); it != formats.end(); it++)
    {
      if(it->is_writable())
        {
          result->append(it->get_name());
        }
    }

  return result;
}

#ifdef ML_GTK_OLD
void
SaveCover::save_dialog_result(int resp, Gtk::FileChooserDialog *fd,
                              Gtk::Window *win)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          saveFunc(win, fl);
        }
    }
  fd->close();
}
#endif

void
SaveCover::saveFunc(Gtk::Window *win, const Glib::RefPtr<Gio::File> &fl)
{
  bool result = false;
  std::filesystem::path save_path = std::filesystem::u8path(fl->get_path());
  Glib::ustring name;
  guint pos = format->get_selected();
  if(pos != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> model
          = std::dynamic_pointer_cast<Gtk::StringList>(format->get_model());
      if(model)
        {
          name = model->get_string(pos);
        }
    }
  CoverPixBuf cpb(bie);
  Glib::RefPtr<Gdk::Pixbuf> buf = cpb;
  if(std::filesystem::exists(save_path))
    {
      std::filesystem::remove_all(save_path);
    }

  if(!name.empty() && buf)
    {
      try
        {
          buf->save(save_path.u8string(), name);
          result = true;
        }
      catch(Glib::Error &er)
        {
          std::cout << "SaveCover::saveFunc error: " << er.what() << std::endl;
          name = er.what();
          result = false;
        }
    }
  else
    {
      result = false;
      name = gettext("Format or gdk-pixbuf error.");
      std::cout << "SaveCover::saveFunc: format or gdk-pixbuf error."
                << std::endl;
    }

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
  lab->set_expand(true);
  lab->set_name("windowLabel");
  if(result)
    {
      lab->set_text(gettext("Cover successfully saved!"));
    }
  else
    {
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_max_width_chars(50);
      lab->set_width_chars(50);
      lab->set_justify(Gtk::Justification::CENTER);
      lab->set_text(Glib::ustring(gettext("Error!")) + "\n" + name);
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
  grid->attach(*close, 0, 1, 1, 1);
}
