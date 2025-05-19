/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef DCPARSER_H
#define DCPARSER_H

#include <AuxFunc.h>
#include <XMLParser.h>

/*!
 * \brief The DCParser class
 *
 * Auxiliary class. Contains methods for <a
 * href="https://www.dublincore.org/">DublinCore</a> files parsing. This class
 * is used in ODTParser and EPUBParser. You do not need to call this class
 * methods directly.
 */
class DCParser : public XMLParser
{
public:
  /*!
   * \brief DCParser constructor.
   * \param af smart pointer to AuxFunc object.
   */
  DCParser(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Gets book title.
   *
   * This method can be used to get title from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return Title if any, empty string otherwise.
   */
  std::string
  dcTitle(const std::string &dc_file_content, const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book author.
   *
   * This method can be used to get author(s) from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return Author if any, empty string otherwise.
   */
  std::string
  dcAuthor(const std::string &dc_file_content, const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book genre.
   *
   * This method can be used to get genre(s) from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return Gernre if any, empty string otherwise.
   */
  std::string
  dcGenre(const std::string &dc_file_content, const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book creation date.
   *
   * This method can be used to get creation date from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return File creation date if any, empty otherwise.
   */
  std::string
  dcDate(const std::string &dc_file_content, const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book language.
   *
   * This method can be used to get language from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return File language if set, empty otherwise.
   */
  std::string
  dcLanguage(const std::string &dc_file_content,
             const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets file publisher.
   *
   * This method can be used to get publisher from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return File publisher if set, empty otherwise.
   */
  std::string
  dcPublisher(const std::string &dc_file_content,
              const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book identifier.
   *
   * This method can be used to get identifier from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return Book identifier if any, empty otherwise.
   */
  std::string
  dcIdentifier(const std::string &dc_file_content,
               const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book source.
   *
   * This method can be used to get book source from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return Book source if set, empty otherwise.
   */
  std::string
  dcSource(const std::string &dc_file_content, const std::vector<XMLTag> &tgv);

  /*!
   * \brief Gets book description.
   *
   * This method can be used to get book description from <a
   * href="https://www.dublincore.org/">DublinCore</a> file.
   *
   * \param dc_file_content string containing file content.
   * \param tgv XMLTag vector obtained from XMLParser::listAllTags method.
   * \return Book description if any, empty otherwise.
   */
  std::string
  dcDescription(const std::string &dc_file_content,
                const std::vector<XMLTag> &tgv);

private:
  std::shared_ptr<AuxFunc> af;
};

#endif // DCPARSER_H
