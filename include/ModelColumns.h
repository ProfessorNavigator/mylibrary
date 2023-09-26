/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_MODELCOLUMNS_H_
#define INCLUDE_MODELCOLUMNS_H_

#include <gtkmm.h>

class ModelColumns : public Glib::Object
{
public:
  Glib::ustring author;
  Glib::ustring book;
  Glib::ustring series;
  Glib::ustring genre;
  Glib::ustring date;
  Glib::ustring path;

  static Glib::RefPtr<ModelColumns>
  create(std::string author, std::string book, std::string series,
	 std::string genre, std::string date, std::string path);
  virtual
  ~ModelColumns();
protected:
  ModelColumns(std::string &author, std::string &book, std::string &series,
	       std::string &genre, std::string &date, std::string &path);
};

#endif /* INCLUDE_MODELCOLUMNS_H_ */
