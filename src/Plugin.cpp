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

#include <Plugin.h>
#include <iostream>
#include <stdexcept>

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
  unloadPlugin();
}

Plugin::Plugin(Plugin &&other)
{
  plugin = other.plugin;
  other.plugin = nullptr;
  plugin_path = std::move(other.plugin_path);
  library_handle = other.library_handle;
  other.library_handle = nullptr;
}

Plugin &
Plugin::operator=(Plugin &&other)
{
  if(this != &other)
    {
      unloadPlugin();
      plugin = other.plugin;
      other.plugin = nullptr;
      plugin_path = std::move(other.plugin_path);
      library_handle = other.library_handle;
      other.library_handle = nullptr;
    }
  return *this;
}

Plugin::Plugin(const std::filesystem::path &path_to_plugin, Bases *bases)
{
  plugin_path = path_to_plugin;
  loadPlugin(bases);
}

MLPlugin *
Plugin::getPlugin()
{
  return plugin;
}

std::filesystem::path
Plugin::getPluginPath() const
{
  return plugin_path;
}

void *
Plugin::getLibraryHandle()
{
  return library_handle;
}

void
Plugin::loadPlugin(Bases *bases)
{
#ifdef __linux
  void *handle = dlopen(plugin_path.c_str(), RTLD_NOW);
#endif
#ifdef _WIN32
  HINSTANCE handle = LoadLibraryA(plugin_path.string().c_str());
#endif
  if(handle != nullptr)
    {
      library_handle = handle;
      MLPlugin *(*create_ptr)(void *bases, void *plugin_path);
#ifdef __linux
      create_ptr = reinterpret_cast<MLPlugin *(*)(void *, void *)>(
          dlsym(handle, "create"));
#endif
#ifdef _WIN32
      create_ptr = reinterpret_cast<MLPlugin *(*)(void *, void *)>(
          GetProcAddress(handle, "create"));
#endif
      if(create_ptr)
        {
          plugin = create_ptr(bases, &plugin_path);

          if(plugin == nullptr)
            {
              std::string er = "Plugin::loadPlugin: error on plugin creation ";
              std::u8string u8str = plugin_path.u8string();
              er += std::string(u8str.begin(), u8str.end());
              throw std::runtime_error(er);
            }
        }
      else
        {
          std::string er
              = "Plugin::loadPlugin: plugin creation function not found ";
          std::u8string u8str = plugin_path.u8string();
          er += std::string(u8str.begin(), u8str.end());
          throw std::runtime_error(er);
        }
    }
  else
    {
      std::string er = "Plugin::loadPlugin ";
      std::u8string u8str = plugin_path.u8string();
      er += std::string(u8str.begin(), u8str.end());
#ifdef __linux
      er += " loading error: ";
      er += dlerror();
#elif defined(_WIN32)
      er += " loading error";
#endif
      throw std::runtime_error(er);
    }
}

void
Plugin::unloadPlugin()
{
  delete plugin;
  if(library_handle != nullptr)
    {
#ifdef __linux
      int ch = dlclose(library_handle);
      if(ch != 0)
        {
          std::cout << "Plugin::unloadPlugin error " << plugin_path << ": "
                    << dlerror() << std::endl;
        }
#elif defined(_WIN32)
      FreeLibrary(library_handle);
#endif
    }
  library_handle = nullptr;
  plugin = nullptr;
  plugin_path.clear();
}
