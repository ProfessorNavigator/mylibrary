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

#include <BookBaseEntry.h>

BookBaseEntry::BookBaseEntry()
{

}

BookBaseEntry::BookBaseEntry(const BookBaseEntry &other)
{
  file_path = other.file_path;
  bpe = other.bpe;
}

BookBaseEntry&
BookBaseEntry::operator =(const BookBaseEntry &other)
{
  if(this != &other)
    {
      file_path = other.file_path;
      bpe = other.bpe;
    }
  return *this;
}

BookBaseEntry::BookBaseEntry(BookBaseEntry &&other)
{
  file_path = std::move(other.file_path);
  bpe = std::move(other.bpe);
}

BookBaseEntry&
BookBaseEntry::operator =(BookBaseEntry &&other)
{
  if(this != &other)
    {
      file_path = std::move(other.file_path);
      bpe = std::move(other.bpe);
    }
  return *this;
}

BookBaseEntry::BookBaseEntry(
    const BookParseEntry &bpe,
    const std::filesystem::path &book_file_path)
{
  file_path = book_file_path;
  this->bpe = bpe;
}

bool
BookBaseEntry::operator ==(const BookBaseEntry &other)
{
  if(file_path == other.file_path && bpe == other.bpe)
    {
      return true;
    }
  else
    {
      return false;
    }
}
