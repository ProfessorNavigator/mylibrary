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

#ifndef REFRESHCOLLECTION_H_
#define REFRESHCOLLECTION_H_

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sstream>
#include <fstream>
#include <tuple>
#include <algorithm>
#include <functional>
#include "CreateCollection.h"
#include "AuxFunc.h"

class RefreshCollection
{
public:
  RefreshCollection(std::string collname, int *cancel);
  virtual
  ~RefreshCollection();
  std::function<void
  ()> refresh_canceled;
  std::function<void
  ()> refresh_finished;
  void
  startRefreshing();
  void
  removeBook(std::string book_str);
  void
  addBook(std::string book_path);
private:
  std::string
  readList();
  void
  readColl(std::string bookpath);
  void
  collRefresh();

  std::string collname;
  int *cancel = nullptr;
  std::vector<std::tuple<std::filesystem::path, std::string>> saved_hashes;
  std::vector<std::filesystem::path> fb2parse;
  std::vector<std::filesystem::path> epubparse;
  std::vector<
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>>> zipparse;
  std::vector<std::filesystem::path> fb2remove;
  std::vector<std::filesystem::path> zipremove;
  std::vector<std::filesystem::path> epubremove;
};

#endif /* REFRESHCOLLECTION_H_ */
