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
#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include <MLPlugin.h>

class ExamplePlugin : public MLPlugin
{
public:
  ExamplePlugin(void *af_ptr);

  void
  createWindow(Gtk::Window *parent_window) override;
};

extern "C"
{
#ifdef __linux
  MLPlugin *
  create(void *af_ptr)
  {
    return new ExamplePlugin(af_ptr);
  }
#endif
#ifdef _WIN32
  __declspec(dllexport) MLPlugin *
  create(void *af_ptr)
  {
    return new ExamplePlugin(af_ptr);
  }
#endif
}
#endif // EXAMPLEPLUGIN_H
