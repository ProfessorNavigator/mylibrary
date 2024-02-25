/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_COLLECTIONCRPROCESSGUI_H_
#define INCLUDE_COLLECTIONCRPROCESSGUI_H_

#include <AuxFunc.h>
#include <BookMarks.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/progressbar.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

class CollectionCrProcessGui
{
public:
  CollectionCrProcessGui(const std::shared_ptr<AuxFunc> &af,
			 Gtk::Window *main_window,
			 const std::filesystem::path &collection_path,
			 const std::filesystem::path &books_path,
			 const bool &rar_support, const std::string &num_thr);
  virtual
  ~CollectionCrProcessGui();

  CollectionCrProcessGui(const std::shared_ptr<AuxFunc> &af,
			 Gtk::Window *main_window, const std::string &coll_name,
			 const std::string &num_thr, const bool &remove_empty,
			 const bool &fast, const bool &refresh_bookmarks,
			 const bool &rar_support,
			 const std::shared_ptr<BookMarks> &bookmarks);

  void
  createWindow(const int &variant);

  std::function<void
  (const std::string &col_name)> add_new_collection;

  std::function<void
  (const std::string &col_name)> collection_refreshed;

private:
  void
  createProcessCreation(Gtk::Window *win);

  void
  createProcessRefresh(Gtk::Window *win);

  void
  finishInfo(Gtk::Window *win, const int &variant);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *main_window = nullptr;
  std::filesystem::path collection_path;
  std::filesystem::path books_path;
  int thr_num = 1;
  std::shared_ptr<BookMarks> bookmarks;

  std::string coll_name;

  std::atomic<bool> cancel_proc;
  Gtk::ProgressBar *creation_progress = nullptr;
  Gtk::Label *process_name = nullptr;
  Gtk::Button *cancel = nullptr;

  Glib::Dispatcher *pulse_disp = nullptr;
  Glib::Dispatcher *creation_finished_disp = nullptr;
  Glib::Dispatcher *new_collection_name_disp = nullptr;
  Glib::Dispatcher *total_files_disp = nullptr;
  Glib::Dispatcher *progress_disp = nullptr;

  Glib::Dispatcher *total_bytes_to_hash_disp = nullptr;
  Glib::Dispatcher *bytes_hashed_disp = nullptr;

  double total_files = 0.0;
  double progress_count = 0.0;

  double total_bytes_to_hash = 0.0;
  double bytes_hashed = 0.0;

  bool remove_empty = false;
  bool fast_refresh = false;
  bool refresh_bookmarkse = false;
  bool rar_support = false;
};

#endif /* INCLUDE_COLLECTIONCRPROCESSGUI_H_ */
