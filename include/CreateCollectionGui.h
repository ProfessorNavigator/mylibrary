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

#ifndef CREATECOLLECTIONGUI_H
#define CREATECOLLECTIONGUI_H

#include <AuxFunc.h>
#include <giomm-2.68/giomm/asyncresult.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/entry.h>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#include <functional>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>

#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class CreateCollectionGui
{
public:
  CreateCollectionGui(const std::shared_ptr<AuxFunc> &af,
                      Gtk::Window *main_window);

  void
  createWindow();

  std::function<void(const std::string &col_name)> add_new_collection;

private:
  void
  bookPathDialog(Gtk::Window *win);

#ifndef ML_GTK_OLD
  void
  bookPathDialogSlot(const Glib::RefPtr<Gio::AsyncResult> &result,
                     const Glib::RefPtr<Gtk::FileDialog> &fd);
#endif
#ifdef ML_GTK_OLD
  void
  bookPathDialogSlot(int resp, Gtk::FileChooserDialog *fd);
#endif

  void
  checkInput(Gtk::Window *win);

  void
  errorDialog(Gtk::Window *win, const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;

  Gtk::Entry *collection_name = nullptr;

  Gtk::Entry *book_path = nullptr;

  Gtk::Entry *thread_num = nullptr;

  Gtk::CheckButton *disable_rar = nullptr;
};

#endif // CREATECOLLECTIONGUI_H
