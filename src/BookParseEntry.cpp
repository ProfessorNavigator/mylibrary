/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <BookParseEntry.h>

BookParseEntry::BookParseEntry()
{

}

BookParseEntry::BookParseEntry(const BookParseEntry &other)
{
  if(this != &other)
    {
      book_path = other.book_path;
      book_author = other.book_author;
      book_name = other.book_name;
      book_series = other.book_series;
      book_genre = other.book_genre;
      book_date = other.book_date;
    }
}

BookParseEntry&
BookParseEntry::operator =(const BookParseEntry &other)
{
  if(this != &other)
    {
      book_path = other.book_path;
      book_author = other.book_author;
      book_name = other.book_name;
      book_series = other.book_series;
      book_genre = other.book_genre;
      book_date = other.book_date;
    }
  return *this;
}

bool
BookParseEntry::operator ==(const BookParseEntry &other)
{
  if(book_path == other.book_path)
    {
      return true;
    }
  else
    {
      return false;
    }
}

BookParseEntry::BookParseEntry(BookParseEntry &&other)
{
  if(this != &other)
    {
      book_path = other.book_path;
      book_author = other.book_author;
      book_name = other.book_name;
      book_series = other.book_series;
      book_genre = other.book_genre;
      book_date = other.book_date;
    }
}

BookParseEntry&
BookParseEntry::operator =(BookParseEntry &&other)
{
  if(this != &other)
    {
      book_path = other.book_path;
      book_author = other.book_author;
      book_name = other.book_name;
      book_series = other.book_series;
      book_genre = other.book_genre;
      book_date = other.book_date;
    }
  return *this;
}
