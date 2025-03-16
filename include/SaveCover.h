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

#ifndef SAVECOVER_H
#define SAVECOVER_H

#include <BookInfoEntry.h>
#include <giomm-2.68/giomm/file.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class SaveCover
{
public:
  SaveCover(const std::shared_ptr<BookInfoEntry> &bie,
            Gtk::Window *parent_window);

  void
  createWindow();

private:
  Glib::RefPtr<Gtk::StringList>
  create_model();

  void
  save_dialog(Gtk::Window *win);

#ifndef ML_GTK_OLD
  void
  save_dialog_result(const Glib::RefPtr<Gio::AsyncResult> &result,
                     const Glib::RefPtr<Gtk::FileDialog> &fd,
                     Gtk::Window *win);
#endif

#ifdef ML_GTK_OLD
  void
  save_dialog_result(int resp, Gtk::FileChooserDialog *fd, Gtk::Window *win);
#endif

  void
  saveFunc(Gtk::Window *win, const Glib::RefPtr<Gio::File> &fl);

  std::shared_ptr<BookInfoEntry> bie;
  Gtk::Window *parent_window = nullptr;

  Gtk::DropDown *format = nullptr;
};

#endif // SAVECOVER_H
