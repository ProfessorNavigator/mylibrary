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
#include <SearchResultModelItemAuth.h>

SearchResultModelItemAuth::~SearchResultModelItemAuth()
{
#ifdef USE_OPENMP
  omp_destroy_lock(&l_lab_mtx);
#endif
}

Glib::RefPtr<SearchResultModelItemAuth>
SearchResultModelItemAuth::create(const std::string &auth)
{
  SearchResultModelItemAuth *mli = new SearchResultModelItemAuth(auth);

  return Glib::make_refptr_for_instance(mli);
}

void
SearchResultModelItemAuth::setLabel(Gtk::Label *lab)
{
#ifndef USE_OPENMP
  l_lab_mtx.lock();
  l_lab = lab;
  l_lab_mtx.unlock();
#else
  omp_set_lock(&l_lab_mtx);
  l_lab = lab;
  omp_unset_lock(&l_lab_mtx);
#endif
}

void
SearchResultModelItemAuth::unsetLabel()
{
#ifndef USE_OPENMP
  l_lab_mtx.lock();
  l_lab = nullptr;
  l_lab_mtx.unlock();
#else
  omp_set_lock(&l_lab_mtx);
  l_lab = nullptr;
  omp_unset_lock(&l_lab_mtx);
#endif
}

void
SearchResultModelItemAuth::activateLab()
{
#ifndef USE_OPENMP
  l_lab_mtx.lock();
#else
  omp_set_lock(&l_lab_mtx);
#endif
  if(l_lab)
    {
      l_lab->set_name("selectedLab");
    }
#ifndef USE_OPENMP
  l_lab_mtx.unlock();
#else
  omp_unset_lock(&l_lab_mtx);
#endif
}

void
SearchResultModelItemAuth::deactivateLab()
{
#ifndef USE_OPENMP
  l_lab_mtx.lock();
#else
  omp_set_lock(&l_lab_mtx);
#endif
  if(l_lab)
    {
      l_lab->set_name("windowLabel");
    }
#ifndef USE_OPENMP
  l_lab_mtx.unlock();
#else
  omp_unset_lock(&l_lab_mtx);
#endif
}

SearchResultModelItemAuth::SearchResultModelItemAuth(const std::string &auth)
{
#ifdef USE_OPENMP
  omp_init_lock(&l_lab_mtx);
#endif
  this->auth = auth;
}
