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

#include <FB2Parser.h>
#include <XMLAlgorithms.h>
#include <XMLTextEncoding.h>
#include <algorithm>
#include <iostream>
#include <syncstream>

FB2Parser::FB2Parser(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
  xml_parser = new XMLParserCPP;
}

FB2Parser::~FB2Parser()
{
  delete xml_parser;
}

UDBElement
FB2Parser::parseBook(const std::string &file_content)
{
  UDBElement result;

  if(file_content.empty())
    {
      return result;
    }

  std::vector<XMLElement> book_xml;
  try
    {
      book_xml = xml_parser->parseDocument(file_content);
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "FB2Parser::parseBook: \"" << er.what() << "\"" << std::endl;
      std::string code_page
          = XMLTextEncoding::detectDocumentEncoding(file_content);
      std::string l_book;
      XMLTextEncoding::convertToEncoding(file_content, l_book, code_page,
                                         "UTF-8");
      std::string find_str1 = "<description>";
      std::string::size_type n1 = l_book.find(find_str1);
      if(n1 == std::string::npos)
        {
          throw std::runtime_error("FB2Parser::parseBook: critical error(1)");
        }
      std::string find_str2 = "</description>";
      std::string::size_type n2
          = l_book.find(find_str2, n1 + find_str1.size());
      if(n2 == std::string::npos)
        {
          throw std::runtime_error("FB2Parser::parseBook: critical error(2)");
        }
      std::string description(l_book.begin() + n1,
                              l_book.begin() + n2 + find_str2.size());
      book_xml = xml_parser->parseDocument(description);
    }

  result = fb2GetInfoForBase(book_xml);

  return result;
}

UDBase
FB2Parser::getBookInfo(const std::string &book_content)
{
  UDBase result;

  if(book_content.empty())
    {
      throw std::runtime_error(
          "FB2Parser::getBookInfo: book_content is empty");
    }

  std::vector<XMLElement> book_xml;
  try
    {
      book_xml = xml_parser->parseDocument(book_content);
    }
  catch(std::exception &er)
    {
      std::cout << "FB2Parser::getBookInfo: \"" << er.what() << "\""
                << std::endl;

      std::string code_page
          = XMLTextEncoding::detectDocumentEncoding(book_content);
      std::string l_book;
      XMLTextEncoding::convertToEncoding(book_content, l_book, code_page,
                                         "UTF-8");

      std::string find_str1 = "<description>";
      std::string::size_type n1 = l_book.find(find_str1);
      if(n1 == std::string::npos)
        {
          throw std::runtime_error(
              "FB2Parser::getBookInfo: critical error(1)");
        }
      std::string find_str2 = "</description>";
      std::string::size_type n2
          = l_book.find(find_str2, n1 + find_str1.size());
      if(n2 == std::string::npos)
        {
          throw std::runtime_error(
              "FB2Parser::getBookInfo: critical error(2)");
        }
      std::string description(l_book.begin() + n1,
                              l_book.begin() + n2 + find_str2.size());
      book_xml = xml_parser->parseDocument(description);
    }

  std::vector<XMLElement *> info;
  XMLAlgorithms::searchElement(book_xml, "title-info", info);

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(info, "annotation", res);
  if(res.size() == 0)
    {
      XMLAlgorithms::searchElement(book_xml, "annotation", res);
    }

  UDBElement el;
  bid.setId(el, BaseID::Annotation);
  fb2Annotation(res, el.content);
  if(!el.content.empty())
    {
      result.addElement(el);
    }

  el = UDBElement();
  fb2Cover(book_xml, el);
  if(!el.content.empty())
    {
      result.addElement(el);
    }

  res.clear();
  XMLAlgorithms::searchElement(info, "author", res);
  std::vector<UDBElement> elements;
  fb2AuthorBookInfo(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::Author);
    }
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "keywords", res);
  elements.clear();
  getResult(res, elements, BaseID::Keywords);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "lang", res);
  elements.clear();
  getResult(res, elements, BaseID::Language);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "src-lang", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceLanguage);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "translator", res);
  elements.clear();
  fb2AuthorBookInfo(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::Translator);
    }
  result.addElements(elements);

  info.clear();
  XMLAlgorithms::searchElement(book_xml, "src-title-info", info);

  res.clear();
  XMLAlgorithms::searchElement(info, "book-title", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceBookTitle);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "author", res);
  elements.clear();
  fb2AuthorBookInfo(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::SourceBookAuthor);
    }
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "sequence", res);
  elements.clear();
  fb2Series(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::SourceBookSequence);
    }
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "genre", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceBookGenre);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "date", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceBookDate);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "keywords", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceBookKeywords);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "lang", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceBookLanguage);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "src-lang", res);
  elements.clear();
  getResult(res, elements, BaseID::SourceBookSourceLanguage);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "translator", res);
  elements.clear();
  fb2AuthorBookInfo(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::SourceBookTranslator);
    }
  result.addElements(elements);

  info.clear();
  XMLAlgorithms::searchElement(book_xml, "document-info", info);

  res.clear();
  XMLAlgorithms::searchElement(info, "author", res);
  elements.clear();
  fb2AuthorBookInfo(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::EbookAuthor);
    }
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "program-used", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookProgramUsed);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "date", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookDate);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "src-url", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookSourceUrl);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "src-ocr", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookSourceOCR);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "id", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookID);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "version", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookVersion);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "history", res);
  elements.clear();
  getResult(res, elements, BaseID::EbookHistory);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "publisher", res);
  elements.clear();
  fb2AuthorBookInfo(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::EbookPublisher);
    }
  result.addElements(elements);

  info.clear();
  XMLAlgorithms::searchElement(book_xml, "publish-info", info);

  res.clear();
  XMLAlgorithms::searchElement(info, "book-name", res);
  elements.clear();
  getResult(res, elements, BaseID::PaperBookName);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "publisher", res);
  elements.clear();
  getResult(res, elements, BaseID::PaperBookPublisher);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "city", res);
  elements.clear();
  getResult(res, elements, BaseID::PaperBookCity);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "year", res);
  elements.clear();
  getResult(res, elements, BaseID::PaperBookYear);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "isbn", res);
  elements.clear();
  getResult(res, elements, BaseID::PaperBookISBN);
  result.addElements(elements);

  res.clear();
  XMLAlgorithms::searchElement(info, "sequence", res);
  elements.clear();
  fb2Series(res, elements);
  for(auto it = elements.begin(); it != elements.end(); it++)
    {
      bid.setId(*it, BaseID::PaperBookSequence);
    }
  result.addElements(elements);

  info.clear();
  XMLAlgorithms::searchElement(book_xml, "custom-info", info);
  elements.clear();
  getResult(info, elements, BaseID::CustomInfo);
  result.addElements(elements);

  result.shrinkToFit();

  return result;
}

UDBElement
FB2Parser::fb2GetInfoForBase(const std::vector<XMLElement> &book_xml)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);

  std::vector<XMLElement *> title_info;
  XMLAlgorithms::searchElement(book_xml, "title-info", title_info);

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(title_info, "author", res);
  result.subelements = fb2Author(res);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "book-title", res);
  getResult(res, result.subelements, BaseID::BookTitle);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "sequence", res);
  fb2Series(res, result.subelements);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "genre", res);
  getResult(res, result.subelements, BaseID::Genre);

  res.clear();
  XMLAlgorithms::searchElement(title_info, "date", res);
  getResult(res, result.subelements, BaseID::Date);

  result.subelements.shrink_to_fit();

  return result;
}

std::vector<UDBElement>
FB2Parser::fb2Author(const std::vector<XMLElement *> &author)
{
  std::vector<UDBElement> result;
  std::vector<XMLElement *> res;
  std::vector<XMLElement *> res2;
  for(size_t i = 0; i < author.size(); i++)
    {
      UDBElement l_result;
      bid.setId(l_result, BaseID::Author);

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "last-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::LastName);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          l_result.subelements.emplace_back(sub_el);
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "first-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::FirstName);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          l_result.subelements.emplace_back(sub_el);
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "middle-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::MiddleName);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          l_result.subelements.emplace_back(sub_el);
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "nickname", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::Nickname);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          l_result.subelements.emplace_back(sub_el);
        }

      if(l_result.subelements.size() > 0)
        {
          l_result.subelements.shrink_to_fit();
          result.emplace_back(l_result);
        }
    }

  return result;
}

void
FB2Parser::fb2AuthorBookInfo(const std::vector<XMLElement *> &author,
                             std::vector<UDBElement> &result)
{
  std::vector<XMLElement *> res;
  std::vector<XMLElement *> res2;
  for(size_t i = 0; i < author.size(); i++)
    {
      UDBElement l_result;
      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "last-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::LastName);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "first-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::FirstName);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "middle-name", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::MiddleName);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "nickname", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::Nickname);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "home-page", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::HomePage);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "email", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::EMail);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      res.clear();
      res2.clear();
      XMLAlgorithms::searchElement(author[i]->elements, "id", res);
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      if(res2.size() > 0)
        {
          UDBElement sub_el;
          bid.setId(sub_el, BaseID::AuthorID);
          sub_el.content = res2[0]->content;
          normalizeString(sub_el.content);
          if(!sub_el.content.empty())
            {
              l_result.subelements.emplace_back(sub_el);
            }
        }

      if(l_result.subelements.size() > 0)
        {
          l_result.subelements.shrink_to_fit();
          result.emplace_back(l_result);
        }
    }
}

void
FB2Parser::fb2Series(const std::vector<XMLElement *> &sequence,
                     std::vector<UDBElement> &book_subelements)
{
  for(size_t i = 0; i < sequence.size(); i++)
    {
      UDBElement l_result;
      bid.setId(l_result, BaseID::Sequence);
      l_result.subelements.reserve(2);
      auto it = std::find_if(sequence[i]->element_attributes.begin(),
                             sequence[i]->element_attributes.end(),
                             [](const XMLElementAttribute &el)
                               {
                                 return el.attribute_id == "name";
                               });
      if(it != sequence[i]->element_attributes.end())
        {
          UDBElement sub;
          bid.setId(sub, BaseID::SequenceName);
          sub.content = it->attribute_value;
          normalizeString(sub.content);
          if(!sub.content.empty())
            {
              l_result.subelements.emplace_back(sub);
            }
        }

      if(l_result.subelements.size() > 0)
        {
          it = std::find_if(sequence[i]->element_attributes.begin(),
                            sequence[i]->element_attributes.end(),
                            [](const XMLElementAttribute &el)
                              {
                                return el.attribute_id == "number";
                              });
          if(it != sequence[i]->element_attributes.end())
            {
              UDBElement sub;
              bid.setId(sub, BaseID::SequenceNumber);
              sub.content = it->attribute_value;
              normalizeString(sub.content);
              if(!sub.content.empty())
                {
                  l_result.subelements.emplace_back(sub);
                }
            }
        }

      if(l_result.subelements.size() > 0)
        {
          book_subelements.emplace_back(l_result);
        }
    }
}

void
FB2Parser::fb2Annotation(const std::vector<XMLElement *> &annotation,
                         std::string &result)
{
  if(annotation.size() > 0)
    {
      std::vector<XMLElement *> v;
      v.push_back(annotation[0]);
      XMLAlgorithms::writeXML(v, result);
    }
}

void
FB2Parser::fb2Cover(const std::vector<XMLElement> &book_xml,
                    UDBElement &result)
{
  bid.setId(result, BaseID::CoverPage);

  std::vector<XMLElement *> coverpage;
  XMLAlgorithms::searchElement(book_xml, "coverpage", coverpage);
  std::vector<XMLElement *> image;
  XMLAlgorithms::searchElement(coverpage, "image", "l:href", image);
  fb2CoverGetImage(book_xml, image, result);
  if(result.content.empty())
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
          XMLAlgorithms::writeXML(image, result.content);
        }
      else if(coverpage.size() > 0)
        {
          XMLAlgorithms::writeXML(coverpage, result.content);
        }
      if(!result.content.empty())
        {
          UDBElement el;
          bid.setId(el, BaseID::CoverType);
          el.content = "text";
          result.subelements.emplace_back(el);
        }
    }
}

void
FB2Parser::fb2CoverGetImage(const std::vector<XMLElement> &book_xml,
                            const std::vector<XMLElement *> &image,
                            UDBElement &result)
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
                      result.content += it->content;
                    }
                }
              if(!result.content.empty())
                {
                  UDBElement el;
                  bid.setId(el, BaseID::CoverType);
                  el.content = "base64";
                  result.subelements.emplace_back(el);
                }
            }
        }
    }
}

void
FB2Parser::getResult(const std::vector<XMLElement *> &elements,
                     std::vector<UDBElement> &result,
                     const BaseID::ID &element_id)
{
  std::vector<XMLElement *> res;
  for(size_t i = 0; i < elements.size(); i++)
    {
      UDBElement el;
      bid.setId(el, element_id);
      res.clear();
      XMLAlgorithms::searchElement(elements[i]->elements,
                                   XMLElement::ElementContent, res);
      if(res.size() == 0)
        {
          continue;
        }
      el.content = res[0]->content;
      normalizeString(el.content);
      if(!el.content.empty())
        {
          result.emplace_back(el);
        }
    }
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
  str.shrink_to_fit();
}
