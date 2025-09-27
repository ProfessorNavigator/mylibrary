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

#include <cstddef>
#include <omp.h>

int
main()
{
  size_t array_size = 1000;
  int *array = new int[array_size];
  int result = 0;

#pragma omp target map(tofrom : array[0 : array_size], result)                \
    map(to : array_size)
  {
    if(omp_is_initial_device())
      {
        result = 1;
      }
#pragma omp parallel
#pragma omp for
    for(size_t i = 0; i < array_size; i++)
      {
        array[i] = static_cast<int>(i + 1);
      }
  }

  delete[] array;
  return result;
}
