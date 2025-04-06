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

#include <SelfRemovingPath.h>

SelfRemovingPath::SelfRemovingPath()
{
  count = nullptr;
}

SelfRemovingPath::~SelfRemovingPath()
{
  deleter();
}

SelfRemovingPath::SelfRemovingPath(const SelfRemovingPath &other)
{
  path = other.path;
  count = other.count;
#ifndef USE_OPENMP
  count->store(count->load() + 1);
#endif
#ifdef USE_OPENMP
#pragma omp atomic update
  *count = *count + 1;
#endif
}

SelfRemovingPath::SelfRemovingPath(SelfRemovingPath &&other)
{
  count = other.count;
  other.count = nullptr;
  path = std::move(other.path);
}

SelfRemovingPath &
SelfRemovingPath::operator=(const SelfRemovingPath &other)
{
  if(this != &other)
    {
      if(path != other.path)
        {
          deleter();
          path = other.path;
          count = other.count;
#ifndef USE_OPENMP
          count->store(count->load() + 1);
#endif
#ifdef USE_OPENMP
#pragma omp atomic update
          *count = *count + 1;
#endif
        }
    }
  return *this;
}

SelfRemovingPath &
SelfRemovingPath::operator=(SelfRemovingPath &&other)
{
  if(this != &other)
    {
      if(path != other.path)
        {
          deleter();
          count = other.count;
          other.count = nullptr;
          path = std::move(other.path);
        }
    }
  return *this;
}

SelfRemovingPath &
SelfRemovingPath::operator=(const std::filesystem::path &path)
{
  if(this->path != path)
    {
      if(this->path != path)
        {
          deleter();
#ifndef USE_OPENMP
          count = new std::atomic<uint64_t>;
          count->store(1);
#endif
#ifdef USE_OPENMP
          count = new uint64_t(1);
#endif
          this->path = path;
        }
    }
  return *this;
}

SelfRemovingPath::SelfRemovingPath(const std::filesystem::path &path)
{
  if(this->path != path)
    {
#ifndef USE_OPENMP
      count = new std::atomic<uint64_t>;
      count->store(1);
#endif
#ifdef USE_OPENMP
      count = new uint64_t(1);
#endif
      this->path = path;
    }
  this->path = path;
}

void
SelfRemovingPath::deleter()
{
  if(count)
    {
#ifndef USE_OPENMP
      count->store(count->load() - 1);
      if(count->load() == 0)
        {
          std::filesystem::remove_all(path);
          delete count;
        }
#endif
#ifdef USE_OPENMP
      uint64_t l_c;
#pragma omp atomic capture
      {
        *count -= 1;
        l_c = *count;
      }
      if(l_c == 0)
        {
          std::filesystem::remove_all(path);
          delete count;
        }
#endif
    }
}
