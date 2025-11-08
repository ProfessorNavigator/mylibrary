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

#include <ODTParser.h>
#include <algorithm>
#include <iostream>

ODTParser::ODTParser(const std::shared_ptr<AuxFunc> &af) : LibArchive(af)
{
  this->af = af;
  dc = new DublinCoreParser;
  xml_parser = new XMLParserCPP;
}

ODTParser::~ODTParser()
{
  delete dc;
  delete xml_parser;
}

BookParseEntry
ODTParser::odtParser(const std::filesystem::path &odt_path)
{
  BookParseEntry bpe;

  if(odt_path.extension().u8string() == ".odt")
    {
      std::vector<ArchEntry> files;
      if(fileNames(odt_path, files) > 0)
        {
          std::filesystem::path sp = std::filesystem::u8path("meta.xml");
          auto it
              = std::find_if(files.begin(), files.end(),
                             [sp](ArchEntry &el)
                               {
                                 std::filesystem::path fp
                                     = std::filesystem::u8path(el.filename);
                                 return fp.filename() == sp;
                               });
          if(it != files.end())
            {
              std::string meta_content = unpackByPositionStr(odt_path, *it);
              try
                {
                  std::vector<XMLElement> elements
                      = xml_parser->parseDocument(meta_content);
                  bpe.book_name = dc->dcTitle(elements);
                  if(bpe.book_name.empty())
                    {
                      bpe.book_name = odt_path.stem().u8string();
                    }

                  bpe.book_author = dc->dcAuthor(elements);

                  bpe.book_genre = dc->dcGenre(elements);

                  bpe.book_date = dc->dcDate(elements);
                }
              catch(std::exception &er)
                {
                  std::cout << "ODTParser::odtParser: \"" << er.what() << "\""
                            << std::endl;
                  bpe.book_name = odt_path.stem().u8string();

                  std::filesystem::file_time_type cr
                      = std::filesystem::last_write_time(odt_path);

                  auto sctp = std::chrono::time_point_cast<
                      std::chrono::system_clock::duration>(
                      cr - std::filesystem::file_time_type::clock::now()
                      + std::chrono::system_clock::now());
                  time_t tt = std::chrono::system_clock::to_time_t(sctp);

                  bpe.book_date = af->time_t_to_date(tt);
                }
            }
          else
            {
              std::cout << "ODTParser::odtParser: cannot finde meta.xml"
                        << std::endl;
              bpe.book_name = odt_path.stem().u8string();

              std::filesystem::file_time_type cr
                  = std::filesystem::last_write_time(odt_path);

              auto sctp = std::chrono::time_point_cast<
                  std::chrono::system_clock::duration>(
                  cr - std::filesystem::file_time_type::clock::now()
                  + std::chrono::system_clock::now());
              time_t tt = std::chrono::system_clock::to_time_t(sctp);

              bpe.book_date = af->time_t_to_date(tt);
            }
        }
      else
        {
          bpe.book_name = odt_path.stem().u8string();

          std::filesystem::file_time_type cr
              = std::filesystem::last_write_time(odt_path);

          auto sctp = std::chrono::time_point_cast<
              std::chrono::system_clock::duration>(
              cr - std::filesystem::file_time_type::clock::now()
              + std::chrono::system_clock::now());
          time_t tt = std::chrono::system_clock::to_time_t(sctp);

          bpe.book_date = af->time_t_to_date(tt);
        }
    }
  else
    {
      throw std::runtime_error("ODTParser::odtParser: not ODT file");
    }

  return bpe;
}

std::shared_ptr<BookInfoEntry>
ODTParser::odtBookInfo(const std::filesystem::path &odt_path)
{
  std::shared_ptr<BookInfoEntry> bie = std::make_shared<BookInfoEntry>();

  if(odt_path.extension().u8string() == ".odt")
    {
      std::vector<ArchEntry> files;
      if(fileNames(odt_path, files) > 0)
        {
          std::filesystem::path sp
              = std::filesystem::u8path("Thumbnails")
                / std::filesystem::u8path("thumbnail.png");
          auto it
              = std::find_if(files.begin(), files.end(),
                             [sp](ArchEntry &el)
                               {
                                 std::filesystem::path fp
                                     = std::filesystem::u8path(el.filename);
                                 return fp == sp;
                               });
          if(it != files.end())
            {
              bie->cover = unpackByPositionStr(odt_path, *it);
              bie->cover_type = BookInfoEntry::cover_types::file;
            }

          sp = std::filesystem::u8path("meta.xml");
          it = std::find_if(files.begin(), files.end(),
                            [sp](ArchEntry &el)
                              {
                                std::filesystem::path fp
                                    = std::filesystem::u8path(el.filename);
                                return fp == sp;
                              });
          if(it != files.end())
            {
              std::string meta_content = unpackByPositionStr(odt_path, *it);

              try
                {
                  std::vector<XMLElement> elements
                      = xml_parser->parseDocument(meta_content);

                  bie->language = dc->dcLanguage(elements);
                  bie->electro->publisher = dc->dcPublisher(elements);
                  if(!bie->electro->publisher.empty())
                    {
                      bie->electro->available = true;
                    }
                  bie->electro->id = dc->dcIdentifier(elements);
                  if(!bie->electro->id.empty())
                    {
                      bie->electro->available = true;
                    }
                  bie->electro->src_url = dc->dcSource(elements);
                  if(!bie->electro->src_url.empty())
                    {
                      bie->electro->available = true;
                    }
                  bie->annotation = dc->dcDescription(elements);
                }
              catch(std::exception &er)
                {
                  std::cout << "ODTParser::odtBookInfo: \"" << er.what()
                            << "\"" << std::endl;
                }
            }
        }
      else
        {
          std::cout << "ODTParser::odtBookInfo error" << std::endl;
        }
    }
  else
    {
      throw std::runtime_error("ODTParser::odtBookInfo: not ODT file");
    }

  return bie;
}
