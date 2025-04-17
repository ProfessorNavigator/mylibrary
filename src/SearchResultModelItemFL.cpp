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
#include <SearchResultModelItemFL.h>

SearchResultModelItemFL::~SearchResultModelItemFL()
{
#ifdef USE_OPENMP
  omp_destroy_lock(&l_lab_mtx);
#endif
}

Glib::RefPtr<SearchResultModelItemFL>
SearchResultModelItemFL::create(const FileParseEntry &entry)
{
  SearchResultModelItemFL *item = new SearchResultModelItemFL(entry);
  return Glib::make_refptr_for_instance(item);
}

void
SearchResultModelItemFL::setLabel(Gtk::Label *lab)
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
SearchResultModelItemFL::unsetLabel()
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
SearchResultModelItemFL::activateLab()
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
SearchResultModelItemFL::deactivateLab()
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

SearchResultModelItemFL::SearchResultModelItemFL(const FileParseEntry &entry)
{
#ifdef USE_OPENMP
  omp_init_lock(&l_lab_mtx);
#endif
  this->entry = entry;
}
