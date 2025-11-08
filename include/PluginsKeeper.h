/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef PLUGINSKEEPER_H
#define PLUGINSKEEPER_H

#include <AuxFunc.h>
#include <MLPlugin.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <iostream>
#include <vector>

#ifdef __linux
#include <dlfcn.h>
#endif
#ifdef _WIN32
#include <libloaderapi.h>
#endif

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#else
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

struct plugin
{
  MLPlugin *plugin_ptr = nullptr;
#ifdef __linux
  void *handle = nullptr;
#endif
#ifdef _WIN32
  HINSTANCE handle = nullptr;
#endif
  std::filesystem::path plugin_path;
  Gtk::Widget *widg = nullptr;

  ~plugin()
  {
    delete plugin_ptr;
    if(handle)
      {
#ifdef __linux
        int ch = dlclose(handle);
        if(ch != 0)
          {
            std::cout << "Plugin unloading error" << plugin_path << ": "
                      << dlerror() << std::endl;
          }
#endif
#ifdef _WIN32
        FreeLibrary(handle);
#endif
      }
  };
};

class PluginsKeeper
{
public:
  PluginsKeeper(Gtk::Window *parent_window,
                const std::shared_ptr<AuxFunc> &af);

  virtual ~PluginsKeeper();

  void
  createWindow();

  std::function<void()> signal_reload_collection_list;

private:
  void
  setWindowSizes();

  void
  loadPlugins();

  void
  parseRawBase(const std::string &raw_base);

  void
  loadPlugin(const std::filesystem::path &pp);

  void
  savePlugins();

  void
  removeConfirmationWindow(Gtk::Widget *widg, Gtk::Widget *flbx);

  void
  addPluginDialog(Gtk::Widget *flbx);

#ifndef ML_GTK_OLD
  void
  addPluginDialogSlot(const Glib::RefPtr<Gio::AsyncResult> &result,
                      const Glib::RefPtr<Gtk::FileDialog> &fd,
                      Gtk::Widget *flbx);
#else
  void
  addPluginDialogSlot(int respons_id, Gtk::FileChooserDialog *fd,
                      Gtk::Widget *flbx);
#endif

  void
  addPluginDialogSlotProc(const Glib::RefPtr<Gio::File> &fl,
                          Gtk::Widget *flbx);

  Gtk::Window *parent_window;
  std::shared_ptr<AuxFunc> af;

  Gtk::Window *main_window;

  std::vector<std::shared_ptr<plugin>> plugin_list;
};

#endif // PLUGINSKEEPER_H
