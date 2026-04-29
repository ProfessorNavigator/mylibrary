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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <Bases.h>
#include <Plugin.h>
#include <UDBase.h>
#include <filesystem>
#include <vector>

class PluginManager
{
public:
  PluginManager(const Bases &bases);

  std::shared_ptr<Plugin>
  addPlugin(const std::filesystem::path &plugin_path);

  void
  removePlugin(const std::shared_ptr<Plugin> &plugin);

  std::vector<std::shared_ptr<Plugin>>
  getPlugins();

private:
  void
  loadPlugins();

  Bases bases;

  std::filesystem::path plugins_base_path;
  UDBase plugins_base;
  std::vector<std::shared_ptr<Plugin>> plugins;
};

#endif // PLUGINMANAGER_H
