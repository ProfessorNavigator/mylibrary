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

#ifndef SAVECOVER_H
#define SAVECOVER_H

#include <BookInfoEntry.h>
#include <giomm-2.68/giomm/file.h>
#include <giomm-2.68/giomm/listmodel.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#else
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class SaveCover : public Gtk::Window
{
public:
  SaveCover(const std::shared_ptr<BookInfoEntry> &bie,
            Gtk::Window *parent_window);

private:
  void
  createWindow();

  Glib::RefPtr<Gio::ListModel>
  createModel();

  void
  saveDialog();

#ifndef ML_GTK_OLD
  void
  saveDialogResult(const Glib::RefPtr<Gio::AsyncResult> &result,
                   const Glib::RefPtr<Gtk::FileDialog> &fd);
#else
  void
  saveDialogResult(int resp, Gtk::FileChooserDialog *fd);
#endif

  void
  saveFunc(const Glib::RefPtr<Gio::File> &fl);

  std::shared_ptr<BookInfoEntry> bie;
  Gtk::Window *parent_window = nullptr;

  Gtk::DropDown *format = nullptr;
};

#endif // SAVECOVER_H
