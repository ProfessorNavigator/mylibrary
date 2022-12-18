/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of MyLibrary.
 MyLibrary is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 MyLibrary is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with MyLibrary. If not,
 see <https://www.gnu.org/licenses/>.
 */

#ifndef SEARCHBOOK_H_
#define SEARCHBOOK_H_

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
  std::function<void
  ()> search_completed;
  virtual
  ~SearchBook();
  void
  searchBook();
  void
  cleanSearchV();

private:
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

#endif /* SEARCHBOOK_H_ */
