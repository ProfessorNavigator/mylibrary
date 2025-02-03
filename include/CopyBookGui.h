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

#ifndef COPYBOOKGUI_H
#define COPYBOOKGUI_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <giomm-2.68/giomm/asyncresult.h>
#include <giomm-2.68/giomm/file.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <glibmm-2.68/glibmm/ustring.h>
#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>

#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class CopyBookGui
{
public:
  CopyBookGui(const std::shared_ptr<AuxFunc> &af, Gtk::Window *parent_window,
              const BookBaseEntry &bbe);

  void
  createWindow();

private:
#ifndef ML_GTK_OLD
  void
  save_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
            const Glib::RefPtr<Gtk::FileDialog> &fd);
#endif
#ifdef ML_GTK_OLD
  void
  save_slot(int resp, Gtk::FileChooserDialog *fd);
#endif

  void
  copy_func(const Glib::RefPtr<Gio::File> &fl);

  void
  result_dialog(const Glib::ustring &text);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;
  BookBaseEntry bbe;
};

#endif // COPYBOOKGUI_H
