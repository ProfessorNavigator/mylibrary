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

#ifndef INCLUDE_OPENBOOK_H_
#define INCLUDE_OPENBOOK_H_

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <SelfRemovingPath.h>
#include <ZipFileEntry.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class OpenBook
{
public:
  OpenBook(const std::shared_ptr<AuxFunc> &af);
  virtual
  ~OpenBook();

  std::filesystem::path
  open_book(const BookBaseEntry &bbe, const bool &copy,
	    const std::filesystem::path &copy_path, const bool &find_fbd,
	    std::function<void
	    (const std::filesystem::path &path)> open_callback);

private:
  std::filesystem::path
  open_archive(const BookBaseEntry &bbe, const std::string &ext,
	       const std::filesystem::path &copy_path, const bool &find_fbd);

  void
  correct_separators(std::vector<ZipFileEntry> &files);

  bool
  compare_func(const ZipFileEntry &ent, const bool &encoding,
	       const std::string &conv_nm, const std::filesystem::path &ch_fbd);

  std::shared_ptr<AuxFunc> af;

  SelfRemovingPath reading;
};

#endif /* INCLUDE_OPENBOOK_H_ */
