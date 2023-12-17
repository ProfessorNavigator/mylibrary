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

#include <ModelColumns.h>

ModelColumns::ModelColumns(std::string &author, std::string &book,
			   std::string &series, std::string &genre,
			   std::string &date, std::string &path)
{
  this->author = Glib::ustring(author);
  this->book = Glib::ustring(book);
  this->series = series;
  this->genre = genre;
  this->date = date;
  this->path = path;
}

ModelColumns::~ModelColumns()
{

}

Glib::RefPtr<ModelColumns>
ModelColumns::create(std::string author, std::string book, std::string series,
		     std::string genre, std::string date, std::string path)
{
  return Glib::make_refptr_for_instance<ModelColumns>(
      new ModelColumns(author, book, series, genre, date, path));
}
