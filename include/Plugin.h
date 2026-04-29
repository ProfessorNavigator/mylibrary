/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef PLUGIN_H
#define PLUGIN_H

#include <MLPlugin.h>

#ifdef __linux
#include <dlfcn.h>
#elif defined(_WIN32)
#include <libloaderapi.h>
#endif

class Plugin
{
public:
  Plugin();

  virtual ~Plugin();

  Plugin(const Plugin &) = delete;

  Plugin(Plugin &&other);

  Plugin &
  operator=(const Plugin &o)
      = delete;

  Plugin &
  operator=(Plugin &&other);

  Plugin(const std::filesystem::path &path_to_plugin, Bases *bases);

  MLPlugin *
  getPlugin();

  std::filesystem::path
  getPluginPath() const;

  void *
  getLibraryHandle();

private:
  void
  loadPlugin(Bases *bases);

  void
  unloadPlugin();

  MLPlugin *plugin = nullptr;
  std::filesystem::path plugin_path;

#ifdef __linux
  void *library_handle = nullptr;
#endif
#ifdef _WIN32
  HINSTANCE library_handle = nullptr;
#endif
};

#endif // PLUGIN_H
