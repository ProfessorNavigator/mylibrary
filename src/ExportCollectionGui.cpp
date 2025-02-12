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
#include <ExportCollectionGui.h>
#include <giomm/cancellable.h>
#include <giomm/file.h>
#include <glibmm/main.h>
#include <glibmm/signalproxy.h>
#include <glibmm/ustring.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#ifndef ML_GTK_OLD
#include <gtkmm/error.h>
#endif
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <gtkmm/stringobject.h>
#include <libintl.h>
#include <sigc++/connection.h>
#include <stddef.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

ExportCollectionGui::ExportCollectionGui(const std::shared_ptr<AuxFunc> &af,
					 Gtk::Window *parent_window)
{
  this->af = af;
  this->parent_window = parent_window;
}

void
ExportCollectionGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Export collection base"));
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
  lab->set_text(gettext("Collection to export"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  Glib::RefPtr<Gtk::StringList> col_list = create_collections_list();

  collection = Gtk::make_managed<Gtk::DropDown>();
  collection->set_margin(5);
  collection->set_halign(Gtk::Align::CENTER);
  collection->set_name("comboBox");
  collection->set_model(col_list);
  collection->set_enable_search(true);
  grid->attach(*collection, 0, 1, 2, 1);

  Gtk::Button *export_col = Gtk::make_managed<Gtk::Button>();
  export_col->set_margin(5);
  export_col->set_halign(Gtk::Align::CENTER);
  export_col->set_name("operationBut");
  export_col->set_label(gettext("Export"));
  export_col->signal_clicked().connect(
      std::bind(&ExportCollectionGui::export_file_dialog, this, window));
  grid->attach(*export_col, 0, 2, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, 2, 1, 1);

  window->signal_close_request().connect([window, this]
  {
    std::unique_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    delete this;
    return true;
  },
					 false);

  window->present();
}

Glib::RefPtr<Gtk::StringList>
ExportCollectionGui::create_collections_list()
{
  Glib::RefPtr<Gtk::StringList> result = Gtk::StringList::create(
      std::vector<Glib::ustring>());

  std::filesystem::path col_p = af->homePath();
  col_p /= std::filesystem::u8path(".local/share/MyLibrary/Collections");

  if(std::filesystem::exists(col_p))
    {
      for(auto &dirit : std::filesystem::directory_iterator(col_p))
	{
	  std::filesystem::path p = dirit.path();
	  if(std::filesystem::is_directory(p))
	    {
	      result->append(Glib::ustring(p.filename().u8string()));
	    }
	}
    }

  return result;
}

void
ExportCollectionGui::export_file_dialog(Gtk::Window *win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  fd->set_title(gettext("Export path"));

  Glib::RefPtr<Gio::File> initial = Gio::File::create_for_path(
      af->homePath().u8string());
  fd->set_initial_folder(initial);

  Glib::RefPtr<Gtk::StringObject> col = std::dynamic_pointer_cast<
      Gtk::StringObject>(collection->get_selected_item());
  if(col)
    {
      std::string fnm(col->get_string());
      fnm = fnm + "_base";
      fd->set_initial_name(fnm);

      Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

      fd->save(
	  *win,
	  std::bind(&ExportCollectionGui::export_file_dialog_slot, this,
		    std::placeholders::_1, fd, win),
	  cncl);
    }
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *win, gettext("Export path"), Gtk::FileChooser::Action::SAVE, true);
  fd->set_application(win->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but = fd->add_button(gettext("Cancel"),
				    Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gio::File> initial = Gio::File::create_for_path(
      af->homePath().u8string());
  fd->set_current_folder(initial);

  Glib::RefPtr<Gtk::StringObject> col = std::dynamic_pointer_cast<
      Gtk::StringObject>(collection->get_selected_item());
  if(col)
    {
      std::string fnm(col->get_string());
      fnm = fnm + "_base";
      fd->set_current_name(Glib::ustring(fnm));

      fd->signal_response().connect(
	  std::bind(&ExportCollectionGui::export_file_dialog_slot, this,
		    std::placeholders::_1, fd, win));
    }
  else
    {
      delete fd;
      return void();
    }

  fd->signal_close_request().connect([fd]
  {
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
ExportCollectionGui::export_file_dialog_slot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->save_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
	{
	  std::cout << "ExportCollectionGui::export_file_dialog_slot error: "
	      << er.what() << std::endl;
	}
    }
  if(fl)
    {
      std::filesystem::path exp_p = std::filesystem::u8path(fl->get_path());
      export_func(exp_p, win);
    }
}
#endif

void
ExportCollectionGui::export_func(const std::filesystem::path &exp_p,
				 Gtk::Window *win)
{
  Glib::RefPtr<Gtk::StringObject> col = std::dynamic_pointer_cast<
      Gtk::StringObject>(collection->get_selected_item());
  if(col)
    {
      std::filesystem::path source_p = af->homePath();
      source_p /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
      source_p /= std::filesystem::u8path(std::string(col->get_string()));
      source_p /= std::filesystem::u8path("base");
      std::fstream f;
      f.open(source_p, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
	{
	  std::fstream out;
	  out.open(exp_p, std::ios_base::out | std::ios_base::binary);
	  if(out.is_open())
	    {
	      f.seekg(0, std::ios_base::end);
	      size_t fsz = static_cast<size_t>(f.tellg());
	      f.seekg(0, std::ios_base::beg);

	      uint16_t val16;
	      size_t sz = sizeof(val16);
	      if(fsz < sz)
		{
		  f.close();
		  out.close();
		  std::cout
		      << "ExportCollectionGui::export_func error: wrong base file size"
		      << std::endl;
		  result_window(win, 4);
		  return void();
		}
	      f.read(reinterpret_cast<char*>(&val16), sz);
	      ByteOrder bo;
	      bo.set_little(val16);
	      val16 = bo;

	      if(fsz < sz + static_cast<size_t>(val16))
		{
		  f.close();
		  out.close();
		  std::cout
		      << "ExportCollectionGui::export_func error: wrong base file size (2)"
		      << std::endl;
		  result_window(win, 5);
		  return void();
		}
	      f.seekg(val16, std::ios_base::cur);
	      val16 = 0;
	      out.write(reinterpret_cast<char*>(&val16), sz);

	      std::string buf;
	      buf.resize(fsz - static_cast<size_t>(f.tellg()));
	      f.read(buf.data(), buf.size());
	      out.write(buf.c_str(), buf.size());

	      out.close();

	      result_window(win, 6);
	    }
	  else
	    {
	      result_window(win, 3);
	    }
	  f.close();
	}
      else
	{
	  result_window(win, 2);
	}
    }
  else
    {
      result_window(win, 1);
    }
}

#ifdef ML_GTK_OLD
void
ExportCollectionGui::export_file_dialog_slot(int resp,
					     Gtk::FileChooserDialog *fd,
					     Gtk::Window *win)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
	{
	  std::filesystem::path exp_p = std::filesystem::u8path(fl->get_path());
	  export_func(exp_p, win);
	}
    }

  fd->close();
}
#endif

void
ExportCollectionGui::result_window(Gtk::Window *win, const int &variant)
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
  lab->set_expand(true);
  lab->set_name("windowLabel");
  switch(variant)
    {
    case 1:
      {
	lab->set_text(gettext("Error!"));
	break;
      }
    case 2:
      {
	lab->set_text(gettext("Error! Base file has not been opened."));
	break;
      }
    case 3:
      {
	lab->set_text(
	    gettext("Error! Out path has not been opened for writing."));
	break;
      }
    case 4:
      {
	lab->set_text(gettext("Error! Wrong base file size."));
	break;
      }
    case 5:
      {
	lab->set_text(gettext("Error! Wrong base file size (2)."));
	break;
      }
    case 6:
      {
	lab->set_text(
	    gettext("Collection base has been exported successfully!"));
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
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
  grid->attach(*close, 0, 1, 1, 1);
}
