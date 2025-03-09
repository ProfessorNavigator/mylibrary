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

#ifndef ADDBOOKGUI_H
#define ADDBOOKGUI_H

#include <AddBookModelItem.h>
#include <AuxFunc.h>
#include <BookMarks.h>
#include <filesystem>
#include <functional>
#include <giomm-2.68/giomm/asyncresult.h>
#include <giomm-2.68/giomm/file.h>
#include <giomm-2.68/giomm/liststore.h>
#include <giomm-2.68/giomm/menu.h>
#include <glibmm-2.68/glibmm/dispatcher.h>
#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/columnviewcolumn.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/entry.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/listitem.h>
#include <gtkmm-4.0/gtkmm/popovermenu.h>
#include <gtkmm-4.0/gtkmm/singleselection.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include <memory>
#include <string>
#include <vector>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/filedialog.h>
#endif
#ifdef ML_GTK_OLD
#include <gtkmm/filechooserdialog.h>
#endif

class AddBookGui
{
public:
  AddBookGui(const std::shared_ptr<AuxFunc> &af, Gtk::Window *parent_window,
             const std::shared_ptr<BookMarks> &bookmarks,             
             const bool &directory_add);

  void
  createWindow();

  std::function<void(const std::string &col_name)> books_added;

private:
  void
  display_sizes();

  Glib::RefPtr<Gtk::StringList>
  form_collections_list();

  Glib::RefPtr<Gtk::StringList>
  form_archive_types_list();

  void
  modeSelector(Gtk::Window *win, Gtk::CheckButton *pack_in_arch,
               Gtk::CheckButton *add_to_arch);

  void
  bookSelectionWindow(Gtk::Window *win, const int &variant);

  void
  book_add_dialog(Gtk::Window *win, const int &variant);

#ifndef ML_GTK_OLD
  void
  book_add_dialog_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
                       const Glib::RefPtr<Gtk::FileDialog> &fd,
                       const int &variant);
#endif

#ifdef ML_GTK_OLD
  void
  book_add_dialog_slot(int resp, Gtk::FileChooserDialog *fd,
                       const int &variant);
#endif

  void
  form_books_list(const std::vector<Glib::RefPtr<Gio::File>> &files,
                  const int &variant);

  Glib::RefPtr<Gtk::ColumnViewColumn>
  form_sources_column();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  form_col_path_column();

  Glib::RefPtr<Gtk::ColumnViewColumn>
  form_col_arch_path_column();

  void
  slot_setup(const Glib::RefPtr<Gtk::ListItem> &list_item, const int &variant);

  void
  slot_bind(const Glib::RefPtr<Gtk::ListItem> &list_item, const int &variant);

  void
  slot_select_book(guint pos);

  void
  form_colletion_path_not_arch(const Glib::RefPtr<AddBookModelItem> &item);

  void
  form_colletion_path_arch_overwrite(
      const Glib::RefPtr<AddBookModelItem> &item);

  void
  form_colletion_path_arch_add(const Glib::RefPtr<AddBookModelItem> &item);

  void
  check_book_path_not_arch(const Glib::RefPtr<AddBookModelItem> &item);

  void
  create_action_group(Gtk::Window *win, const int &variant);

  void
  action_remove_book();

  void
  action_chage_path_notarch(Gtk::Window *win);

  void
  action_chage_path_arch(Gtk::Window *win, const int &variant);

#ifndef ML_GTK_OLD
  void
  action_chage_path_notarch_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
                                 const Glib::RefPtr<Gtk::FileDialog> &fd);
#endif

#ifdef ML_GTK_OLD
  void
  action_chage_path_notarch_slot(int resp, Gtk::FileChooserDialog *fd);
#endif

  void
  action_chage_path_arch_slot(Gtk::Entry *path, Gtk::Window *win,
                              const int &variant);

  Glib::RefPtr<Gio::Menu>
  create_menu(const int &variant);

  void
  show_popup_menu(int nclck, double x, double y, Gtk::PopoverMenu *menu,
                  const Glib::RefPtr<Gtk::SingleSelection> &selection);

  bool
  add_books(Gtk::Window *win, const int &variant);

  void
  add_books_window(Gtk::Window *win);

  void
  finish(Gtk::Window *win, const int &variant);

  void
  error_alert_dialog(Gtk::Window *win, const int &variant);

  void
  check_conflict_names(const Glib::RefPtr<AddBookModelItem> &item);

  void
  archive_selection_dialog_overwrite(Gtk::Window *win);

  void
  archive_selection_dialog_add(Gtk::Window *win);

#ifndef ML_GTK_OLD
  void
  archive_selection_dialog_overwrite_slot(
      const Glib::RefPtr<Gio::AsyncResult> &result,
      const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win);
#endif

#ifdef ML_GTK_OLD
  void
  archive_selection_dialog_overwrite_slot(int resp, Gtk::FileChooserDialog *fd,
                                          Gtk::Window *win);
#endif

#ifndef ML_GTK_OLD
  void
  archive_selection_dialog_add_slot(
      const Glib::RefPtr<Gio::AsyncResult> &result,
      const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win);
#endif

#ifdef ML_GTK_OLD
  void
  archive_selection_dialog_add_slot(int resp, Gtk::FileChooserDialog *fd,
                                    Gtk::Window *win);
#endif

  Gtk::Window *
  wait_window(Gtk::Window *win);

  std::shared_ptr<AuxFunc> af;
  Gtk::Window *parent_window = nullptr;
  std::shared_ptr<BookMarks> bookmarks;
  bool directory_add = false;

  int width = 0;
  int height = 0;

  Gtk::DropDown *collection = nullptr;
  Glib::RefPtr<Gio::ListStore<AddBookModelItem>> books_list;

  Glib::RefPtr<AddBookModelItem> selected_book;

  std::filesystem::path books_path;

  Gtk::Label *warn_lab = nullptr;
  Gtk::Label *error_lab = nullptr;

  std::string collection_name;

  std::shared_ptr<Glib::Dispatcher> finish_add_disp;
  std::shared_ptr<Glib::Dispatcher> finish_add_err_disp;
  std::shared_ptr<Glib::Dispatcher> finish_wait_disp;

  std::vector<std::vector<Glib::RefPtr<AddBookModelItem>>> name_conflicts;

  Gtk::Button *add_books_col = nullptr;

  Gtk::DropDown *arch_t_dd = nullptr;

  std::filesystem::path result_archive_path;

  std::vector<std::string> archive_filenames;

  bool remove_sources = false;
};

#endif // ADDBOOKGUI_H
