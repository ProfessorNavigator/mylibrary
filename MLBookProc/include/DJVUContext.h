/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef DJVUCONTEXT_H
#define DJVUCONTEXT_H

#include <condition_variable>
#include <libdjvu/ddjvuapi.h>
#include <mutex>

/*!
 * \brief The DJVUContext class
 *
 * Auxiliary class containing pointer to ddjvu_context_t object and support
 * objects. See DJVUParser source code for examples of usage.
 *
 * \warning Do not create this class objects yourself. If you need this class
 * object, it should be obtained from MLBookProc::getDJVUContext method only.
 */
class DJVUContext
{
public:
  DJVUContext();

  virtual ~DJVUContext();

  DJVUContext(const DJVUContext &) = delete;

  DJVUContext(DJVUContext &&) = delete;

  /*!
   * Pointer to ddjvu_context_t object.
   */
  ddjvu_context_t *context;

  /*!
   * This object locking mutex.
   */
  std::mutex context_mtx;

  /*!
   * This object locking condition varibale.
   */
  std::condition_variable context_var;
};

#endif // DJVUCONTEXT_H
