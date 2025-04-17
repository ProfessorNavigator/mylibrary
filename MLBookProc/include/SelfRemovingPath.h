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

#ifndef SELFREMOVINGPATH_H
#define SELFREMOVINGPATH_H

#include <filesystem>
#ifndef USE_OPENMP
#include <atomic>
#else
#include <omp.h>
#endif

/*!
 * \brief The SelfRemovingPath class.
 *
 * Auxiliary class. Removes underlying path on destruction, if no any copies of
 * SelfRemovingPath object have been created. Removes path on last copy
 * destruction otherwise.
 */
class SelfRemovingPath
{
public:
  /*!
   * \brief SelfRemovingPath constructor.
   */
  SelfRemovingPath();

  /*!
   * \brief SelfRemovingPath destructor.
   */
  virtual ~SelfRemovingPath();

  /*!
   * \brief SelfRemovingPath copy constructor.
   */
  SelfRemovingPath(const SelfRemovingPath &other);

  /*!
   * \brief SelfRemovingPath move constructor.
   */
  SelfRemovingPath(SelfRemovingPath &&other);

  /*!
   * \brief operator =
   */
  SelfRemovingPath &
  operator=(const SelfRemovingPath &other);

  /*!
   * \brief operator =
   */
  SelfRemovingPath &
  operator=(SelfRemovingPath &&other);

  /*!
   * \brief operator =
   * \param path path to be removed on destruction.
   * \return Returns self.
   */
  SelfRemovingPath &
  operator=(const std::filesystem::path &path);

  /*!
   * \brief SelfRemovingPath constructor.
   * \param path path to be removed on destruction.
   */
  explicit SelfRemovingPath(const std::filesystem::path &path);

  /*!
   * \brief Path to be removed on destruction.
   */
  std::filesystem::path path;

private:
  void
  deleter();

#ifndef USE_OPENMP
  std::atomic<uint64_t> *count;
#else
  uint64_t *count;
#endif
};

#endif // SELFREMOVINGPATH_H
