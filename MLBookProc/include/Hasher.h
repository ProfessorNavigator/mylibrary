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

#ifndef HASHER_H
#define HASHER_H

#include <AuxFunc.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#ifndef USE_OPENMP
#include <atomic>
#endif

/*!
 * \brief The Hasher class.
 *
 * This class contains methods for hash sums creating (Blake-256 algorithm).
 */
class Hasher
{
public:
  /*!
   * \brief Hasher constructor.
   * \param af smart pointer to AuxFunc object.
   */
  Hasher(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Creates hash sum for given buffer.
   *
   * \note This method can throw MLException in case of error.
   * \param buf source buffer.
   * \return 32 bytes of hash sum value.
   */
  std::string
  buf_hashing(const std::string &buf);

  /*!
   * \brief Creates hash sum for given file.
   *
   * \note This method can throw MLException in case of error.
   * \param filepath absolute path to file to be hashed.
   * \return 32 bytes of hash sum value.
   */
  std::string
  file_hashing(const std::filesystem::path &filepath);

  /*!
   * \brief Stops all operations.
   */
  void
  cancelAll();

protected:
#ifndef USE_OPENMP
  /*!
   * \brief Stops all operations if \a true.
   *
   * \warning Do not call or set this variable yourself!
   */
  std::atomic<bool> cancel;
#endif
#ifdef USE_OPENMP
  /*!
   * \brief Stops all operations if \a true.
   *
   * \warning Do not call or set this variable yourself!
   */
  bool cancel;
#endif

  /*!
   * \brief Stop signal for heir classes.
   *
   * \warning Do not call or set this variable yourself!
   */
  std::function<void()> stop_all_signal;

private:
  std::shared_ptr<AuxFunc> af;
};

#endif // HASHER_H
