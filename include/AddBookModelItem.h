/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_ADDBOOKMODELITEM_H_
#define INCLUDE_ADDBOOKMODELITEM_H_

#include <glibmm-2.68/glibmm/refptr.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <string>

class AddBookModelItem : public Glib::Object
{
public:
  virtual
  ~AddBookModelItem();

  static
  Glib::RefPtr<AddBookModelItem>
  create(const std::string &source_path);

  std::string source_path;
  std::string collection_path;
  bool correct = true;
  bool out_of_col = false;
  bool conflict_names = false;

protected:
  AddBookModelItem(const std::string &source_path);
};

#endif /* INCLUDE_ADDBOOKMODELITEM_H_ */
