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

#include <chrono>
#include <pthread.h>
#include <thread>
#ifdef _WIN32
#include <errhandlingapi.h>
#include <winbase.h>
#endif

int
main()
{
  int result = 0;
  std::thread thr(
      []
        {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        });

#ifdef __linux
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(static_cast<unsigned>(std::thread::hardware_concurrency() - 1),
          &cpu_set);
  result = pthread_setaffinity_np(thr.native_handle(), sizeof(cpu_set_t),
                                  &cpu_set);
#elif defined(_WIN32)
  DWORD_PTR mask = 1;
  unsigned cpu
      = static_cast<unsigned>(std::thread::hardware_concurrency() - 1);
  mask = mask << cpu;
  HANDLE handle = pthread_gethandle(thr.native_handle());
  if(handle)
    {
      if(SetThreadAffinityMask(handle, mask) == 0)
        {
          result = 1;
        }
    }
  else
    {
      result = 1;
    }
#endif
  thr.join();

  return result;
}
