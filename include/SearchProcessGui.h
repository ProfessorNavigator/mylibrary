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

#ifndef SEARCHPROCESSGUI_H
#define SEARCHPROCESSGUI_H

#include <BaseKeeper.h>
#include <BookBaseEntry.h>
#include <functional>
#include <gtkmm-4.0/gtkmm/window.h>
#include <vector>

class SearchProcessGui
{
public:
  SearchProcessGui(BaseKeeper *bk, Gtk::Window *main_window);

  void
  createWindow(const BookBaseEntry &search);

  void
  createWindow(const std::string &collection_name,
               std::shared_ptr<AuxFunc> af);

  std::function<void(const std::vector<BookBaseEntry> &result)>
      search_result_show;

  std::function<void(const std::vector<FileParseEntry> &result)>
      search_result_file;

private:
  void
  startSearch(Gtk::Window *win, const BookBaseEntry &search);

  void
  copyFiles(Gtk::Window *win, const std::string &collection_name,
            std::shared_ptr<AuxFunc> af);

  BaseKeeper *bk = nullptr;
  Gtk::Window *main_window = nullptr;
  std::vector<BookBaseEntry> search_result;
  std::vector<FileParseEntry> files;
};

#endif // SEARCHPROCESSGUI_H
