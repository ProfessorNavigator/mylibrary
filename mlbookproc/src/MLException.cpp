/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <MLException.h>

MLException::MLException()
{
}

MLException::MLException(const std::string &msg)
{
  this->msg = msg;
}

MLException::MLException(const MLException &other)
{
  msg = other.msg;
}

MLException::MLException(MLException &&other)
{
  msg = std::move(other.msg);
}

MLException &
MLException::operator=(const MLException &other)
{
  if(this != &other)
    {
      msg = other.msg;
    }
  return *this;
}

MLException &
MLException::operator=(MLException &&other)
{
  if(this != &other)
    {
      msg = std::move(other.msg);
    }
  return *this;
}

std::string
MLException::what()
{
  return msg;
}

MLException::operator bool()
{
  return !msg.empty();
}
