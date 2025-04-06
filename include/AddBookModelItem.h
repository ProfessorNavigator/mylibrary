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

#ifndef ADDBOOKMODELITEM_H
#define ADDBOOKMODELITEM_H

#include <glibmm-2.68/glibmm/object.h>
#include <string>

class AddBookModelItem : public Glib::Object
{
public:
  static Glib::RefPtr<AddBookModelItem>
  create(const std::string &source_path);

  std::string source_path;
  std::string collection_path;
  bool correct = true;
  bool out_of_col = false;
  bool conflict_names = false;

protected:
  AddBookModelItem(const std::string &source_path);
};

#endif // ADDBOOKMODELITEM_H
