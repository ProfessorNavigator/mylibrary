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

#ifndef CREATECOLLECTION_H
#define CREATECOLLECTION_H

#include <iostream>
#include <string.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <vector>
#include <tuple>
#include <mutex>
#include "AuxFunc.h"

class CreateCollection
{
public:
  CreateCollection(std::string coll_nm, std::filesystem::path book_p,
		   int *cancel);
  virtual
  ~CreateCollection();
  std::function<void
  ()> creation_finished;
  std::function<void
  ()> collection_exist;
  std::function<void
  (int)> total_files;
  std::function<void
  (int)> files_added;
  std::function<void
  ()> op_canceled;
  void
  createFileList(
      std::vector<std::filesystem::path> *fb2in,
      std::vector<std::filesystem::path> *epubin,
      std::vector<
	  std::tuple<std::filesystem::path,
	      std::vector<std::tuple<int, int, std::string>>>> *zipvectin);
  void
  createCol();
private:
  void
  createFileList();
  void
  createDatabase();
  std::vector<std::tuple<std::string, std::string>>
  fb2Parser(std::filesystem::path filepath);
  std::vector<std::tuple<std::string, std::string>>
  fb2Parser(std::string input);
  std::vector<std::tuple<std::string, std::string>>
  epubparser(std::filesystem::path input);

  std::string coll_nm;
  std::filesystem::path book_p;
  std::vector<std::filesystem::path> fb2;
  std::vector<std::filesystem::path> epub;
  std::vector<
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>>> zipvect;
  int threadnum = 0;
  int *cancel = nullptr;
};

#endif // CREATECOLLECTION_H
