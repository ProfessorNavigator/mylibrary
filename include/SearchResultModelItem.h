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

#ifndef SEARCHRESULTMODELITEM_H
#define SEARCHRESULTMODELITEM_H

#include <BookBaseEntry.h>
#include <glibmm-2.68/glibmm/object.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <mutex>
#endif

class SearchResultModelItem : public Glib::Object
{
public:
  virtual ~SearchResultModelItem();

  static Glib::RefPtr<SearchResultModelItem>
  create(const BookBaseEntry &bbe);

  BookBaseEntry bbe;

  void
  addLabel(Gtk::Label *lab);

  void
  removeLabel(Gtk::Label *lab);

  void
  activateLabels();

  void
  deactivateLabels();

protected:
  SearchResultModelItem(const BookBaseEntry &bbe);

  std::vector<Gtk::Label *> labels;
#ifndef USE_OPENMP
  std::mutex labels_mtx;
#else
  omp_lock_t labels_mtx;
#endif
};

#endif // SEARCHRESULTMODELITEM_H
