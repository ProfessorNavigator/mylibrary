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
#include <giomm-2.68/giomm/memoryinputstream.h>
#include <glibmm-2.68/glibmm/base64.h>
#include <iostream>
#include <string>

CoverPixBuf::CoverPixBuf(const std::shared_ptr<BookInfoEntry> &bie,
                         const int &width, const int &height)
{
  this->bie = bie;
  this->width = width;
  this->height = height;
  createBuffer();
}

CoverPixBuf::CoverPixBuf(const std::shared_ptr<BookInfoEntry> &bie)
{
  this->bie = bie;
  createBuffer();
}

CoverPixBuf::CoverPixBuf(const CoverPixBuf &other)
{
  bie = other.bie;
  buffer = other.buffer;
  width = other.width;
  height = other.height;
}

CoverPixBuf::CoverPixBuf(CoverPixBuf &&other)
{
  bie = std::move(other.bie);
  buffer = std::move(other.buffer);
  width = std::move(other.width);
  height = std::move(other.height);
}

CoverPixBuf &
CoverPixBuf::operator=(const CoverPixBuf &other)
{
  bie = other.bie;
  buffer = other.buffer;
  width = other.width;
  height = other.height;
  return *this;
}

CoverPixBuf &
CoverPixBuf::operator=(CoverPixBuf &&other)
{
  if(this != &other)
    {
      bie = std::move(other.bie);
      buffer = std::move(other.buffer);
      width = std::move(other.width);
      height = std::move(other.height);
    }
  return *this;
}

void
CoverPixBuf::createBuffer()
{
  if(bie)
    {
      switch(bie->cover_type)
        {
        case BookInfoEntry::cover_types::base64:
          {
            std::string base64 = Glib::Base64::decode(bie->cover);
            Glib::RefPtr<Glib::Bytes> bytes
                = Glib::Bytes::create(base64.c_str(), base64.size());
            createFromStream(bytes);
            break;
          }
        case BookInfoEntry::cover_types::file:
          {
            Glib::RefPtr<Glib::Bytes> bytes
                = Glib::Bytes::create(bie->cover.c_str(), bie->cover.size());
            createFromStream(bytes);
            break;
          }
        case BookInfoEntry::cover_types::rgb:
          {
            createFromRgba(false);
            break;
          }
        case BookInfoEntry::cover_types::rgba:
          {
            createFromRgba(true);
            break;
          }
        default:
          break;
        }      
    }
}

void
CoverPixBuf::createFromStream(const Glib::RefPtr<Glib::Bytes> &bytes)
{
  std::string base64 = Glib::Base64::decode(bie->cover);
  Glib::RefPtr<Gio::MemoryInputStream> strm = Gio::MemoryInputStream::create();
  strm->add_bytes(bytes);
  try
    {
      if(width > 0 && height > 0)
        {
          buffer = Gdk::Pixbuf::create_from_stream_at_scale(strm, width,
                                                            height, true);
        }
      else
        {
          buffer = Gdk::Pixbuf::create_from_stream(strm);
          if(buffer)
            {
              width = buffer->get_width();
              height = buffer->get_height();
            }
        }
    }
  catch(Glib::Error &er)
    {
      std::cout << "CoverPixBuf::createFromStream: " << er.what() << std::endl;
    }
}

CoverPixBuf::operator Glib::RefPtr<Gdk::Pixbuf>()
{
  return buffer;
}

int
CoverPixBuf::get_width()
{
  return width;
}

int
CoverPixBuf::get_height()
{
  return height;
}

void
CoverPixBuf::createFromRgba(const bool &alpha)
{
  int ih = static_cast<int>(bie->cover.size()) / bie->bytes_per_row;
  int iw;
  if(!alpha)
    {
      iw = bie->bytes_per_row / 3;
    }
  else
    {
      iw = bie->bytes_per_row / 4;
    }

  try
    {
      buffer = Gdk::Pixbuf::create_from_data(
          reinterpret_cast<const guint8 *>(bie->cover.c_str()),
          Gdk::Colorspace::RGB, alpha, 8, iw, ih, bie->bytes_per_row);
    }
  catch(Glib::Error &er)
    {
      std::cout << "CoverPixBuf::createFromRgba(" << bie->cover_type
                << ") error: " << er.what() << std::endl;
    }

  if(buffer)
    {
      if(width > 0 && height > 0)
        {
          if(width >= height)
            {
              height = width * ih / iw;
              width = height * iw / ih;
            }
          else
            {
              width = height * iw / ih;
              height = width * ih / iw;
            }
          buffer
              = buffer->scale_simple(width, height, Gdk::InterpType::BILINEAR);
        }
      else
        {
          width = buffer->get_width();
          height = buffer->get_height();
        }
    }
}
