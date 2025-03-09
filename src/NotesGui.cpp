/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <NotesGui.h>
#include <fstream>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <libintl.h>

NotesGui::NotesGui(Gtk::Window *parent_window,
                   const std::shared_ptr<NotesKeeper> notes)
{
  this->parent_window = parent_window;
  this->notes = notes;
}

void
NotesGui::creatWindow(const std::string &collection_name,
                      const BookBaseEntry &bbe)
{
  nbe = notes->getNote(collection_name, bbe.file_path, bbe.bpe.book_path);

  notes_window = new Gtk::Window;
  notes_window->set_application(parent_window->get_application());
  notes_window->set_transient_for(*parent_window);
  notes_window->set_modal(true);
  notes_window->set_name("MLwindow");
  Gdk::Rectangle rec = screen_size();
  notes_window->set_default_size(rec.get_width() * 0.7,
                                 rec.get_height() * 0.7);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  notes_window->set_child(*grid);

  Gtk::Grid *header_grid = Gtk::make_managed<Gtk::Grid>();
  header_grid->set_halign(Gtk::Align::FILL);
  grid->attach(*header_grid, 0, 0, 2, 1);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text(gettext("Collection:"));
  header_grid->attach(*lab, 0, 0, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text(nbe.collection_name);
  header_grid->attach(*lab, 1, 0, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text(gettext("Book file path:"));
  header_grid->attach(*lab, 0, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text(nbe.book_file_full_path.u8string());
  header_grid->attach(*lab, 1, 1, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text(gettext("Book path:"));
  header_grid->attach(*lab, 0, 2, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  std::string vl = nbe.book_path;
  for(auto it = vl.begin(); it != vl.end(); it++)
    {
      char v = *it;
      if(v == '\n')
        {
          v = '/';
        }
      *it = v;
    }
  lab->set_text(vl);
  header_grid->attach(*lab, 1, 2, 1, 1);

  Gtk::ScrolledWindow *txt_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  txt_scrl->set_margin(5);
  txt_scrl->set_halign(Gtk::Align::FILL);
  txt_scrl->set_valign(Gtk::Align::FILL);
  txt_scrl->set_expand(true);
  txt_scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  grid->attach(*txt_scrl, 0, 1, 2, 1);

  note_txt = Gtk::make_managed<Gtk::TextView>();
  note_txt->set_name("textField");
  note_txt->set_left_margin(5);
  note_txt->set_top_margin(5);
  note_txt->set_right_margin(5);
  note_txt->set_bottom_margin(5);
  std::fstream f;
  f.open(nbe.note_file_full_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.seekg(0, std::ios_base::end);
      note_buffer.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(note_buffer.data(), note_buffer.size());
      f.close();

      std::string find_str = "\n\n";
      std::string::size_type n = note_buffer.find(find_str);
      if(n != std::string::npos)
        {
          note_buffer.erase(0, n + find_str.size());
        }

      Glib::RefPtr<Gtk::TextBuffer> tb = note_txt->get_buffer();
      tb->set_text(note_buffer);
    }
  txt_scrl->set_child(*note_txt);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_margin(5);
  save->set_halign(Gtk::Align::CENTER);
  save->set_name("applyBut");
  save->set_label(gettext("Save"));
  save->signal_clicked().connect(
      std::bind(&NotesGui::confirmationDialog, this));
  grid->attach(*save, 0, 2, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("cancelBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect([this] {
    Glib::RefPtr<Gtk::TextBuffer> tb = note_txt->get_buffer();
    if(std::string(tb->get_text()) == note_buffer)
      {
        notes_window->close();
      }
    else
      {
        closeDialog();
      }
  });
  grid->attach(*close, 1, 2, 1, 1);

  notes_window->signal_close_request().connect(
      [this] {
        std::unique_ptr<Gtk::Window> win(notes_window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);

  notes_window->present();
}

Gdk::Rectangle
NotesGui::screen_size()
{
  Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
  Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);

  req.set_width(req.get_width() * mon->get_scale_factor());
  req.set_height(req.get_height() * mon->get_scale_factor());

  return req;
}

void
NotesGui::confirmationDialog()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(notes_window->get_application());
  window->set_transient_for(*notes_window);
  window->set_name("MLwindow");
  window->set_modal(true);
  window->set_default_size(1, 1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_max_width_chars(50);
  lab->set_width_chars(50);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_justify(Gtk::Justification::CENTER);
  lab->set_name("windowLabel");
  lab->set_text(gettext("Are you sure?"));
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_margin(5);
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_name("applyBut");
  yes->set_label(gettext("Yes"));
  yes->signal_clicked().connect([this, window] {
    Glib::RefPtr<Gtk::TextBuffer> tb = note_txt->get_buffer();
    note_buffer = tb->get_text();
    notes->editNote(nbe, note_buffer);
    window->close();
  });
  grid->attach(*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_margin(5);
  no->set_halign(Gtk::Align::CENTER);
  no->set_name("cancelBut");
  no->set_label(gettext("No"));
  no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*no, 1, 1, 1, 1);

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
NotesGui::closeDialog()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(notes_window->get_application());
  window->set_transient_for(*notes_window);
  window->set_name("MLwindow");
  window->set_modal(true);
  window->set_default_size(1, 1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_max_width_chars(50);
  lab->set_width_chars(50);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_justify(Gtk::Justification::CENTER);
  lab->set_name("windowLabel");
  lab->set_text(gettext("There are some unsaved changes. Close anyway?"));
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_margin(5);
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_name("applyBut");
  yes->set_label(gettext("Yes"));
  yes->signal_clicked().connect([this, window] {
    window->unset_transient_for();
    notes_window->close();
    window->close();
  });
  grid->attach(*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_margin(5);
  no->set_halign(Gtk::Align::CENTER);
  no->set_name("cancelBut");
  no->set_label(gettext("No"));
  no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*no, 1, 1, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}
