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
#include <XMLParserCPP.h>
#include <XMLTextEncoding.h>
#include <algorithm>
#include <sstream>
#include <unicode/unistr.h>

XMLParserCPP::XMLParserCPP()
{
  replacement.reserve(5);
  replacement.push_back(std::make_tuple("amp", "&"));
  replacement.push_back(std::make_tuple("apos", "'"));
  replacement.push_back(std::make_tuple("gt", ">"));
  replacement.push_back(std::make_tuple("lt", "<"));
  replacement.push_back(std::make_tuple("quot", "\""));
}

std::vector<XMLElement>
XMLParserCPP::parseDocument(const std::string &xml_document)
{
  std::vector<XMLElement> result;
  if(xml_document.size() == 0)
    {
      return result;
    }

  std::string code_page;
  {
    std::vector<std::string> cp
        = XMLTextEncoding::detectStringEncoding(xml_document, true);
    if(cp.size() > 0)
      {
        code_page = cp[0];
      }
    else
      {
        throw std::runtime_error(
            "XMLParserCPP::parseDocument: cannot determine document encoding");
      }
  }

  std::string document;
  XMLTextEncoding::convertToEncoding(xml_document, document, code_page,
                                     "UTF-8");

  if(document.empty())
    {
      throw std::runtime_error(
          "XMLParserCPP::parseDocument: error on conversion to UTF-8");
    }

  std::string find_str("<");
  std::string::size_type n = document.find(find_str);
  if(n == std::string::npos)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseDocument: incorrect document(1)");
    }

  std::vector<XMLElement> elements;

  size_t limit = document.size();
  XMLElement content;
  content.element_type = XMLElement::ElementContent;

  for(size_t i = n; i < limit; i++)
    {
      switch(document[i])
        {
        case '<':
          {
            if(!content.content.empty())
              {
                auto it_str = std::find_if(content.content.begin(),
                                           content.content.end(),
                                           [](const char &el)
                                             {
                                               return el < 0 || el > ' ';
                                             });
                if(it_str != content.content.end())
                  {
                    elements.emplace_back(content);
                  }
                content = XMLElement();
                content.element_type = XMLElement::ElementContent;
              }
            elements.emplace_back(parseTag(document, i));
            break;
          }
        default:
          {
            content.content.push_back(document[i]);
            break;
          }
        }
    }

  formResult(result, elements.begin(), elements.end());

  replaceXMLEntities(result);

  return result;
}

void
XMLParserCPP::replaceXMLEntities(std::vector<XMLElement> &elements)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      it_el->element_name.shrink_to_fit();
      it_el->element_attributes.shrink_to_fit();
      replacementFunc(it_el->content);
      for(auto it_attr = it_el->element_attributes.begin();
          it_attr != it_el->element_attributes.end(); it_attr++)
        {
          it_attr->attribute_id.shrink_to_fit();
          replacementFunc(it_attr->attribute_value);
        }
      replaceXMLEntities(it_el->elements);
    }
  elements.shrink_to_fit();
}

void
XMLParserCPP::replacementFunc(std::string &str)
{
  if(str.size() == 0)
    {
      return void();
    }
  std::string find_str("&");
  std::string find_str2(";");
  std::string::size_type n = 0;
  std::string::size_type n2;
  for(;;)
    {
      n = str.find(find_str, n);
      if(n == std::string::npos)
        {
          break;
        }
      n2 = str.find(find_str2, n);
      if(n2 == std::string::npos)
        {
          break;
        }
      std::string to_replace(str.begin() + n + find_str.size(),
                             str.begin() + n2);
      auto it = std::find_if(
          replacement.begin(), replacement.end(),
          [to_replace](const std::tuple<std::string, std::string> &el)
            {
              return to_replace == std::get<0>(el);
            });
      if(it != replacement.end())
        {
          str.erase(str.begin() + n, str.begin() + n2 + find_str2.size());
          str.insert(str.begin() + n, std::get<1>(*it).begin(),
                     std::get<1>(*it).end());
        }
      else
        {
          if(to_replace.size() >= 2)
            {
              if(to_replace[0] == '#')
                {
                  if(to_replace[1] == 'x')
                    {
                      to_replace.erase(to_replace.begin(),
                                       to_replace.begin() + 2);
                      if(to_replace.size() > 0)
                        {
                          std::stringstream strm;
                          strm.imbue(std::locale("C"));
                          strm << std::hex << str;
                          UChar32 ch;
                          strm >> ch;
                          icu::UnicodeString ustr(ch);
                          to_replace.clear();
                          ustr.toUTF8String(to_replace);
                          str.erase(str.begin() + n,
                                    str.begin() + n2 + find_str2.size());
                          str.insert(str.begin() + n, to_replace.begin(),
                                     to_replace.end());
                        }
                      else
                        {
                          n++;
                        }
                    }
                  else
                    {
                      to_replace.erase(to_replace.begin());
                      bool num = true;
                      for(auto it_tr = to_replace.begin();
                          it_tr != to_replace.end(); it_tr++)
                        {
                          if((*it_tr) < 48 || (*it_tr) > 57)
                            {
                              num = false;
                              break;
                            }
                        }
                      if(num)
                        {
                          std::stringstream strm;
                          strm.imbue(std::locale("C"));
                          strm.str(to_replace);
                          UChar32 ch;
                          strm >> ch;
                          icu::UnicodeString ustr(ch);
                          to_replace.clear();
                          ustr.toUTF8String(to_replace);
                          str.erase(str.begin() + n,
                                    str.begin() + n2 + find_str2.size());
                          str.insert(str.begin() + n, to_replace.begin(),
                                     to_replace.end());
                        }
                      else
                        {
                          n++;
                        }
                    }
                }
              else
                {
                  n++;
                }
            }
          else
            {
              n++;
            }
        }
    }
  str.shrink_to_fit();
}

XMLElement
XMLParserCPP::parseTag(const std::string &document, size_t &position)
{
  position++;
  size_t limit = document.size();
  if(position >= limit)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseTag: incorrect document(1)");
    }
  XMLElement result;
  if(document[position] == '!')
    {
      parseSpecialElement(document, position, result);
      return result;
    }

  bool stop = false;
  bool element_name = true;
  for(; position < limit; position++)
    {
      switch(document[position])
        {
        case '\n':
        case ' ':
          {
            element_name = false;
            break;
          }
        case '?':
          {
            result.empty = XMLElement::XML;
            result.element_type = XMLElement::ProgramControlElement;
            break;
          }
        case '/':
          {
            result.empty = XMLElement::XML;
            break;
          }
        case '>':
          {
            stop = true;
            break;
          }
        default:
          {
            if(element_name)
              {
                result.element_name.push_back(document[position]);
              }
            else
              {
                result.element_attributes.emplace_back(
                    parseElementAttribute(document, position));
              }
            break;
          }
        }
      if(stop)
        {
          break;
        }
    }

  if(position >= limit)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseTag: incorrect document(2)");
    }

  return result;
}

void
XMLParserCPP::parseSpecialElement(const std::string &document,
                                  size_t &position, XMLElement &element)
{
  position++;

  if(position >= document.size())
    {
      throw std::runtime_error(
          "XMLParserCPP::parseTag: incorrect document(2)");
    }

  switch(document[position])
    {
    case '[':
      {
        element.element_type = XMLElement::CharData;
        break;
      }
    case '-':
      {
        element.element_type = XMLElement::Comment;
        break;
      }
    default:
      {
        element.element_type = XMLElement::Type::SpecialElement;
        break;
      }
    }

  switch(element.element_type)
    {
    case XMLElement::CharData:
      {
        std::string find_str = "[CDATA[";
        std::string::size_type n = document.find(find_str, position);
        if(n != position)
          {
            throw std::runtime_error("XMLParserCPP::parseSpecialElement: "
                                     "incorrect CDATA element");
          }
        n += find_str.size();
        find_str = "]]>";
        std::string::size_type n2 = document.find(find_str, n);
        if(n2 == std::string::npos)
          {
            throw std::runtime_error("XMLParserCPP::parseSpecialElement: "
                                     "CDATA element has not been completed");
          }
        std::copy(document.begin() + n, document.begin() + n2,
                  std::back_inserter(element.content));
        position = n2 + find_str.size() - 1;
        break;
      }
    case XMLElement::Comment:
      {
        std::string find_str("--");
        std::string::size_type n = document.find(find_str, position);
        if(n != position)
          {
            throw std::runtime_error(
                "XMLParserCPP::parseSpecialElement: incorrect comment");
          }
        position = n + find_str.size();
        find_str = "-->";
        n = document.find(find_str, position);
        if(n == std::string::npos)
          {
            throw std::runtime_error("XMLParserCPP::parseSpecialElement: "
                                     "comment element has not been completed");
          }
        std::copy(document.begin() + position, document.begin() + n,
                  std::back_inserter(element.content));
        position = n + find_str.size() - 1;
        break;
      }
    default:
      {
        bool stop = false;

        for(; position < document.size(); position++)
          {
            switch(document[position])
              {
              case '>':
                {
                  stop = true;
                  break;
                }
              default:
                {
                  element.content.push_back(document[position]);
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
            throw std::runtime_error("XMLParserCPP::parseSpecialElement: "
                                     "special element has not bin completed");
          }
        break;
      }
    }

  while(element.content.size() > 0)
    {
      auto it = element.content.rbegin();
      if(*it >= 0 && *it <= 32)
        {
          element.content.pop_back();
        }
      else
        {
          break;
        }
    }

  for(auto it = element.content.begin(); it != element.content.end();)
    {
      if(*it >= 0 && *it <= 32)
        {
          element.content.erase(it);
        }
      else
        {
          break;
        }
    }
}

XMLElementAttribute
XMLParserCPP::parseElementAttribute(const std::string &document,
                                    size_t &position)
{
  XMLElementAttribute result;
  size_t limit = document.size();

  for(; position < limit; position++)
    {
      if(document[position] > ' ' || document[position] < 0)
        {
          break;
        }
    }
  if(position >= limit)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseElementAttribute: incorrect attribute(1)");
    }

  bool stop = false;
  for(; position < limit; position++)
    {
      switch(document[position])
        {
        case '=':
          {
            stop = true;
            break;
          }
        default:
          {
            result.attribute_id.push_back(document[position]);
            break;
          }
        }
      if(stop)
        {
          break;
        }
    }
  if(position >= limit)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseElementAttribute: incorrect attribute(2)");
    }

  while(result.attribute_id.size() > 0)
    {
      if(*result.attribute_id.rbegin() == ' ')
        {
          result.attribute_id.pop_back();
        }
      else
        {
          break;
        }
    }

  char attr_end;
  for(; position < limit; position++)
    {
      attr_end = document[position];
      if(attr_end == '\'' || attr_end == '\"')
        {
          break;
        }
    }
  position++;
  if(position >= limit)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseElementAttribute: incorrect attribute(3)");
    }

  for(; position < limit; position++)
    {
      char el = document[position];
      if(el == attr_end)
        {
          break;
        }
      else
        {
          result.attribute_value.push_back(el);
        }
    }

  if(position >= limit)
    {
      throw std::runtime_error(
          "XMLParserCPP::parseElementAttribute: incorrect attribute(4)");
    }

  return result;
}

void
XMLParserCPP::formResult(std::vector<XMLElement> &result,
                         std::vector<XMLElement>::iterator start,
                         std::vector<XMLElement>::iterator end)
{
  for(auto it = start; it != end; it++)
    {
      switch(it->element_type)
        {
        case XMLElement::OrdinaryElement:
          {
            switch(it->empty)
              {
              case XMLElement::NotEmpty:
                {
                  std::vector<XMLElement>::iterator it_sub = std::find_if(
                      it + 1, end,
                      [it](const XMLElement &el)
                        {
                          if(it->element_type == el.element_type
                             && el.empty == XMLElement::XML)
                            {
                              if(it->element_name == el.element_name)
                                {
                                  return true;
                                }
                            }
                          return false;
                        });
                  if(it_sub == end)
                    {
                      it->empty = XMLElement::HTML;
                      result.emplace_back(*it);
                    }
                  else
                    {
                      formResult(it->elements, it + 1, it_sub);
                      result.emplace_back(*it);
                      it = it_sub;
                    }
                  break;
                }
              default:
                {
                  result.emplace_back(*it);
                  break;
                }
              }
            break;
          }
        default:
          {
            result.emplace_back(*it);
            break;
          }
        }
    }
}
