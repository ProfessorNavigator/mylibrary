/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_ARCHPARSER_H_
#define INCLUDE_ARCHPARSER_H_

#include <archive.h>
#include <AuxFunc.h>
#include <BookParseEntry.h>
#include <LibArchive.h>
#include <atomic>
#include <filesystem>
#include <memory>
#include <vector>

class ARCHParser : public LibArchive
{
public:
  ARCHParser(const std::shared_ptr<AuxFunc> &af,
	     std::atomic<bool> *cancel);
  virtual
  ~ARCHParser();

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

  std::shared_ptr<AuxFunc> af;
  std::atomic<bool> *cancel = nullptr;

  std::vector<BookParseEntry> result;
  std::vector<BookParseEntry> fbd;
};

#endif /* INCLUDE_ARCHPARSER_H_ */
