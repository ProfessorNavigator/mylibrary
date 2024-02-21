/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef INCLUDE_EPUBPARSER_H_
#define INCLUDE_EPUBPARSER_H_

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <LibArchive.h>
#include <XMLParser.h>
#include <XMLTag.h>
#include <ZipFileEntry.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class EPUBParser : public XMLParser, public LibArchive
{
public:
  EPUBParser(const std::shared_ptr<AuxFunc> &af);
  virtual
  ~EPUBParser();

  BookParseEntry
  epub_parser(const std::filesystem::path &filepath);

  std::shared_ptr<BookInfoEntry>
  epub_book_info(const std::filesystem::path &filepath);

private:
  std::string
  epub_get_root_file_address(
      const std::filesystem::path &filepath,
      const std::vector<ZipFileEntry> &filenames);

  BookParseEntry
  epub_parse_root_file(const std::string &root_file_content);

  std::string
  epub_annotation(const std::string &root_file_content);

  std::string
  epub_cover_address(const std::string &root_file_content);

  bool
  epub_find_role_in_meta(XMLTag &tag, const std::string &id,
			 const std::string &role);

  void
  epub_language(const std::string &root_file_content,
		BookInfoEntry &result);

  void
  epub_translator(const std::string &root_file_content,
		  BookInfoEntry &result);

  std::vector<std::string>
  epub_get_tags_by_role(const std::string &root_file_content,
			const std::string &tag_val,
			const std::string &role_input,
			const bool &exact_search);

  void
  epub_publisher(const std::string &root_file_content,
		 BookInfoEntry &result);

  void
  epub_identifier(const std::string &root_file_content,
		  BookInfoEntry &result);

  void
  epub_source(const std::string &root_file_content,
	      BookInfoEntry &result);

  std::shared_ptr<AuxFunc> af;
};

#endif /* INCLUDE_EPUBPARSER_H_ */
