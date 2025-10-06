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
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <omp.h>
#include <stdexcept>
#include <thread>
#include <vector>

template <class InputIt,
          class T = typename std::iterator_traits<InputIt>::value_type>
static InputIt
parallelFind(InputIt start, InputIt end, const T &val)
{
  InputIt res = end;
  const T *val_ptr = &val;
#pragma omp parallel
  {
#pragma omp for
    for(InputIt i = start; i != end; i++)
      {
        if(*i == *val_ptr)
          {
#pragma omp critical
            {
              if(i < res)
                {
                  res = i;
                }
            }
#pragma omp cancel for
          }
      }
  }

  return res;
}

template <class InputIt,
          class T = typename std::iterator_traits<InputIt>::value_type>
static InputIt
parallelRemove(InputIt start, InputIt end, const T &val)
{
  start = parallelFind(start, end, val);
  if(start != end)
    {
      T *s = start.base();
      T *e = end.base();
#pragma omp parallel
#pragma omp for ordered
      for(T *i = s + 1; i != e; i++)
        {
          if((*i) != val)
            {
#pragma omp ordered
              {
                *s = std::move((*i));
                s++;
              }
            }
        }
      start = InputIt(s);
    }
  return start;
}

int
main()
{
  int result = 1;
#pragma omp parallel num_threads(1)
  {
#pragma omp masked
    {
      omp_event_handle_t event;
#pragma omp task detach(event)
      {
        result = 0;
        omp_fulfill_event(event);
      }
    }
  }

  if(result != 0)
    {
      return result;
    }

  bool finished = false;
  std::mutex finished_mtx;
  std::condition_variable finished_var;

  std::thread thr([&finished, &finished_mtx, &finished_var] {
    std::vector<int> v;
    std::vector<int> control;
    for(int i = 0; i < 1000; i++)
      {
        for(int j = 0; j < omp_get_max_threads(); j++)
          {
            v.push_back(j);
            if(i == 0)
              {
                control.push_back(j);
              }
          }
      }

    int find = omp_get_max_threads() - 1;
    control.erase(std::remove(control.begin(), control.end(), find),
                  control.end());

    v.erase(parallelRemove(v.begin(), v.end(), find), v.end());

    for(size_t i = 0; i < 10; i++)
      {
        for(size_t j = 0; j < control.size(); j++)
          {
            if(v[i * control.size() + j] != control[j])
              {
                throw std::runtime_error("Incorrect value");
              }
          }
      }

    std::lock_guard<std::mutex> lglock(finished_mtx);
    finished = true;
    finished_var.notify_all();
  });
  thr.detach();

  std::unique_lock<std::mutex> ullock(finished_mtx);
  finished_var.wait_for(ullock, std::chrono::seconds(1), [&finished] {
    return finished;
  });
  if(!finished)
    {
      throw std::runtime_error("Thread not finished");
    }

  return result;
}
