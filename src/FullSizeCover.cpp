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

#include <CoverPixBuf.h>
#include <FullSizeCover.h>
#include <functional>
#include <gdkmm/display.h>
#include <gdkmm/general.h>
#include <gdkmm/monitor.h>
#include <gdkmm/rectangle.h>
#include <gdkmm/surface.h>
#include <glibmm/signalproxy.h>
#include <gtkmm/application.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/enums.h>
#include <gtkmm/object.h>
#include <gtkmm/scrolledwindow.h>
#include <libintl.h>
#include <sigc++/connection.h>

FullSizeCover::FullSizeCover(const std::shared_ptr<BookInfoEntry> &bie,
                             Gtk::Window *parent_window)
{
  this->bie = bie;
  this->parent_window = parent_window;
  CoverPixBuf cpb(bie);
  cover_buf = cpb;
  if(cover_buf)
    {
      calculateSizes();
    }
}

void
FullSizeCover::createWindow()
{
  std::shared_ptr<FullSizeCover> fszc;
  if(cover_buf)
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(parent_window->get_application());
      window->set_title(gettext("Cover"));
      window->set_transient_for(*parent_window);
      window->set_modal(true);

      Gtk::ScrolledWindow *cover_scrl
          = Gtk::make_managed<Gtk::ScrolledWindow>();
      cover_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                             Gtk::PolicyType::AUTOMATIC);
      cover_scrl->set_halign(Gtk::Align::FILL);
      cover_scrl->set_valign(Gtk::Align::FILL);
      cover_scrl->set_expand(true);
      window->set_child(*cover_scrl);

      if(width >= cover_buf->get_width())
        {
          cover_scrl->set_min_content_width(cover_buf->get_width());
        }
      else
        {
          cover_scrl->set_min_content_width(width);
        }

      if(height >= cover_buf->get_height())
        {
          cover_scrl->set_min_content_height(cover_buf->get_height());
        }
      else
        {
          cover_scrl->set_min_content_height(height);
        }

      Gtk::DrawingArea *cover = Gtk::make_managed<Gtk::DrawingArea>();
      cover->set_content_height(cover_buf->get_height());
      cover->set_content_width(cover_buf->get_width());
      cover->set_draw_func(
          std::bind(&FullSizeCover::cover_draw, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3));
      cover_scrl->set_child(*cover);

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
  else
    {
      fszc = std::shared_ptr<FullSizeCover>(this);
    }
}

void
FullSizeCover::calculateSizes()
{
  Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
  Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);

  req.set_width(req.get_width() * mon->get_scale_factor());
  req.set_height(req.get_height() * mon->get_scale_factor() * 0.9);

  if(cover_buf->get_width() >= req.get_width())
    {
      width = req.get_width();
    }
  else
    {
      width = cover_buf->get_width();
    }

  if(cover_buf->get_height() >= req.get_height())
    {
      height = req.get_height();
    }
  else
    {
      height = cover_buf->get_height();
    }
}

void
FullSizeCover::cover_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                          int height)
{
  Glib::RefPtr<Gdk::Pixbuf> l_cover
      = cover_buf->scale_simple(width, height, Gdk::InterpType::BILINEAR);
  Gdk::Cairo::set_source_pixbuf(cr, l_cover, 0, 0);
  cr->rectangle(0, 0, static_cast<double>(width), static_cast<double>(height));
  cr->fill();
}
