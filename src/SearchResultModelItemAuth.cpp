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

Glib::RefPtr<SearchResultModelItemAuth>
SearchResultModelItemAuth::create(const std::string &auth)
{
  SearchResultModelItemAuth *mli = new SearchResultModelItemAuth(auth);

  return Glib::make_refptr_for_instance(mli);
}

void
SearchResultModelItemAuth::setLabel(Gtk::Label *lab)
{
  l_lab_mtx.lock();
  l_lab = lab;
  l_lab_mtx.unlock();
}

void
SearchResultModelItemAuth::unsetLabel()
{
  l_lab_mtx.lock();
  l_lab = nullptr;
  l_lab_mtx.unlock();
}

void
SearchResultModelItemAuth::activateLab()
{
  l_lab_mtx.lock();
  if(l_lab)
    {
      l_lab->set_name("selectedLab");
    }
  l_lab_mtx.unlock();
}

void
SearchResultModelItemAuth::deactivateLab()
{

  l_lab_mtx.lock();
  if(l_lab)
    {
      l_lab->set_name("windowLabel");
    }
  l_lab_mtx.unlock();
}

SearchResultModelItemAuth::SearchResultModelItemAuth(const std::string &auth)
{
  this->auth = auth;
}
