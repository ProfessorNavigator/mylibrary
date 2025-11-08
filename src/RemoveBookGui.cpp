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

#include <BookParseEntry.h>
#include <RemoveBook.h>
#include <RemoveBookGui.h>
#include <filesystem>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/checkbutton.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <iostream>
#include <libintl.h>
#include <thread>

RemoveBookGui::RemoveBookGui(const std::shared_ptr<AuxFunc> &af,
                             Gtk::Window *parent_window,
                             const BookBaseEntry &bbe,
                             const std::string &col_name,
                             const std::shared_ptr<BookMarks> &bookmarks,
                             const std::shared_ptr<NotesKeeper> &notes)
{
  this->af = af;
  this->parent_window = parent_window;
  this->bbe = bbe;
  this->col_name = col_name;
  this->bookmarks = bookmarks;
  this->notes = notes;
  notes_reserve_path = af->homePath();
  notes_reserve_path /= std::filesystem::u8path(af->randomFileName());
  while(std::filesystem::exists(notes_reserve_path))
    {
      notes_reserve_path = af->homePath();
      notes_reserve_path /= std::filesystem::u8path(af->randomFileName());
    }
  remove_callback_disp = std::make_shared<Glib::Dispatcher>();
}

void
RemoveBookGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Confirmation"));
  window->set_transient_for(*parent_window);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  int row_numb = 0;

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(
      gettext("This action will remove book from base and from filesystem."));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_numb, 2, 1);
  row_numb++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_margin_top(10);
  lab->set_margin_bottom(10);
  lab->set_halign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_text(gettext("For removing:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_numb, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_max_width_chars(50);
  lab->set_width_chars(50);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_name("windowLabel");
  Glib::ustring text;
  if(!bbe.bpe.book_author.empty())
    {
      text = Glib::ustring(bbe.bpe.book_author) + " - "
             + Glib::ustring(bbe.bpe.book_name);
    }
  else
    {
      text = Glib::ustring(bbe.bpe.book_name);
    }
  lab->set_text(text);
  grid->attach(*lab, 1, row_numb, 1, 1);
  row_numb++;

  bool ch_rar = false;
  std::string ch_str = bbe.bpe.book_path;
  std::string::size_type n;
  std::filesystem::path ch_p;
  std::string ext;
  std::string sstr = "\n";
  while(ch_str.size() > 0)
    {
      n = ch_str.find(sstr);
      if(n != std::string::npos)
        {
          ch_p = std::filesystem::u8path(ch_str.substr(0, n));
          ext = ch_p.extension().u8string();
          ext = af->stringToLower(ext);
          if(ext == ".rar")
            {
              ch_rar = true;
              ch_str.clear();
            }
          else
            {
              ch_str.erase(0, n + sstr.size());
            }
        }
      else
        {
          ch_p = std::filesystem::u8path(ch_str);
          ext = ch_p.extension().u8string();
          ext = af->stringToLower(ext);
          if(ext == ".rar")
            {
              ch_rar = true;
            }
          ch_str.clear();
        }
    }

  if(bbe.file_path.extension() == ".rar" || ch_rar)
    {
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_margin_top(10);
      lab->set_margin_bottom(10);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_justify(Gtk::Justification::CENTER);
      lab->set_expand(true);
      lab->set_use_markup(true);
      text = Glib::ustring("<b><span color=\"red\">")
             + gettext("Warning! Book is packed in rar archive.") + "\n"
             + gettext("Books removing from rar archive is impossible.") + "\n"
             + gettext("This action will remove whole archive.")
             + "</span></b>";
      lab->set_markup(text);
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, row_numb, 2, 1);
      row_numb++;
    }

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  Glib::ustring vl = gettext("Notes will be transfered to ");
  vl += notes_reserve_path.u8string();
  lab->set_text(vl);
  lab->set_visible(false);
  lab->set_selectable(true);

  Gtk::CheckButton *make_notes_reserve = Gtk::make_managed<Gtk::CheckButton>();
  make_notes_reserve->set_margin(5);
  make_notes_reserve->set_halign(Gtk::Align::START);
  make_notes_reserve->set_label(gettext("Create reserve copy of notes"));
  make_notes_reserve->set_active(false);
  make_notes_reserve->set_name("windowLabel");
  make_notes_reserve->signal_toggled().connect(
      [make_notes_reserve, lab, this] {
        notes_reserve = make_notes_reserve->get_active();
        if(notes_reserve)
          {
            lab->set_visible(true);
          }
        else
          {
            lab->set_visible(false);
          }
      });
  grid->attach(*make_notes_reserve, 0, row_numb, 2, 1);
  row_numb++;

  grid->attach(*lab, 0, row_numb, 2, 1);
  row_numb++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_margin_top(10);
  lab->set_margin_bottom(10);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_use_markup(true);
  lab->set_markup("<b>" + Glib::ustring(gettext("Are you sure?")) + "</b>");
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_numb, 2, 1);
  row_numb++;

  Gtk::Grid *action_group_grid = Gtk::make_managed<Gtk::Grid>();
  action_group_grid->set_halign(Gtk::Align::FILL);
  action_group_grid->set_valign(Gtk::Align::FILL);
  action_group_grid->set_expand(true);
  action_group_grid->set_column_homogeneous(true);
  grid->attach(*action_group_grid, 0, row_numb, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_margin(5);
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_name("removeBut");
  yes->set_label(gettext("Yes"));
  yes->signal_clicked().connect(
      std::bind(&RemoveBookGui::removeBookFunc, this, window));
  action_group_grid->attach(*yes, 0, 0, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_margin(5);
  no->set_halign(Gtk::Align::CENTER);
  no->set_name("cancelBut");
  no->set_label(gettext("No"));
  no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  action_group_grid->attach(*no, 1, 0, 1, 1);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);

  window->present();

  remove_callback_disp->connect([this, window] {
    if(remove_callback)
      {
        remove_callback(bbe);
      }
    removeFinished(window);
  });
}

void
RemoveBookGui::removeBookFunc(Gtk::Window *win)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(false);
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
  lab->set_expand(true);
  lab->set_text(gettext("Removing in progress..."));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 1, 1);

  std::thread thr([this] {
    RemoveBook *rb = new RemoveBook(af, bbe, col_name, bookmarks);
    try
      {
        rb->removeBook();
        remove_result = 1;
      }
    catch(std::exception &er)
      {
        std::cout << er.what() << std::endl;
        remove_result = -1;
      }
    delete rb;
    remove_callback_disp->emit();
  });
  thr.detach();
}

void
RemoveBookGui::removeFinished(Gtk::Window *win)
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
  lab->set_expand(true);
  lab->set_name("windowLabel");
  if(remove_result > 0)
    {
      NotesBaseEntry nbe
          = notes->getNote(col_name, bbe.file_path, bbe.bpe.book_path);
      notes->removeNotes(nbe, notes_reserve_path, notes_reserve);
      lab->set_text(gettext("Book successfully removed."));
    }
  else
    {
      lab->set_text(gettext("Error! See system log for details."));
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
