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

#ifndef INCLUDE_AUXFUNC_H_
#define INCLUDE_AUXFUNC_H_
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <functional>
#include <vector>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <zip.h>
#include <gcrypt.h>

#ifdef _WIN32
#include <Windows.h>
#endif

class AuxFunc
{
public:
  AuxFunc();
  virtual
  ~AuxFunc();
  void
  homePath(std::string *filename);
  void
  toutf8(std::string &line);
  void
  toutf8(std::string &line, std::string conv_name);
  std::string
  get_selfpath();
  int
  fileNames(std::string address,
	    std::vector<std::tuple<int, int, std::string>> &filenames);
  std::string
  randomFileName();
  void
  unpackByIndex(std::string archaddress, std::string outfolder, int index);
  std::string
  unpackByIndex(std::string archaddress, int index, size_t filesz);
  int
  packing(std::string source, std::string out);
  void
  stringToLower(std::string &line);
  std::vector<char>
  filehash(std::filesystem::path filepath, std::shared_ptr<int> cancel);
  std::vector<char>
  filehash(std::filesystem::path filepath, std::function<void
  (uint64_t)> progress,
	   std::shared_ptr<int> cancel);
  std::string
  to_hex(std::vector<char> *source);
  std::string
  utf8to(std::string line);
  int
  removeFmArch(std::string archpath, uint64_t index);
  std::vector<std::tuple<std::string, std::string>>
  fileinfo(std::string address, int index);
};

#endif /* INCLUDE_AUXFUNC_H_ */
