/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EPUBPARSER_H
#define EPUBPARSER_H

#include <ArchEntry.h>
#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <DublinCoreParser.h>
#include <LibArchive.h>
#include <XMLAlgorithms.h>
#include <XMLParserCPP.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

/*!
 * \brief The EPUBParser class.
 *
 * This class contains various methods for epub books parsing. In most cases
 * you do not need to use this class directly. Use CreateCollection,
 * RefreshCollection and BookInfo instead.
 */
class EPUBParser : public LibArchive
{
public:
  /*!
   * \brief EPUBParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  EPUBParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief EPUBParser destructor.
   */
  virtual ~EPUBParser();

  /*!
   * \brief Parses epub book.
   *
   * \deprecated This method will be removed in future releases. Use
   * epubParser() instead.
   *
   * \param filepath absolute path to epub file.
   * \return BookParseEntry object.
   */
  __attribute__((deprecated)) BookParseEntry
  epub_parser(const std::filesystem::path &filepath);

  /*!
   * \brief Parses epub book.
   *
   * \note This method can throw std::exception in case of error.
   *
   * \param filepath absolute path to epub file.
   * \return BookParseEntry object.
   */
  BookParseEntry
  epubParser(const std::filesystem::path &filepath);

  /*!
   * \brief Returns epub book info and cover.
   *
   * \deprecated This method will be removed in future releases. Use
   * epubBookInfo() instead.
   *
   * \param filepath absolute path to epub file.
   * \return Smart pointer to BookInfoEntry object.
   */
  __attribute__((deprecated)) std::shared_ptr<BookInfoEntry>
  epub_book_info(const std::filesystem::path &filepath);

  /*!
   * \brief Returns epub book info and cover.
   * \param filepath absolute path to epub file.
   * \return Smart pointer to BookInfoEntry object.
   */
  std::shared_ptr<BookInfoEntry>
  epubBookInfo(const std::filesystem::path &filepath);

private:
  std::string
  epubGetRootFileAddress(const std::filesystem::path &filepath,
                         const std::vector<ArchEntry> &filenames);

  BookParseEntry
  epubParseRootFile(const std::string &root_file_content);

  std::string
  epubAnnotation(const std::vector<XMLElement> &elements);

  std::string
  epubCoverAddress(const std::vector<XMLElement> &elements,
                   const std::filesystem::path &filepath,
                   const std::vector<ArchEntry> &filenames);

  void
  epubLanguage(const std::vector<XMLElement> &elements, BookInfoEntry &result);

  void
  epubTranslator(const std::vector<XMLElement> &elements,
                 BookInfoEntry &result);

  void
  epubPublisher(const std::vector<XMLElement> &elements,
                BookInfoEntry &result);

  void
  epubIdentifier(const std::vector<XMLElement> &elements,
                 BookInfoEntry &result);

  void
  epubSource(const std::vector<XMLElement> &elements, BookInfoEntry &result);

  std::string
  getEncoding(const std::vector<XMLElement> &elements);

  std::shared_ptr<AuxFunc> af;

  DublinCoreParser *dc;

  XMLParserCPP *xml_parser;
};

#endif // EPUBPARSER_H
