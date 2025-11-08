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

#include <FB2Parser.h>
#include <XMLAlgorithms.h>
#include <algorithm>
#include <iostream>

FB2Parser::FB2Parser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  xml_parser = new XMLParserCPP;
}

FB2Parser::~FB2Parser()
{
  delete xml_parser;
}

BookParseEntry
FB2Parser::fb2_parser(const std::string &book)
{
  return fb2Parser(book);
}

BookParseEntry
FB2Parser::fb2Parser(const std::string &book)
{
  BookParseEntry result;
  if(book.empty())
    {
      return result;
    }
  std::string book_code_page = getBookEncoding(book);
  if(book_code_page.empty())
    {
      book_code_page = af->detectEncoding(book);
    }
  std::string l_book = af->toUTF8(book, book_code_page.c_str());
  try
    {
      book_xml = xml_parser->parseDocument(l_book);
    }
  catch(std::exception &er)
    {
      std::cout << "FB2Parser::fb2Parser: \"" << er.what() << "\""
                << std::endl;
      std::string find_str1 = "<description>";
      std::string::size_type n1 = l_book.find(find_str1);
      if(n1 == std::string::npos)
        {
          throw std::runtime_error("FB2Parser::fb2Parser: critical error(1)");
        }
      std::string find_str2 = "</description>";
      std::string::size_type n2
          = l_book.find(find_str2, n1 + find_str1.size());
      if(n2 == std::string::npos)
        {
          throw std::runtime_error("FB2Parser::fb2Parser: critical error(2)");
        }
      std::string description(l_book.begin() + n1,
                              l_book.begin() + n2 + find_str2.size());
      book_xml = xml_parser->parseDocument(description);
    }

  result = fb2Description();

  return result;
}

std::string
FB2Parser::fb2Author(const std::vector<XMLElement *> &author)
{
  std::string result;
  std::vector<XMLElement *> res;
  std::vector<XMLElement *> res2;
  std::string l_result;
  for(size_t i = 0; i < author.size(); i++)
    {
      l_result.clear();

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "last-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          l_result = res2[0]->content;
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "first-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          if(!l_result.empty())
            {
              l_result += " ";
            }
          l_result += res2[0]->content;
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "middle-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          if(!l_result.empty())
            {
              l_result += " ";
            }
          l_result += res2[0]->content;
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "nickname", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          if(!l_result.empty())
            {
              l_result += " aka ";
            }
          l_result += res2[0]->content;
        }
      normalizeString(l_result);
      if(!result.empty() && !l_result.empty())
        {
          result += ", ";
        }
      result += l_result;
    }

  return result;
}

std::string
FB2Parser::fb2Series(const std::vector<XMLElement *> &sequence)
{
  std::string result;
  std::string l_result;

  for(size_t i = 0; i < sequence.size(); i++)
    {
      l_result.clear();
      auto it = std::find_if(sequence[i]->element_attributes.begin(),
                             sequence[i]->element_attributes.end(),
                             [](const XMLElementAttribute &el)
                               {
                                 return el.attribute_id == "name";
                               });
      if(it != sequence[i]->element_attributes.end())
        {
          l_result += it->attribute_value;
        }

      if(!l_result.empty())
        {
          it = std::find_if(sequence[i]->element_attributes.begin(),
                            sequence[i]->element_attributes.end(),
                            [](const XMLElementAttribute &el)
                              {
                                return el.attribute_id == "number";
                              });
          if(it != sequence[i]->element_attributes.end())
            {
              l_result += " ";
              l_result += it->attribute_value;
            }
        }

      normalizeString(l_result);

      if(!result.empty() && !l_result.empty())
        {
          result += ", ";
        }
      result += l_result;
    }

  return result;
}

std::string
FB2Parser::fb2Genres(const std::vector<XMLElement *> &genres)
{
  std::string result;
  std::string l_result;
  std::vector<XMLElement *> res;
  for(size_t i = 0; i < genres.size(); i++)
    {
      l_result.clear();
      res.clear();
      XMLAlgorithms::searchElement(genres[i]->elements,
                                   XMLElement::ElementContent, res);
      if(res.size() > 0)
        {
          l_result += res[0]->content;
        }

      normalizeString(l_result);

      if(!result.empty() && !l_result.empty())
        {
          result += ", ";
        }
      result += l_result;
    }

  return result;
}

std::string
FB2Parser::fb2Date(const std::vector<XMLElement *> &date)
{
  std::string result;
  std::string l_result;
  std::vector<XMLElement *> res;
  for(size_t i = 0; i < date.size(); i++)
    {
      l_result.clear();
      res.clear();
      XMLAlgorithms::searchElement(date[i]->elements,
                                   XMLElement::ElementContent, res);
      if(res.size() > 0)
        {
          l_result += res[0]->content;
        }

      normalizeString(l_result);

      if(!result.empty() && !l_result.empty())
        {
          result += ", ";
        }
      result += l_result;
    }

  return result;
}

BookParseEntry
FB2Parser::fb2Description()
{
  BookParseEntry result;

  XMLAlgorithms::searchElement(book_xml, "title-info", title_info);

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(title_info, "author", res);
  result.book_author = fb2Author(res);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "book-title", res);
  std::string l_result;
  std::vector<XMLElement *> res2;
  for(size_t i = 0; i < res.size(); i++)
    {
      l_result.clear();
      res2.clear();
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          l_result = res2[0]->content;
        }

      normalizeString(l_result);

      if(!result.book_name.empty() && !l_result.empty())
        {
          result.book_name += ". ";
        }
      result.book_name += l_result;
    }

  res.clear();
  XMLAlgorithms::searchElement(title_info, "sequence", res);
  result.book_series = fb2Series(res);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "genre", res);
  result.book_genre = fb2Genres(res);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "date", res);
  result.book_date = fb2Date(res);

  return result;
}

std::shared_ptr<BookInfoEntry>
FB2Parser::fb2_book_info(const std::string &book)
{
  return fb2BookInfo(book);
}

std::shared_ptr<BookInfoEntry>
FB2Parser::fb2BookInfo(const std::string &book)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();

  if(book.empty())
    {
      return result;
    }
  std::string book_code_page = getBookEncoding(book);

  if(book_code_page.empty())
    {
      book_code_page = af->detectEncoding(book);
    }
  std::string l_book = af->toUTF8(book, book_code_page.c_str());
  try
    {
      book_xml = xml_parser->parseDocument(l_book);
    }
  catch(std::exception &er)
    {
      std::cout << "FB2Parser::fb2BookInfo: \"" << er.what() << "\""
                << std::endl;
      std::string find_str1 = "<description>";
      std::string::size_type n1 = l_book.find(find_str1);
      if(n1 == std::string::npos)
        {
          throw std::runtime_error(
              "FB2Parser::fb2BookInfo: critical error(1)");
        }
      std::string find_str2 = "</description>";
      std::string::size_type n2
          = l_book.find(find_str2, n1 + find_str1.size());
      if(n2 == std::string::npos)
        {
          throw std::runtime_error(
              "FB2Parser::fb2BookInfo: critical error(2)");
        }
      std::string description(l_book.begin() + n1,
                              l_book.begin() + n2 + find_str2.size());
      book_xml = xml_parser->parseDocument(description);
    }

  fb2AnnotationDecode(result->annotation);
  result->cover_type = fb2Cover(result->cover);

  fb2ExtraInfo(*result);

  return result;
}

void
FB2Parser::fb2AnnotationDecode(std::string &result)
{
  std::vector<XMLElement *> annotation;
  XMLAlgorithms::searchElement(book_xml, "annotation", annotation);
  XMLAlgorithms::writeXML(annotation, result);
}

BookInfoEntry::cover_types
FB2Parser::fb2Cover(std::string &cover)
{
  BookInfoEntry::cover_types result = BookInfoEntry::cover_types::error;
  std::vector<XMLElement *> coverpage;
  XMLAlgorithms::searchElement(book_xml, "coverpage", coverpage);
  std::vector<XMLElement *> image;
  XMLAlgorithms::searchElement(coverpage, "image", "l:href", image);
  fb2CoverGetImage(image, cover, result);
  if(cover.empty())
    {
      image.clear();
      coverpage.clear();
      XMLAlgorithms::searchElement(book_xml, "body", coverpage);
      XMLAlgorithms::searchElement(coverpage, "p", image);
      if(image.size() > 0)
        {
          if(image.size() > 50)
            {
              image.resize(50);
            }
          XMLAlgorithms::writeXML(image, cover);
        }
      else if(coverpage.size() > 0)
        {
          XMLAlgorithms::writeXML(coverpage, cover);
        }
      if(!cover.empty())
        {
          result = BookInfoEntry::cover_types::text;
        }
    }
  return result;
}

void
FB2Parser::fb2CoverGetImage(const std::vector<XMLElement *> &image,
                            std::string &cover,
                            BookInfoEntry::cover_types &result)
{
  if(image.size() > 0)
    {
      auto it_attr = std::find_if(image[0]->element_attributes.begin(),
                                  image[0]->element_attributes.end(),
                                  [](const XMLElementAttribute &el)
                                    {
                                      return el.attribute_id == "l:href";
                                    });
      if(it_attr != image[0]->element_attributes.end())
        {
          std::string cover_name = it_attr->attribute_value;
          for(auto it = cover_name.begin(); it != cover_name.end();)
            {
              char ch = *it;
              if(ch >= 0 && ch <= 32)
                {
                  cover_name.erase(it);
                }
              else if(ch == '#')
                {
                  cover_name.erase(it);
                }
              else
                {
                  break;
                }
            }
          std::vector<XMLElement *> binary;
          XMLAlgorithms::searchElement(book_xml, "binary", "id", cover_name,
                                       binary);
          if(binary.size() > 0)
            {
              for(auto it = binary[0]->elements.begin();
                  it != binary[0]->elements.end(); it++)
                {
                  if(it->element_type == XMLElement::ElementContent)
                    {
                      cover += it->content;
                    }
                }
              if(!cover.empty())
                {
                  result = BookInfoEntry::cover_types::base64;
                }
            }
        }
    }
}

void
FB2Parser::fb2ExtraInfo(BookInfoEntry &result)
{
  std::vector<XMLElement *> res;
  std::vector<XMLElement *> res2;

  XMLAlgorithms::searchElement(book_xml, "title-info", title_info);
  XMLAlgorithms::searchElement(title_info, "lang", res);
  XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
  fb2Language(res2, result.language);

  res.clear();
  res2.clear();
  XMLAlgorithms::searchElement(title_info, "src-lang", res);
  XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
  fb2Language(res2, result.src_language);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "translator", res);
  result.translator = fb2Author(res);

  fb2PublisherInfo(result);

  fb2ElectroDocInfo(result);
}

void
FB2Parser::fb2Language(const std::vector<XMLElement *> &lang,
                       std::string &result)
{
  for(size_t i = 0; i < lang.size(); i++)
    {
      if(!result.empty())
        {
          result += ", ";
        }
      result += lang[i]->content;
    }
}

void
FB2Parser::fb2PublisherInfo(BookInfoEntry &result)
{
  std::vector<XMLElement *> publish_info;
  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(book_xml, "publish-info", publish_info);

  XMLAlgorithms::searchElement(publish_info, "book-name", res);
  fb2PublisherInfoString(res, result.paper->book_name);
  if(!result.paper->book_name.empty())
    {
      result.paper->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(publish_info, "publisher", res);
  fb2PublisherInfoString(res, result.paper->publisher);
  if(!result.paper->publisher.empty())
    {
      result.paper->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(publish_info, "city", res);
  fb2PublisherInfoString(res, result.paper->city);
  if(!result.paper->city.empty())
    {
      result.paper->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(publish_info, "year", res);
  fb2PublisherInfoString(res, result.paper->year);
  if(!result.paper->year.empty())
    {
      result.paper->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(publish_info, "isbn", res);
  fb2PublisherInfoString(res, result.paper->isbn);
  if(!result.paper->isbn.empty())
    {
      result.paper->available = true;
    }
}

void
FB2Parser::fb2PublisherInfoString(const std::vector<XMLElement *> &source,
                                  std::string &result)
{
  for(size_t i = 0; i < source.size(); i++)
    {
      if(source[i]->element_type == XMLElement::ElementContent)
        {
          if(!result.empty())
            {
              result += ", ";
            }
          result += source[i]->content;
        }
      fb2PublisherInfoString(source[i]->elements, result);
    }
}

void
FB2Parser::fb2PublisherInfoString(const std::vector<XMLElement> &source,
                                  std::string &result)
{
  for(auto it = source.begin(); it != source.end(); it++)
    {
      if(it->element_type == XMLElement::ElementContent)
        {
          if(!result.empty())
            {
              result += ", ";
            }
          result += it->content;
        }
      fb2PublisherInfoString(it->elements, result);
    }
}

void
FB2Parser::fb2ElectroDocInfo(BookInfoEntry &result)
{
  std::vector<XMLElement *> document_info;
  XMLAlgorithms::searchElement(book_xml, "document-info", document_info);

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(document_info, "author", res);
  result.electro->author = fb2Author(res);
  if(!result.electro->author.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "program-used", res);
  fb2PublisherInfoString(res, result.electro->program_used);
  if(!result.electro->program_used.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "date", res);
  fb2PublisherInfoString(res, result.electro->date);
  if(!result.electro->date.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "src-url", res);
  fb2PublisherInfoString(res, result.electro->src_url);
  if(!result.electro->src_url.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "src-ocr", res);
  fb2PublisherInfoString(res, result.electro->src_ocr);
  if(!result.electro->src_ocr.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "id", res);
  fb2PublisherInfoString(res, result.electro->id);
  if(!result.electro->id.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "version", res);
  fb2PublisherInfoString(res, result.electro->version);
  if(!result.electro->version.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "history", res);
  XMLAlgorithms::writeXML(res, result.electro->history);
  if(!result.electro->history.empty())
    {
      result.electro->available = true;
    }

  res.clear();
  XMLAlgorithms::searchElement(document_info, "publisher", res);
  std::vector<XMLElement *> res2;
  XMLAlgorithms::searchElement(res, "author", res2);
  result.electro->publisher = fb2Author(res2);
  if(!result.electro->publisher.empty())
    {
      result.electro->available = true;
    }
}

std::string
FB2Parser::getBookEncoding()
{
  std::string result;
  std::vector<XMLElement *> search_res;
  XMLAlgorithms::searchElement(book_xml, "xml", "encoding", search_res);
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

std::string
FB2Parser::getBookEncoding(const std::string &book)
{
  std::string result;

  std::string find_str1("<?xml");
  std::string::size_type n1 = book.find(find_str1);
  if(n1 == std::string::npos)
    {
      return result;
    }
  std::string find_str2("?>");
  std::string::size_type n2 = book.find(find_str2, n1 + find_str1.size());
  if(n2 == std::string::npos)
    {
      return result;
    }

  bool s_quot = false;
  bool d_quot = false;
  bool stop = false;
  bool attr_name_finished = false;
  std::string attribute_name;
  for(size_t i = n1 + find_str1.size(); i < n2; i++)
    {
      switch(book[i])
        {
        case '\'':
          {
            if(s_quot)
              {
                if(attribute_name == "encoding")
                  {
                    stop = true;
                  }
                else
                  {
                    s_quot = false;
                    attribute_name.clear();
                    result.clear();
                  }
              }
            else if(d_quot)
              {
                result.push_back(book[i]);
              }
            else
              {
                s_quot = true;
              }
            break;
          }
        case '\"':
          {
            if(d_quot)
              {
                if(attribute_name == "encoding")
                  {
                    stop = true;
                  }
                else
                  {
                    d_quot = false;
                    attribute_name.clear();
                    result.clear();
                  }
              }
            else if(s_quot)
              {
                result.push_back(book[i]);
              }
            else
              {
                d_quot = true;
              }
            break;
          }
        case ' ':
          {
            if(s_quot || d_quot)
              {
                result.push_back(book[i]);
              }
            else
              {
                attr_name_finished = false;
              }
            break;
          }
        case '=':
          {
            if(!s_quot && !d_quot)
              {
                attr_name_finished = true;
              }
            else
              {
                result.push_back(book[i]);
              }
            break;
          }
        default:
          {
            if(attr_name_finished)
              {
                result.push_back(book[i]);
              }
            else
              {
                attribute_name.push_back(book[i]);
              }
            break;
          }
        }

      if(stop)
        {
          break;
        }
    }
  if(!stop)
    {
      result.clear();
    }
  return result;
}

void
FB2Parser::findAnnotationFallback(const std::string &book,
                                  BookInfoEntry &result)
{
  std::string find_str1("<annotation>");
  std::string::size_type n1 = book.find(find_str1);
  if(n1 == std::string::npos)
    {
      return void();
    }
  std::string find_str2("</annotation>");
  std::string::size_type n2 = book.find(find_str2, n1 + find_str1.size());
  if(n2 == std::string::npos)
    {
      return void();
    }
  std::copy(book.begin() + n1, book.begin() + n2 + find_str2.size(),
            std::back_inserter(result.annotation));
}

void
FB2Parser::normalizeString(std::string &str)
{
  for(auto it = str.begin(); it != str.end();)
    {
      char el = *it;
      if(el >= 0 && el <= 32)
        {
          str.erase(it);
        }
      else
        {
          break;
        }
    }

  while(str.size() > 0)
    {
      char el = *str.rbegin();
      if(el >= 0 && el <= 32)
        {
          str.pop_back();
        }
      else
        {
          break;
        }
    }

  std::string find_str = "  ";
  std::string::size_type n = 0;
  for(;;)
    {
      n = str.find(find_str, n);
      if(n == std::string::npos)
        {
          break;
        }
      str.erase(str.begin() + n);
    }
}
