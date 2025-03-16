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

#ifndef MLEXCEPTION_H
#define MLEXCEPTION_H

#include <string>

class MLException
{
public:
  MLException();

  MLException(const std::string &msg);

  MLException(const MLException &other);

  MLException(MLException &&other);

  MLException &
  operator=(const MLException &other);

  MLException &
  operator=(MLException &&other);

  operator bool();

  std::string
  what();  

private:
  std::string msg;
};

#endif // MLEXCEPTION_H
