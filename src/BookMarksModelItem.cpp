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
#include <BookMarksModelItem.h>

Glib::RefPtr<BookMarksModelItem>
BookMarksModelItem::create(const std::string &coll_name,
                           const BookBaseEntry &bbe)
{
  BookMarksModelItem *item = new BookMarksModelItem(coll_name, bbe);
  return Glib::make_refptr_for_instance(item);
}

void
BookMarksModelItem::addLabel(Gtk::Label *lab)
{
  labels_mtx.lock();
  auto it = std::find(labels.begin(), labels.end(), lab);
  if(it == labels.end())
    {
      if(labels.capacity() == 0)
        {
          labels.reserve(6);
        }
      labels.push_back(lab);
    }
  labels_mtx.unlock();
}

void
BookMarksModelItem::removeLabel(Gtk::Label *lab)
{
  labels_mtx.lock();
  auto it = std::find(labels.begin(), labels.end(), lab);
  if(it != labels.end())
    {
      labels.erase(it);
      if(labels.size() == 0)
        {
          labels.shrink_to_fit();
        }
    }
  labels_mtx.unlock();
}

void
BookMarksModelItem::activateLabels()
{
  labels_mtx.lock();
  for(size_t i = 0; i < labels.size(); i++)
    {
      labels[i]->set_name("selectedLab");
    }
  labels_mtx.unlock();
}

void
BookMarksModelItem::deactivateLabels()
{
  labels_mtx.lock();
  for(size_t i = 0; i < labels.size(); i++)
    {
      labels[i]->set_name("windowLabel");
    }
  labels_mtx.unlock();
}

BookMarksModelItem::BookMarksModelItem(const std::string &coll_name,
                                       const BookBaseEntry &bbe)
{
  element = std::make_tuple(coll_name, bbe);
}
