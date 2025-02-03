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

#ifndef REMOVECOLLECTIONGUI_H
#define REMOVECOLLECTIONGUI_H

#include <AuxFunc.h>
#include <functional>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>

class RemoveCollectionGui
{
public:
  RemoveCollectionGui(const std::shared_ptr<AuxFunc> &af,
                      Gtk::Window *main_window);

  void
  createWindow();

  std::function<void(const std::string &collection_name)> collection_removed;

private:
  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;

  Gtk::DropDown *collection_name = nullptr;

  Glib::RefPtr<Gtk::StringList>
  formCollectionsModel();

  void
  confirmationDialog(Gtk::Window *win);

  void
  successDialog(Gtk::Window *win, const std::string &filename);
};

#endif // REMOVECOLLECTIONGUI_H
