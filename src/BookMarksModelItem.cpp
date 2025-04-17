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

BookMarksModelItem::~BookMarksModelItem()
{
#ifdef USE_OPENMP
  omp_destroy_lock(&labels_mtx);
#endif
}

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
#ifndef USE_OPENMP
  labels_mtx.lock();
#else
  omp_set_lock(&labels_mtx);
#endif
  auto it = std::find(labels.begin(), labels.end(), lab);
  if(it == labels.end())
    {
      if(labels.capacity() == 0)
        {
          labels.reserve(6);
        }
      labels.push_back(lab);
    }
#ifndef USE_OPENMP
  labels_mtx.unlock();
#else
  omp_unset_lock(&labels_mtx);
#endif
}

void
BookMarksModelItem::removeLabel(Gtk::Label *lab)
{
#ifndef USE_OPENMP
  labels_mtx.lock();
#else
  omp_set_lock(&labels_mtx);
#endif
  auto it = std::find(labels.begin(), labels.end(), lab);
  if(it != labels.end())
    {
      labels.erase(it);
      if(labels.size() == 0)
        {
          labels.shrink_to_fit();
        }
    }
#ifndef USE_OPENMP
  labels_mtx.unlock();
#else
  omp_unset_lock(&labels_mtx);
#endif
}

void
BookMarksModelItem::activateLabels()
{
#ifndef USE_OPENMP
  labels_mtx.lock();
#else
  omp_set_lock(&labels_mtx);
#endif
  for(size_t i = 0; i < labels.size(); i++)
    {
      labels[i]->set_name("selectedLab");
    }
#ifndef USE_OPENMP
  labels_mtx.unlock();
#else
  omp_unset_lock(&labels_mtx);
#endif
}

void
BookMarksModelItem::deactivateLabels()
{
#ifndef USE_OPENMP
  labels_mtx.lock();
#else
  omp_set_lock(&labels_mtx);
#endif
  for(size_t i = 0; i < labels.size(); i++)
    {
      labels[i]->set_name("windowLabel");
    }
#ifndef USE_OPENMP
  labels_mtx.unlock();
#else
  omp_unset_lock(&labels_mtx);
#endif
}

BookMarksModelItem::BookMarksModelItem(const std::string &coll_name,
                                       const BookBaseEntry &bbe)
{
#ifdef USE_OPENMP
  omp_init_lock(&labels_mtx);
#endif
  element = std::make_tuple(coll_name, bbe);
}
