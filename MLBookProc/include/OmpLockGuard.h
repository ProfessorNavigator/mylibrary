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
#ifndef OMPLOCKGUARD_H
#define OMPLOCKGUARD_H

#include <omp.h>

/*!
 * \brief The OmpLockGuard class.
 *
 * Auxiliary class. Locks omp_lock_t variable on creation and unlocks it on
 * destruction.
 */
class OmpLockGuard
{
public:
  /*!
   * \brief OmpLockGuard constructor.
   *
   * \warning \b omp_mtx must be initialized (see <A
   * HREF="https://www.openmp.org/spec-html/5.0/openmpsu154.html">omp_init_lock</A>).
   * \param omp_mtx omp_lock_t variable.
   */
  OmpLockGuard(omp_lock_t &omp_mtx);

  /*!
   * \brief OmpLockGuard destructor.
   */
  virtual ~OmpLockGuard();

private:
  omp_lock_t *omp_mtx_ptr;
};

#endif // OMPLOCKGUARD_H
