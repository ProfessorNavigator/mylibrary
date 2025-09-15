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

#include <FullSizeCover.h>
#include <functional>
#include <gtkmm-4.0/gdkmm/display.h>
#include <gtkmm-4.0/gdkmm/general.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gdkmm/surface.h>
#include <gtkmm-4.0/gtkmm/drawingarea.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <libintl.h>

FullSizeCover::FullSizeCover(Gtk::Window *parent_window,
                             const CoverPixBuf &cover_buf)
{
  this->parent_window = parent_window;
  pb = cover_buf;

  createWindow();
}

void
FullSizeCover::createWindow()
{
  this->set_application(parent_window->get_application());
  this->set_title(gettext("Cover"));
  this->set_transient_for(*parent_window);
  this->set_modal(true);

  Gdk::Rectangle rec = screenSize();
  int width = rec.get_width() * 0.9;
  int height = rec.get_height() * 0.9;

  if(width < static_cast<int>(pb.getWidth())
     || height < static_cast<int>(pb.getHeight()))
    {
      this->maximize();
      this->signal_realize().connect([this, width, height] {
        Glib::PropertyProxy<bool> max = this->property_maximized();
        if(!max.get_value())
          {
            this->set_default_size(width, height);
          }
      });
    }
  else
    {
      this->set_default_size(static_cast<int>(pb.getWidth()),
                             static_cast<int>(pb.getHeight()));
    }

  Gtk::ScrolledWindow *cover_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  cover_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                         Gtk::PolicyType::AUTOMATIC);
  cover_scrl->set_halign(Gtk::Align::FILL);
  cover_scrl->set_valign(Gtk::Align::FILL);
  cover_scrl->set_expand(true);
  this->set_child(*cover_scrl);

  Gtk::DrawingArea *cover = Gtk::make_managed<Gtk::DrawingArea>();
  cover->set_content_height(static_cast<int>(pb.getHeight()));
  cover->set_content_width(static_cast<int>(pb.getWidth()));
  cover->set_draw_func(std::bind(&FullSizeCover::coverDraw, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
  cover_scrl->set_child(*cover);
}

void
FullSizeCover::coverDraw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                         int height)
{
  Cairo::RefPtr<Cairo::ImageSurface> surf = pb.getSurface();
  if(surf)
    {
      double x = 0.0;
      if(width > surf->get_width())
        {
          x = static_cast<double>(width - surf->get_width()) * 0.5;
        }
      double y = 0.0;
      if(height > surf->get_height())
        {
          y = static_cast<double>(height - surf->get_height()) * 0.5;
        }

      cr->set_source(surf, x, y);
      cr->paint();
    }
}

Gdk::Rectangle
FullSizeCover::screenSize()
{
  Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
  Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle rec;
  mon->get_geometry(rec);

  rec.set_width(rec.get_width() * mon->get_scale_factor());
  rec.set_height(rec.get_height() * mon->get_scale_factor());

  return rec;
}
