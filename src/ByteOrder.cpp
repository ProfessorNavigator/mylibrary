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

#include <ByteOrder.h>

ByteOrder::ByteOrder()
{

}

ByteOrder::~ByteOrder()
{

}

ByteOrder::ByteOrder(const ByteOrder &other)
{
  inner = other.inner;
  native_order = other.native_order;
}

ByteOrder::ByteOrder(uint64_t val)
{
  uint64_t control = 506097522914230528;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(uint32_t val)
{
  uint32_t control = 50462976;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(uint16_t val)
{
  uint16_t control = 256;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(int64_t val)
{
  uint64_t control = 506097522914230528;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(int32_t val)
{
  uint32_t control = 50462976;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(int16_t val)
{
  uint16_t control = 256;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(float val)
{
  float control = 3.82047e-37;
  form_native_order(control);
  form_inner(val);
}

ByteOrder::ByteOrder(double val)
{
  double control = 7.949928895127363e-275;
  form_native_order(control);
  form_inner(val);
}

ByteOrder&
ByteOrder::operator =(const ByteOrder &other)
{
  bomtx.lock();
  inner = other.inner;
  native_order = other.native_order;
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const uint64_t &val)
{
  uint64_t control = 506097522914230528;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const uint32_t &val)
{
  uint32_t control = 50462976;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const uint16_t &val)
{
  uint16_t control = 256;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const int64_t &val)
{
  int64_t control = 506097522914230528;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const int32_t &val)
{
  int32_t control = 50462976;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const int16_t &val)
{
  int16_t control = 256;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const float &val)
{
  float control = 3.82047e-37;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder&
ByteOrder::operator =(const double &val)
{
  double control = 7.949928895127363e-275;
  bomtx.lock();
  form_native_order(control);
  form_inner(val);
  bomtx.unlock();
  return *this;
}

ByteOrder::operator uint64_t()
{
  uint64_t result;
  get_native(result);
  return result;
}

ByteOrder::operator uint32_t()
{
  uint32_t result;
  get_native(result);
  return result;
}

ByteOrder::operator uint16_t()
{
  uint16_t result;
  get_native(result);
  return result;
}

ByteOrder::operator int64_t()
{
  int64_t result;
  get_native(result);
  return result;
}

ByteOrder::operator int32_t()
{
  int32_t result;
  get_native(result);
  return result;
}

ByteOrder::operator int16_t()
{
  int16_t result;
  get_native(result);
  return result;
}

ByteOrder::operator float()
{
  float result;
  get_native(result);
  return result;
}

ByteOrder::operator double()
{
  double result;
  get_native(result);
  return result;
}

template<typename T>
  void
  ByteOrder::form_native_order(T &control)
  {
    size_t sz = sizeof(control);
    uint8_t arr[sz];
    std::memcpy(&arr, &control, sz);
    native_order.clear();
    native_order.resize(sz);
    for(uint8_t i = 0; i < static_cast<uint8_t>(sz); i++)
      {
	for(size_t j = 0; j < sz; j++)
	  {
	    if(arr[j] == i)
	      {
		native_order[static_cast<size_t>(i)] = j;
		break;
	      }
	  }
      }
  }

template<typename T>
  void
  ByteOrder::form_inner(T &val)
  {
    inner.clear();
    size_t sz = sizeof(val);
    uint8_t arr[sz];
    std::memcpy(&arr, &val, sz);
    inner.resize(sz);
    for(size_t i = 0; i < sz; i++)
      {
	size_t v = native_order[i];
	inner[v] = arr[i];
      }
  }

template<typename T>
  void
  ByteOrder::get_native(T &result)
  {

    size_t sz = sizeof(T);
    uint8_t arr[sz];
    bomtx.lock();
    if(sz > inner.size())
      {
	sz = inner.size();
      }
    for(size_t i = 0; i < sz; i++)
      {
	size_t j = native_order[i];
	arr[j] = inner[i];
      }
    bomtx.unlock();
    std::memcpy(&result, &arr, sz);
  }

template void
ByteOrder::get_native<uint64_t>(uint64_t&);
template void
ByteOrder::get_native<uint32_t>(uint32_t&);
template void
ByteOrder::get_native<uint16_t>(uint16_t&);

template void
ByteOrder::get_native<int64_t>(int64_t&);
template void
ByteOrder::get_native<int32_t>(int32_t&);
template void
ByteOrder::get_native<int16_t>(int16_t&);

template void
ByteOrder::get_native<float>(float&);
template void
ByteOrder::get_native<double>(double&);

template<typename T>
  void
  ByteOrder::get_big(T &result)
  {
    size_t sz = sizeof(T);
    uint8_t arr[sz];
    bomtx.lock();
    if(sz > inner.size())
      {
	sz = inner.size();
      }
    for(size_t i = 0; i < sz; i++)
      {
	arr[sz - 1 - i] = inner[i];
      }
    bomtx.unlock();
    std::memcpy(&result, &arr, sz);
  }

template
void
ByteOrder::get_big<uint64_t>(uint64_t&);
template
void
ByteOrder::get_big<uint32_t>(uint32_t&);
template
void
ByteOrder::get_big<uint16_t>(uint16_t&);

template
void
ByteOrder::get_big<int64_t>(int64_t&);
template
void
ByteOrder::get_big<int32_t>(int32_t&);
template
void
ByteOrder::get_big<int16_t>(int16_t&);

template
void
ByteOrder::get_big<float>(float&);
template
void
ByteOrder::get_big<double>(double&);

template<typename T>
  void
  ByteOrder::get_little(T &result)
  {
    size_t sz = sizeof(T);
    bomtx.lock();
    if(sz > inner.size())
      {
	sz = inner.size();
      }
    std::memcpy(&result, inner.data(), sz);
    bomtx.unlock();
  }

template
void
ByteOrder::get_little<uint64_t>(uint64_t&);
template
void
ByteOrder::get_little<uint32_t>(uint32_t&);
template
void
ByteOrder::get_little<uint16_t>(uint16_t&);

template
void
ByteOrder::get_little<int64_t>(int64_t&);
template
void
ByteOrder::get_little<int32_t>(int32_t&);
template
void
ByteOrder::get_little<int16_t>(int16_t&);

template
void
ByteOrder::get_little<float>(float&);
template
void
ByteOrder::get_little<double>(double&);

template<typename T>
  void
  ByteOrder::set_big(T val)
  {
    bomtx.lock();
    inner.clear();
    native_order.clear();
    size_t sz = sizeof(T);
    T control;
    switch (sz)
      {
      case 8:
	{
	  if(typeid(val) == typeid(double))
	    {
	      control = T(7.949928895127363e-275);
	    }
	  else
	    {
	      control = T(506097522914230528);
	    }
	  break;
	}
      case 4:
	{
	  if(typeid(val) == typeid(float))
	    {
	      control = T(3.82047e-37);
	    }
	  else
	    {
	      control = T(50462976);
	    }
	  break;
	}
      case 2:
	{
	  control = T(256);
	  break;
	}
      default:
	{
	  sz = 0;
	  break;
	}
      }
    if(sz == 0)
      {
	bomtx.unlock();
	return void();
      }
    else
      {
	form_native_order(control);
	uint8_t arr[sz];
	std::memcpy(&arr, &val, sz);
	inner.resize(sz);
	for(size_t i = 0; i < sz; i++)
	  {
	    inner[sz - 1 - i] = arr[i];
	  }
      }
    bomtx.unlock();
  }

template
void
ByteOrder::set_big<uint64_t>(uint64_t);
template
void
ByteOrder::set_big<uint32_t>(uint32_t);
template
void
ByteOrder::set_big<uint16_t>(uint16_t);

template
void
ByteOrder::set_big<int64_t>(int64_t);
template
void
ByteOrder::set_big<int32_t>(int32_t);
template
void
ByteOrder::set_big<int16_t>(int16_t);

template
void
ByteOrder::set_big<float>(float);
template
void
ByteOrder::set_big<double>(double);

template<typename T>
  void
  ByteOrder::set_little(T val)
  {
    bomtx.lock();
    inner.clear();
    native_order.clear();
    size_t sz = sizeof(T);
    T control;
    switch (sz)
      {
      case 8:
	{
	  if(typeid(val) == typeid(double))
	    {
	      control = T(7.949928895127363e-275);
	    }
	  else
	    {
	      control = T(506097522914230528);
	    }
	  break;
	}
      case 4:
	{
	  if(typeid(val) == typeid(float))
	    {
	      control = T(3.82047e-37);
	    }
	  else
	    {
	      control = T(50462976);
	    }
	  break;
	}
      case 2:
	{
	  control = T(256);
	  break;
	}
      default:
	{
	  sz = 0;
	  break;
	}
      }
    if(sz == 0)
      {
	bomtx.unlock();
	return void();
      }
    else
      {
	form_native_order(control);
	inner.resize(sz);
	std::memcpy(inner.data(), &val, sz);
      }
    bomtx.unlock();
  }

template
void
ByteOrder::set_little<uint64_t>(uint64_t);
template
void
ByteOrder::set_little<uint32_t>(uint32_t);
template
void
ByteOrder::set_little<uint16_t>(uint16_t);

template
void
ByteOrder::set_little<int64_t>(int64_t);
template
void
ByteOrder::set_little<int32_t>(int32_t);
template
void
ByteOrder::set_little<int16_t>(int16_t);

template
void
ByteOrder::set_little<float>(float);
template
void
ByteOrder::set_little<double>(double);
