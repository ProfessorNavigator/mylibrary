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

#ifndef INCLUDE_BASEKEEPER_H_
#define INCLUDE_BASEKEEPER_H_

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <FileParseEntry.h>
#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class BaseKeeper
{
public:
  BaseKeeper(const std::shared_ptr<AuxFunc> &af);
  virtual
  ~BaseKeeper();

  void
  loadCollection(const std::string &col_name);

  std::vector<BookBaseEntry>
  searchBook(const BookBaseEntry &search);

  void
  stopSearch();

  void
  clearBase();

  std::vector<FileParseEntry>
  get_base_vector();

  static std::filesystem::path
  get_books_path(const std::string &collection_name,
		 const std::shared_ptr<AuxFunc> &af);

private:
  FileParseEntry
  readFileEntry(std::string &base);

  std::vector<BookParseEntry>
  readBookEntry(std::string &entry);

  void
  parseBookEntry(std::string &e, std::string &read_val);

  bool
  searchLineFunc(const std::string &to_search, const std::string &source);

  bool
  searchSurname(const BookBaseEntry &search,
		std::vector<BookBaseEntry> &result);
  bool
  searchFirstName(const BookBaseEntry &search,
		  std::vector<BookBaseEntry> &result);

  bool
  searchLastName(const BookBaseEntry &search,
		 std::vector<BookBaseEntry> &result);

  void
  searchBook(const BookBaseEntry &search, std::vector<BookBaseEntry> &result);
  void
  searchSeries(const BookBaseEntry &search, std::vector<BookBaseEntry> &result);

  void
  searchGenre(const BookBaseEntry &search, std::vector<BookBaseEntry> &result);

  std::shared_ptr<AuxFunc> af;

  std::vector<FileParseEntry> base;
  std::string collection_name;
  std::filesystem::path collection_path;
  std::mutex basemtx;
  std::atomic<bool> cancel_search;
};

#endif /* INCLUDE_BASEKEEPER_H_ */
