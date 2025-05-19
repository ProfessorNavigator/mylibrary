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
#include <MLException.h>
#include <ODTParser.h>
#include <algorithm>
#include <iostream>

ODTParser::ODTParser(const std::shared_ptr<AuxFunc> &af)
    : XMLParser(af), LibArchive(af)
{
  this->af = af;
  dc = new DCParser(af);
}

ODTParser::~ODTParser()
{
  delete dc;
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
              = std::find_if(files.begin(), files.end(), [sp](ArchEntry &el) {
                  std::filesystem::path fp
                      = std::filesystem::u8path(el.filename);
                  return fp.filename() == sp;
                });
          if(it != files.end())
            {
              std::string meta_content = unpackByPositionStr(odt_path, *it);
              std::vector<XMLTag> meta_tags = listAllTags(meta_content);
              bpe.book_name = dc->dcTitle(meta_content, meta_tags);
              if(bpe.book_name.empty())
                {
                  bpe.book_name = odt_path.stem().u8string();
                }

              bpe.book_author = dc->dcAuthor(meta_content, meta_tags);

              bpe.book_genre = dc->dcGenre(meta_content, meta_tags);

              bpe.book_date = dc->dcDate(meta_content, meta_tags);
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
      throw MLException("ODTParser::odtParser: not ODT file");
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
              = std::find_if(files.begin(), files.end(), [sp](ArchEntry &el) {
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
          it = std::find_if(files.begin(), files.end(), [sp](ArchEntry &el) {
            std::filesystem::path fp = std::filesystem::u8path(el.filename);
            return fp == sp;
          });
          if(it != files.end())
            {
              std::string meta_content = unpackByPositionStr(odt_path, *it);
              std::vector<XMLTag> tgv = listAllTags(meta_content);
              bie->language = dc->dcLanguage(meta_content, tgv);
              bie->electro->publisher = dc->dcPublisher(meta_content, tgv);
              if(!bie->electro->publisher.empty())
                {
                  bie->electro->available = true;
                }
              bie->electro->id = dc->dcIdentifier(meta_content, tgv);
              if(!bie->electro->id.empty())
                {
                  bie->electro->available = true;
                }
              bie->electro->src_url = dc->dcSource(meta_content, tgv);
              if(!bie->electro->src_url.empty())
                {
                  bie->electro->available = true;
                }
              bie->annotation = dc->dcDescription(meta_content, tgv);
            }
        }
      else
        {
          std::cout << "ODTParser::odtBookInfo error" << std::endl;
        }
    }
  else
    {
      throw MLException("ODTParser::odtBookInfo: not ODT file");
    }

  return bie;
}
