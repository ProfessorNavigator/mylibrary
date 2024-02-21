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

#ifndef INCLUDE_SEARCHPROCESSGUI_H_
#define INCLUDE_SEARCHPROCESSGUI_H_

#include <BaseKeeper.h>
#include <BookBaseEntry.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <functional>
#include <vector>

class SearchProcessGui
{
public:
  SearchProcessGui(BaseKeeper *bk, Gtk::Window *main_window);
  virtual
  ~SearchProcessGui();

  void
  createWindow(const BookBaseEntry &search);

  std::function<void
  (const std::vector<BookBaseEntry> &result)> search_result_show;

private:
  void
  startSearch(Gtk::Window *win, const BookBaseEntry &search);

  BaseKeeper *bk = nullptr;
  Gtk::Window *main_window = nullptr;
  std::vector<BookBaseEntry> search_result;
};

#endif /* INCLUDE_SEARCHPROCESSGUI_H_ */
