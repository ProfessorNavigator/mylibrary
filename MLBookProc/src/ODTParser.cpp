/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <XMLAlgorithms.h>
#include <algorithm>

ODTParser::ODTParser(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;

  la = new LibArchive(mlbp);
  dc_parser = new DublinCoreParser;
  xml_parser = new XMLParserCPP;
}

ODTParser::~ODTParser()
{
  delete la;
  delete dc_parser;
  delete xml_parser;
}

UDBElement
ODTParser::parseBook(const std::string &book_content)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> files;
  la->listFilesInZipBuffer(book_content, files);

  std::filesystem::path sp = std::filesystem::path(u8"meta.xml");
  auto it = std::find_if(
      files.begin(), files.end(),
      [sp](std::tuple<std::string, uint64_t, uint64_t> &el)
        {
          std::filesystem::path fp = std::filesystem::path(
              std::u8string(std::get<0>(el).begin(), std::get<0>(el).end()));
          return fp.filename() == sp;
        });
  if(it != files.end())
    {
      std::string meta_content
          = la->unpackZipBufferFileToBuffer(book_content, std::get<0>(*it));
      std::vector<XMLElement> elements
          = xml_parser->parseDocument(meta_content);

      std::vector<UDBElement> res = dc_parser->dcTitle(elements);
      std::copy(res.begin(), res.end(),
                std::back_inserter(result.subelements));

      res = dc_parser->dcAuthor(elements);
      std::copy(res.begin(), res.end(),
                std::back_inserter(result.subelements));

      res = dc_parser->dcGenre(elements);
      std::copy(res.begin(), res.end(),
                std::back_inserter(result.subelements));

      res = dc_parser->dcDate(elements);
      std::copy(res.begin(), res.end(),
                std::back_inserter(result.subelements));
    }

  return result;
}

UDBase
ODTParser::getBookInfo(const std::string &book_content)
{
  UDBase result;

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> filenames;
  la->listFilesInZipBuffer(book_content, filenames);

  auto it
      = std::find_if(filenames.begin(), filenames.end(),
                     [](const std::tuple<std::string, uint64_t, uint64_t> &el)
                       {
                         return std::get<0>(el) == "meta.xml";
                       });
  if(it != filenames.end())
    {
      std::string buf = la->unpackBufferFileToBuffer(
          book_content, std::get<0>(*it), std::get<2>(*it));
      if(buf.size() > 0)
        {
          std::vector<XMLElement> elements = xml_parser->parseDocument(buf);
          UDBElement el;
          bid.setId(el, BaseID::Annotation);
          el.content = dc_parser->dcDescription(elements);
          normalizeString(el.content);
          if(!el.content.empty())
            {
              result.addElement(el);
            }

          std::vector<UDBElement> res = dc_parser->dcLanguage(elements);
          result.addElements(res);

          res = dc_parser->dcTranslator(elements);
          result.addElements(res);

          res = dc_parser->dcPublisher(elements);
          result.addElements(res);

          res = dc_parser->dcIdentifier(elements);
          result.addElements(res);

          res = dc_parser->dcSource(elements);
          result.addElements(res);

          std::vector<XMLElement *> meta;
          XMLAlgorithms::searchElement(elements, "meta:generator", meta);
          for(size_t i = 0; i < meta.size(); i++)
            {
              std::vector<XMLElement *> res2;
              XMLAlgorithms::searchElement(meta, XMLElement::ElementContent,
                                           res2);
              if(res2.size() == 0)
                {
                  continue;
                }
              el = UDBElement();
              bid.setId(el, BaseID::EbookProgramUsed);
              el.content = res2[0]->content;
              normalizeString(el.content);
              if(!el.content.empty())
                {
                  result.addElement(el);
                }
            }
        }
    }

  it = std::find_if(filenames.begin(), filenames.end(),
                    [](const std::tuple<std::string, uint64_t, uint64_t> &el)
                      {
                        return std::get<0>(el) == "Thumbnails/thumbnail.png";
                      });
  if(it != filenames.end())
    {
      UDBElement el;
      bid.setId(el, BaseID::CoverPage);
      el.content = la->unpackBufferFileToBuffer(book_content, std::get<0>(*it),
                                                std::get<2>(*it));
      if(!el.content.empty())
        {
          UDBElement type;
          bid.setId(type, BaseID::CoverType);
          type.content = "image";
          el.subelements.emplace_back(type);

          result.addElement(el);
        }
    }

  result.shrinkToFit();

  return result;
}

void
ODTParser::normalizeString(std::string &str)
{
  while(str.size() > 0)
    {
      char el = *str.rbegin();
      if(el >= 0 && el <= ' ')
        {
          str.pop_back();
        }
      else
        {
          break;
        }
    }

  for(auto it = str.begin(); it != str.end();)
    {
      if(*it >= 0 && *it <= ' ')
        {
          str.erase(it);
        }
      else
        {
          break;
        }
    }
}
