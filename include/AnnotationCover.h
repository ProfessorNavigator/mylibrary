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

#ifndef ANNOTATIONCOVER_H_
#define ANNOTATIONCOVER_H_

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "AuxFunc.h"

class AnnotationCover
{
public:
  AnnotationCover(std::string filename);
  virtual
  ~AnnotationCover();
  std::string
  annotationRet();
  std::string
  coverRet();
private:
  std::string
  annotationEpub();
  std::string
  coverEpub();
  void
  fileRead();
  std::string rcvd_filename = "";
  std::filesystem::path epub_path;
  std::string file = "";
  bool epub_ch_f = false;
};

#endif /* ANNOTATIONCOVER_H_ */
