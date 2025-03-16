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

#ifndef ARCHPARSER_H
#define ARCHPARSER_H

#include <AuxFunc.h>
#include <BookParseEntry.h>
#include <LibArchive.h>
#include <archive.h>
#include <atomic>
#include <filesystem>
#include <memory>
#include <thread>
#include <vector>

class ARCHParser : public LibArchive
{
public:
  ARCHParser(const std::shared_ptr<AuxFunc> &af, const bool &rar_support,
             std::atomic<bool> *cancel);

  virtual ~ARCHParser();

  std::vector<BookParseEntry>
  arch_parser(const std::filesystem::path &filepath);

private:
  void
  arch_process(const std::shared_ptr<archive> &a);

  void
  unpack_entry(const std::filesystem::path &ch_p,
               const std::shared_ptr<archive> &a,
               const std::shared_ptr<archive_entry> &e);

  void
  check_for_fbd();

  static void
  signalHandler(int sig);

  std::shared_ptr<AuxFunc> af;
  bool rar_support = false;
  std::atomic<bool> *cancel = nullptr;

  std::vector<BookParseEntry> result;
  std::vector<BookParseEntry> fbd;

  std::thread parse_thr;

  std::filesystem::path arch_path;
};

#endif // ARCHPARSER_H
