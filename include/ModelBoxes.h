/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of MyLibrary.
 MyLibrary is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 MyLibrary is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with MyLibrary. If not,
 see <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_MODELBOXES_H_
#define INCLUDE_MODELBOXES_H_

#include <gtkmm.h>

class ModelBoxes : public Glib::Object
{
public:
  virtual
  ~ModelBoxes();
  Glib::ustring menu_line;
  static Glib::RefPtr<ModelBoxes>
  create(std::string menu_line);
protected:
  ModelBoxes(std::string &menu_line);
};

#endif /* INCLUDE_MODELBOXES_H_ */
