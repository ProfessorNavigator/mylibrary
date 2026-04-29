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

#include <PluginManager.h>
#include <QDir>
#include <iostream>

PluginManager::PluginManager(const Bases &bases)
{
  this->bases = bases;

  std::string str = QDir::homePath().toStdString();
  plugins_base_path = std::u8string(str.begin(), str.end());
  plugins_base_path /= std::filesystem::path(u8".local");
  plugins_base_path /= std::filesystem::path(u8"share");
  plugins_base_path /= std::filesystem::path(u8"MyLibrary");
  plugins_base_path /= std::filesystem::path(u8"plugins_base");
  loadPlugins();
}

std::shared_ptr<Plugin>
PluginManager::addPlugin(const std::filesystem::path &plugin_path)
{
  std::shared_ptr<Plugin> result;
  UDBase bs = plugins_base.searchElement(
      [plugin_path](const UDBElement &el)
        {
          std::filesystem::path p
              = std::u8string(el.content.begin(), el.content.end());
          return p == plugin_path;
        });
  if(bs.baseSize() > 0)
    {
      return result;
    }
  std::shared_ptr<Plugin> plugin;
  try
    {
      plugin = std::shared_ptr<Plugin>(new Plugin(plugin_path, &bases));
    }
  catch(std::exception &er)
    {
      std::cout << "PluginManager::addPlugin error: \"" << er.what() << "\""
                << std::endl;
      return result;
    }

  if(!plugin.operator bool())
    {
      return result;
    }

  MLPlugin *pl = plugin->getPlugin();
  if(pl == nullptr)
    {
      return result;
    }
  plugins.push_back(plugin);
  QString name = pl->getPluginName();
  UDBElement el;
  el.id = name.toStdString();
  std::u8string u8str = plugin_path.u8string();
  el.content = std::string(u8str.begin(), u8str.end());
  plugins_base.addElement(el);
  plugins_base.shrinkToFit();
  plugins_base.writeToFile(plugins_base_path);

  result = plugin;

  return result;
}

void
PluginManager::removePlugin(const std::shared_ptr<Plugin> &plugin)
{
  std::filesystem::path p = plugin->getPluginPath();
  plugins.erase(std::remove_if(plugins.begin(), plugins.end(),
                               [p](const std::shared_ptr<Plugin> &el)
                                 {
                                   return p == el->getPluginPath();
                                 }),
                plugins.end());
  plugins_base.removeElements(
      [p](const UDBElement &el)
        {
          std::filesystem::path lp
              = std::u8string(el.content.begin(), el.content.end());
          return p == lp;
        });
  plugins_base.writeToFile(plugins_base_path);
}

std::vector<std::shared_ptr<Plugin>>
PluginManager::getPlugins()
{
  return plugins;
}

void
PluginManager::loadPlugins()
{
  try
    {
      plugins_base.readFromFile(plugins_base_path);
    }
  catch(std::exception &er)
    {
      std::cout << "PluginManager::loadPlugins: \"" << er.what() << "\""
                << std::endl;
      return void();
    }

  std::vector<UDBElement> *raw_base = plugins_base.getRawBase();
  for(auto it = raw_base->begin(); it != raw_base->end(); it++)
    {
      std::filesystem::path p
          = std::u8string(it->content.begin(), it->content.end());
      std::shared_ptr<Plugin> plugin;
      try
        {
          plugin = std::shared_ptr<Plugin>(new Plugin(p, &bases));
        }
      catch(std::exception &er)
        {
          std::cout << "PluginManager::loadPlugins: \"" << er.what() << "\""
                    << std::endl;
        }
      if(plugin)
        {
          plugins.push_back(plugin);
        }
    }
  plugins.shrink_to_fit();
}
