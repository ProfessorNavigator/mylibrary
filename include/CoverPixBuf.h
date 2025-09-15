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

#ifndef COVERPIXBUF_H
#define COVERPIXBUF_H

#include <BookInfoEntry.h>
#include <FormatAnnotation.h>
#include <Magick++.h>
#include <cairomm-1.16/cairomm/surface.h>
#include <filesystem>
#include <memory>

class CoverPixBuf
{
public:
  CoverPixBuf();

  CoverPixBuf(const std::shared_ptr<BookInfoEntry> &bie,
              FormatAnnotation *formatter = nullptr);

  CoverPixBuf(const CoverPixBuf &other);

  CoverPixBuf(CoverPixBuf &&other);

  CoverPixBuf &
  operator=(const CoverPixBuf &other);

  CoverPixBuf &
  operator=(CoverPixBuf &&other);

  operator bool();

  void
  setImage(const std::shared_ptr<BookInfoEntry> &bie,
           FormatAnnotation *formatter);

  size_t
  getWidth();

  size_t
  getHeight();

  Cairo::RefPtr<Cairo::ImageSurface>
  getSurface(const int &width, const int &height);

  Cairo::RefPtr<Cairo::ImageSurface>
  getSurface();

  bool
  saveImage(const std::filesystem::path &p,
            const std::string &format = std::string());

private:
  void
  createImage(const std::shared_ptr<BookInfoEntry> &bie);

  void
  createImageFromBlob(const Magick::Blob &blob);

  void
  createImageFromText(const std::shared_ptr<BookInfoEntry> &bie);

  Magick::Image image;

  FormatAnnotation *formatter = nullptr;
};

#endif // COVERPIXBUF_H
