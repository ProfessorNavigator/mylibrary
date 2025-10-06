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
#include <vector>

int
main()
{
  size_t array_size = 1000;
  int *array_gpu = reinterpret_cast<int *>(
      omp_target_alloc(array_size * sizeof(int), omp_get_default_device()));
  std::vector<int> control;
  control.reserve(array_size);
  for(int i = 0; i < 1000; i++)
    {
      control.push_back(i);
    }
  int result = 0;
#pragma omp target map(to : array_gpu) map(tofrom : result)
  {
    if(omp_is_initial_device())
      {
        result = 1;
      }
    else
      {
#pragma omp parallel
#pragma omp for
        for(int i = 0; i < 1000; i++)
          {
            array_gpu[i] = i;
          }
      }
  }

  if(result != 0)
    {
      omp_target_free(array_gpu, omp_get_default_device());
      return result;
    }

  std::vector<int> result_gpu;
  result_gpu.resize(array_size);
  result = omp_target_memcpy(
      result_gpu.data(), array_gpu, array_size * sizeof(int), 0, 0,
      omp_get_initial_device(), omp_get_default_device());
  if(result != 0)
    {
      omp_target_free(array_gpu, omp_get_default_device());
      return result;
    }

  for(size_t i = 0; i < result_gpu.size(); i++)
    {
      if(control[i] != result_gpu[i])
        {
          result = 1;
          break;
        }
    }
  omp_target_free(array_gpu, omp_get_default_device());
  return result;
}
