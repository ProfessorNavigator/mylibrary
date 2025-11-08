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

#include <EPUBParser.h>
#include <XMLTag.h>
#include <algorithm>
#include <iostream>

EPUBParser::EPUBParser(const std::shared_ptr<AuxFunc> &af) : LibArchive(af)
{
  this->af = af;
  dc = new DublinCoreParser;
  xml_parser = new XMLParserCPP;
}

EPUBParser::~EPUBParser()
{
  delete dc;
  delete xml_parser;
}

BookParseEntry
EPUBParser::epub_parser(const std::filesystem::path &filepath)
{
  return epubParser(filepath);
}

BookParseEntry
EPUBParser::epubParser(const std::filesystem::path &filepath)
{
  BookParseEntry be;
  std::vector<ArchEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epubGetRootFileAddress(filepath, filenames);
  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(it->filename == rootfile)
            {
              root_file_content = unpackByPositionStr(filepath, *it);
            }
        }
    }

  if(!root_file_content.empty())
    {
      be = epubParseRootFile(root_file_content);
    }

  return be;
}

std::string
EPUBParser::epubGetRootFileAddress(const std::filesystem::path &filepath,
                                   const std::vector<ArchEntry> &filenames)
{
  std::string buf;
  for(auto it = filenames.begin(); it != filenames.end(); it++)
    {
      if(it->filename == "META-INF/container.xml")
        {
          buf = unpackByPositionStr(filepath, *it);
          break;
        }
    }

  std::string content;
  if(buf.size() > 0)
    {
      std::vector<XMLElement> elements;
      std::string code_page;
      try
        {
          elements = xml_parser->parseDocument(buf);
          code_page = getEncoding(elements);
        }
      catch(std::exception &er)
        {
          std::cout << "EPUBParser::epubGetRootFileAddress: " << er.what()
                    << std::endl;
        }

      if(code_page.empty())
        {
          code_page = af->detectEncoding(buf);
        }

      if(code_page != "UTF-8")
        {
          buf = af->toUTF8(buf, code_page.c_str());
          elements = xml_parser->parseDocument(buf);
        }
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

BookParseEntry
EPUBParser::epubParseRootFile(const std::string &root_file_content)
{
  BookParseEntry result;

  std::vector<XMLElement> elements;
  std::string code_page;
  try
    {
      elements = xml_parser->parseDocument(root_file_content);
      code_page = getEncoding(elements);
    }
  catch(std::exception &er)
    {
      std::cout << "EPUBParser::epubParseRootFile: " << er.what() << std::endl;
    }

  if(code_page.empty())
    {
      code_page = af->detectEncoding(root_file_content);
    }
  if(code_page != "UTF-8")
    {
      std::string buf = af->toUTF8(root_file_content, code_page.c_str());
      elements = xml_parser->parseDocument(buf);
    }
  result.book_name = dc->dcTitle(elements);
  result.book_author = dc->dcAuthor(elements);
  result.book_genre = dc->dcGenre(elements);
  result.book_date = dc->dcDate(elements);

  return result;
}

std::shared_ptr<BookInfoEntry>
EPUBParser::epub_book_info(const std::filesystem::path &filepath)
{
  return epubBookInfo(filepath);
}

std::shared_ptr<BookInfoEntry>
EPUBParser::epubBookInfo(const std::filesystem::path &filepath)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();
  std::vector<ArchEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epubGetRootFileAddress(filepath, filenames);
  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(it->filename == rootfile)
            {
              root_file_content = unpackByPositionStr(filepath, *it);
            }
        }
    }

  if(!root_file_content.empty())
    {
      std::vector<XMLElement> elements;
      std::string code_page;
      try
        {
          elements = xml_parser->parseDocument(root_file_content);
          code_page = getEncoding(elements);
        }
      catch(std::exception &er)
        {
          std::cout << "EPUBParser::epubParseRootFile: " << er.what()
                    << std::endl;
        }

      if(code_page.empty())
        {
          code_page = af->detectEncoding(root_file_content);
        }
      if(code_page != "UTF-8")
        {
          std::string buf = af->toUTF8(root_file_content, code_page.c_str());
          elements = xml_parser->parseDocument(buf);
        }

      result->annotation = epubAnnotation(elements);
      epubLanguage(elements, *result);
      epubTranslator(elements, *result);
      epubPublisher(elements, *result);
      epubIdentifier(elements, *result);
      epubSource(elements, *result);

      std::string cover_fnm = epubCoverAddress(elements, filepath, filenames);
      std::filesystem::path root = std::filesystem::u8path(rootfile);
      std::string root_path = root.parent_path().u8string();
      if(!root_path.empty())
        {
          cover_fnm = root_path + "/" + cover_fnm;
        }
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(it->filename == cover_fnm)
            {
              result->cover = unpackByPositionStr(filepath, *it);
              result->cover_type = BookInfoEntry::cover_types::file;
              break;
            }
        }
    }

  return result;
}

std::string
EPUBParser::epubAnnotation(const std::vector<XMLElement> &elements)
{
  std::string result;

  result = dc->dcDescription(elements);

  return result;
}

std::string
EPUBParser::epubCoverAddress(const std::vector<XMLElement> &elements,
                             const std::filesystem::path &filepath,
                             const std::vector<ArchEntry> &filenames)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "meta", "name", "cover", res);
  if(res.size() > 0)
    {
      auto it_attr = std::find_if(res[0]->element_attributes.begin(),
                                  res[0]->element_attributes.end(),
                                  [](const XMLElementAttribute &el)
                                    {
                                      return el.attribute_id == "content";
                                    });
      if(it_attr != res[0]->element_attributes.end())
        {
          std::string attr_val = it_attr->attribute_value;
          res.clear();
          XMLAlgorithms::searchElement(elements, "item", "id", attr_val, res);

          if(res.size() > 0)
            {
              it_attr = std::find_if(res[0]->element_attributes.begin(),
                                     res[0]->element_attributes.end(),
                                     [](const XMLElementAttribute &el)
                                       {
                                         return el.attribute_id == "href";
                                       });
              if(it_attr != res[0]->element_attributes.end())
                {
                  result = it_attr->attribute_value;
                }
            }
        }
    }

  if(result.empty())
    {
      res.clear();
      XMLAlgorithms::searchElement(elements, "item", "id", "cover", res);
      if(res.size() > 0)
        {
          auto it_attr = std::find_if(res[0]->element_attributes.begin(),
                                      res[0]->element_attributes.end(),
                                      [](const XMLElementAttribute &el)
                                        {
                                          return el.attribute_id == "href";
                                        });
          if(it_attr != res[0]->element_attributes.end())
            {
              std::string href = it_attr->attribute_value;
              auto it_fnm = std::find_if(filenames.begin(), filenames.end(),
                                         [href](const ArchEntry &el)
                                           {
                                             return href == el.filename;
                                           });
              if(it_fnm != filenames.end())
                {
                  std::string buf = unpackByPositionStr(filepath, *it_fnm);

                  std::vector<XMLElement> el_v;
                  std::string code_page;
                  try
                    {
                      el_v = xml_parser->parseDocument(buf);
                      code_page = getEncoding(el_v);
                    }
                  catch(std::exception &er)
                    {
                      std::cout << "EPUBParser::epubCoverAddress(1): \""
                                << er.what() << "\"" << std::endl;
                      code_page = af->detectEncoding(buf);
                    }
                  if(code_page != "UTF-8")
                    {
                      buf = af->toUTF8(buf, code_page.c_str());
                      el_v.clear();
                      try
                        {
                          el_v = xml_parser->parseDocument(buf);
                        }
                      catch(std::exception &er)
                        {
                          std::cout << "EPUBParser::epubCoverAddress(2): \""
                                    << er.what() << "\"" << std::endl;
                        }
                    }
                  res.clear();
                  XMLAlgorithms::searchElement(el_v, "img", "src", res);
                  if(res.size() > 0)
                    {
                      it_attr
                          = std::find_if(res[0]->element_attributes.begin(),
                                         res[0]->element_attributes.end(),
                                         [](const XMLElementAttribute &el)
                                           {
                                             return el.attribute_id == "src";
                                           });
                      if(it_attr != res[0]->element_attributes.end())
                        {
                          result = it_attr->attribute_value;
                        }
                    }
                }
            }
        }
    }
  if(result.empty())
    {
      res.clear();
      XMLAlgorithms::searchElement(elements, "item", "properties",
                                   "cover-image", res);
      if(res.size() > 0)
        {
          auto it_attr = std::find_if(res[0]->element_attributes.begin(),
                                      res[0]->element_attributes.end(),
                                      [](const XMLElementAttribute &el)
                                        {
                                          return el.attribute_id == "href";
                                        });
          if(it_attr != res[0]->element_attributes.end())
            {
              result = it_attr->attribute_value;
            }
        }
    }

  return result;
}

void
EPUBParser::epubLanguage(const std::vector<XMLElement> &elements,
                         BookInfoEntry &result)
{
  result.language = dc->dcLanguage(elements);
}

void
EPUBParser::epubTranslator(const std::vector<XMLElement> &elements,
                           BookInfoEntry &result)
{
  result.translator = dc->dcTranslator(elements);
}

void
EPUBParser::epubPublisher(const std::vector<XMLElement> &elements,
                          BookInfoEntry &result)
{
  result.electro->publisher = dc->dcPublisher(elements);
  if(!result.electro->publisher.empty())
    {
      result.electro->available = true;
    }
}

void
EPUBParser::epubIdentifier(const std::vector<XMLElement> &elements,
                           BookInfoEntry &result)
{
  result.electro->id = dc->dcIdentifier(elements);
  if(!result.electro->id.empty())
    {
      result.electro->available = true;
    }
}

void
EPUBParser::epubSource(const std::vector<XMLElement> &elements,
                       BookInfoEntry &result)
{
  result.electro->src_url = dc->dcSource(elements);
  if(!result.electro->src_url.empty())
    {
      result.electro->available = true;
    }
}

std::string
EPUBParser::getEncoding(const std::vector<XMLElement> &elements)
{
  std::string result;
  std::vector<XMLElement *> search_res;
  XMLAlgorithms::searchElement(elements, "xml", "encoding", search_res);
  if(search_res.size() > 0)
    {
      auto it = std::find_if(search_res[0]->element_attributes.begin(),
                             search_res[0]->element_attributes.end(),
                             [](const XMLElementAttribute &el)
                               {
                                 return el.attribute_id == "encoding";
                               });
      if(it != search_res[0]->element_attributes.end())
        {
          result = it->attribute_value;
        }
    }
  return result;
}
