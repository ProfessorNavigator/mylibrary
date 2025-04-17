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

#ifndef IMPORTCOLLECTIONGUI_H
#define IMPORTCOLLECTIONGUI_H

#include <AuxFunc.h>
#include <filesystem>
#include <functional>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#else
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class ImportCollectionGui
{
public:
  ImportCollectionGui(const std::shared_ptr<AuxFunc> &af,
                      Gtk::Window *parent_window);

  void
  createWindow();

  std::function<void(const std::string &col_name)> signal_success;

private:
  void
  open_file_dialog(Gtk::Window *win, Gtk::Entry *ent, const int &variant);

#ifndef ML_GTK_OLD
  void
  open_file_dialog_slot_base(const Glib::RefPtr<Gio::AsyncResult> &result,
                             const Glib::RefPtr<Gtk::FileDialog> &fd,
                             Gtk::Entry *ent);

  void
  open_file_dialog_slot_books(const Glib::RefPtr<Gio::AsyncResult> &result,
                              const Glib::RefPtr<Gtk::FileDialog> &fd,
                              Gtk::Entry *ent);
#else
  void
  open_file_dialog_slot_base(int resp, Gtk::FileChooserDialog *fd,
                             Gtk::Entry *ent);

  void
  open_file_dialog_slot_books(int resp, Gtk::FileChooserDialog *fd,
                              Gtk::Entry *ent);
#endif

  void
  error_dialog(Gtk::Window *win, const int &variant);

  void
  check_function(Gtk::Window *win);

  void
  import_collection(Gtk::Window *win, std::filesystem::path &col_base_path,
                    const std::filesystem::path &base_p,
                    const std::filesystem::path &books_p);

  void
  final_dialog(Gtk::Window *win);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;

  Gtk::Entry *col_name = nullptr;
  Gtk::Entry *base_path = nullptr;
  Gtk::Entry *books_path = nullptr;
};

#endif // IMPORTCOLLECTIONGUI_H
