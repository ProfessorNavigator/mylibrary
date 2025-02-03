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

#ifndef TRANSFERBOOKGUI_H
#define TRANSFERBOOKGUI_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <giomm-2.68/giomm/asyncresult.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#include <LibArchive.h>
#include <filesystem>
#include <functional>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>
#include <vector>

#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#endif

class TransferBookGui
{
public:
  TransferBookGui(const std::shared_ptr<AuxFunc> &af,
                  const std::shared_ptr<BookMarks> &bookmarks,
                  const BookBaseEntry &bbe_from,
                  const std::string &collection_from,
                  Gtk::Window *parent_window);

  void
  createWindow();

  std::function<void(const BookBaseEntry &remove, const std::string &col_name)>
      success_signal;

private:
  Glib::RefPtr<Gtk::StringList>
  create_collections_model();

  Glib::RefPtr<Gtk::StringList>
  create_archive_types_model();

  void
  mode_selector(Gtk::Window *win);

  void
  path_choose_dialog(Gtk::Window *win, const int &variant);

#ifndef ML_GTK_OLD
  void
  path_choose_dialog_overwrite_slot(
      const Glib::RefPtr<Gio::AsyncResult> &result,
      const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win,
      const int &variant);

  void
  path_choose_dialog_add_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
                              const Glib::RefPtr<Gtk::FileDialog> &fd,
                              Gtk::Window *win);
#endif
#ifdef ML_GTK_OLD
  void
  path_choose_dialog_overwrite_slot(int resp, Gtk::FileChooserDialog *fd,
                                    Gtk::Window *win, const int &variant);

  void
  path_choose_dialog_add_slot(int resp, Gtk::FileChooserDialog *fd,
                              Gtk::Window *win);
#endif

  void
  copy_process_window(Gtk::Window *win);

  void
  copy_overwrite(const int &variant, const std::shared_ptr<int> &res_var);

  void
  finish_window(Gtk::Window *win, const int &variant);

  void
  path_in_archive_window(Gtk::Window *win, const int &variant);

  void
  copy_archive(Gtk::Window *parent_win, Gtk::Window *win,
               Gtk::Entry *path_in_arch, const int &variant);

  void
  alert_dialog(Gtk::Window *win, const int &variant);

  std::shared_ptr<AuxFunc> af;
  std::shared_ptr<BookMarks> bookmarks;
  BookBaseEntry bbe_from;
  std::string collection_from;
  Gtk::Window *parent_window = nullptr;

  Gtk::CheckButton *transfer_fbd = nullptr;
  Gtk::CheckButton *compress = nullptr;
  Gtk::CheckButton *add_to_arch = nullptr;
  Gtk::DropDown *collections = nullptr;
  Gtk::DropDown *arch_types = nullptr;

  std::filesystem::path books_path;
  std::string collection_to;

  std::filesystem::path out_file_path;

  std::shared_ptr<Glib::Dispatcher> copy_result_disp;
  std::shared_ptr<Glib::Dispatcher> form_arch_filelist_disp;

  std::string path_in_arch;

  std::vector<std::string> arch_filelist;

  bool _transfer_fbd = false;
};

#endif // TRANSFERBOOKGUI_H
