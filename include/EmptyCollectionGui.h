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

#ifndef EMPTYCOLLECTIONGUI_H
#define EMPTYCOLLECTIONGUI_H

#include <AuxFunc.h>
#include <filesystem>
#include <functional>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class EmptyCollectionGui
{
public:
  EmptyCollectionGui(const std::shared_ptr<AuxFunc> &af,
                     Gtk::Window *parent_window);

  void
  createWindow();

  std::function<void(const std::string &col_name)> signal_success;

private:
  void
  open_directory_dialog(Gtk::Window *win, Gtk::Entry *ent);

#ifndef ML_GTK_OLD
  void
  open_directory_dialog_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
                             const Glib::RefPtr<Gtk::FileDialog> &fd,
                             Gtk::Entry *ent);
#endif
#ifdef ML_GTK_OLD
  void
  open_directory_dialog_slot(int resp, Gtk::FileChooserDialog *fd,
                             Gtk::Entry *ent);
#endif

  void
  error_dialog(Gtk::Window *win, const int &variant);

  void
  check_function(Gtk::Window *win);

  void
  creat_collection(Gtk::Window *win, std::filesystem::path &col_base_path,
                   const std::filesystem::path &books_p);

  void
  final_dialog(Gtk::Window *win);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;

  Gtk::Entry *col_name_ent = nullptr;
  Gtk::Entry *books_path_ent = nullptr;
};

#endif // EMPTYCOLLECTIONGUI_H
