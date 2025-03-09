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
#include <MLPlugin.h>

void
MLPlugin::createWindow(Gtk::Window *parent_window)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_name("Empty plugin window");
  window->set_transient_for(*parent_window);
  window->set_name("MLwindow");
  window->set_modal(true);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

Glib::ustring
MLPlugin::getPluginName()
{
  return plugin_name;
}

Glib::ustring
MLPlugin::getPluginDescription()
{
  return plugin_description;
}

MLPlugin::MLPlugin(void *af_ptr)
{  
  af = *reinterpret_cast<std::shared_ptr<AuxFunc> *>(af_ptr);
}

MLPlugin::~MLPlugin()
{
}
