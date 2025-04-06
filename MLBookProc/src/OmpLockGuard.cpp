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
#include <OmpLockGuard.h>

OmpLockGuard::OmpLockGuard(omp_lock_t &omp_mtx)
{
  omp_mtx_ptr = &omp_mtx;
  omp_set_lock(omp_mtx_ptr);
}

OmpLockGuard::~OmpLockGuard()
{
  omp_unset_lock(omp_mtx_ptr);
}
