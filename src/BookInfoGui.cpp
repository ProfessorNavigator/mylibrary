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

#include <BookInfoGui.h>
#include <BookParseEntry.h>
#include <CoverPixBuf.h>
#include <ElectroBookInfoEntry.h>
#include <FullSizeCover.h>
#include <Genre.h>
#include <MLException.h>
#include <PaperBookInfoEntry.h>
#include <SaveCover.h>
#include <filesystem>
#include <functional>
#include <giomm-2.68/giomm/menuitem.h>
#include <giomm-2.68/giomm/simpleaction.h>
#include <giomm-2.68/giomm/simpleactiongroup.h>
#include <gtkmm-4.0/gdkmm/display.h>
#include <gtkmm-4.0/gdkmm/general.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gdkmm/surface.h>
#include <gtkmm-4.0/gtkmm/drawingarea.h>
#include <gtkmm-4.0/gtkmm/gestureclick.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/linkbutton.h>
#include <gtkmm-4.0/gtkmm/requisition.h>
#include <iostream>
#include <libintl.h>

BookInfoGui::BookInfoGui(const std::shared_ptr<AuxFunc> &af,
                         Gtk::Window *parent_window)
{
  this->af = af;
  this->parent_window = parent_window;
  bi = new BookInfo(af);
}

BookInfoGui::~BookInfoGui()
{
  delete bi;
}

BookInfoGui::BookInfoGui(const std::shared_ptr<AuxFunc> &af,
                         Gtk::Window *parent_window,
                         const std::shared_ptr<BookInfoEntry> &bie)
{
  this->af = af;
  this->parent_window = parent_window;
  this->bie = bie;
  if(bie)
    {
      CoverPixBuf cpb(bie);
      cover_buf = cpb;
    }
}

void
BookInfoGui::creatWindow(const BookBaseEntry &bbe)
{
  if(!bie && bi)
    {
      try
        {
          bie = bi->get_book_info(bbe);
        }
      catch(MLException &e)
        {
          bie.reset();
          std::cout << e.what() << std::endl;
        }
      if(bie)
        {
          CoverPixBuf cpb(bie);
          cover_buf = cpb;
        }
    }
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Book info"));
  window->set_transient_for(*parent_window);
  window->set_modal(true);
  window->set_name("MLwindow");

  cover_operations_action_group(window);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::ScrolledWindow *info_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  info_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                        Gtk::PolicyType::AUTOMATIC);
  info_scrl->set_halign(Gtk::Align::FILL);
  info_scrl->set_valign(Gtk::Align::FILL);
  info_scrl->set_expand(true);
  info_scrl->set_margin(5);
  grid->attach(*info_scrl, 1, 0, 1, 1);

  Gtk::Grid *info_scrl_grid = Gtk::make_managed<Gtk::Grid>();
  info_scrl_grid->set_halign(Gtk::Align::FILL);
  info_scrl_grid->set_valign(Gtk::Align::FILL);
  info_scrl_grid->set_expand(true);
  info_scrl->set_child(*info_scrl_grid);

  int row_num = 0;

  formBookSection(bbe, info_scrl_grid, row_num);

  formEectordocInfoSection(bbe, info_scrl_grid, row_num);

  formPaperBookInfoSection(bbe, info_scrl_grid, row_num);

  formFileSection(bbe, info_scrl_grid, row_num);

  Gdk::Rectangle scr_sz = screen_size();
  Gtk::Requisition min, nat;
  int width, height, w, h;
  width = scr_sz.get_width() * 0.45;
  height = scr_sz.get_height() * 0.9;

  info_scrl_grid->get_preferred_size(min, nat);
  w = nat.get_width();
  h = nat.get_height();
  if(w <= width)
    {
      info_scrl->set_min_content_width(w);
    }
  else
    {
      info_scrl->set_min_content_width(width);
    }

  if(h <= height)
    {
      info_scrl->set_min_content_height(h);
    }
  else
    {
      info_scrl->set_min_content_height(height);
    }

  int cover_w = cover_width(info_scrl);

  Gtk::DrawingArea *cover = Gtk::make_managed<Gtk::DrawingArea>();
  cover->set_margin(5);
  cover->set_valign(Gtk::Align::FILL);
  cover->set_halign(Gtk::Align::FILL);
  cover->set_expand(true);
  cover->set_content_width(cover_w);
  cover->set_draw_func(std::bind(&BookInfoGui::cover_draw, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
  grid->attach(*cover, 0, 0, 1, 1);

  Glib::RefPtr<Gio::Menu> menu = cover_menu();

  Gtk::PopoverMenu *pop_menu = Gtk::make_managed<Gtk::PopoverMenu>();
  pop_menu->set_menu_model(menu);
  pop_menu->set_parent(*cover);
  cover->signal_unrealize().connect([pop_menu] {
    pop_menu->unparent();
  });

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect(
      std::bind(&BookInfoGui::cover_full_size, this, window));
  cover->add_controller(clck);

  clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect(std::bind(
      &BookInfoGui::show_cover_popup_menu, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, pop_menu));
  cover->add_controller(clck);

  window->signal_close_request().connect(
      [window, this] {
        std::unique_ptr<Gtk::Window> win(window);
        window->set_visible(false);
        delete this;
        return true;
      },
      false);

  window->present();
}

Glib::ustring
BookInfoGui::translate_genre(const std::string &genre_str)
{
  Glib::ustring result;
  std::vector<GenreGroup> genre_list = af->get_genre_list();
  std::string genre_loc = genre_str;
  std::string genre;
  std::string::size_type n;
  Glib::ustring val;
  bool interrupt = false;
  std::string sstr = ",";
  for(;;)
    {
      n = genre_loc.find(sstr);
      if(n != std::string::npos)
        {
          genre = genre_loc.substr(0, n);
          genre_loc.erase(0, n + sstr.size());
          val = translate_genre_func(genre, genre_list);
        }
      else
        {
          if(!genre_loc.empty())
            {
              val = translate_genre_func(genre_loc, genre_list);
            }
          interrupt = true;
        }
      if(!val.empty())
        {
          if(result.empty())
            {
              result = val;
            }
          else
            {
              result = result + "\n" + val;
            }
        }
      if(interrupt)
        {
          break;
        }
    }

  return result;
}

int
BookInfoGui::cover_width(Gtk::ScrolledWindow *scrl)
{
  int result = 0;

  if(cover_buf)
    {
      double w = static_cast<double>(cover_buf->get_width());
      double h = static_cast<double>(cover_buf->get_height());
      double scale = w / h;
      result = (scrl->get_min_content_height() - 10) * scale;
    }

  return result;
}

Glib::ustring
BookInfoGui::translate_genre_func(std::string &genre,
                                  const std::vector<GenreGroup> &genre_list)
{
  Glib::ustring result;

  for(auto it = genre.begin(); it != genre.end();)
    {
      if(*it == ' ')
        {
          genre.erase(it);
        }
      else
        {
          break;
        }
    }

  if(!genre.empty())
    {
      Genre g;
      GenreGroup gg;
      bool stop = false;
      for(auto it = genre_list.begin(); it != genre_list.end(); it++)
        {
          for(auto itgg = it->genres.begin(); itgg != it->genres.end(); itgg++)
            {
              if(itgg->genre_code == genre)
                {
                  g = *itgg;
                  gg = *it;
                  stop = true;
                  break;
                }
            }
          if(stop)
            {
              break;
            }
        }
      if(!g.genre_name.empty())
        {
          if(gg.group_name == g.genre_name)
            {
              result = Glib::ustring(gg.group_name);
            }
          else
            {
              result = Glib::ustring(gg.group_name + ", "
                                     + af->stringToLower(g.genre_name));
            }
        }
      else
        {
          result = Glib::ustring(genre);
        }
    }

  return result;
}

void
BookInfoGui::formBookSection(const BookBaseEntry &bbe, Gtk::Grid *grid,
                             int &row_num)
{
  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + gettext("Book")
                  + Glib::ustring("</b>"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_num, 2, 1);
  row_num++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_text(gettext("Book name:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_num, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_selectable(true);
  lab->set_text(bbe.bpe.book_name);
  lab->set_name("windowLabel");
  grid->attach(*lab, 1, row_num, 1, 1);
  row_num++;

  if(!bbe.bpe.book_author.empty())
    {
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_valign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_text(gettext("Author(s):"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, row_num, 1, 1);

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_selectable(true);
      lab->set_max_width_chars(50);
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_text(bbe.bpe.book_author);
      lab->set_name("windowLabel");
      grid->attach(*lab, 1, row_num, 1, 1);
      row_num++;
    }

  if(!bbe.bpe.book_genre.empty())
    {
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_valign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_text(gettext("Genre(s):"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, row_num, 1, 1);

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_selectable(true);
      lab->set_max_width_chars(50);
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_text(translate_genre(bbe.bpe.book_genre));
      lab->set_name("windowLabel");
      grid->attach(*lab, 1, row_num, 1, 1);
      row_num++;
    }

  if(!bbe.bpe.book_series.empty())
    {
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_valign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_text(gettext("Series:"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, row_num, 1, 1);

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_selectable(true);
      lab->set_max_width_chars(50);
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_text(bbe.bpe.book_series);
      lab->set_name("windowLabel");
      grid->attach(*lab, 1, row_num, 1, 1);
      row_num++;
    }

  if(bie)
    {
      if(!bie->language.empty())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_expand(true);
          lab->set_text(gettext("Language:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row_num, 1, 1);

          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_expand(true);
          lab->set_selectable(true);
          lab->set_text(Glib::ustring(bie->language));
          lab->set_name("windowLabel");
          grid->attach(*lab, 1, row_num, 1, 1);
          row_num++;
        }

      if(!bie->src_language.empty())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_expand(true);
          lab->set_text(gettext("Source language:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row_num, 1, 1);

          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_expand(true);
          lab->set_selectable(true);
          lab->set_text(Glib::ustring(bie->src_language));
          lab->set_name("windowLabel");
          grid->attach(*lab, 1, row_num, 1, 1);
          row_num++;
        }

      if(!bie->translator.empty())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_valign(Gtk::Align::START);
          lab->set_expand(true);
          lab->set_text(gettext("Translator(s):"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row_num, 1, 1);

          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_expand(true);
          lab->set_selectable(true);
          lab->set_max_width_chars(50);
          lab->set_wrap(true);
          lab->set_wrap_mode(Pango::WrapMode::WORD);
          lab->set_text(Glib::ustring(bie->translator));
          lab->set_name("windowLabel");
          grid->attach(*lab, 1, row_num, 1, 1);
          row_num++;
        }
    }
}

Gdk::Rectangle
BookInfoGui::screen_size()
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
BookInfoGui::cover_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                        int height)
{
  if(cover_buf)
    {
      Glib::RefPtr<Gdk::Pixbuf> l_cover
          = cover_buf->scale_simple(width, height, Gdk::InterpType::BILINEAR);
      Gdk::Cairo::set_source_pixbuf(cr, l_cover, 0, 0);
      cr->rectangle(0, 0, static_cast<double>(width),
                    static_cast<double>(height));
      cr->fill();
    }
}

void
BookInfoGui::formFileSection(const BookBaseEntry &bbe, Gtk::Grid *grid,
                             int &row_num)
{
  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + gettext("File")
                  + Glib::ustring("</b>"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_num, 2, 1);
  row_num++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_valign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_text(gettext("File path:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row_num, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_expand(true);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_max_width_chars(50);
  lab->set_selectable(true);
  lab->set_name("windowLabel");
  if(std::filesystem::exists(bbe.file_path))
    {
      lab->set_text(bbe.file_path.u8string());
    }
  else
    {
      lab->set_text(Glib::ustring(gettext("file not found")) + "\n"
                    + bbe.file_path.u8string());
    }
  grid->attach(*lab, 1, row_num, 1, 1);
  row_num++;

  std::string bpth = bbe.bpe.book_path;
  std::string::size_type n = 0;
  bool arch = false;
  if(!bpth.empty())
    {
      std::string sstr = "\n";
      for(;;)
        {
          n = bpth.find(sstr, n);
          if(n != std::string::npos)
            {
              arch = true;
              bpth.erase(bpth.begin() + n);
              bpth.insert(n, "\" \"");
            }
          else
            {
              break;
            }
        }
    }
  if(arch)
    {
      bpth = "\"" + bpth + "\"";
    }

  if(!bpth.empty())
    {
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_valign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_text(gettext("Path in archive:"));
      lab->set_name("windowLabel");
      grid->attach(*lab, 0, row_num, 1, 1);

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_max_width_chars(50);
      lab->set_selectable(true);
      lab->set_text(Glib::ustring(bpth));
      lab->set_name("windowLabel");
      grid->attach(*lab, 1, row_num, 1, 1);
    }
  row_num++;
}

void
BookInfoGui::formPaperBookInfoSection(const BookBaseEntry &, Gtk::Grid *grid,
                                      int &row_num)
{
  if(bie)
    {
      if(bie->paper->available)
        {
          Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::CENTER);
          lab->set_expand(true);
          lab->set_use_markup(true);
          lab->set_markup(Glib::ustring("<b>") + gettext("Paper book info")
                          + Glib::ustring("</b>"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row_num, 2, 1);
          row_num++;

          if(!bie->paper->book_name.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Book name:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->paper->book_name));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->paper->publisher.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Publisher:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->paper->publisher));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->paper->city.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("City:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->paper->city));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->paper->year.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Year:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->paper->year));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->paper->isbn.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text("ISBN:");
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->paper->isbn));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }
        }
    }
}

void
BookInfoGui::formEectordocInfoSection(const BookBaseEntry &, Gtk::Grid *grid,
                                      int &row_num)
{
  if(bie)
    {
      if(bie->electro->available)
        {
          Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::CENTER);
          lab->set_expand(true);
          lab->set_use_markup(true);
          lab->set_markup(Glib::ustring("<b>") + gettext("E-Book info")
                          + Glib::ustring("</b>"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row_num, 2, 1);
          row_num++;

          if(!bie->electro->author.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_valign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Author(s):"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_max_width_chars(50);
              lab->set_wrap(true);
              lab->set_wrap_mode(Pango::WrapMode::WORD);
              lab->set_text(Glib::ustring(bie->electro->author));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->program_used.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Software:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_max_width_chars(50);
              lab->set_wrap(true);
              lab->set_wrap_mode(Pango::WrapMode::WORD);
              lab->set_text(Glib::ustring(bie->electro->program_used));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->date.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Date:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->electro->date));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->src_url.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Source:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              std::string::size_type n = bie->electro->src_url.find("http://");
              if(n == std::string::npos)
                {
                  n = bie->electro->src_url.find("https://");
                }
              if(n != std::string::npos)
                {
                  std::string::size_type n_end;
                  n_end = bie->electro->src_url.find(" ");
                  std::string url;
                  if(n_end != std::string::npos)
                    {
                      url = std::string(bie->electro->src_url.begin() + n,
                                        bie->electro->src_url.begin() + n_end);
                    }
                  else
                    {
                      url = std::string(bie->electro->src_url.begin() + n,
                                        bie->electro->src_url.end());
                    }

                  Gtk::LinkButton *link = Gtk::make_managed<Gtk::LinkButton>();
                  link->set_margin(5);
                  link->set_halign(Gtk::Align::START);
                  link->set_uri(Glib::ustring(url));
                  link->set_label(Glib::ustring(url));
                  link->set_visited(false);
                  grid->attach(*link, 1, row_num, 1, 1);
                  row_num++;
                }
              else
                {
                  lab = Gtk::make_managed<Gtk::Label>();
                  lab->set_margin(5);
                  lab->set_halign(Gtk::Align::START);
                  lab->set_expand(true);
                  lab->set_selectable(true);
                  lab->set_text(Glib::ustring(bie->electro->src_url));
                  lab->set_name("windowLabel");
                  grid->attach(*lab, 1, row_num, 1, 1);
                  row_num++;
                }
            }

          if(!bie->electro->src_ocr.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_valign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Author of original doc:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_wrap(true);
              lab->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
              lab->set_max_width_chars(50);
              lab->set_text(Glib::ustring(bie->electro->src_ocr));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->id.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("ID:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_max_width_chars(50);
              lab->set_wrap(true);
              lab->set_wrap_mode(Pango::WrapMode::WORD);
              lab->set_text(Glib::ustring(bie->electro->id));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->version.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Version:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_text(Glib::ustring(bie->electro->version));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->history.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_valign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("History:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              std::string hist = bie->electro->history;
              FormatAnnotation fa(af);
              std::vector<ReplaceTagItem> tag_replacement_table;
              formReplacementTable(tag_replacement_table);
              fa.setTagReplacementTable(tag_replacement_table);
              fa.remove_escape_sequences(hist);
              fa.replace_tags(hist);
              fa.final_cleaning(hist);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_wrap(true);
              lab->set_wrap_mode(Pango::WrapMode::WORD);
              lab->set_max_width_chars(50);
              lab->set_use_markup(true);
              lab->set_markup(Glib::ustring(hist));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }

          if(!bie->electro->publisher.empty())
            {
              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_text(gettext("Publisher:"));
              lab->set_name("windowLabel");
              grid->attach(*lab, 0, row_num, 1, 1);

              lab = Gtk::make_managed<Gtk::Label>();
              lab->set_margin(5);
              lab->set_halign(Gtk::Align::START);
              lab->set_expand(true);
              lab->set_selectable(true);
              lab->set_wrap(true);
              lab->set_wrap_mode(Pango::WrapMode::WORD);
              lab->set_max_width_chars(50);
              lab->set_text(Glib::ustring(bie->electro->publisher));
              lab->set_name("windowLabel");
              grid->attach(*lab, 1, row_num, 1, 1);
              row_num++;
            }
        }
    }
}

void
BookInfoGui::cover_operations_action_group(Gtk::Window *win)
{
  Glib::RefPtr<Gio::SimpleActionGroup> cover_actions
      = Gio::SimpleActionGroup::create();

  cover_actions->add_action(
      "full_size", std::bind(&BookInfoGui::cover_full_size, this, win));

  cover_actions->add_action("save_cover",
                            std::bind(&BookInfoGui::save_cover, this, win));

  win->insert_action_group("cover_ops_info_gui", cover_actions);
}

Glib::RefPtr<Gio::Menu>
BookInfoGui::cover_menu()
{
  Glib::RefPtr<Gio::Menu> result = Gio::Menu::create();

  Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create(
      gettext("Show full size"), "cover_ops_info_gui.full_size");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Save cover"),
                               "cover_ops_info_gui.save_cover");
  result->append_item(item);

  return result;
}

void
BookInfoGui::show_cover_popup_menu(int, double x, double y,
                                   Gtk::PopoverMenu *pop_menu)
{
  if(bie)
    {
      if(!bie->cover.empty()
         && bie->cover_type != BookInfoEntry::cover_types::error)
        {
          Gdk::Rectangle rec(static_cast<int>(x), static_cast<int>(y), 1, 1);
          pop_menu->set_pointing_to(rec);
          pop_menu->popup();
        }
    }
}

void
BookInfoGui::cover_full_size(Gtk::Window *win)
{
  if(bie)
    {
      if(!bie->cover.empty()
         && bie->cover_type != BookInfoEntry::cover_types::error)
        {
          FullSizeCover *fsc = new FullSizeCover(bie, win);
          fsc->createWindow();
        }
    }
}

void
BookInfoGui::save_cover(Gtk::Window *win)
{
  if(bie)
    {
      if(!bie->cover.empty()
         && bie->cover_type != BookInfoEntry::cover_types::error)
        {
          SaveCover *sc = new SaveCover(bie, win);
          sc->createWindow();
        }
    }
}

void
BookInfoGui::formReplacementTable(
    std::vector<ReplaceTagItem> &replacement_table)
{
  ReplaceTagItem tag;

  tag.tag_to_replace = "p";
  tag.begin_replacement = "  ";
  tag.end_replacement = "\n";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "empty-line";
  tag.begin_replacement = "";
  tag.end_replacement = "\n\n  ";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "sub";
  tag.begin_replacement = "<span rise=\"-5pt\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "sup";
  tag.begin_replacement = "<span rise=\"5pt\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "strong";
  tag.begin_replacement = "<span font_weight=\"bold\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "emphasis";
  tag.begin_replacement = "<span font_style=\"italic\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "strikethrough";
  tag.begin_replacement = "<span strikethrough=\"true\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);
}
