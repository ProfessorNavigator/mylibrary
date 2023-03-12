/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

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

#ifndef INCLUDE_REFRESHCOLLECTION_H_
#define INCLUDE_REFRESHCOLLECTION_H_

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sstream>
#include <fstream>
#include <tuple>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "CreateCollection.h"
#include "AuxFunc.h"

class RefreshCollection
{
public:
  RefreshCollection(std::string collname, unsigned int thr_num,
		    bool fast_refresh, int *cancel);
  virtual
  ~RefreshCollection();
  std::function<void
  ()> refresh_canceled;
  std::function<void
  ()> refresh_finished;
  std::function<void
  (int)> total_files;
  std::function<void
  (long unsigned int)> total_hash;
  std::function<void
  (long unsigned int)> byte_hashed;
  std::function<void
  (int)> files_added;
  std::function<void
  ()> collection_not_exists;
  std::function<void
  (std::mutex*, int*)> file_exists;
  void
  startRefreshing();
  void
  removeBook(std::string book_str);
  void
  removeRar(std::string archaddr);
  void
  addBook(std::string book_path, std::string book_name, std::string arch_ext);
  void
  removeEmptyDirs();
  void
  editBook(std::vector<std::tuple<std::string, std::string>> *newbasev,
	   std::vector<std::tuple<std::string, std::string>> *oldbasev);
private:
  void
  readList();
  void
  readHash(std::filesystem::path filepath);
  void
  readColl();
  void
  collRefresh();
  void
  collRefreshFb2(std::string rand);
  void
  collRefreshZip(std::string rand);
  void
  collRefreshEpub(std::string rand);
  void
  collRefreshPdf(std::string rand);
  void
  collRefreshDjvu(std::string rand);
  void
  fb2ThrFunc(std::filesystem::path p);
  void
  epubThrFunc(std::filesystem::path p);
  void
  pdfThrFunc(std::filesystem::path p);
  void
  djvuThrFunc(std::filesystem::path p);
  void
  zipThrFunc(
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>> ziptup);
  void
  editBook(std::string book_str, std::string filename,
	   std::vector<std::tuple<std::string, std::string>> *newbasev);
  void
  editBookZip(std::string book_str, std::string filename,
	      std::vector<std::tuple<std::string, std::string>> *newbasev,
	      std::vector<std::tuple<std::string, std::string>> *oldbasev);

  std::string collname;
  bool fast_refresh = false;
  int *cancel = nullptr;
  std::vector<std::tuple<std::filesystem::path, std::string>> saved_hashes;
  std::vector<std::filesystem::path> fb2parse;
  std::mutex fb2parsemtx;
  std::vector<std::filesystem::path> epubparse;
  std::mutex epubparsemtx;
  std::vector<std::filesystem::path> pdfparse;
  std::mutex pdfparsemtx;
  std::vector<std::filesystem::path> djvuparse;
  std::mutex djvuparsemtx;
  std::vector<
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>>> zipparse;
  std::vector<std::tuple<std::filesystem::path, std::vector<char>>> already_hashed;
  std::mutex already_hashedmtx;
  std::mutex zipparsemtx;
  std::vector<std::filesystem::path> fb2remove;
  std::vector<std::filesystem::path> zipremove;
  std::vector<std::filesystem::path> epubremove;
  std::vector<std::filesystem::path> pdfremove;
  std::vector<std::filesystem::path> djvuremove;
  unsigned int thr_num = 1;
  unsigned int run_thr = 0;
  std::mutex run_thrmtx;
  std::mutex cmtx;
  std::string bookpath;
};

#endif /* INCLUDE_REFRESHCOLLECTION_H_ */
