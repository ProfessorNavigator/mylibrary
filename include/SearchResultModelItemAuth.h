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
#ifndef SEARCHRESULTMODELITEMAUTH_H
#define SEARCHRESULTMODELITEMAUTH_H

#include <glibmm-2.68/glibmm/object.h>
#include <gtkmm-4.0/gtkmm/label.h>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <mutex>
#endif

class SearchResultModelItemAuth : public Glib::Object
{
public:
  virtual ~SearchResultModelItemAuth();

  static Glib::RefPtr<SearchResultModelItemAuth>
  create(const std::string &auth);

  std::string auth;

  void
  setLabel(Gtk::Label *lab);

  void
  unsetLabel();

  void
  activateLab();

  void
  deactivateLab();

protected:
  SearchResultModelItemAuth(const std::string &auth);

  Gtk::Label *l_lab = nullptr;

#ifndef USE_OPENMP
  std::mutex l_lab_mtx;
#else
  omp_lock_t l_lab_mtx;
#endif
};

#endif // SEARCHRESULTMODELITEMAUTH_H
