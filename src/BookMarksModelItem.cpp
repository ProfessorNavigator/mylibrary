/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <BookMarksModelItem.h>

Glib::RefPtr<BookMarksModelItem>
BookMarksModelItem::create(const std::string &coll_name,
                           const BookBaseEntry &bbe)
{
  BookMarksModelItem *item = new BookMarksModelItem(coll_name, bbe);
  return Glib::make_refptr_for_instance(item);
}

BookMarksModelItem::BookMarksModelItem(const std::string &coll_name,
                                       const BookBaseEntry &bbe)
{
  element = std::make_tuple(coll_name, bbe);
}
