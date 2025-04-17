/*
 * Copyright (C) 2023-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <cstdint>
#include <cstring>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <mutex>
#endif
/*!
 * \brief The ByteOrder class
 *
 * Auxiliary class. Is used to convert numbers to different byte orders.
 * \warning If you get number from ByteOrder, resulting number must
 * have same type as input value, otherwise behavior is undefined.
 */
class ByteOrder
{
public:
  /*!
   * \brief ByteOrder constructor.
   */
  ByteOrder();

  /*!
   * \brief ByteOrder destructor.
   */
  virtual ~ByteOrder();

  /*!
   * \brief ByteOrder copy constructor.
   */
  ByteOrder(const ByteOrder &other);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(uint64_t val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(uint32_t val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(uint16_t val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(int64_t val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(int32_t val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(int16_t val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(float val);

  /*!
   * \brief ByteOrder constructor.
   * \param val value in native byte order.
   */
  ByteOrder(double val);

  /*!
   * \brief operator =
   */
  ByteOrder &
  operator=(const ByteOrder &other);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const uint64_t &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const uint32_t &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const uint16_t &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const int64_t &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const int32_t &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const int16_t &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const float &val);

  /*!
   * \brief operator =
   * \param val value in native byte order.
   */
  ByteOrder &
  operator=(const double &val);

  /*!
   * \brief Returns number in native byte order.
   */
  operator uint64_t();

  /*!
   * \brief Returns number in native byte order.
   */
  operator uint32_t();

  /*!
   * \brief Returns number in native byte order.
   */
  operator uint16_t();

  /*!
   * \brief Returns number in native byte order.
   */
  operator int64_t();

  /*!
   * \brief Returns number in native byte order.
   */
  operator int32_t();

  /*!
   * \brief Returns number in native byte order.
   */
  operator int16_t();

  /*!
   * \brief Returns number in native byte order.
   */
  operator float();

  /*!
   * \brief Returns number in native byte order.
   */
  operator double();

  /*!
   * \brief Returns number in native byte order.
   * \param result resulting number.
   */
  template <typename T>
  void
  get_native(T &result);

  /*!
   * \brief Returns "big endian" number.
   * \param result resulting number.
   */
  template <typename T>
  void
  get_big(T &result);

  /*!
   * \brief Returns "little endian" number.
   * \param result resulting number.
   */
  template <typename T>
  void
  get_little(T &result);

  /*!
   * \brief Sets inner value to val.
   *
   * Sets inner value to val. val will be processed as "big endian".
   * \param val value to be set.
   */
  template <typename T>
  void
  set_big(T val);

  /*!
   * \brief Sets inner value to val.
   *
   * Sets inner value to val. val will be processed as "little endian".
   * \param val value to be set.
   */
  template <typename T>
  void
  set_little(T val);

private:
  template <typename T>
  void
  form_native_order(T &control);

  template <typename T>
  void
  form_inner(T &val);

  std::vector<uint8_t> inner;
  std::vector<size_t> native_order;
#ifndef USE_OPENMP
  std::mutex bomtx;
#else
  omp_lock_t bomtx;
#endif
};

#endif // BYTEORDER_H
