/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef MLPLUGIN_H
#define MLPLUGIN_H

#include <AuxFunc.h>
#include <gtkmm-4.0/gtkmm/window.h>

class MLPlugin
{
public:
  MLPlugin(void *af_ptr);

  virtual ~MLPlugin();

  virtual void
  createWindow(Gtk::Window *parent_window);

  Glib::ustring
  getPluginName();

  Glib::ustring
  getPluginDescription();

protected:
  std::shared_ptr<AuxFunc> af;

  Glib::ustring plugin_name;
  Glib::ustring plugin_description;
};

#endif // MLPLUGIN_H
