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
#include <execution>
#include <iostream>
#include <vector>

int
main()
{
  std::vector<int> ch_v;
  ch_v.reserve(10);
  for(int i = 0; i < 1000; i++)
    {
      ch_v.push_back(i);
    }
  auto it = std::find(std::execution::par, ch_v.begin(), ch_v.end(), 500);
  if(it != ch_v.end())
    {
      std::cout << "Value found" << std::endl;
    }
  return 0;
}
