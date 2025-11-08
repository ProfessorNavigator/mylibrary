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

#include <CoverPixBuf.h>
#include <cairomm-1.16/cairomm/context.h>
#include <cairomm-1.16/cairomm/surface.h>
#include <fstream>
#include <giomm-2.68/giomm/listmodel.h>
#include <gtkmm-4.0/gdkmm/display.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <iostream>
#include <pangomm-2.48/pangomm/layout.h>
#include <string>

CoverPixBuf::CoverPixBuf()
{
}

CoverPixBuf::CoverPixBuf(const std::shared_ptr<BookInfoEntry> &bie,
                         FormatAnnotation *formatter)
{
  this->formatter = formatter;
  createImage(bie);
}

CoverPixBuf::CoverPixBuf(const CoverPixBuf &other)
{
  image = other.image;
  formatter = other.formatter;
}

CoverPixBuf::CoverPixBuf(CoverPixBuf &&other)
{
  image = std::move(other.image);
  formatter = other.formatter;
  other.formatter = nullptr;
}

CoverPixBuf &
CoverPixBuf::operator=(const CoverPixBuf &other)
{
  if(this != &other)
    {
      image = other.image;
      formatter = other.formatter;
    }
  return *this;
}

CoverPixBuf::
operator bool()
{
  if(image.columns() > 0 && image.rows() > 0)
    {
      return true;
    }
  else
    {
      return false;
    }
}

void
CoverPixBuf::setImage(const std::shared_ptr<BookInfoEntry> &bie,
                      FormatAnnotation *formatter)
{
  this->formatter = formatter;
  createImage(bie);
}

CoverPixBuf &
CoverPixBuf::operator=(CoverPixBuf &&other)
{
  if(this != &other)
    {
      image = std::move(other.image);
      formatter = other.formatter;
      other.formatter = nullptr;
    }
  return *this;
}

void
CoverPixBuf::createImage(const std::shared_ptr<BookInfoEntry> &bie)
{
  if(bie)
    {
      if(bie->cover.size() > 0
         && bie->cover_type != BookInfoEntry::cover_types::error)
        {

          switch(bie->cover_type)
            {
            case BookInfoEntry::cover_types::base64:
              {
                Magick::Blob blob;
                blob.base64(bie->cover);
                createImageFromBlob(blob);
                break;
              }
            case BookInfoEntry::cover_types::rgb:
              {
                try
                  {
                    image.read(static_cast<size_t>(bie->bytes_per_row / 3),
                               bie->cover.size()
                                   / static_cast<size_t>(bie->bytes_per_row),
                               "RGB", Magick::CharPixel, bie->cover.c_str());
                  }
                catch(Magick::Exception &er)
                  {
                    std::cout
                        << "CoverPixBuf::createImage (rgb): " << er.what()
                        << std::endl;
                  }

                break;
              }
            case BookInfoEntry::cover_types::rgba:
              {
                try
                  {
                    image.read(static_cast<size_t>(bie->bytes_per_row * 0.25),
                               bie->cover.size()
                                   / static_cast<size_t>(bie->bytes_per_row),
                               "RGBA", Magick::CharPixel, bie->cover.c_str());
                  }
                catch(Magick::Exception &er)
                  {
                    std::cout
                        << "CoverPixBuf::createImage (rgba): " << er.what()
                        << std::endl;
                  }
                break;
              }
            case BookInfoEntry::cover_types::bgra:
              {
                try
                  {
                    image.read(static_cast<size_t>(bie->bytes_per_row * 0.25),
                               bie->cover.size()
                                   / static_cast<size_t>(bie->bytes_per_row),
                               "BGRA", Magick::CharPixel, bie->cover.c_str());
                  }
                catch(Magick::Exception &er)
                  {
                    std::cout
                        << "CoverPixBuf::createImage (bgra): " << er.what()
                        << std::endl;
                  }
                break;
              }
            case BookInfoEntry::cover_types::file:
              {
                Magick::Blob blob;
                blob.update(bie->cover.c_str(), bie->cover.size());
                createImageFromBlob(blob);
                break;
              }
            case BookInfoEntry::cover_types::text:
              {
                createImageFromText(bie);
                break;
              }
            default:
              break;
            }
        }
    }
}

void
CoverPixBuf::createImageFromBlob(const Magick::Blob &blob)
{
  try
    {
      image.read(blob);
    }
  catch(Magick::Exception &er)
    {
      std::cout << "CoverPixBuf::createImageFromBlob: " << er.what()
                << std::endl;
    }
}

void
CoverPixBuf::createImageFromText(const std::shared_ptr<BookInfoEntry> &bie)
{
  Glib::RefPtr<Gdk::Display> disp = Gdk::Display::get_default();
  Glib::RefPtr<Gio::ListModel> monitor_list = disp->get_monitors();
  Gdk::Rectangle rect;
  rect.set_width(0);
  rect.set_height(0);
  Glib::RefPtr<Gdk::Monitor> selected_monitor;
  for(guint i = 0; i < monitor_list->get_n_items(); i++)
    {
      Glib::RefPtr<Gdk::Monitor> monitor
          = std::dynamic_pointer_cast<Gdk::Monitor>(
              monitor_list->get_object(i));
      if(monitor)
        {
          Gdk::Rectangle l_rect;
          monitor->get_geometry(l_rect);
          if(l_rect.get_height() > rect.get_height()
             && l_rect.get_width() > rect.get_width())
            {
              rect = l_rect;
              selected_monitor = monitor;
            }
        }
    }

  double x_coef = static_cast<double>(rect.get_width())
                  / static_cast<double>(selected_monitor->get_width_mm());
  double y_coef = static_cast<double>(rect.get_height())
                  / static_cast<double>(selected_monitor->get_height_mm());
  int width = 210 * x_coef;
  int height = 297 * y_coef;

  Cairo::RefPtr<Cairo::ImageSurface> surf = Cairo::ImageSurface::create(
      Cairo::ImageSurface::Format::ARGB32, width, height);

  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surf);
  cr->set_source_rgb(1.0, 1.0, 1.0);
  cr->rectangle(0.0, 0.0, static_cast<double>(width),
                static_cast<double>(height));
  cr->fill();

  cr->set_source_rgb(0.0, 0.0, 0.0);

  Glib::RefPtr<Pango::Layout> pl = Pango::Layout::create(cr);

  std::string buf = bie->cover;

  if(formatter)
    {
      formatter->removeEscapeSequences(buf);
      std::string reserve_annot = buf;
      formatter->replaceTags(buf);
      formatter->finalCleaning(buf);

      pl->set_markup(Glib::ustring(buf));
      if(pl->get_text().size() == 0)
        {
          formatter->removeAllTags(reserve_annot);
          formatter->finalCleaning(reserve_annot);
          pl->set_text(Glib::ustring(reserve_annot));
        }
    }
  else
    {
      pl->set_text(Glib::ustring(bie->cover));
    }

  pl->set_width(width * 0.9 * Pango::SCALE);
  pl->set_height(height * 0.9 * Pango::SCALE);
  pl->set_justify(true);

  cr->move_to(width * 0.05, height * 0.05);

  pl->show_in_cairo_context(cr);

  std::string result;
  surf->write_to_png_stream(
      [&result](const unsigned char *data, unsigned int length)
        {
          for(unsigned int i = 0; i < length; i++)
            {
              result.push_back(data[i]);
            }
          return CAIRO_STATUS_SUCCESS;
        });
  Magick::Blob blob(result.c_str(), result.size());
  createImageFromBlob(blob);
}

size_t
CoverPixBuf::getWidth()
{
  return image.columns();
}

size_t
CoverPixBuf::getHeight()
{
  return image.rows();
}

Cairo::RefPtr<Cairo::ImageSurface>
CoverPixBuf::getSurface(const int &width, const int &height)
{
  Cairo::RefPtr<Cairo::ImageSurface> result;

  if(image.columns() > 0 && image.rows() > 0 && width > 0 && height > 0)
    {
      Magick::Image img = image;
      size_t w = static_cast<size_t>(width);
      if(w < img.columns())
        {
          double coef
              = static_cast<double>(w) / static_cast<double>(img.columns());
          size_t lh = img.rows() * coef;
          img.scale(Magick::Geometry(w, lh));
        }
      size_t h = static_cast<size_t>(height);
      if(h < img.rows())
        {
          double coef
              = static_cast<double>(h) / static_cast<double>(img.rows());
          size_t lw = img.columns() * coef;
          img.scale(Magick::Geometry(lw, h));
        }

      result = Cairo::ImageSurface::create(Cairo::ImageSurface::Format::RGB24,
                                           static_cast<int>(img.columns()),
                                           static_cast<int>(img.rows()));
      img.write(0, 0, img.columns(), img.rows(), "BGRA", Magick::CharPixel,
                result->get_data());
    }

  return result;
}

Cairo::RefPtr<Cairo::ImageSurface>
CoverPixBuf::getSurface()
{
  Cairo::RefPtr<Cairo::ImageSurface> result = Cairo::ImageSurface::create(
      Cairo::ImageSurface::Format::RGB24, static_cast<int>(image.columns()),
      static_cast<int>(image.rows()));
  image.write(0, 0, image.columns(), image.rows(), "BGRA", Magick::CharPixel,
              result->get_data());
  return result;
}

bool
CoverPixBuf::saveImage(const std::filesystem::path &p,
                       const std::string &format)
{
  bool result = false;
  if(p.empty())
    {
      return result;
    }

  if(image.columns() == 0 || image.rows() == 0)
    {
      return result;
    }

  std::filesystem::remove_all(p);

  try
    {
      if(format.empty())
        {
          image.write(p.string());
          result = true;
        }
      else
        {
          Magick::Blob blob;
          image.write(&blob, format);
          const char *src = reinterpret_cast<const char *>(blob.data());
          std::fstream f;
          f.open(p, std::ios_base::out | std::ios_base::binary);
          if(f.is_open())
            {
              f.write(src, blob.length());
              f.close();

              result = true;
            }
        }
    }
  catch(Magick::Exception &er)
    {
      std::cout << "CoverPixBuf::saveImage: " << er.what() << std::endl;
    }

  return result;
}

void
CoverPixBuf::clearImage()
{
  image = Magick::Image();
}
