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
#include <NotesBaseEntry.h>

NotesBaseEntry::NotesBaseEntry()
{
}

NotesBaseEntry::NotesBaseEntry(
    const std::string &collection_name,
    const std::filesystem::path &book_file_full_path,
    const std::string &book_path)
{
  this->collection_name = collection_name;
  this->book_file_full_path = book_file_full_path;
  this->book_path = book_path;
}

NotesBaseEntry::NotesBaseEntry(const NotesBaseEntry &other)
{
  collection_name = other.collection_name;
  book_file_full_path = other.book_file_full_path;
  book_path = other.book_path;
  note_file_full_path = other.note_file_full_path;
}

NotesBaseEntry::NotesBaseEntry(NotesBaseEntry &&other)
{
  collection_name = std::move(other.collection_name);
  book_file_full_path = std::move(other.book_file_full_path);
  book_path = std::move(other.book_path);
  note_file_full_path = std::move(other.note_file_full_path);
}

NotesBaseEntry &
NotesBaseEntry::operator=(const NotesBaseEntry &other)
{
  if(this != &other)
    {
      collection_name = other.collection_name;
      book_file_full_path = other.book_file_full_path;
      book_path = other.book_path;
      note_file_full_path = other.note_file_full_path;
    }

  return *this;
}

bool
NotesBaseEntry::operator==(const NotesBaseEntry &other) const
{
  if(collection_name == other.collection_name)
    {
      if(book_file_full_path == other.book_file_full_path)
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
      else
        {
          return false;
        }
    }
  else
    {
      return false;
    }
}

NotesBaseEntry &
NotesBaseEntry::operator=(NotesBaseEntry &&other)
{
  if(this != &other)
    {
      collection_name = std::move(other.collection_name);
      book_file_full_path = std::move(other.book_file_full_path);
      book_path = std::move(other.book_path);
      note_file_full_path = std::move(other.note_file_full_path);
    }

  return *this;
}
