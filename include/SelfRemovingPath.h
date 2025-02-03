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

#ifndef SELFREMOVINGPATH_H
#define SELFREMOVINGPATH_H

#include <atomic>
#include <filesystem>

class SelfRemovingPath
{
public:
  SelfRemovingPath();

  virtual ~SelfRemovingPath();

  SelfRemovingPath(const SelfRemovingPath &other);

  SelfRemovingPath(SelfRemovingPath &&other);

  SelfRemovingPath &
  operator=(const SelfRemovingPath &other);

  SelfRemovingPath &
  operator=(SelfRemovingPath &&other);

  SelfRemovingPath &
  operator=(const std::filesystem::path &path);

  explicit SelfRemovingPath(const std::filesystem::path &path);

  std::filesystem::path path;

private:
  void
  deleter();

  std::atomic<uint64_t> *count;
};

#endif // SELFREMOVINGPATH_H
