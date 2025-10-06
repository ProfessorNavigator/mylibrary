/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef SEARCHRESULTMODELITEMFL_H
#define SEARCHRESULTMODELITEMFL_H

#include <FileParseEntry.h>
#include <glibmm-2.68/glibmm/object.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <mutex>

class SearchResultModelItemFL : public Glib::Object
{
public:
  static Glib::RefPtr<SearchResultModelItemFL>
  create(const FileParseEntry &entry);

  FileParseEntry entry;

  void
  setLabel(Gtk::Label *lab);

  void
  unsetLabel();

  void
  activateLab();

  void
  deactivateLab();

protected:
  SearchResultModelItemFL(const FileParseEntry &entry);

  Gtk::Label *l_lab = nullptr;
  std::mutex l_lab_mtx;
};

#endif // SEARCHRESULTMODELITEMFL_H
