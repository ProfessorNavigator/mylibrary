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
#include <BaseKeeper.h>
#include <BookParseEntry.h>
#include <MLException.h>
#include <OpenBook.h>
#include <RefreshCollection.h>
#include <RemoveBook.h>
#include <SelfRemovingPath.h>
#include <TransferBookGui.h>
#include <giomm-2.68/giomm/file.h>
#include <giomm-2.68/giomm/liststore.h>
#include <glibmm-2.68/glibmm/main.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/filefilter.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/stringobject.h>
#include <iostream>
#include <libintl.h>
#include <tuple>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#endif
#ifndef USE_OPENMP
#include <thread>
#endif

#ifndef ML_GTK_OLD
#include <giomm-2.68/giomm/cancellable.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif

TransferBookGui::TransferBookGui(const std::shared_ptr<AuxFunc> &af,
                                 const std::shared_ptr<BookMarks> &bookmarks,
                                 const std::shared_ptr<NotesKeeper> &notes,
                                 const BookBaseEntry &bbe_from,
                                 const std::string &collection_from,
                                 Gtk::Window *parent_window)
{
  this->af = af;
  this->bookmarks = bookmarks;
  this->notes = notes;
  this->bbe_from = bbe_from;
  this->collection_from = collection_from;
  this->parent_window = parent_window;
}

void
TransferBookGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Moving"));
  window->set_transient_for(*parent_window);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  std::string ext = bbe_from.file_path.extension().u8string();
  ext = af->stringToLower(ext);
  if(ext == ".rar")
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_justify(Gtk::Justification::CENTER);
      lab->set_name("windowLabel");
      lab->set_text(
          Glib::ustring(gettext("Book is packed in rar archive.")) + "\n"
          + gettext("This operation is not available for rar archives.") + "\n"
          + gettext("Use \"Add books\" function from main menu instead.")

      );
      grid->attach(*lab, 0, 0, 1, 1);

      Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
      close->set_margin(5);
      close->set_halign(Gtk::Align::CENTER);
      close->set_name("operationBut");
      close->set_label(gettext("Close"));
      close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
      grid->attach(*close, 0, 1, 1, 1);
    }
  else
    {
      int row_num = 0;

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_text(gettext("Collection book to be moved to"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, row_num, 2, 1);
      row_num++;

      Glib::RefPtr<Gtk::StringList> col_list = create_collections_model();

      collections = Gtk::make_managed<Gtk::DropDown>();
      collections->set_margin(5);
      collections->set_halign(Gtk::Align::CENTER);
      collections->set_name("comboBox");
      collections->set_model(col_list);
      collections->set_enable_search(true);
      grid->attach(*collections, 0, row_num, 2, 1);
      row_num++;

      transfer_fbd = Gtk::make_managed<Gtk::CheckButton>();
      transfer_fbd->set_margin(5);
      transfer_fbd->set_halign(Gtk::Align::START);
      transfer_fbd->set_active(false);
      transfer_fbd->set_label(gettext("Transfer also fbd file if available"));
      transfer_fbd->set_name("windowLabel");
      grid->attach(*transfer_fbd, 0, row_num, 2, 1);
      row_num++;

      compress = Gtk::make_managed<Gtk::CheckButton>();
      compress->set_margin(5);
      compress->set_halign(Gtk::Align::START);
      compress->set_active(false);
      compress->set_label(gettext("Pack book in archive"));
      compress->set_name("windowLabel");
      grid->attach(*compress, 0, row_num, 2, 1);
      row_num++;

      Gtk::Grid *arch_t_gr = Gtk::make_managed<Gtk::Grid>();
      arch_t_gr->set_visible(false);
      grid->attach(*arch_t_gr, 0, row_num, 2, 1);
      row_num++;

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_text(gettext("Archive type:"));
      lab->set_name("windowLabel");
      arch_t_gr->attach(*lab, 0, 0, 1, 1);

      Glib::RefPtr<Gtk::StringList> arch_types_list
          = create_archive_types_model();

      arch_types = Gtk::make_managed<Gtk::DropDown>();
      arch_types->set_margin(5);
      arch_types->set_halign(Gtk::Align::START);
      arch_types->set_name("comboBox");
      arch_types->set_model(arch_types_list);
      arch_t_gr->attach(*arch_types, 1, 0, 1, 1);

      add_to_arch = Gtk::make_managed<Gtk::CheckButton>();
      add_to_arch->set_margin(5);
      add_to_arch->set_halign(Gtk::Align::START);
      add_to_arch->set_active(false);
      add_to_arch->set_label(gettext("Add to existing archive"));
      add_to_arch->set_visible(false);
      add_to_arch->set_name("windowLabel");
      grid->attach(*add_to_arch, 0, row_num, 2, 1);
      row_num++;

      Glib::PropertyProxy<bool> prop_compress_active
          = compress->property_active();
      prop_compress_active.signal_changed().connect([window, this, arch_t_gr] {
        if(compress->get_active())
          {
            add_to_arch->set_visible(true);
            arch_t_gr->set_visible(true);
          }
        else
          {
            add_to_arch->set_visible(false);
            add_to_arch->set_active(false);
            arch_t_gr->set_visible(false);
            window->set_default_size(1, 1);
          }
      });

      Glib::PropertyProxy<bool> prop_add_arch_active
          = add_to_arch->property_active();
      prop_add_arch_active.signal_changed().connect([window, this, arch_t_gr] {
        if(add_to_arch->get_active())
          {
            arch_t_gr->set_visible(false);
            window->set_default_size(1, 1);
          }
        else
          {
            if(compress->get_active())
              {
                arch_t_gr->set_visible(true);
              }
            else
              {
                arch_t_gr->set_visible(false);
              }
          }
      });

      Glib::PropertyProxy<bool> prop_tr_fbd = transfer_fbd->property_active();
      prop_tr_fbd.signal_changed().connect([this, window] {
        _transfer_fbd = transfer_fbd->get_active();
        if(_transfer_fbd)
          {
            compress->set_active(true);
            compress->set_sensitive(false);
            add_to_arch->set_active(false);
            add_to_arch->set_visible(false);
            window->set_default_size(1, 1);
          }
        else
          {
            compress->set_sensitive(true);
            add_to_arch->set_active(false);
            add_to_arch->set_visible(true);
          }
      });

      Gtk::Button *choose_path = Gtk::make_managed<Gtk::Button>();
      choose_path->set_margin(5);
      choose_path->set_halign(Gtk::Align::CENTER);
      choose_path->set_name("operationBut");
      choose_path->set_label(gettext("Choose path"));
      choose_path->signal_clicked().connect(
          std::bind(&TransferBookGui::mode_selector, this, window));
      grid->attach(*choose_path, 0, row_num, 1, 1);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
      cancel->set_margin(5);
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_name("cancelBut");
      cancel->set_label(gettext("Cancel"));
      cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
      grid->attach(*cancel, 1, row_num, 1, 1);
      row_num++;
    }

  window->signal_close_request().connect(
      [window, this] {
        std::shared_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);

  window->present();
}

Glib::RefPtr<Gtk::StringList>
TransferBookGui::create_collections_model()
{
  Glib::RefPtr<Gtk::StringList> result
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::filesystem::path col_p = af->homePath();
  col_p /= std::filesystem::u8path(".local") / std::filesystem::u8path("share")
           / std::filesystem::u8path("MyLibrary")
           / std::filesystem::u8path("Collections");
  if(std::filesystem::exists(col_p))
    {
      std::string col_nm;
      for(auto &dirit : std::filesystem::directory_iterator(col_p))
        {
          std::filesystem::path p = dirit.path();
          if(std::filesystem::is_directory(p))
            {
              col_nm = p.filename().u8string();
              if(col_nm != collection_from)
                {
                  result->append(Glib::ustring(col_nm));
                }
            }
        }
    }

  return result;
}

Glib::RefPtr<Gtk::StringList>
TransferBookGui::create_archive_types_model()
{
  Glib::RefPtr<Gtk::StringList> result
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::vector<std::string> types = af->get_supported_archive_types_packing();

  for(auto it = types.begin(); it != types.end(); it++)
    {
      result->append(Glib::ustring(*it));
    }

  return result;
}

void
TransferBookGui::mode_selector(Gtk::Window *win)
{
  Glib::RefPtr<Gtk::StringObject> item
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collections->get_selected_item());
  if(item)
    {
      collection_to = std::string(item->get_string());
      books_path = BaseKeeper::get_books_path(collection_to, af);
      if(compress->get_active())
        {
          if(add_to_arch->get_active())
            {
              path_choose_dialog(win, 3);
            }
          else
            {
              path_choose_dialog(win, 2);
            }
        }
      else
        {
          path_choose_dialog(win, 1);
        }
    }
  else
    {
      books_path.clear();
    }
}

void
TransferBookGui::path_choose_dialog(Gtk::Window *win, const int &variant)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);
  fd->set_title(gettext("Path in collection"));

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(books_path.u8string());
  fd->set_initial_folder(initial);

  std::vector<std::string> types;
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  switch(variant)
    {
    case 1:
      {
        if(bbe_from.bpe.book_path.empty())
          {
            std::string ext = af->get_extension(bbe_from.file_path);
            ext = "*" + ext;
            filter->add_pattern(Glib::ustring(ext));
            fd->set_initial_name(
                Glib::ustring(bbe_from.file_path.filename().u8string()));
          }
        else
          {
            std::string bp = bbe_from.bpe.book_path;
            std::string sstr = "\n";
            std::string::size_type n = bp.rfind(sstr);
            if(n != std::string::npos)
              {
                bp.erase(0, n + sstr.size());
              }
            std::filesystem::path p = std::filesystem::u8path(bp);
            std::string ext = af->get_extension(p);
            ext = "*" + ext;
            filter->add_pattern(Glib::ustring(ext));
            fd->set_initial_name(p.filename().u8string());
          }
        break;
      }
    case 2:
      {
        std::filesystem::path p;
        if(bbe_from.bpe.book_path.empty())
          {
            p = bbe_from.file_path;
          }
        else
          {
            std::string bp = bbe_from.bpe.book_path;
            std::string sstr = "\n";
            std::string::size_type n = bp.rfind(sstr);
            if(n != std::string::npos)
              {
                bp.erase(0, n + sstr.size());
              }
            p = std::filesystem::u8path(bp);
          }
        Glib::RefPtr<Gtk::StringObject> type
            = std::dynamic_pointer_cast<Gtk::StringObject>(
                arch_types->get_selected_item());
        if(type)
          {
            std::string suffix(type->get_string());
            filter->add_suffix(suffix);
            fd->set_initial_name(p.stem().u8string() + "." + suffix);
          }
        else
          {
            std::cout
                << "TransferBookGui::path_choose_dialog_overwrite error: "
                   "archive type is null"
                << std::endl;
            return void();
          }
        break;
      }
    case 3:
      {
        types = af->get_supported_archive_types_packing();
        for(auto it = types.begin(); it != types.end(); it++)
          {
            filter->add_suffix(*it);
          }
        filter->set_name(gettext("All supported"));
        break;
      }
    default:
      return void();
    }

  fd->set_default_filter(filter);

  if(variant == 3)
    {
      Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> f_list
          = Gio::ListStore<Gtk::FileFilter>::create();
      f_list->append(filter);
      for(auto it = types.begin(); it != types.end(); it++)
        {
          filter = Gtk::FileFilter::create();
          filter->add_suffix(*it);
          f_list->append(filter);
        }
      fd->set_filters(f_list);
    }

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

  switch(variant)
    {
    case 1:
    case 2:
      {
        fd->save(*win,
                 std::bind(&TransferBookGui::path_choose_dialog_overwrite_slot,
                           this, std::placeholders::_1, fd, win, variant),
                 cncl);
        break;
      }
    case 3:
      {
        fd->open(*win,
                 std::bind(&TransferBookGui::path_choose_dialog_add_slot, this,
                           std::placeholders::_1, fd, win),
                 cncl);
        break;
      }
    default:
      break;
    }
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fd;
  switch(variant)
    {
    case 1:
    case 2:
      {
        fd = new Gtk::FileChooserDialog(*win, gettext("Path in collection"),
                                        Gtk::FileChooser::Action::SAVE, true);
        break;
      }
    case 3:
      {
        fd = new Gtk::FileChooserDialog(*win, gettext("Path in collection"),
                                        Gtk::FileChooser::Action::OPEN, true);
        break;
      }
    default:
      return void();
    }
  fd->set_application(win->get_application());
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(books_path.u8string());
  fd->set_current_folder(initial);

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  switch(variant)
    {
    case 1:
    case 2:
      {
        but = fd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);
        break;
      }
    case 3:
      {
        but = fd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);
        break;
      }
    default:
      {
        delete fd;
        return void();
      }
    }
  but->set_margin(5);
  but->set_name("applyBut");

  std::vector<std::string> types;
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  switch(variant)
    {
    case 1:
      {
        if(bbe_from.bpe.book_path.empty())
          {
            std::string ext = af->get_extension(bbe_from.file_path);
            ext = "*" + ext;
            filter->add_pattern(Glib::ustring(ext));
            fd->set_current_name(
                Glib::ustring(bbe_from.file_path.filename().u8string()));
          }
        else
          {
            std::string bp = bbe_from.bpe.book_path;
            std::string::size_type n = bp.rfind("\n");
            if(n != std::string::npos)
              {
                bp.erase(0, n + std::string("\n").size());
              }
            std::filesystem::path p = std::filesystem::u8path(bp);
            std::string ext = af->get_extension(p);
            ext = "*" + ext;
            filter->add_pattern(Glib::ustring(ext));
            fd->set_current_name(p.filename().u8string());
          }
        break;
      }
    case 2:
      {
        std::filesystem::path p;
        if(bbe_from.bpe.book_path.empty())
          {
            p = bbe_from.file_path;
          }
        else
          {
            std::string bp = bbe_from.bpe.book_path;
            std::string::size_type n = bp.rfind("\n");
            if(n != std::string::npos)
              {
                bp.erase(0, n + std::string("\n").size());
              }
            p = std::filesystem::u8path(bp);
          }
        Glib::RefPtr<Gtk::StringObject> type
            = std::dynamic_pointer_cast<Gtk::StringObject>(
                arch_types->get_selected_item());
        if(type)
          {
            std::string suffix(type->get_string());
            filter->add_suffix(suffix);
            fd->set_current_name(p.stem().u8string() + "." + suffix);
          }
        else
          {
            std::cout
                << "TransferBookGui::path_choose_dialog_overwrite error: "
                   "archive type is null"
                << std::endl;
            return void();
          }
        break;
      }
    case 3:
      {
        types = af->get_supported_archive_types_packing();
        for(auto it = types.begin(); it != types.end(); it++)
          {
            filter->add_suffix(*it);
          }
        filter->set_name(gettext("All supported"));
        break;
      }
    default:
      return void();
    }

  if(variant == 3)
    {
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
  else
    {
      fd->set_filter(filter);
    }

  switch(variant)
    {
    case 1:
    case 2:
      {
        fd->signal_response().connect(
            std::bind(&TransferBookGui::path_choose_dialog_overwrite_slot,
                      this, std::placeholders::_1, fd, win, variant));
        break;
      }
    case 3:
      {
        fd->signal_response().connect(
            std::bind(&TransferBookGui::path_choose_dialog_add_slot, this,
                      std::placeholders::_1, fd, win));
        break;
      }
    default:
      {
        delete fd;
        return void();
      }
    }

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
TransferBookGui::path_choose_dialog_overwrite_slot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Window *win,
    const int &variant)
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
              << "TransferBookGui::path_choose_dialog_overwrite_slot error: "
              << er.what() << std::endl;
        }
    }
  if(fl)
    {
      out_file_path = std::filesystem::u8path(fl->get_path());
      switch(variant)
        {
        case 1:
          {
            copy_process_window(win);

            std::shared_ptr<int> res_var = std::make_shared<int>(0);

            copy_result_disp = std::make_shared<Glib::Dispatcher>();
            copy_result_disp->connect([this, res_var, win] {
              finish_window(win, *res_var);
              if(*res_var == 3)
                {
                  if(success_signal)
                    {
                      success_signal(bbe_from, collection_from);
                    }
                }
            });
#ifndef USE_OPENMP
            std::thread thr([variant, res_var, this] {
              try
                {
                  copy_overwrite(variant, res_var);
                }
              catch(MLException &er)
                {
                  std::cout << er.what() << std::endl;
                  *res_var = 1;
                  copy_result_disp->emit();
                }
            });
            thr.detach();
#endif
#ifdef USE_OPENMP
#pragma omp masked
            {
              omp_event_handle_t event;
#pragma omp task detach(event)
              {
                try
                  {
                    copy_overwrite(variant, res_var);
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    *res_var = 1;
                    copy_result_disp->emit();
                  }
                omp_fulfill_event(event);
              }
            }
#endif
            break;
          }
        case 2:
          {
            path_in_archive_window(win, variant);
            break;
          }
        default:
          break;
        }
    }
}
#endif

void
TransferBookGui::copy_process_window(Gtk::Window *win)
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
  lab->set_valign(Gtk::Align::FILL);
  lab->set_expand(true);
  lab->set_text(gettext("Wait..."));
  lab->set_name("windowLabel");
  win->set_child(*lab);
}

void
TransferBookGui::copy_overwrite(const int &variant,
                                const std::shared_ptr<int> &res_var)
{
  std::filesystem::path tmp = af->temp_path();
  tmp /= std::filesystem::u8path(af->randomFileName());
  SelfRemovingPath tmp_p(tmp);
  OpenBook ob(af);
  std::filesystem::path sbp
      = ob.open_book(bbe_from, false, tmp_p.path, true, nullptr);
  BookBaseEntry bbe_out = bbe_from;
  bbe_out.file_path = out_file_path;
  bbe_out.bpe.book_path.clear();

  if(std::filesystem::exists(sbp))
    {
      AddBook ab(af, collection_to, false, bookmarks);
      std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
          books;
      if(!_transfer_fbd)
        {
          switch(variant)
            {
            case 1:
              {
                books.emplace_back(std::make_tuple(sbp, out_file_path));
                bbe_out.bpe.book_path.clear();
                try
                  {
                    ab.simple_add(books);
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    *res_var = 1;
                    copy_result_disp->emit();
                    return void();
                  }
                break;
              }
            case 2:
              {
                books.emplace_back(std::make_tuple(
                    sbp, std::filesystem::u8path(path_in_arch)));
                bbe_out.bpe.book_path = path_in_arch;
                try
                  {
                    ab.overwrite_archive(out_file_path, books);
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    *res_var = 1;
                    copy_result_disp->emit();
                    return void();
                  }
                break;
              }
            case 3:
              {
                books.emplace_back(std::make_tuple(
                    sbp, std::filesystem::u8path(path_in_arch)));
                bbe_out.bpe.book_path = path_in_arch;
                try
                  {
                    ab.add_to_existing_archive(out_file_path, books);
                  }
                catch(MLException &er)
                  {
                    std::cout << er.what() << std::endl;
                    *res_var = 1;
                    copy_result_disp->emit();
                    return void();
                  }
                break;
              }
            default:
              break;
            }
        }
      else
        {
          books.emplace_back(
              std::make_tuple(sbp, std::filesystem::u8path(path_in_arch)));
          bbe_out.bpe.book_path = path_in_arch;
          std::filesystem::path fbd_p_arch
              = std::filesystem::u8path(path_in_arch);

          std::string sstr = sbp.stem().u8string();
          std::string ext;
          for(auto &dirit :
              std::filesystem::recursive_directory_iterator(sbp.parent_path()))
            {
              std::filesystem::path p = dirit.path();
              ext = p.extension().u8string();
              ext = af->stringToLower(ext);
              if(!std::filesystem::is_directory(p)
                 && p.stem().u8string() == sstr && ext == ".fbd")
                {
                  fbd_p_arch.replace_extension(p.extension());
                  books.push_back(std::make_tuple(p, fbd_p_arch));
                  break;
                }
            }
          ab.overwrite_archive(out_file_path, books);
        }
    }

  if(std::filesystem::exists(out_file_path))
    {
#ifndef USE_OPENMP
      std::shared_ptr<RefreshCollection> rfr
          = std::make_shared<RefreshCollection>(
              af, collection_to, std::thread::hardware_concurrency(), false,
              true, false, bookmarks);
#endif
#ifdef USE_OPENMP
      std::shared_ptr<RefreshCollection> rfr
          = std::make_shared<RefreshCollection>(af, collection_to,
                                                omp_get_num_procs(), false,
                                                true, false, bookmarks);
#endif

      if(rfr->refreshBook(bbe_out))
        {
          NotesBaseEntry nbe = notes->getNote(
              collection_from, bbe_from.file_path, bbe_from.bpe.book_path);
          std::fstream f;
          f.open(nbe.note_file_full_path,
                 std::ios_base::in | std::ios_base::binary);
          if(f.is_open())
            {
              std::string note_buf;
              f.seekg(0, std::ios_base::end);
              note_buf.resize(f.tellg());
              f.seekg(0, std::ios_base::beg);
              f.read(note_buf.data(), note_buf.size());
              f.close();

              std::string find_str = "\n\n";
              std::string::size_type n = note_buf.find(find_str);
              if(n != std::string::npos)
                {
                  note_buf.erase(0, n + find_str.size());
                }

              notes->editNote(nbe, "");

              nbe = notes->getNote(collection_to, bbe_out.file_path,
                                   bbe_out.bpe.book_path);
              notes->editNote(nbe, note_buf);
            }

          rfr.reset();
          std::shared_ptr<RemoveBook> rmb = std::make_shared<RemoveBook>(
              af, bbe_from, collection_from, bookmarks);
          rmb->removeBook();
          *res_var = 3;
          copy_result_disp->emit();
        }
      else
        {
          *res_var = 2;
          copy_result_disp->emit();
        }
    }
}

void
TransferBookGui::finish_window(Gtk::Window *win, const int &variant)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(true);
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

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
        lab->set_text(gettext("Error! See system log for details."));
        break;
      }
    case 2:
      {
        lab->set_text(
            gettext("Error! Book has not been found in new location!"));
        break;
      }
    default:
      {
        lab->set_text(gettext("Book has been successfully moved!"));
        break;
      }
    }

  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
  grid->attach(*close, 0, 1, 1, 1);
}

void
TransferBookGui::path_in_archive_window(Gtk::Window *win, const int &variant)
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
  lab->set_text(gettext("Archive file path:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_margin_start(20);
  lab->set_halign(Gtk::Align::START);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<i>") + out_file_path.u8string() + "</i>");
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 1, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Path in archive:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 2, 2, 1);

  Gtk::Entry *path_in_archive_ent = Gtk::make_managed<Gtk::Entry>();
  path_in_archive_ent->set_margin(5);
  path_in_archive_ent->set_halign(Gtk::Align::FILL);
  path_in_archive_ent->set_width_chars(50);
  path_in_archive_ent->set_name("windowEntry");
  std::filesystem::path p;
  if(bbe_from.bpe.book_path.empty())
    {
      p = bbe_from.file_path;
    }
  else
    {
      std::string bp = bbe_from.bpe.book_path;
      std::string sstr = "\n";
      std::string::size_type n = bp.rfind(sstr);
      if(n != std::string::npos)
        {
          bp.erase(0, n + sstr.size());
        }
      p = std::filesystem::u8path(bp);
    }
  path_in_archive_ent->set_text(p.filename().u8string());
  grid->attach(*path_in_archive_ent, 0, 3, 2, 1);

  Gtk::Button *move = Gtk::make_managed<Gtk::Button>();
  move->set_margin(5);
  move->set_halign(Gtk::Align::CENTER);
  move->set_name("applyBut");
  move->set_label(gettext("Move"));
  move->signal_clicked().connect(std::bind(&TransferBookGui::copy_archive,
                                           this, win, window,
                                           path_in_archive_ent, variant));
  grid->attach(*move, 0, 4, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 1, 4, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::shared_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

void
TransferBookGui::copy_archive(Gtk::Window *parent_win, Gtk::Window *win,
                              Gtk::Entry *p_in_arch, const int &variant)
{
  path_in_arch = std::string(p_in_arch->get_text());
  if(path_in_arch.empty())
    {
      alert_dialog(win, 1);
      return void();
    }
  else
    {
      std::string sstr = "\\";
      std::string::size_type n = 0;
      for(;;)
        {
          n = path_in_arch.find(sstr, n);
          if(n != std::string::npos)
            {
              path_in_arch.erase(n, sstr.size());
              path_in_arch.insert(n, "/");
            }
          else
            {
              break;
            }
        }
    }

  if(variant == 3)
    {
      auto itinarch = std::find(arch_filelist.begin(), arch_filelist.end(),
                                path_in_arch);
      if(itinarch != arch_filelist.end())
        {
          alert_dialog(win, 2);
          return void();
        }
    }

  win->close();

  copy_process_window(parent_win);

  std::shared_ptr<int> res_var = std::make_shared<int>(0);

  copy_result_disp = std::make_shared<Glib::Dispatcher>();
  copy_result_disp->connect([this, res_var, parent_win] {
    finish_window(parent_win, *res_var);
    if(*res_var == 3)
      {
        if(success_signal)
          {
            success_signal(bbe_from, collection_from);
          }
      }
  });

#ifndef USE_OPENMP
  std::thread thr([res_var, this, variant] {
    try
      {
        copy_overwrite(variant, res_var);
      }
    catch(MLException &er)
      {
        std::cout << er.what() << std::endl;
        *res_var = 1;
        copy_result_disp->emit();
      }
  });
  thr.detach();
#endif
#ifdef USE_OPENMP
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      try
        {
          copy_overwrite(variant, res_var);
        }
      catch(MLException &er)
        {
          std::cout << er.what() << std::endl;
          *res_var = 1;
          copy_result_disp->emit();
        }
      omp_fulfill_event(event);
    }
  }
#endif
}

#ifndef ML_GTK_OLD
void
TransferBookGui::path_choose_dialog_add_slot(
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
          std::cout << "TransferBookGui::path_choose_dialog_add_slot error: "
                    << er.what() << std::endl;
        }
    }
  if(fl)
    {
      out_file_path = std::filesystem::u8path(fl->get_path());

      Gtk::Window *window = new Gtk::Window;
      window->set_application(win->get_application());
      window->set_title(gettext("Wait..."));
      window->set_transient_for(*win);
      window->set_modal(true);
      window->set_deletable(false);

      window->signal_close_request().connect(
          [window] {
            std::shared_ptr<Gtk::Window> win(window);
            win->set_visible(false);
            return true;
          },
          false);

      window->present();

      copy_process_window(window);

      form_arch_filelist_disp = std::make_shared<Glib::Dispatcher>();
      form_arch_filelist_disp->connect([window, win, this] {
        window->close();
        path_in_archive_window(win, 3);
      });

#ifndef USE_OPENMP
      std::thread thr([this] {
        arch_filelist = AddBook::archive_filenames(out_file_path, af);
        form_arch_filelist_disp->emit();
      });
      thr.detach();
#endif
#ifdef USE_OPENMP
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          arch_filelist = AddBook::archive_filenames(out_file_path, af);
          form_arch_filelist_disp->emit();
          omp_fulfill_event(event);
        }
      }
#endif
    }
}
#endif

#ifdef ML_GTK_OLD
void
TransferBookGui::path_choose_dialog_overwrite_slot(int resp,
                                                   Gtk::FileChooserDialog *fd,
                                                   Gtk::Window *win,
                                                   const int &variant)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          out_file_path = std::filesystem::u8path(fl->get_path());
          switch(variant)
            {
            case 1:
              {
                copy_process_window(win);

                std::shared_ptr<int> res_var = std::make_shared<int>(0);

                copy_result_disp = std::make_shared<Glib::Dispatcher>();
                copy_result_disp->connect([this, res_var, win] {
                  finish_window(win, *res_var);
                  if(*res_var == 3)
                    {
                      if(success_signal)
                        {
                          success_signal(bbe_from, collection_from);
                        }
                    }
                });

                std::thread thr([variant, res_var, this] {
                  try
                    {
                      copy_overwrite(variant, res_var);
                    }
                  catch(MLException &er)
                    {
                      std::cout << er.what() << std::endl;
                      *res_var = 1;
                      copy_result_disp->emit();
                    }
                });
                thr.detach();
                break;
              }
            case 2:
              {
                path_in_archive_window(win, variant);
                break;
              }
            default:
              break;
            }
        }
    }
  fd->close();
}

void
TransferBookGui::path_choose_dialog_add_slot(int resp,
                                             Gtk::FileChooserDialog *fd,
                                             Gtk::Window *win)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();

      if(fl)
        {
          out_file_path = std::filesystem::u8path(fl->get_path());

          Gtk::Window *window = new Gtk::Window;
          window->set_application(win->get_application());
          window->set_title(gettext("Wait..."));
          window->set_transient_for(*win);
          window->set_modal(true);
          window->set_deletable(false);

          window->signal_close_request().connect(
              [window] {
                std::shared_ptr<Gtk::Window> win(window);
                win->set_visible(false);
                return true;
              },
              false);

          window->present();

          copy_process_window(window);

          form_arch_filelist_disp = std::make_shared<Glib::Dispatcher>();
          form_arch_filelist_disp->connect([window, win, this] {
            window->close();
            path_in_archive_window(win, 3);
          });

          std::thread thr([this] {
            arch_filelist = AddBook::archive_filenames(out_file_path, af);
            form_arch_filelist_disp->emit();
          });
          thr.detach();
        }
    }

  fd->close();
}
#endif

void
TransferBookGui::alert_dialog(Gtk::Window *win, const int &variant)
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
  lab->set_name("windowLabel");
  switch(variant)
    {
    case 1:
      {
        lab->set_text(gettext("Error! Path in archive cannot be empty!"));
        break;
      }
    case 2:
      {
        lab->set_text(
            gettext("Error! File with such name is already in archive!"));
        break;
      }
    default:
      {
        delete window;
        return void();
      }
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
        std::shared_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}
