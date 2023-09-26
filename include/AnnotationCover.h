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

#ifndef INCLUDE_ANNOTATIONCOVER_H_
#define INCLUDE_ANNOTATIONCOVER_H_

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <poppler-document.h>
#include <poppler-image.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>
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
  void
  fb2Parse(std::filesystem::path filepath);
  void
  epubParse(std::filesystem::path filepath);
  void
  pdfParse(std::filesystem::path filepath);
  void
  pdfZipParse(std::filesystem::path temp_path,
	      std::filesystem::path unpacked_path, std::string archaddr);
  void
  djvuParse(std::filesystem::path filepath);

  void
  handle_ddjvu_messages(ddjvu_context_t *ctx, bool wait);
  std::string rcvd_filename = "";
  std::filesystem::path epub_path;
  std::filesystem::path pdf_cover_path;
  std::string file;
  std::string djvu_cover_bufer;
  bool fb2_ch_f = false;
  bool zip_ch_f = false;
  bool epub_ch_f = false;
  bool pdf_ch_f = false;
  bool djvu_ch_f = false;
};

#endif /* INCLUDE_ANNOTATIONCOVER_H_ */
