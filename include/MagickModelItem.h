/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef MAGICKMODELITEM_H
#define MAGICKMODELITEM_H

#include <Magick++.h>
#include <glibmm-2.68/glibmm/object.h>

class MagickModelItem : public Glib::Object
{
public:
  static Glib::RefPtr<MagickModelItem>
  create(const Magick::CoderInfo &info);

  Magick::CoderInfo info;

protected:
  MagickModelItem(const Magick::CoderInfo &info);
};

#endif // MAGICKMODELITEM_H
