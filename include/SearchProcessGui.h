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

#ifndef SEARCHPROCESSGUI_H
#define SEARCHPROCESSGUI_H

#include <BaseKeeper.h>
#include <BookBaseEntry.h>
#include <NotesBaseEntry.h>
#include <functional>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/progressbar.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <vector>

class SearchProcessGui
{
public:
  SearchProcessGui(BaseKeeper *bk, Gtk::Window *main_window);

  void
  createWindow(const BookBaseEntry &search, const double &coef_coincedence);

  void
  createWindow(const std::string &collection_name, std::shared_ptr<AuxFunc> af,
               const int &variant);

  void
  createWindow(const std::vector<NotesBaseEntry> &notes);

  std::function<void(const std::vector<BookBaseEntry> &result)>
      search_result_show;

  std::function<void(const std::vector<FileParseEntry> &result)>
      search_result_file;

  std::function<void(const std::vector<std::string> &result)>
      search_result_authors;

private:
  void
  startSearch(Gtk::Window *win, const BookBaseEntry &search,
              const double &coef_coincedence);

  void
  copyFiles(Gtk::Window *win, const std::string &collection_name,
            std::shared_ptr<AuxFunc> af);

  struct AuthShowStruct
  {
    Gtk::Label *operation_name_lab = nullptr;
    Gtk::Label *cpu_label = nullptr;
    Gtk::Label *gpu_label = nullptr;
    Gtk::Label *cpu_progr_val = nullptr;
    Gtk::Label *gpu_progr_val = nullptr;
    Gtk::ProgressBar *cpu_progr_bar = nullptr;
    Gtk::ProgressBar *gpu_progr_bar = nullptr;
  };

  void
  showAuthors(Gtk::Window *win, AuthShowStruct &s_struct,
              const std::string &collection_name);

  void
  showBooksWithNotes(Gtk::Window *win,
                     const std::vector<NotesBaseEntry> &notes);

  BaseKeeper *bk = nullptr;
  Gtk::Window *main_window = nullptr;
  std::vector<BookBaseEntry> search_result;
  std::vector<FileParseEntry> files;
  std::vector<std::string> authors;
};

#endif // SEARCHPROCESSGUI_H
