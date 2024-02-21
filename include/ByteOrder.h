/*
 * Copyright (C) 2023-2024 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_BYTEORDER_H_
#define INCLUDE_BYTEORDER_H_

#include <cstdint>
#include <cstring>
#include <vector>
#include <typeinfo>
#include <mutex>

class ByteOrder
{
public:
  ByteOrder();
  virtual
  ~ByteOrder();
  ByteOrder(const ByteOrder &other);

  ByteOrder(uint64_t val);

  ByteOrder(uint32_t val);

  ByteOrder(uint16_t val);

  ByteOrder(int64_t val);

  ByteOrder(int32_t val);

  ByteOrder(int16_t val);

  ByteOrder(float val);

  ByteOrder(double val);

  ByteOrder&
  operator =(const ByteOrder &other);

  ByteOrder&
  operator =(const uint64_t &val);

  ByteOrder&
  operator =(const uint32_t &val);

  ByteOrder&
  operator =(const uint16_t &val);

  ByteOrder&
  operator =(const int64_t &val);

  ByteOrder&
  operator =(const int32_t &val);

  ByteOrder&
  operator =(const int16_t &val);

  ByteOrder&
  operator =(const float &val);

  ByteOrder&
  operator =(const double &val);

  operator uint64_t();
  operator uint32_t();
  operator uint16_t();

  operator int64_t();
  operator int32_t();
  operator int16_t();

  operator float();
  operator double();

  template<typename T>
    void
    get_native(T &result);

  template<typename T>
    void
    get_big(T &result);

  template<typename T>
    void
    get_little(T &result);

  template<typename T>
    void
    set_big(T val);

  template<typename T>
    void
    set_little(T val);

private:
  template<typename T>
    void
    form_native_order(T &control);

  template<typename T>
    void
    form_inner(T &val);

  std::vector<uint8_t> inner;
  std::vector<size_t> native_order;
  std::mutex bomtx;
};

#endif /* INCLUDE_BYTEORDER_H_ */
