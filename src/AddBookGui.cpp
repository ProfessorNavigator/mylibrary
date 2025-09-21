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

#include <AddBook.h>
#include <AddBookGui.h>
#include <BaseKeeper.h>
#include <MLException.h>
#include <algorithm>
#include <giomm-2.68/giomm/menuitem.h>
#include <giomm-2.68/giomm/simpleaction.h>
#include <giomm-2.68/giomm/simpleactiongroup.h>
#include <glibmm-2.68/glibmm/main.h>
#include <gtkmm-4.0/gdkmm/display.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gdkmm/surface.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/columnview.h>
#include <gtkmm-4.0/gtkmm/dialog.h>
#include <gtkmm-4.0/gtkmm/filefilter.h>
#include <gtkmm-4.0/gtkmm/gestureclick.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/menubutton.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>
#include <gtkmm-4.0/gtkmm/stringobject.h>
#include <iostream>
#include <libintl.h>
#include <tuple>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#endif

#ifndef ML_GTK_OLD
#include <giomm-2.68/giomm/cancellable.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif

AddBookGui::AddBookGui(const std::shared_ptr<AuxFunc> &af,
                       Gtk::Window *parent_window,
                       const std::shared_ptr<BookMarks> &bookmarks,
                       const bool &directory_add)
{
  this->af = af;
  this->parent_window = parent_window;
  this->bookmarks = bookmarks;
  this->directory_add = directory_add;
  display_sizes();
}

void
AddBookGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  if(directory_add)
    {
      window->set_title(gettext("Directory adding"));
    }
  else
    {
      window->set_title(gettext("Books adding"));
    }
  window->set_transient_for(*parent_window);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  lab->set_vexpand(false);
  lab->set_use_markup(true);
  lab->set_name("windowLabel");
  if(directory_add)
    {
      lab->set_markup(Glib::ustring("<b>")
                      + gettext("Collection directories to be added to")
                      + "</b>");
    }
  else
    {
      lab->set_markup(Glib::ustring("<b>")
                      + gettext("Collection books to be added to") + "</b>");
    }
  grid->attach(*lab, 0, 0, 2, 1);

  Glib::RefPtr<Gtk::StringList> col_list = form_collections_list();

  collection = Gtk::make_managed<Gtk::DropDown>();
  collection->set_margin(5);
  collection->set_halign(Gtk::Align::CENTER);
  collection->set_name("comboBox");
  collection->set_model(col_list);
  grid->attach(*collection, 0, 1, 2, 1);

  Gtk::CheckButton *remove_src = Gtk::make_managed<Gtk::CheckButton>();
  remove_src->set_margin(5);
  remove_src->set_halign(Gtk::Align::START);
  remove_src->set_label(gettext("Remove source files"));
  remove_src->set_active(false);
  remove_src->set_name("windowLabel");
  Glib::PropertyProxy<bool> r_src_prop = remove_src->property_active();
  r_src_prop.signal_changed().connect([this, remove_src] {
    remove_sources = remove_src->get_active();
  });
  grid->attach(*remove_src, 0, 2, 2, 1);

  Gtk::CheckButton *pack_in_arch = Gtk::make_managed<Gtk::CheckButton>();
  pack_in_arch->set_margin(5);
  pack_in_arch->set_halign(Gtk::Align::START);
  pack_in_arch->set_name("windowLabel");
  if(directory_add)
    {
      pack_in_arch->set_label(gettext("Pack directories in archive"));
    }
  else
    {
      pack_in_arch->set_label(gettext("Pack books in archive"));
    }
  pack_in_arch->set_active(false);
  grid->attach(*pack_in_arch, 0, 3, 2, 1);

  Gtk::CheckButton *add_to_arch = Gtk::make_managed<Gtk::CheckButton>();
  add_to_arch->set_margin(5);
  add_to_arch->set_halign(Gtk::Align::START);
  add_to_arch->set_name("windowLabel");
  if(directory_add)
    {
      add_to_arch->set_label(
          Glib::ustring(gettext("Add directories to existing archive")) + "\n"
          + gettext("(archive will be recreated)"));
    }
  else
    {
      add_to_arch->set_label(
          Glib::ustring(gettext("Add books to existing archive")) + "\n"
          + gettext("(archive will be recreated)"));
    }
  add_to_arch->set_active(false);
  add_to_arch->set_visible(false);
  grid->attach(*add_to_arch, 0, 4, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Archive type:"));
  lab->set_visible(false);
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 5, 1, 1);

  Glib::RefPtr<Gtk::StringList> arch_types = form_archive_types_list();

  arch_t_dd = Gtk::make_managed<Gtk::DropDown>();
  arch_t_dd->set_margin(5);
  arch_t_dd->set_halign(Gtk::Align::START);
  arch_t_dd->set_name("comboBox");
  arch_t_dd->set_model(arch_types);
  arch_t_dd->set_visible(false);
  grid->attach(*arch_t_dd, 1, 5, 1, 1);

  Gtk::Button *apply = Gtk::make_managed<Gtk::Button>();
  apply->set_margin(5);
  apply->set_halign(Gtk::Align::CENTER);
  apply->set_name("operationBut");
  if(directory_add)
    {
      apply->set_label(gettext("Add directories"));
    }
  else
    {
      apply->set_label(gettext("Add books"));
    }
  apply->signal_clicked().connect(std::bind(
      &AddBookGui::modeSelector, this, window, pack_in_arch, add_to_arch));
  grid->attach(*apply, 0, 6, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, 6, 1, 1);

  Glib::PropertyProxy<bool> pack_prop = pack_in_arch->property_active();
  pack_prop.signal_changed().connect(
      [pack_in_arch, add_to_arch, this, lab, apply, window] {
        if(pack_in_arch->get_active())
          {
            add_to_arch->set_visible(true);
            lab->set_visible(true);
            arch_t_dd->set_visible(true);
            apply->set_label(gettext("Archive path"));
          }
        else
          {
            add_to_arch->set_visible(false);
            add_to_arch->set_active(false);
            lab->set_visible(false);
            arch_t_dd->set_visible(false);
            if(directory_add)
              {
                apply->set_label(gettext("Add directories"));
              }
            else
              {
                apply->set_label(gettext("Add books"));
              }
            window->set_default_size(1, 1);
          }
      });

  Glib::PropertyProxy<bool> add_prop = add_to_arch->property_active();
  add_prop.signal_changed().connect(
      [pack_in_arch, add_to_arch, apply, this, lab, window] {
        if(add_to_arch->get_active())
          {
            lab->set_visible(false);
            arch_t_dd->set_visible(false);
            window->set_default_size(1, 1);
            apply->set_label(gettext("Select archive"));
          }
        else
          {
            apply->set_label(gettext("Archive path"));
            if(pack_in_arch->get_active())
              {
                lab->set_visible(true);
                arch_t_dd->set_visible(true);
              }
          }
      });

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);

  window->present();
}

void
AddBookGui::display_sizes()
{
  Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
  Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
  Glib::RefPtr<Gdk::Monitor> monitor = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle rec;
  monitor->get_geometry(rec);
  rec.set_width(rec.get_width() * monitor->get_scale_factor());
  rec.set_height(rec.get_height() * monitor->get_scale_factor());
  width = rec.get_width();
  height = rec.get_height();
}

Glib::RefPtr<Gtk::StringList>
AddBookGui::form_collections_list()
{
  Glib::RefPtr<Gtk::StringList> result
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::filesystem::path col_path = af->homePath();
  col_path /= std::filesystem::u8path(".local")
              / std::filesystem::u8path("share")
              / std::filesystem::u8path("MyLibrary")
              / std::filesystem::u8path("Collections");
  if(std::filesystem::exists(col_path))
    {
      for(auto &dirit : std::filesystem::directory_iterator(col_path))
        {
          std::filesystem::path p = dirit.path();
          if(std::filesystem::is_directory(p))
            {
              result->append(Glib::ustring(p.filename().u8string()));
            }
        }
    }

  return result;
}

Glib::RefPtr<Gtk::StringList>
AddBookGui::form_archive_types_list()
{
  Glib::RefPtr<Gtk::StringList> result
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::vector<std::string> list = af->get_supported_archive_types_packing();
  for(auto it = list.begin(); it != list.end(); it++)
    {
      result->append(Glib::ustring(*it));
    }

  return result;
}

void
AddBookGui::modeSelector(Gtk::Window *win, Gtk::CheckButton *pack_in_arch,
                         Gtk::CheckButton *add_to_arch)
{
  Glib::RefPtr<Gtk::StringObject> item
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection->get_selected_item());

  if(item)
    {
      collection_name = std::string(item->get_string());
      books_path = BaseKeeper::get_books_path(collection_name, af);
      if(pack_in_arch->get_active())
        {
          if(add_to_arch->get_active())
            {
              archive_selection_dialog_add(win);
            }
          else
            {
              archive_selection_dialog_overwrite(win);
            }
        }
      else
        {
          bookSelectionWindow(win, 1);
        }
    }
  else
    {
      books_path.clear();
    }
}

void
AddBookGui::bookSelectionWindow(Gtk::Window *win, const int &variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
  window->set_title(gettext("Book selection"));
  window->set_transient_for(*win);
  window->set_modal(true);
  window->set_name("MLwindow");
  window->set_default_size(width * 0.5, height * 0.4);

  create_action_group(window, variant);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  int row_num = 0;

  switch(variant)
    {
    case 2:
    case 3:
      {
        Gtk::Grid *arch_grid = Gtk::make_managed<Gtk::Grid>();
        arch_grid->set_halign(Gtk::Align::FILL);
        arch_grid->set_valign(Gtk::Align::FILL);
        arch_grid->set_hexpand(true);
        arch_grid->set_vexpand(false);
        grid->attach(*arch_grid, 0, row_num, 2, 1);
        row_num++;

        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_margin(5);
        lab->set_halign(Gtk::Align::START);
        lab->set_use_markup(true);
        lab->set_markup(Glib::ustring("<b>")
                        + gettext("Path of archive in collection:") + "</b>");
        lab->set_name("windowLabel");
        arch_grid->attach(*lab, 0, 0, 1, 1);

        lab = Gtk::make_managed<Gtk::Label>();
        lab->set_margin(5);
        lab->set_halign(Gtk::Align::START);
        lab->set_use_markup(true);
        lab->set_markup(Glib::ustring("<i>") + result_archive_path.u8string()
                        + "</i>");
        lab->set_name("windowLabel");
        arch_grid->attach(*lab, 1, 0, 1, 1);
        break;
      }
    default:
      break;
    }

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_name("windowLabel");
  if(directory_add)
    {
      lab->set_markup(Glib::ustring("<b>") + gettext("Directories") + "</b>");
    }
  else
    {
      lab->set_markup(Glib::ustring("<b>") + gettext("Books") + "</b>");
    }
  grid->attach(*lab, 0, row_num, 2, 1);
  row_num++;

  int scrl_sz = 5;
  Gtk::ScrolledWindow *books_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  books_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                         Gtk::PolicyType::AUTOMATIC);
  books_scrl->set_margin(5);
  books_scrl->set_halign(Gtk::Align::FILL);
  books_scrl->set_valign(Gtk::Align::FILL);
  books_scrl->set_expand(true);
  grid->attach(*books_scrl, 0, row_num, 2, scrl_sz);
  row_num = row_num + scrl_sz;

  books_list = Gio::ListStore<AddBookModelItem>::create();

  Glib::RefPtr<Gtk::SingleSelection> selection
      = Gtk::SingleSelection::create(books_list);

  Glib::RefPtr<Gtk::ColumnViewColumn> col_src = form_sources_column();
  Glib::RefPtr<Gtk::ColumnViewColumn> col_col;
  switch(variant)
    {
    case 1:
      {
        col_col = form_col_path_column();
        break;
      }
    case 2:
    case 3:
      {
        col_col = form_col_arch_path_column();
        break;
      }
    default:
      break;
    }

  Gtk::ColumnView *books = Gtk::make_managed<Gtk::ColumnView>();
  books->set_halign(Gtk::Align::FILL);
  books->set_valign(Gtk::Align::FILL);
  books->set_model(selection);
  books->append_column(col_src);
  books->append_column(col_col);
  books->set_reorderable(false);
  books->set_single_click_activate(true);
  books->signal_activate().connect(
      std::bind(&AddBookGui::slot_select_book, this, std::placeholders::_1));
  books->set_name("tablesView");
  books_scrl->set_child(*books);

  Glib::RefPtr<Gio::Menu> menu = create_menu(variant);

  Gtk::PopoverMenu *pop_menu = Gtk::make_managed<Gtk::PopoverMenu>();
  pop_menu->set_parent(*books);
  pop_menu->set_menu_model(menu);
  books->signal_unrealize().connect([pop_menu] {
    pop_menu->unparent();
  });

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect(std::bind(
      &AddBookGui::show_popup_menu, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, pop_menu, selection));
  books->add_controller(clck);

  Gtk::Button *add_book = Gtk::make_managed<Gtk::Button>();
  add_book->set_margin(5);
  add_book->set_halign(Gtk::Align::CENTER);
  add_book->set_name("operationBut");
  if(directory_add)
    {
      add_book->set_label(gettext("Add directories"));
    }
  else
    {
      add_book->set_label(gettext("Add books"));
    }
  add_book->signal_clicked().connect(
      std::bind(&AddBookGui::book_add_dialog, this, window, variant));
  grid->attach(*add_book, 0, row_num, 1, 1);

  Gtk::MenuButton *books_men_but = Gtk::make_managed<Gtk::MenuButton>();
  books_men_but->set_margin(5);
  books_men_but->set_halign(Gtk::Align::CENTER);
  books_men_but->set_name("menBut");
  if(directory_add)
    {
      books_men_but->set_label(gettext("Directory operations"));
    }
  else
    {
      books_men_but->set_label(gettext("Books operations"));
    }
  books_men_but->set_menu_model(menu);
  grid->attach(*books_men_but, 1, row_num, 1, 1);
  row_num++;

  warn_lab = Gtk::make_managed<Gtk::Label>();
  warn_lab->set_margin(5);
  warn_lab->set_halign(Gtk::Align::CENTER);
  warn_lab->set_wrap(true);
  warn_lab->set_wrap_mode(Pango::WrapMode::WORD);
  warn_lab->set_visible(false);
  warn_lab->set_use_markup(true);
  Glib::PropertyProxy<bool> vis_warn = warn_lab->property_visible();
  vis_warn.signal_changed().connect([this, window] {
    if(!warn_lab->is_visible())
      {
        window->set_default_size(1, 1);
      }
  });
  grid->attach(*warn_lab, 0, row_num, 2, 1);
  row_num++;

  error_lab = Gtk::make_managed<Gtk::Label>();
  error_lab->set_margin(5);
  error_lab->set_halign(Gtk::Align::CENTER);
  error_lab->set_wrap(true);
  error_lab->set_wrap_mode(Pango::WrapMode::WORD);
  error_lab->set_visible(false);
  error_lab->set_use_markup(true);
  Glib::PropertyProxy<bool> vis_err = error_lab->property_visible();
  vis_err.signal_changed().connect([this, window] {
    if(!error_lab->is_visible())
      {
        window->set_default_size(1, 1);
      }
  });
  grid->attach(*error_lab, 0, row_num, 2, 1);
  row_num++;

  add_books_col = Gtk::make_managed<Gtk::Button>();
  add_books_col->set_margin(5);
  add_books_col->set_halign(Gtk::Align::CENTER);
  add_books_col->set_label(gettext("Add"));
  add_books_col->set_sensitive(false);
  add_books_col->signal_clicked().connect([this, variant, win, window] {
    bool conflict = add_books(win, variant);
    if(!conflict)
      {
        window->close();
      }
    else
      {
        error_alert_dialog(window, 1);
      }
  });
  grid->attach(*add_books_col, 0, row_num, 1, 1);

  books_list->signal_items_changed().connect([this](guint, guint, guint) {
    if(books_list->get_n_items() == 0)
      {
        error_lab->set_visible(false);
        name_conflicts.clear();
      }
    if(books_list->get_n_items() > 0 && !error_lab->is_visible()
       && name_conflicts.size() == 0)
      {
        add_books_col->set_sensitive(true);
        add_books_col->set_name("applyBut");
      }
    else
      {
        add_books_col->set_sensitive(false);
        add_books_col->set_name("");
      }
  });

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, row_num, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

void
AddBookGui::book_add_dialog(Gtk::Window *win, const int &variant)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  if(directory_add)
    {
      fd->set_title(gettext("Directories"));
    }
  else
    {
      fd->set_title(gettext("Books"));
    }

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_initial_folder(initial);

  if(!directory_add)
    {
      std::vector<std::string> types = af->get_supported_types();
      Glib::RefPtr<Gtk::FileFilter> default_filter = Gtk::FileFilter::create();
      default_filter->set_name(gettext("All supported"));

      Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> filters
          = Gio::ListStore<Gtk::FileFilter>::create();
      filters->append(default_filter);

      for(auto it = types.begin(); it != types.end(); it++)
        {
          Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
          filter->set_name(*it);
          filter->add_suffix(*it);
          default_filter->add_suffix(*it);
          filters->append(filter);
        }

      fd->set_filters(filters);
      fd->set_default_filter(default_filter);
    }

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

  if(directory_add)
    {
      fd->select_multiple_folders(*win,
                                  std::bind(&AddBookGui::book_add_dialog_slot,
                                            this, std::placeholders::_1, fd,
                                            variant),
                                  cncl);
    }
  else
    {
      fd->open_multiple(*win,
                        std::bind(&AddBookGui::book_add_dialog_slot, this,
                                  std::placeholders::_1, fd, variant),
                        cncl);
    }
#else
  Gtk::FileChooserDialog *fd;
  if(directory_add)
    {
      fd = new Gtk::FileChooserDialog(
          *win, gettext("Directories"),
          Gtk::FileChooserDialog::Action::SELECT_FOLDER, true);
    }
  else
    {
      fd = new Gtk::FileChooserDialog(
          *win, gettext("Books"), Gtk::FileChooserDialog::Action::OPEN, true);

      std::vector<std::string> types = af->get_supported_types();
      Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
      filter->set_name(gettext("All supported"));

      for(auto it = types.begin(); it != types.end(); it++)
        {
          filter->add_suffix(*it);
        }

      fd->add_filter(filter);
      fd->set_filter(filter);

      for(auto it = types.begin(); it != types.end(); it++)
        {
          filter = Gtk::FileFilter::create();
          filter->add_suffix(*it);
          filter->set_name(Glib::ustring("*.") + *it);
          fd->add_filter(filter);
        }
    }

  fd->set_application(parent_window->get_application());
  fd->set_name("MLwindow");

  fd->set_select_multiple(true);
  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_current_folder(initial);

  Gtk::Box *content = fd->get_content_area();
  content->set_hexpand(true);
  content->set_halign(Gtk::Align::FILL);
  content->set_homogeneous(true);

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_hexpand(true);
  but->set_halign(Gtk::Align::START);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_hexpand(true);
  but->set_halign(Gtk::Align::END);
  but->set_name("applyBut");

  fd->signal_response().connect(std::bind(&AddBookGui::book_add_dialog_slot,
                                          this, std::placeholders::_1, fd,
                                          variant));

  fd->signal_close_request().connect(
      [fd] {
        std::shared_ptr<Gtk::FileChooserDialog> fdl(fd);
        fdl->close();
        return true;
      },
      false);

  fd->present();
#endif
}

#ifndef ML_GTK_OLD
void
AddBookGui::book_add_dialog_slot(const Glib::RefPtr<Gio::AsyncResult> &result,
                                 const Glib::RefPtr<Gtk::FileDialog> &fd,
                                 const int &variant)
{
  std::vector<Glib::RefPtr<Gio::File>> files;
  try
    {
      if(directory_add)
        {
          files = fd->select_multiple_folders_finish(result);
        }
      else
        {
          files = fd->open_multiple_finish(result);
        }
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout << "AddBookGui::book_add_dialog_slot error: " << er.what()
                    << std::endl;
        }
    }
  form_books_list(files, variant);
}
#endif

void
AddBookGui::form_books_list(const std::vector<Glib::RefPtr<Gio::File>> &files,
                            const int &variant)
{
  bool warn_hide = true;
  bool error_hide = true;
  for(auto it = files.rbegin(); it != files.rend(); it++)
    {
      Glib::RefPtr<Gio::File> fl = *it;
      Glib::RefPtr<AddBookModelItem> item
          = AddBookModelItem::create(fl->get_path());
      bool append = true;
      for(guint i = 0; i < books_list->get_n_items(); i++)
        {
          if(books_list->get_item(i)->source_path == item->source_path)
            {
              append = false;
              break;
            }
        }
      if(append)
        {
          switch(variant)
            {
            case 1:
              {
                form_colletion_path_not_arch(item);
                break;
              }
            case 2:
              {
                form_colletion_path_arch_overwrite(item);
                break;
              }
            case 3:
              {
                form_colletion_path_arch_add(item);
                break;
              }
            default:
              {
                return void();
              }
            }
          if(!item->correct)
            {
              warn_hide = false;
            }
          if(item->out_of_col)
            {
              error_hide = false;
            }
          books_list->append(item);
        }
    }
  if(warn_hide)
    {
      warn_lab->set_visible(false);
    }
  if(error_hide)
    {
      error_lab->set_visible(false);
    }
}

Glib::RefPtr<Gtk::ColumnViewColumn>
AddBookGui::form_sources_column()
{
  Glib::RefPtr<Gtk::ColumnViewColumn> result
      = Gtk::ColumnViewColumn::create(gettext("Sources"));

  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&AddBookGui::slot_setup, this, std::placeholders::_1, 1));
  factory->signal_bind().connect(
      std::bind(&AddBookGui::slot_bind, this, std::placeholders::_1, 1));

  result->set_factory(factory);

  result->set_expand(true);
  result->set_resizable(true);

  return result;
}

void
AddBookGui::slot_setup(const Glib::RefPtr<Gtk::ListItem> &list_item,
                       const int &variant)
{
  switch(variant)
    {
    case 1:
    case 2:
    case 3:
      {
        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_halign(Gtk::Align::FILL);
        lab->set_expand(true);
        lab->set_ellipsize(Pango::EllipsizeMode::START);
        list_item->set_child(*lab);
        break;
      }
    default:
      break;
    }
}

void
AddBookGui::slot_bind(const Glib::RefPtr<Gtk::ListItem> &list_item,
                      const int &variant)
{
  Gtk::Label *lab = dynamic_cast<Gtk::Label *>(list_item->get_child());
  Glib::RefPtr<AddBookModelItem> item
      = std::dynamic_pointer_cast<AddBookModelItem>(list_item->get_item());
  if(item)
    {
      switch(variant)
        {
        case 1:
          {
            lab->set_text(Glib::ustring(item->source_path));
            if(item == selected_book)
              {
                lab->set_name("selectedLab");
              }
            else
              {
                lab->set_name("windowLabel");
              }
            break;
          }
        case 2:
        case 3:
          {
            lab->set_text(item->collection_path);
            if(item->out_of_col || item->conflict_names)
              {
                lab->set_name("badLabel");
              }
            else
              {
                if(item->correct)
                  {
                    lab->set_name("windowLabel");
                  }
                else
                  {
                    lab->set_name("yellowLab");
                  }
              }
            break;
          }
        default:
          break;
        }
    }
}

void
AddBookGui::slot_select_book(guint pos)
{
  if(pos != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<AddBookModelItem> item = selected_book;

      selected_book = books_list->get_item(pos);
      books_list->remove(pos);
      books_list->insert(pos, selected_book);

      for(guint i = 0; i < books_list->get_n_items(); i++)
        {
          if(books_list->get_item(i) == item)
            {
              books_list->remove(i);
              books_list->insert(i, item);
              break;
            }
        }
    }
}

Glib::RefPtr<Gtk::ColumnViewColumn>
AddBookGui::form_col_path_column()
{
  Glib::RefPtr<Gtk::ColumnViewColumn> result
      = Gtk::ColumnViewColumn::create(gettext("Collection paths"));

  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&AddBookGui::slot_setup, this, std::placeholders::_1, 2));
  factory->signal_bind().connect(
      std::bind(&AddBookGui::slot_bind, this, std::placeholders::_1, 2));

  result->set_factory(factory);

  result->set_expand(true);
  result->set_resizable(true);

  return result;
}

void
AddBookGui::form_colletion_path_not_arch(
    const Glib::RefPtr<AddBookModelItem> &item)
{
  std::filesystem::path source = std::filesystem::u8path(item->source_path);
  std::filesystem::path col_path = books_path;
  col_path /= source.filename();
  item->collection_path = col_path.u8string();

  check_book_path_not_arch(item);
}

void
AddBookGui::check_book_path_not_arch(
    const Glib::RefPtr<AddBookModelItem> &item)
{
  std::filesystem::path col_path
      = std::filesystem::u8path(item->collection_path);
  std::string ch_str = col_path.lexically_proximate(books_path).u8string();
  std::string::size_type n = ch_str.find("../");
  if(n == std::string::npos)
    {
      n = ch_str.find("..\\");
    }
  if(n != std::string::npos)
    {
      item->out_of_col = true;
      error_lab->set_markup(
          Glib::ustring("<b>")
          + gettext("Warning! Some result paths are out of collection!")
          + "</b>");
      error_lab->set_name("badLabel");
      error_lab->set_visible(true);
      item->correct = false;
    }
  else
    {
      item->out_of_col = false;
    }

  if(std::filesystem::exists(col_path))
    {
      if(!item->out_of_col)
        {
          item->correct = false;
          if(directory_add)
            {
              warn_lab->set_markup(
                  Glib::ustring("<b>")
                  + gettext("Warning! Some result directories exist and will "
                            "be overwritten!")
                  + "</b>");
            }
          else
            {
              warn_lab->set_markup(Glib::ustring("<b>")
                                   + gettext("Warning! Some result files "
                                             "exist and will be overwritten!")
                                   + "</b>");
            }
          warn_lab->set_name("yellowLab");

          warn_lab->set_visible(true);
        }
      else
        {
          item->correct = true;
        }
    }
  else
    {
      item->correct = true;
    }
  check_conflict_names(item);
}

void
AddBookGui::create_action_group(Gtk::Window *win, const int &variant)
{
  Glib::RefPtr<Gio::SimpleActionGroup> ac_group
      = Gio::SimpleActionGroup::create();

  ac_group->add_action("remove_book",
                       std::bind(&AddBookGui::action_remove_book, this));
  switch(variant)
    {
    case 1:
      {
        ac_group->add_action(
            "edit_book",
            std::bind(&AddBookGui::action_change_path_notarch, this, win));
        break;
      }
    case 2:
    case 3:
      {
        ac_group->add_action("edit_book",
                             std::bind(&AddBookGui::action_change_path_arch,
                                       this, win, variant));
        break;
      }
    default:
      break;
    }

  win->insert_action_group("entries_ops", ac_group);
}

void
AddBookGui::action_remove_book()
{
  for(guint i = 0; i < books_list->get_n_items(); i++)
    {
      if(books_list->get_item(i) == selected_book)
        {
          books_list->remove(i);
          selected_book.reset();
          break;
        }
    }
}

Glib::RefPtr<Gio::Menu>
AddBookGui::create_menu(const int &variant)
{
  Glib::RefPtr<Gio::Menu> result = Gio::Menu::create();

  Glib::RefPtr<Gio::MenuItem> item;
  if(directory_add)
    {
      item = Gio::MenuItem::create(gettext("Remove directory"),
                                   "entries_ops.remove_book");
    }
  else
    {
      item = Gio::MenuItem::create(gettext("Remove book"),
                                   "entries_ops.remove_book");
    }
  result->append_item(item);

  switch(variant)
    {
    case 1:
      {
        item = Gio::MenuItem::create(gettext("Change collection path"),
                                     "entries_ops.edit_book");
        result->append_item(item);
        break;
      }
    case 2:
    case 3:
      {
        item = Gio::MenuItem::create(gettext("Change archive path"),
                                     "entries_ops.edit_book");
        result->append_item(item);
        break;
      }
    default:
      break;
    }

  return result;
}

void
AddBookGui::action_change_path_notarch(Gtk::Window *win)
{
  if(selected_book)
    {
#ifndef ML_GTK_OLD
      Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
      fd->set_modal(true);
      fd->set_title(gettext("Collection path"));

      Glib::RefPtr<Gio::File> initial
          = Gio::File::create_for_path(books_path.u8string());
      fd->set_initial_folder(initial);

      std::filesystem::path source
          = std::filesystem::u8path(selected_book->source_path);

      Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
      filter->add_pattern(Glib::ustring("*") + source.extension().u8string());
      fd->set_default_filter(filter);

      Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> filters_list
          = Gio::ListStore<Gtk::FileFilter>::create();
      filters_list->append(filter);
      fd->set_filters(filters_list);

      fd->set_initial_name(source.filename().u8string());

      Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

      fd->save(*win,
               std::bind(&AddBookGui::action_chage_path_notarch_slot, this,
                         std::placeholders::_1, fd),
               cncl);
#else
      Gtk::FileChooserDialog *fd
          = new Gtk::FileChooserDialog(*win, gettext("Collection path"),
                                       Gtk::FileChooser::Action::SAVE, true);
      fd->set_application(win->get_application());
      fd->set_modal(true);
      fd->set_name("MLwindow");

      Gtk::Button *but
          = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
      but->set_margin(5);
      but->set_name("cancelBut");

      but = fd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);
      but->set_margin(5);
      but->set_name("applyBut");

      Glib::RefPtr<Gio::File> initial
          = Gio::File::create_for_path(books_path.u8string());

      fd->set_current_folder(initial);

      std::filesystem::path source
          = std::filesystem::u8path(selected_book->source_path);

      Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
      filter->add_pattern(Glib::ustring("*") + source.extension().u8string());
      fd->set_filter(filter);

      fd->set_current_name(Glib::ustring(source.filename().u8string()));

      fd->signal_response().connect(
          std::bind(&AddBookGui::action_chage_path_notarch_slot, this,
                    std::placeholders::_1, fd));

      fd->signal_close_request().connect(
          [fd] {
            std::shared_ptr<Gtk::FileChooserDialog> fdl(fd);
            fdl->set_visible(false);
            return true;
          },
          false);

      fd->present();
#endif
    }
}

#ifndef ML_GTK_OLD
void
AddBookGui::action_chage_path_notarch_slot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->save_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout << "AddBookGui::action_chage_path_notarch_slot error: "
                    << er.what() << std::endl;
        }
    }
  if(fl)
    {
      selected_book->collection_path = fl->get_path();
      check_book_path_not_arch(selected_book);
      bool warn_hide = true;
      bool error_hide = true;

      for(guint i = 0; i < books_list->get_n_items(); i++)
        {
          Glib::RefPtr<AddBookModelItem> it = books_list->get_item(i);
          if(it == selected_book)
            {
              books_list->remove(i);
              books_list->insert(i, it);
            }
          if(!it->correct)
            {
              warn_hide = false;
            }
          if(it->out_of_col)
            {
              error_hide = false;
            }
        }
      if(warn_hide)
        {
          warn_lab->set_visible(false);
        }
      if(error_hide)
        {
          error_lab->set_visible(false);
        }
    }
}
#endif

void
AddBookGui::show_popup_menu(
    int, double x, double y, Gtk::PopoverMenu *menu,
    const Glib::RefPtr<Gtk::SingleSelection> &selection)
{
  Glib::RefPtr<AddBookModelItem> item = selected_book;
  selected_book = std::dynamic_pointer_cast<AddBookModelItem>(
      selection->get_selected_item());
  for(guint i = 0; i < books_list->get_n_items(); i++)
    {
      Glib::RefPtr<AddBookModelItem> it = books_list->get_item(i);
      if(it == item || it == selected_book)
        {
          books_list->remove(i);
          books_list->insert(i, it);
        }
    }
  Gdk::Rectangle rec(static_cast<int>(x), static_cast<int>(y), 1, 1);
  menu->set_pointing_to(rec);
  menu->popup();
}

bool
AddBookGui::add_books(Gtk::Window *win, const int &variant)
{
  bool conflict = false;
  guint size = books_list->get_n_items();
  for(guint i = 0; i < size; i++)
    {
      Glib::RefPtr<AddBookModelItem> item = books_list->get_item(i);
      std::vector<Glib::RefPtr<AddBookModelItem>> conf_v;
      conf_v.push_back(item);
      if(i + 1 < size)
        {
          for(guint j = i + 1; j < size; j++)
            {
              Glib::RefPtr<AddBookModelItem> it = books_list->get_item(j);
              if(it->collection_path == item->collection_path)
                {
                  conflict = true;
                  item->conflict_names = true;
                  it->conflict_names = true;
                  books_list->remove(j);
                  books_list->insert(j, it);
                  books_list->remove(i);
                  books_list->insert(i, item);
                  conf_v.emplace_back(it);
                }
            }
        }
      if(conf_v.size() > 1)
        {
          name_conflicts.emplace_back(conf_v);
        }
    }

  if(!conflict)
    {
      std::vector<Glib::RefPtr<AddBookModelItem>> conf_v;
      std::string sstr;
      for(guint i = 0; i < size; i++)
        {
          Glib::RefPtr<AddBookModelItem> item = books_list->get_item(i);
          sstr = item->collection_path;
          auto itarch = std::find(archive_filenames.begin(),
                                  archive_filenames.end(), sstr);
          if(itarch != archive_filenames.end())
            {
              conf_v.push_back(item);
            }
          else if(directory_add)
            {
              sstr = sstr + "/";
              itarch = std::find(archive_filenames.begin(),
                                 archive_filenames.end(), sstr);
              if(itarch != archive_filenames.end())
                {
                  item->conflict_names = true;
                  books_list->remove(i);
                  books_list->insert(i, item);
                  conf_v.push_back(item);
                }
            }
        }
      if(conf_v.size() > 0)
        {
          conflict = true;
          name_conflicts.emplace_back(conf_v);
        }
    }

  if(conflict)
    {
      add_books_col->set_sensitive(false);
      add_books_col->set_name("");
      return conflict;
    }
  Glib::RefPtr<Gtk::StringObject> item
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection->get_selected_item());
  if(item)
    {
      add_books_window(win);

      std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
          result;
      for(guint i = 0; i < books_list->get_n_items(); i++)
        {
          Glib::RefPtr<AddBookModelItem> it = books_list->get_item(i);
          std::tuple<std::filesystem::path, std::filesystem::path> ttup;
          std::get<0>(ttup) = std::filesystem::u8path(it->source_path);
          std::get<1>(ttup) = std::filesystem::u8path(it->collection_path);
          result.emplace_back(ttup);
        }

      switch(variant)
        {
        case 1:
          {
#ifndef USE_OPENMP
            std::thread thr([this, result] {
              AddBook ab(af, collection_name, remove_sources, bookmarks);
              try
                {
                  if(directory_add)
                    {
                      ab.simple_add_dir(result);
                    }
                  else
                    {
                      ab.simple_add(result);
                    }
                  finish_add_disp->emit();
                }
              catch(MLException &er)
                {
                  std::cout << er.what() << std::endl;
                  finish_add_err_disp->emit();
                }
            });
            thr.detach();
#else
#pragma omp masked
            {
              omp_event_handle_t event;
#pragma omp task detach(event)
              {
                AddBook ab(af, collection_name, remove_sources, bookmarks);
                try
                  {
                    if(directory_add)
                      {
                        ab.simple_add_dir(result);
                      }
                    else
                      {
                        ab.simple_add(result);
                      }
                    finish_add_disp->emit();
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    finish_add_err_disp->emit();
                  }
                omp_fulfill_event(event);
              }
            }
#endif
            break;
          }
        case 2:
          {
#ifndef USE_OPENMP
            std::thread thr([this, result] {
              AddBook ab(af, collection_name, remove_sources, bookmarks);
              try
                {
                  if(directory_add)
                    {
                      ab.overwrite_archive_dir(result_archive_path, result);
                    }
                  else
                    {
                      ab.overwrite_archive(result_archive_path, result);
                    }
                  finish_add_disp->emit();
                }
              catch(MLException &er)
                {
                  std::cout << er.what() << std::endl;
                  finish_add_err_disp->emit();
                }
            });
            thr.detach();
#else
#pragma omp masked
            {
              omp_event_handle_t event;
#pragma omp task detach(event)
              {
                AddBook ab(af, collection_name, remove_sources, bookmarks);
                try
                  {
                    if(directory_add)
                      {
                        ab.overwrite_archive_dir(result_archive_path, result);
                      }
                    else
                      {
                        ab.overwrite_archive(result_archive_path, result);
                      }
                    finish_add_disp->emit();
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    finish_add_err_disp->emit();
                  }
                omp_fulfill_event(event);
              }
            }
#endif
            break;
          }
        case 3:
          {
#ifndef USE_OPENMP
            std::thread thr([this, result] {
              AddBook ab(af, collection_name, remove_sources, bookmarks);
              try
                {
                  if(directory_add)
                    {
                      ab.add_to_existing_archive_dir(result_archive_path,
                                                     result);
                    }
                  else
                    {
                      ab.add_to_existing_archive(result_archive_path, result);
                    }
                  finish_add_disp->emit();
                }
              catch(MLException &er)
                {
                  std::cout << er.what() << std::endl;
                  finish_add_err_disp->emit();
                }
            });
            thr.detach();
#else
#pragma omp masked
            {
              omp_event_handle_t event;
#pragma omp task detach(event)
              {
                AddBook ab(af, collection_name, remove_sources, bookmarks);
                try
                  {
                    if(directory_add)
                      {
                        ab.add_to_existing_archive_dir(result_archive_path,
                                                       result);
                      }
                    else
                      {
                        ab.add_to_existing_archive(result_archive_path,
                                                   result);
                      }
                    finish_add_disp->emit();
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    finish_add_err_disp->emit();
                  }
                omp_fulfill_event(event);
              }
            }
#endif
            break;
          }
        default:
          break;
        }
    }
  return conflict;
}

void
AddBookGui::add_books_window(Gtk::Window *win)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(false);

  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(20);
  lab->set_halign(Gtk::Align::FILL);
  lab->set_name("windowLabel");
  if(directory_add)
    {
      lab->set_text(gettext("Directories adding in progress..."));
    }
  else
    {
      lab->set_text(gettext("Books adding in progress..."));
    }
  win->set_child(*lab);

  finish_add_disp = std::make_shared<Glib::Dispatcher>();
  finish_add_err_disp = std::make_shared<Glib::Dispatcher>();
  finish_add_disp->connect(std::bind(&AddBookGui::finish, this, win, 1));
  finish_add_err_disp->connect(std::bind(&AddBookGui::finish, this, win, 2));
}

void
AddBookGui::finish(Gtk::Window *win, const int &variant)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  win->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  lab->set_name("windowLabel");
  switch(variant)
    {
    case 1:
      {
        if(directory_add)
          {
            lab->set_text(gettext("Directories were successfully added!"));
          }
        else
          {
            lab->set_text(gettext("Books were successfully added!"));
          }
        break;
      }
    case 2:
      {
        lab->set_text(gettext("Error! See system log for details."));
        break;
      }
    default:
      break;
    }

  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
  grid->attach(*close, 0, 1, 1, 1);

  if(books_added && variant == 1)
    {
      books_added(collection_name);
    }
}

void
AddBookGui::error_alert_dialog(Gtk::Window *win, const int &variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
  window->set_title(gettext("Error!"));
  window->set_transient_for(*win);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  lab->set_name("windowLabel");
  switch(variant)
    {
    case 1:
      {
        lab->set_text(gettext("Error: name conflict!"));
        break;
      }
    case 2:
      {
        lab->set_text(
            gettext("Error: result archive path is out of collection"));
        break;
      }
    case 3:
      {
        lab->set_text(gettext("Error: path cannot be empty"));
        break;
      }
    case 4:
      {
        lab->set_text(gettext("Error: files or directories adding to fbd "
                              "archives is prohibited"));
        break;
      }
    default:
      break;
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*close, 0, 1, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

void
AddBookGui::check_conflict_names(const Glib::RefPtr<AddBookModelItem> &item)
{
  std::vector<Glib::RefPtr<AddBookModelItem>> recursive_check;
  int conf_count = 0;
  for(auto it = name_conflicts.begin(); it != name_conflicts.end();)
    {
      std::vector<Glib::RefPtr<AddBookModelItem>> ch_v = *it;
      bool remove = false;
      for(auto itch = ch_v.begin(); itch != ch_v.end();)
        {
          if(item == *itch)
            {
              auto it_conf = std::find_if(
                  ch_v.begin(), ch_v.end(),
                  [item](const Glib::RefPtr<AddBookModelItem> &el) {
                    if(el == item)
                      {
                        return false;
                      }
                    else
                      {
                        if(el->collection_path == item->collection_path)
                          {
                            return true;
                          }
                        else
                          {
                            return false;
                          }
                      }
                  });
              if(it_conf != ch_v.end())
                {
                  itch++;
                  conf_count++;
                }
              else
                {
                  ch_v.erase(itch);
                  *it = ch_v;
                  if(ch_v.size() <= 1)
                    {
                      remove = true;
                      if(ch_v.size() == 1)
                        {
                          Glib::RefPtr<AddBookModelItem> loc = ch_v[0];
                          auto itrch = std::find_if(
                              recursive_check.begin(), recursive_check.end(),
                              [loc](Glib::RefPtr<AddBookModelItem> &el) {
                                return el == loc;
                              });
                          if(itrch == recursive_check.end())
                            {
                              recursive_check.emplace_back(loc);
                            }
                        }
                    }
                }
            }
          else
            {
              itch++;
            }
        }
      if(remove)
        {
          name_conflicts.erase(it);
        }
      else
        {
          it++;
        }
    }
  if(conf_count == 0)
    {
      item->conflict_names = false;
      for(guint i = 0; i < books_list->get_n_items(); i++)
        {
          if(books_list->get_item(i) == item)
            {
              books_list->remove(i);
              books_list->insert(i, item);
            }
        }
    }

  for(auto it = recursive_check.begin(); it != recursive_check.end(); it++)
    {
      check_conflict_names(*it);
    }

  if(name_conflicts.size() == 0 && !error_lab->get_visible())
    {
      add_books_col->set_sensitive(true);
      add_books_col->set_name("applyBut");
    }
}

void
AddBookGui::archive_selection_dialog_overwrite(Gtk::Window *win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  fd->set_title(gettext("Archive name"));

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(books_path.u8string());

  fd->set_initial_folder(initial);

  Glib::RefPtr<Gtk::StringObject> arch_type
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          arch_t_dd->get_selected_item());
  if(!arch_type)
    {
      std::cout << "AddBookGui::archive_selection_dialog_overwrite fatal error"
                << std::endl;
      return void();
    }

  Glib::RefPtr<Gtk::FileFilter> default_filter = Gtk::FileFilter::create();

  Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> filters_list
      = Gio::ListStore<Gtk::FileFilter>::create();
  filters_list->append(default_filter);

  default_filter->set_name(arch_type->get_string());
  default_filter->add_suffix(arch_type->get_string());

  fd->set_filters(filters_list);
  fd->set_default_filter(default_filter);

  fd->set_initial_name(std::string("Archive.") + arch_type->get_string());

  fd->save(*win,
           std::bind(&AddBookGui::archive_selection_dialog_overwrite_slot,
                     this, std::placeholders::_1, fd, win));
#else
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *win, gettext("Archive name"), Gtk::FileChooser::Action::SAVE, true);
  fd->set_application(win->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(books_path.u8string());
  fd->set_current_folder(initial);

  Glib::RefPtr<Gtk::StringObject> arch_type
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          arch_t_dd->get_selected_item());
  if(!arch_type)
    {
      std::cout << "AddBookGui::archive_selection_dialog_overwrite fatal error"
                << std::endl;
      return void();
    }
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->add_suffix(arch_type->get_string());

  fd->set_filter(filter);

  fd->set_current_name(Glib::ustring("Archive.") + arch_type->get_string());

  fd->signal_response().connect(
      std::bind(&AddBookGui::archive_selection_dialog_overwrite_slot, this,
                std::placeholders::_1, fd, win));

  fd->signal_close_request().connect(
      [fd] {
        std::shared_ptr<Gtk::FileChooserDialog> fdl(fd);
        fdl->set_visible(false);
        return true;
      },
      false);

  fd->present();
#endif
}

void
AddBookGui::form_colletion_path_arch_overwrite(
    const Glib::RefPtr<AddBookModelItem> &item)
{
  item->collection_path
      = std::filesystem::u8path(item->source_path).filename().u8string();
}

Glib::RefPtr<Gtk::ColumnViewColumn>
AddBookGui::form_col_arch_path_column()
{
  Glib::RefPtr<Gtk::ColumnViewColumn> result
      = Gtk::ColumnViewColumn::create(gettext("Paths in archive"));

  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&AddBookGui::slot_setup, this, std::placeholders::_1, 3));
  factory->signal_bind().connect(
      std::bind(&AddBookGui::slot_bind, this, std::placeholders::_1, 3));

  result->set_factory(factory);

  result->set_expand(true);
  result->set_resizable(true);

  return result;
}

void
AddBookGui::action_change_path_arch(Gtk::Window *win, const int &variant)
{
  if(selected_book)
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(win->get_application());
      window->set_title(gettext("Path in archive"));
      window->set_transient_for(*win);
      window->set_modal(true);
      window->set_name("MLwindow");

      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
      grid->set_halign(Gtk::Align::FILL);
      grid->set_valign(Gtk::Align::FILL);
      grid->set_expand(true);
      window->set_child(*grid);

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_text(gettext("Path in archive:"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, 0, 2, 1);

      Gtk::Entry *path = Gtk::make_managed<Gtk::Entry>();
      path->set_margin(5);
      path->set_halign(Gtk::Align::START);
      path->set_width_chars(50);
      path->set_alignment(Gtk::Align::START);
      path->set_text(selected_book->collection_path);
      path->set_name("windowEntry");
      grid->attach(*path, 0, 1, 2, 1);

      Gtk::Button *apply = Gtk::make_managed<Gtk::Button>();
      apply->set_margin(5);
      apply->set_halign(Gtk::Align::CENTER);
      apply->set_name("applyBut");
      apply->set_label(gettext("Apply"));
      apply->signal_clicked().connect(
          std::bind(&AddBookGui::action_chage_path_arch_slot, this, path,
                    window, variant));
      grid->attach(*apply, 0, 2, 1, 1);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
      cancel->set_margin(5);
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_name("cancelBut");
      cancel->set_label(gettext("Cancel"));
      cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
      grid->attach(*cancel, 1, 2, 1, 1);

      window->signal_close_request().connect(
          [window] {
            std::unique_ptr<Gtk::Window> win(window);
            win->set_visible(false);
            return true;
          },
          false);

      window->present();
    }
}

void
AddBookGui::action_chage_path_arch_slot(Gtk::Entry *path, Gtk::Window *win,
                                        const int &variant)
{
  std::string p(path->get_text());
  if(p.empty())
    {
      error_alert_dialog(win, 3);
    }
  else
    {
      std::string::size_type n = 0;
      std::string sstr = "\\";
      for(;;)
        {
          n = p.find(sstr, n);
          if(n != std::string::npos)
            {
              p.erase(n, sstr.size());
              p.insert(n, "/");
            }
          else
            {
              break;
            }
        }
      selected_book->collection_path = p;

      if(variant == 3)
        {
          auto it
              = std::find(archive_filenames.begin(), archive_filenames.end(),
                          selected_book->collection_path);
          if(it != archive_filenames.end())
            {
              selected_book->out_of_col = true;
              error_lab->set_visible(true);
            }
          else
            {
              selected_book->out_of_col = false;
              bool error_hide = true;
              for(guint i = 0; i < books_list->get_n_items(); i++)
                {
                  Glib::RefPtr<AddBookModelItem> it = books_list->get_item(i);
                  if(it->out_of_col)
                    {
                      error_hide = false;
                      break;
                    }
                }
              if(error_hide)
                {
                  error_lab->set_visible(false);
                }
            }
        }

      for(guint i = 0; i < books_list->get_n_items(); i++)
        {
          Glib::RefPtr<AddBookModelItem> it = books_list->get_item(i);
          if(it == selected_book)
            {
              books_list->remove(i);
              books_list->insert(i, it);
            }
          check_conflict_names(it);
        }
      win->close();
    }
}

void
AddBookGui::archive_selection_dialog_add(Gtk::Window *win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  fd->set_title(gettext("Archive name"));

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(books_path.u8string());

  fd->set_initial_folder(initial);

  std::vector<std::string> types = af->get_supported_archive_types_packing();

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(gettext("Supported archives"));
  Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> filters_list
      = Gio::ListStore<Gtk::FileFilter>::create();
  filters_list->append(filter);
  fd->set_filters(filters_list);

  for(auto it = types.begin(); it != types.end(); it++)
    {
      filter->add_suffix(*it);
    }

  fd->set_default_filter(filter);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

  fd->open(*win,
           std::bind(&AddBookGui::archive_selection_dialog_add_slot, this,
                     std::placeholders::_1, fd, win),
           cncl);
#else
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *win, gettext("Archive name"), Gtk::FileChooser::Action::OPEN, true);
  fd->set_application(win->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(gettext("Supported archives"));

  std::vector<std::string> types = af->get_supported_archive_types_packing();
  for(auto it = types.begin(); it != types.end(); it++)
    {
      filter->add_suffix(*it);
    }

  fd->set_filter(filter);

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(books_path.u8string());
  fd->set_current_folder(initial);

  fd->signal_response().connect(
      std::bind(&AddBookGui::archive_selection_dialog_add_slot, this,
                std::placeholders::_1, fd, win));

  fd->signal_close_request().connect(
      [fd] {
        std::shared_ptr<Gtk::FileChooserDialog> fdl(fd);
        fdl->set_visible(false);
        return true;
      },
      false);

  fd->present();
#endif
}

#ifndef ML_GTK_OLD
void
AddBookGui::archive_selection_dialog_overwrite_slot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->save_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout
              << "AddBookGui::archive_selection_dialog_overwrite_slot error: "
              << er.code() << std::endl;
        }
    }
  if(fl)
    {
      std::filesystem::path p = std::filesystem::u8path(fl->get_path());
      std::string ch_str = p.lexically_proximate(books_path).u8string();
      std::string::size_type n = ch_str.find("../");
      if(n == std::string::npos)
        {
          n = ch_str.find("..\\");
        }

      if(n != std::string::npos)
        {
          error_alert_dialog(win, 2);
        }
      else
        {
          result_archive_path = p;
          bookSelectionWindow(win, 2);
        }
    }
}

void
AddBookGui::archive_selection_dialog_add_slot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->open_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::FAILED)
        {
          std::cout << "AddBookGui::archive_selection_dialog_add_slot error: "
                    << er.code() << std::endl;
        }
    }
  if(fl)
    {
      std::filesystem::path p = std::filesystem::u8path(fl->get_path());
      if(std::filesystem::exists(p))
        {
          std::vector<std::string> fl_list = AddBook::archive_filenames(p, af);
          std::string find_str(".fbd");
          auto it = std::find_if(fl_list.begin(), fl_list.end(),
                                 [find_str](const std::string &el) {
                                   if(el.size() > find_str.size())
                                     {
                                       std::string::size_type n
                                           = el.rfind(find_str);
                                       if(n == el.size() - find_str.size())
                                         {
                                           return true;
                                         }
                                     }
                                   return false;
                                 });
          if(it != fl_list.end())
            {
              error_alert_dialog(win, 4);
              return void();
            }
        }
      std::string ch_str = p.lexically_proximate(books_path).u8string();
      std::string::size_type n = ch_str.find("../");
      if(n == std::string::npos)
        {
          n = ch_str.find("..\\");
        }

      if(n != std::string::npos)
        {
          error_alert_dialog(win, 2);
        }
      else
        {
          result_archive_path = p;

          Gtk::Window *w_win = wait_window(win);

          finish_wait_disp = std::make_shared<Glib::Dispatcher>();

          finish_wait_disp->connect([win, w_win, this] {
            w_win->close();
            bookSelectionWindow(win, 3);
          });

#ifndef USE_OPENMP
          std::thread thr([this, p] {
            archive_filenames = AddBook::archive_filenames(p, af);
            finish_wait_disp->emit();
          });
          thr.detach();
#else
#pragma omp masked
          {
            omp_event_handle_t event;
#pragma omp task detach(event)
            {
              archive_filenames = AddBook::archive_filenames(p, af);
              finish_wait_disp->emit();
              omp_fulfill_event(event);
            }
          }
#endif
        }
    }
}
#endif

void
AddBookGui::form_colletion_path_arch_add(
    const Glib::RefPtr<AddBookModelItem> &item)
{
  item->collection_path
      = std::filesystem::u8path(item->source_path).filename().u8string();
  auto it = std::find(archive_filenames.begin(), archive_filenames.end(),
                      item->collection_path);
  if(it != archive_filenames.end())
    {
      item->out_of_col = true;
      error_lab->set_visible(true);
      error_lab->set_markup(
          Glib::ustring("<b>")
          + gettext("Warning! Some files are already in archive!") + "</b>");
      error_lab->set_name("badLabel");
    }
}

#ifdef ML_GTK_OLD
void
AddBookGui::book_add_dialog_slot(int resp, Gtk::FileChooserDialog *fd,
                                 const int &variant)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      GListModel *gmodel
          = gtk_file_chooser_get_files(((Gtk::FileChooser *)fd)->gobj());
      guint n = g_list_model_get_n_items(gmodel);

      std::vector<Glib::RefPtr<Gio::File>> files;
      for(guint i = 0; i < n; i++)
        {
          GObject *gobject_i = g_list_model_get_object(gmodel, i);
          GFile *gfile = (GFile *)gobject_i;
          Glib::RefPtr<Gio::File> fl = Glib::wrap(gfile, true);
          if(fl)
            {
              files.emplace_back(fl);
            }
        }
      form_books_list(files, variant);
    }
  fd->close();
}

void
AddBookGui::action_chage_path_notarch_slot(int resp,
                                           Gtk::FileChooserDialog *fd)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          selected_book->collection_path = fl->get_path();
          check_book_path_not_arch(selected_book);
          bool warn_hide = true;
          bool error_hide = true;
          for(guint i = 0; i < books_list->get_n_items(); i++)
            {
              Glib::RefPtr<AddBookModelItem> it = books_list->get_item(i);
              if(it == selected_book)
                {
                  books_list->insert(i, it);
                  books_list->remove(i);
                }
              if(!it->correct)
                {
                  warn_hide = false;
                }
              if(it->out_of_col)
                {
                  error_hide = false;
                }
            }
          if(warn_hide)
            {
              warn_lab->set_visible(false);
            }
          if(error_hide)
            {
              error_lab->set_visible(false);
            }
        }
    }
  fd->close();
}

void
AddBookGui::archive_selection_dialog_overwrite_slot(int resp,
                                                    Gtk::FileChooserDialog *fd,
                                                    Gtk::Window *win)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          std::filesystem::path p = std::filesystem::u8path(fl->get_path());
          std::string ch_str = p.lexically_proximate(books_path).u8string();
          std::string::size_type n = ch_str.find("../");
          if(n == std::string::npos)
            {
              n = ch_str.find("..\\");
            }

          if(n != std::string::npos)
            {
              error_alert_dialog(win, 2);
            }
          else
            {
              result_archive_path = p;
              bookSelectionWindow(win, 2);
            }
        }
    }
  fd->close();
}

void
AddBookGui::archive_selection_dialog_add_slot(int resp,
                                              Gtk::FileChooserDialog *fd,
                                              Gtk::Window *win)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();

      if(fl)
        {
          std::filesystem::path p = std::filesystem::u8path(fl->get_path());
          if(std::filesystem::exists(p))
            {
              std::vector<std::string> fl_list
                  = AddBook::archive_filenames(p, af);
              std::string find_str(".fbd");
              auto it = std::find_if(fl_list.begin(), fl_list.end(),
                                     [find_str](const std::string &el) {
                                       if(el.size() > find_str.size())
                                         {
                                           std::string::size_type n
                                               = el.rfind(find_str);
                                           if(n == el.size() - find_str.size())
                                             {
                                               return true;
                                             }
                                         }
                                       return false;
                                     });
              if(it != fl_list.end())
                {
                  error_alert_dialog(win, 4);
                  return void();
                }
            }
          std::string ch_str = p.lexically_proximate(books_path).u8string();
          std::string::size_type n = ch_str.find("../");
          if(n == std::string::npos)
            {
              n = ch_str.find("..\\");
            }

          if(n != std::string::npos)
            {
              error_alert_dialog(win, 2);
            }
          else
            {
              result_archive_path = p;

              Gtk::Window *w_win = wait_window(win);

              finish_wait_disp = std::make_shared<Glib::Dispatcher>();

              finish_wait_disp->connect([win, w_win, this] {
                w_win->close();
                bookSelectionWindow(win, 3);
              });

#ifndef USE_OPENMP
              std::thread thr([this, p] {
                archive_filenames = AddBook::archive_filenames(p, af);
                finish_wait_disp->emit();
              });
              thr.detach();
#else
#pragma omp masked
              {
                omp_event_handle_t event;
#pragma omp task detach(event)
                {
                  archive_filenames = AddBook::archive_filenames(p, af);
                  finish_wait_disp->emit();
                  omp_fulfill_event(event);
                }
              }
#endif
            }
        }
    }

  fd->close();
}
#endif

Gtk::Window *
AddBookGui::wait_window(Gtk::Window *win)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
  window->set_title(gettext("Wait..."));
  window->set_transient_for(*win);
  window->set_modal(false);
  window->set_deletable(false);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_valign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Wait..."));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();

  return window;
}
