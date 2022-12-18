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

#ifndef AUXFUNC_H
#define AUXFUNC_H

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <filesystem>
#include <fstream>
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
  fileNames(std::string adress,
	    std::vector<std::tuple<int, int, std::string>> &filenames);
  std::string
  randomFileName();
  void
  unpackByIndex(std::string archadress, std::string outfolder, int index);
  std::string
  unpackByIndex(std::string archadress, int index, size_t filesz);
  void
  stringToLower(std::string &line);
  std::vector<char>
  filehash(std::filesystem::path filepath);
  std::string
  to_hex(std::vector<char> *source);
  std::string
  utf8to(std::string line);
  int
  removeFmArch(std::string archpath, uint64_t index);
};

#endif // AUXFUNC_H
