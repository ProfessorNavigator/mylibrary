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

#include <EPUBParser.h>
#include <XMLAlgorithms.h>
#include <algorithm>
#include <filesystem>
#include <iostream>

EPUBParser::EPUBParser(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
  dc_parser = new DublinCoreParser;
  xml_parser = new XMLParserCPP;
  arch = new LibArchive(mlbp);
}

EPUBParser::~EPUBParser()
{
  delete dc_parser;
  delete xml_parser;
  delete arch;
}

UDBElement
EPUBParser::parseBook(const std::string &book_content)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> filenames;
  arch->listFilesInZipBuffer(book_content, filenames);

  std::string rootfile = epubGetRootFileAddress(book_content, filenames);

  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(std::get<0>(*it) == rootfile)
            {
              root_file_content = arch->unpackBufferFileToBuffer(
                  book_content, rootfile,
                  static_cast<size_t>(std::get<2>(*it)));
              break;
            }
        }
    }

  if(!root_file_content.empty())
    {
      epubParseRootFile(root_file_content, result);
    }

  return result;
}

UDBase
EPUBParser::getBookInfo(const std::string &book_content)
{
  UDBase result;

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> filenames;
  arch->listFilesInZipBuffer(book_content, filenames);

  std::string rootfile = epubGetRootFileAddress(book_content, filenames);

  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(std::get<0>(*it) == rootfile)
            {
              root_file_content = arch->unpackBufferFileToBuffer(
                  book_content, rootfile,
                  static_cast<size_t>(std::get<2>(*it)));
              break;
            }
        }
    }

  if(root_file_content.empty())
    {
      return result;
    }

  std::vector<XMLElement> doc = xml_parser->parseDocument(root_file_content);

  UDBElement el;
  bid.setId(el, BaseID::Annotation);
  el.content = dc_parser->dcDescription(doc);
  normalizeString(el.content);
  if(!el.content.empty())
    {
      result.addElement(el);
    }

  el = UDBElement();
  bid.setId(el, BaseID::CoverPage);
  std::string cover_path = epubCoverAddress1(doc);
  if(cover_path.empty())
    {
      cover_path = epubCoverAddress2(doc, rootfile, book_content, filenames);
    }
  else
    {
      std::filesystem::path p
          = std::u8string(rootfile.begin(), rootfile.end());
      std::u8string parent = p.parent_path().u8string();

      if(!parent.empty())
        {
          cover_path
              = std::string(parent.begin(), parent.end()) + "/" + cover_path;
        }
    }

  if(!cover_path.empty())
    {
      auto it = std::find_if(
          filenames.begin(), filenames.end(),
          [cover_path](const std::tuple<std::string, uint64_t, uint64_t> &el)
            {
              return std::get<0>(el) == cover_path;
            });
      if(it != filenames.end())
        {
          el.content = arch->unpackBufferFileToBuffer(book_content, cover_path,
                                                      std::get<2>(*it));
          if(!el.content.empty())
            {
              UDBElement type;
              bid.setId(type, BaseID::CoverType);
              type.content = "image";
              el.subelements.emplace_back(type);
            }
        }

      if(!el.content.empty())
        {
          result.addElement(el);
        }
    }

  std::vector<UDBElement> res = dc_parser->dcLanguage(doc);
  result.addElements(res);

  res = dc_parser->dcTranslator(doc);
  result.addElements(res);

  res = dc_parser->dcPublisher(doc);
  result.addElements(res);

  res = dc_parser->dcIdentifier(doc);
  result.addElements(res);

  res = dc_parser->dcSource(doc);
  result.addElements(res);

  result.shrinkToFit();

  return result;
}

std::string
EPUBParser::epubGetRootFileAddress(
    const std::string &book_content,
    const std::vector<std::tuple<std::string, uint64_t, uint64_t>> &filenames)
{
  std::string buf;
  for(auto it = filenames.begin(); it != filenames.end(); it++)
    {
      if(std::get<0>(*it) == "META-INF/container.xml")
        {
          buf = arch->unpackBufferFileToBuffer(
              book_content, std::get<0>(*it),
              static_cast<size_t>(std::get<2>(*it)));
          break;
        }
    }

  std::string content;
  if(buf.size() > 0)
    {
      std::vector<XMLElement> elements;
      elements = xml_parser->parseDocument(buf);

      std::vector<XMLElement *> res;
      XMLAlgorithms::searchElement(elements, "rootfiles", res);
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res, "rootfile", "full-path", res2);
      if(res2.size() > 0)
        {
          auto it = std::find_if(res2[0]->element_attributes.begin(),
                                 res2[0]->element_attributes.end(),
                                 [](const XMLElementAttribute &el)
                                   {
                                     return el.attribute_id == "full-path";
                                   });
          if(it != res2[0]->element_attributes.end())
            {
              content = it->attribute_value;
            }
        }
    }
  return content;
}

void
EPUBParser::epubParseRootFile(const std::string &root_file_content,
                              UDBElement &result)
{
  std::vector<XMLElement> elements;

  elements = xml_parser->parseDocument(root_file_content);

  std::vector<UDBElement> res = dc_parser->dcTitle(elements);
  std::copy(res.begin(), res.end(), std::back_inserter(result.subelements));

  res = dc_parser->dcAuthor(elements);
  std::copy(res.begin(), res.end(), std::back_inserter(result.subelements));

  res = dc_parser->dcGenre(elements);
  std::copy(res.begin(), res.end(), std::back_inserter(result.subelements));

  res = dc_parser->dcDate(elements);
  std::copy(res.begin(), res.end(), std::back_inserter(result.subelements));
}

std::string
EPUBParser::epubCoverAddress1(const std::vector<XMLElement> &root_file_content)
{
  std::string result;

  std::vector<XMLElement *> manifest;
  XMLAlgorithms::searchElement(root_file_content, "manifest", manifest);

  std::vector<XMLElement *> image;

  XMLAlgorithms::searchElement(manifest,
                               std::bind(&EPUBParser::imageSearchFunction1,
                                         this, std::placeholders::_1),
                               image);

  for(size_t i = 0; i < image.size(); i++)
    {
      std::vector<XMLElementAttribute>::iterator it
          = std::find_if(image[i]->element_attributes.begin(),
                         image[i]->element_attributes.end(),
                         [](const XMLElementAttribute &el)
                           {
                             return el.attribute_id == "href";
                           });
      if(it == image[i]->element_attributes.end())
        {
          continue;
        }
      result = it->attribute_value;
      break;
    }

  return result;
}

std::string
EPUBParser::epubCoverAddress2(
    const std::vector<XMLElement> &root_file_content,
    const std::string &root_file_path, const std::string &book_content,
    const std::vector<std::tuple<std::string, uint64_t, uint64_t>> &filenames)
{
  std::string result;

  std::vector<XMLElement *> manifest;
  XMLAlgorithms::searchElement(root_file_content, "manifest", manifest);

  std::vector<XMLElement *> image;
  XMLAlgorithms::searchElement(manifest,
                               std::bind(&EPUBParser::imageSearchFunction2,
                                         this, std::placeholders::_1),
                               image);
  for(size_t i = 0; i < image.size(); i++)
    {
      std::vector<XMLElementAttribute>::iterator it
          = std::find_if(image[i]->element_attributes.begin(),
                         image[i]->element_attributes.end(),
                         [](const XMLElementAttribute &el)
                           {
                             return el.attribute_id == "href";
                           });
      if(it == image[i]->element_attributes.end())
        {
          continue;
        }
      result = it->attribute_value;
      break;
    }

  if(result.empty())
    {
      return result;
    }

  std::filesystem::path p
      = std::u8string(root_file_path.begin(), root_file_path.end());
  std::u8string parent = p.parent_path().u8string();
  if(!parent.empty())
    {
      result = std::string(parent.begin(), parent.end()) + "/" + result;
    }

  auto it_fnm = std::find_if(
      filenames.begin(), filenames.end(),
      [result](const std::tuple<std::string, uint64_t, uint64_t> &el)
        {
          return std::get<0>(el) == result;
        });
  if(it_fnm == filenames.end())
    {
      result.clear();
      return result;
    }

  std::string buf = arch->unpackBufferFileToBuffer(book_content, result,
                                                   std::get<2>(*it_fnm));
  if(buf.empty())
    {
      result.clear();
      return result;
    }

  std::vector<XMLElement> elements;
  try
    {
      elements = xml_parser->parseDocument(buf);
    }
  catch(std::exception &er)
    {
      std::cout << "EPUBParser::epubCoverAddress2: \"" << er.what() << "\""
                << std::endl;
      result.clear();
      return result;
    }

  image.clear();
  XMLAlgorithms::searchElement(elements, "img", image);
  size_t i;
  for(i = 0; i < image.size(); i++)
    {
      std::vector<XMLElementAttribute>::iterator it
          = std::find_if(image[i]->element_attributes.begin(),
                         image[i]->element_attributes.end(),
                         [](const XMLElementAttribute &el)
                           {
                             return el.attribute_id == "src";
                           });
      if(it == image[i]->element_attributes.end())
        {
          continue;
        }
      p = std::u8string(result.begin(), result.end());
      parent = p.parent_path().u8string();
      if(parent.empty())
        {
          result = it->attribute_value;
        }
      else
        {

          result = std::string(parent.begin(), parent.end()) + "/"
                   + it->attribute_value;
        }
      break;
    }
  if(i >= image.size())
    {
      result.clear();
    }

  return result;
}

bool
EPUBParser::imageSearchFunction1(const XMLElement &el)
{
  bool result = false;

  std::vector<XMLElementAttribute>::const_iterator it = std::find_if(
      el.element_attributes.begin(), el.element_attributes.end(),
      [](const XMLElementAttribute &el)
        {
          return el.attribute_id == "media-type";
        });
  if(it == el.element_attributes.end())
    {
      return result;
    }

  std::string find_str("image");
  std::string::size_type n = it->attribute_value.find(find_str);
  if(n == std::string::npos)
    {
      return result;
    }

  it = std::find_if(el.element_attributes.begin(), el.element_attributes.end(),
                    [](const XMLElementAttribute &el)
                      {
                        return el.attribute_id == "id";
                      });
  if(it == el.element_attributes.end())
    {
      return result;
    }

  find_str = "cover";
  std::string str = mlbp->stringToLower(it->attribute_value);
  n = str.find(find_str);
  if(n != std::string::npos)
    {
      result = true;
    }

  return result;
}

bool
EPUBParser::imageSearchFunction2(const XMLElement &el)
{
  bool result = false;

  std::vector<XMLElementAttribute>::const_iterator it = std::find_if(
      el.element_attributes.begin(), el.element_attributes.end(),
      [](const XMLElementAttribute &el)
        {
          return el.attribute_id == "media-type";
        });
  if(it == el.element_attributes.end())
    {
      return result;
    }

  if(it->attribute_value != "application/xhtml+xml")
    {
      return result;
    }

  it = std::find_if(el.element_attributes.begin(), el.element_attributes.end(),
                    [](const XMLElementAttribute &el)
                      {
                        return el.attribute_id == "id";
                      });
  if(it == el.element_attributes.end())
    {
      return result;
    }

  std::string str = mlbp->stringToLower(it->attribute_value);
  std::string::size_type n = str.find("cover");
  if(n != std::string::npos)
    {
      result = true;
    }

  return result;
}

void
EPUBParser::normalizeString(std::string &str)
{
  while(str.size() > 0)
    {
      char val = *str.rbegin();
      if(val >= 0 && val <= ' ')
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
