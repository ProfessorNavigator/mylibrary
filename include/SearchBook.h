/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_SEARCHBOOK_H_
#define INCLUDE_SEARCHBOOK_H_

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <fstream>
#include <filesystem>
#include "AuxFunc.h"

class SearchBook
{
public:
  SearchBook(
      std::string collnm,
      std::string surnm,
      std::string name,
      std::string secname,
      std::string book,
      std::string series,
      std::string genre,
      std::string *prev_search_nm,
      std::vector<
	  std::tuple<std::string, std::string, std::string, std::string,
	      std::string, std::string>> *base_v,
      std::vector<
	  std::tuple<std::string, std::string, std::string, std::string,
	      std::string, std::string>> *search_result_v,
      int *cancel);
  virtual
  ~SearchBook();
  void
  searchBook();
  void
  cleanSearchV();

private:
  void
  readBase(std::filesystem::path filepath);
  void
  readZipBase(std::filesystem::path filepath);
  bool
  searchAuth(
      std::tuple<std::vector<std::tuple<std::string, std::string, std::string>>,
	  std::string, std::string, std::string, std::string, std::string> &stup,
      std::string to_search, int variant);
  std::string collnm;
  std::string surnm;
  std::string name;
  std::string secname;
  std::string book;
  std::string series;
  std::string genre;
  std::string *prev_search_nm;
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> *base_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  std::vector<
      std::tuple<std::string, std::string, std::string, std::string,
	  std::string, std::string>> *search_result_v; //0-authors, 1-book, 2-series, 3-genre, 4-date, 5-path to book
  int *cancel = nullptr;
};

#endif /* INCLUDE_SEARCHBOOK_H_ */
