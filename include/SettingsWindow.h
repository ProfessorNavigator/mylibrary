/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <AuxFunc.h>
#include <filesystem>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/window.h>
#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#else
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class SettingsWindow
{
public:
  SettingsWindow(const std::shared_ptr<AuxFunc> &af,
                 Gtk::Window *parent_window);

  void
  createWindow();

private:
  Gtk::Widget *
  windowsSection();

  enum widget_type
  {
    apply_button,
    cancel_button,
    operation_button,
    remove_button,
    combo_box,
    text_field,
    entry,
    label,
    main_menu,
    menu_button,
    error_label,
    warning_label,
    progress_bar,
    column_view,
    frames
  };

  Gtk::Widget *
  formSection(const widget_type &wt);

  void
  readSettings();

  void
  parseSettings();

  struct setting
  {
    std::string attribute_id;
    std::string value;
  };

  struct section
  {
    std::string section_id;
    std::vector<setting> settings;
  };

  void
  parseSection(section &s, const std::string &section_str);

  void
  applySettings();

  void
  fileDialog(Gtk::Entry *ent);

#ifndef ML_GTK_OLD
  void
  fileDialogSlot(const Glib::RefPtr<Gio::AsyncResult> &result,
                 const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Entry *ent);
#else
  void
  fileDialogSlot(int respons_id, Gtk::FileChooserDialog *fd, Gtk::Entry *ent);
#endif

  void
  windowSize();

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window;

  Gtk::Window *window;

  std::filesystem::path save_path;

  std::string source_settings;

  std::vector<section> settings_v;
};

#endif // SETTINGSWINDOW_H
