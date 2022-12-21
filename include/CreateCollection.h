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
#include <unistd.h>
#include <thread>
#include <mutex>
#include "AuxFunc.h"

class CreateCollection
{
public:
  CreateCollection(std::string coll_nm, std::filesystem::path book_p,
		   unsigned int nm_thr, int *cancel);
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
  void
  fb2ThreadFunc(std::filesystem::path fp, std::filesystem::path filepath,
		std::filesystem::path fb2_hashp);
  void
  zipThreadFunc(
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>> arch_tup,
      std::filesystem::path filepath, std::filesystem::path zip_hashp);
  void
  epubThreadFunc(std::filesystem::path fp, std::filesystem::path filepath,
		std::filesystem::path epub_hashp);

  std::string coll_nm;
  std::filesystem::path book_p;
  std::vector<std::filesystem::path> fb2;
  std::vector<std::filesystem::path> epub;
  std::vector<
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>>> zipvect;
  unsigned int threadnum = 1;
  int *cancel = nullptr;
  std::mutex fb2basemtx;
  std::mutex fb2hashmtx;
  std::mutex zipbasemtx;
  std::mutex ziphashmtx;
  std::mutex epubbasemtx;
  std::mutex epubhashmtx;

  std::mutex cmtx;
  unsigned int num_thr_run = 0;
  std::mutex num_thr_runmtx;
  int file_count = 0;
  std::mutex file_countmtx;
};

#endif // CREATECOLLECTION_H
