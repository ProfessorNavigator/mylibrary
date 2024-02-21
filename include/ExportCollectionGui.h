/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_EXPORTCOLLECTIONGUI_H_
#define INCLUDE_EXPORTCOLLECTIONGUI_H_

#include <AuxFunc.h>
#include <giomm-2.68/giomm/asyncresult.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class ExportCollectionGui
{
public:
  ExportCollectionGui(const std::shared_ptr<AuxFunc> &af,
		      Gtk::Window *parent_window);
  virtual
  ~ExportCollectionGui();

  void
  createWindow();

private:
  Glib::RefPtr<Gtk::StringList>
  create_collections_list();

  void
  export_file_dialog(Gtk::Window *win);

#ifndef ML_GTK_OLD
  void
  export_file_dialog_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
			  const Glib::RefPtr<Gtk::FileDialog> &fd,
			  Gtk::Window *win);
#endif

#ifdef ML_GTK_OLD
  void
  export_file_dialog_slot(int resp, Gtk::FileChooserDialog *fd,
  			  Gtk::Window *win);
#endif

  void
  export_func(const std::filesystem::path &exp_p, Gtk::Window *win);

  void
  result_window(Gtk::Window *win, const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;

  Gtk::DropDown *collection = nullptr;
};

#endif /* INCLUDE_EXPORTCOLLECTIONGUI_H_ */
