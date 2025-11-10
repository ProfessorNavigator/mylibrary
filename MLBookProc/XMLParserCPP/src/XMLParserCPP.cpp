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

  std::vector<std::shared_ptr<XMLElement>> not_completed;

  size_t limit = document.size();
  bool element_name_finished = false;
  bool empty_element = false;
  std::shared_ptr<XMLElement> in_process;
  for(size_t i = n; i < limit; i++)
    {
      switch(document[i])
        {
        case '<':
          {
            element_name_finished = false;
            empty_element = false;
            in_process = std::make_shared<XMLElement>();
            not_completed.push_back(in_process);
            break;
          }
        case '\n':
          {
            if(!element_name_finished)
              {
                element_name_finished = true;
                parseElementAttribute(document, i, in_process);
              }
            break;
          }
        case 9:
        case ' ':
          {
            element_name_finished = true;
            parseElementAttribute(document, i, in_process);
            break;
          }
        case '!':
          {
            parseSpecialElement(document, i, in_process);
            not_completed.erase(std::remove(not_completed.begin(),
                                            not_completed.end(), in_process),
                                not_completed.end());
            if(not_completed.size() > 0)
              {
                std::shared_ptr<XMLElement> last = *not_completed.rbegin();
                last->elements.emplace_back(*in_process);
              }
            else
              {
                result.emplace_back(*in_process);
              }
            n = document.find(find_str, i);
            i = n - 1;
            break;
          }
        case '?':
          {
            in_process->element_type = XMLElement::Type::ProgramControlElement;
            parseProgramControlElement(document, i, in_process);
            not_completed.erase(std::remove(not_completed.begin(),
                                            not_completed.end(), in_process),
                                not_completed.end());
            if(not_completed.size() > 0)
              {
                std::shared_ptr<XMLElement> last = *not_completed.rbegin();
                last->elements.emplace_back(*in_process);
              }
            else
              {
                result.emplace_back(*in_process);
              }
            n = document.find(find_str, i);
            i = n - 1;
            break;
          }
        case '/':
          {
            empty_element = true;
            if(element_name_finished)
              {
                in_process->empty = true;
              }
            else
              {
                in_process->element_name.push_back(document[i]);
              }
            break;
          }
        case '>':
          {
            n = document.find(find_str, i);
            if(n == std::string::npos)
              {
                bool incorrect = false;
                for(size_t j = i + 1; j < limit; j++)
                  {
                    if(document[j] < 0 || document[j] > 32)
                      {
                        incorrect = true;
                        break;
                      }
                  }
                if(incorrect)
                  {
                    throw std::runtime_error(
                        "XMLParserCPP::parseDocument: incorrect document(2)");
                  }
              }
            else
              {
                parseElementContent(document, i, in_process);
                i--;
              }
            if(empty_element)
              {
                not_completed.erase(std::remove(not_completed.begin(),
                                                not_completed.end(),
                                                in_process),
                                    not_completed.end());
                if(in_process->element_name.size() > 0)
                  {
                    if(in_process->element_name[0] == '/')
                      {
                        in_process->element_name.erase(
                            in_process->element_name.begin());
                      }
                    else if(*in_process->element_name.rbegin() == '/')
                      {
                        in_process->element_name.pop_back();
                        in_process->empty = true;
                      }
                  }
                else
                  {
                    throw std::runtime_error("XMLParserCPP::parseDocument: "
                                             "incorrect "
                                             "document(4)");
                  }
                if(in_process->empty)
                  {
                    if(not_completed.size() > 0)
                      {
                        std::shared_ptr<XMLElement> last
                            = *not_completed.rbegin();

                        std::vector<XMLElement> el_v
                            = std::move(in_process->elements);
                        in_process->elements.clear();

                        last->elements.emplace_back(*in_process);

                        for(auto it_elv = el_v.begin(); it_elv != el_v.end();
                            it_elv++)
                          {
                            last->elements.emplace_back(*it_elv);
                          }
                      }
                    else
                      {
                        result.emplace_back(*in_process);
                      }
                  }
                else
                  {
                    if(not_completed.size() > 0)
                      {
                        std::shared_ptr<XMLElement> last
                            = *not_completed.rbegin();
                        if(last->element_name == in_process->element_name)
                          {
                            not_completed.pop_back();
                            if(not_completed.size() > 0)
                              {
                                std::shared_ptr<XMLElement> last2
                                    = *not_completed.rbegin();
                                last2->elements.emplace_back(*last);

                                std::vector<XMLElement> el_v
                                    = std::move(in_process->elements);

                                for(auto it_elv = el_v.begin();
                                    it_elv != el_v.end(); it_elv++)
                                  {
                                    last2->elements.emplace_back(*it_elv);
                                  }
                              }
                            else
                              {
                                result.emplace_back(*last);
                              }
                          }
                        else
                          {
                            throw std::runtime_error(
                                "XMLParserCPP::parseDocument: "
                                "incorrect "
                                "document(5)");
                          }
                      }
                    else
                      {
                        throw std::runtime_error(
                            "XMLParserCPP::parseDocument: "
                            "incorrect "
                            "document(6)");
                      }
                  }
              }
            else
              {
                n = document.find(find_str, i);
                if(n == std::string::npos)
                  {
                    i = n - 1;
                    break;
                  }
                XMLElement el;
                el.element_type = XMLElement::ElementContent;
                for(size_t j = i + 1; j < n; j++)
                  {
                    if(document[j] < 0 || document[j] >= 32)
                      {
                        el.content.push_back(document[j]);
                      }
                  }
                for(auto it = el.content.begin(); it != el.content.end();)
                  {
                    if(*it == ' ')
                      {
                        el.content.erase(it);
                      }
                    else
                      {
                        break;
                      }
                  }
                if(el.content.size() > 0)
                  {
                    in_process->elements.emplace_back(el);
                  }
                i = n - 1;
              }
            element_name_finished = false;
            empty_element = false;
            break;
          }
        default:
          {
            if(document[i] >= 0 && document[i] < 32)
              {
                break;
              }
            if(!element_name_finished)
              {
                in_process->element_name.push_back(document[i]);
              }
            else
              {
                throw std::runtime_error(
                    "XMLParserCPP::parseDocument: incorrect "
                    "document(7)");
              }
            break;
          }
        }
    }

  if(not_completed.size() > 0)
    {
      throw std::runtime_error("XMLParserCPP::parseDocument: incorrect "
                               "document(8)");
    }

  replaceXMLEntities(result);

  return result;
}

void
XMLParserCPP::parseElementAttribute(const std::string &document,
                                    size_t &position,
                                    std::shared_ptr<XMLElement> element)
{
  for(; document[position]; position++)
    {
      if(document[position] < 0 || document[position] > 32)
        {
          break;
        }
    }
  switch(document[position])
    {
    case '/':
    case '?':
      {
        position--;
        return void();
      }
    case '>':
      {
        position--;
        return void();
      }
    default:
      {
        if(position >= document.size())
          {
            return void();
          }
        break;
      }
    }

  XMLElementAttribute attr;
  bool attr_id_finished = false;
  bool s_quot = false;
  bool d_quot = false;
  bool stop = false;

  for(; position < document.size(); position++)
    {
      switch(document[position])
        {
        case 9:
        case 32:
          {
            if(s_quot || d_quot)
              {
                attr.attribute_value.push_back(document[position]);
              }
            break;
          }
        case '\'':
          {
            if(attr_id_finished)
              {
                if(s_quot)
                  {
                    stop = true;
                  }
                else if(d_quot)
                  {
                    attr.attribute_value.push_back(document[position]);
                  }
                else
                  {
                    s_quot = true;
                  }
              }
            else
              {
                throw std::runtime_error("XMLParserCPP::parseElementAttribute:"
                                         " incorrect attribute(2)");
              }
            break;
          }
        case '\"':
          {
            if(attr_id_finished)
              {
                if(d_quot)
                  {
                    stop = true;
                  }
                else if(s_quot)
                  {
                    attr.attribute_value.push_back(document[position]);
                  }
                else
                  {
                    d_quot = true;
                  }
              }
            else
              {
                throw std::runtime_error("XMLParserCPP::parseElementAttribute:"
                                         " incorrect attribute(3)");
              }
            break;
          }
        case '=':
          {
            if(attr_id_finished)
              {
                if(s_quot || d_quot)
                  {
                    attr.attribute_value.push_back(document[position]);
                  }
              }
            else
              {
                attr_id_finished = true;
              }
            break;
          }
        default:
          {
            if(document[position] >= 0 && document[position] < 32)
              {
                break;
              }
            if(attr_id_finished)
              {
                attr.attribute_value.push_back(document[position]);
              }
            else
              {
                attr.attribute_id.push_back(document[position]);
              }
            break;
          }
        }
      if(stop)
        {
          if(attr.attribute_id.size() > 0)
            {
              element->element_attributes.emplace_back(attr);
            }
          break;
        }
    }
  if(!stop)
    {
      throw std::runtime_error("XMLParserCPP::parseElementAttribute:"
                               " incorrect attribute(4)");
    }
}

void
XMLParserCPP::parseProgramControlElement(const std::string &document,
                                         size_t &position,
                                         std::shared_ptr<XMLElement> element)
{
  position++;
  bool stop = false;
  bool el_name_finished = false;
  for(; position < document.size(); position++)
    {
      switch(document[position])
        {
        case 9:
        case ' ':
          {
            el_name_finished = true;
            parseElementAttribute(document, position, element);
            break;
          }
        case '?':
        case '>':
          {
            stop = true;
            break;
          }
        default:
          {
            if(document[position] >= 0 && document[position] < 32)
              {
                break;
              }
            if(!el_name_finished)
              {
                element->element_name.push_back(document[position]);
              }
            else
              {
                throw std::runtime_error(
                    "XMLParserCPP::parseProgramControlElement: incorrect "
                    "element(1)");
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
      throw std::runtime_error(
          "XMLParserCPP::parseProgramControlElement: incorrect "
          "element(2)");
    }
}

void
XMLParserCPP::parseSpecialElement(const std::string &document,
                                  size_t &position,
                                  std::shared_ptr<XMLElement> element)
{
  position++;

  switch(document[position])
    {
    case '[':
      {
        element->element_type = XMLElement::CharData;
        break;
      }
    case '-':
      {
        element->element_type = XMLElement::Comment;
        break;
      }
    default:
      {
        element->element_type = XMLElement::Type::SpecialElement;
        break;
      }
    }

  switch(element->element_type)
    {
    case XMLElement::CharData:
      {
        std::string find_str = "[CDATA[";
        std::string::size_type n = document.find(find_str, position);
        if(n == std::string::npos)
          {
            throw std::runtime_error(
                "XMLParserCPP::parseSpecialElement: incorrect element(1)");
          }
        n += find_str.size();
        find_str = "]]>";
        std::string::size_type n2 = document.find(find_str, n);
        if(n2 == std::string::npos)
          {
            throw std::runtime_error(
                "XMLParserCPP::parseSpecialElement: incorrect element(2)");
          }
        std::copy(document.begin() + n, document.begin() + n2,
                  std::back_inserter(element->content));
        position = n2 + find_str.size() - 1;
        break;
      }
    case XMLElement::Comment:
      {
        std::string find_str("-->");
        std::string::size_type n = document.find(find_str, position);
        if(n == std::string::npos)
          {
            throw std::runtime_error(
                "XMLParserCPP::parseSpecialElement: incorrect element(3)");
          }
        std::copy(document.begin() + position, document.begin() + n,
                  std::back_inserter(element->content));
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
                  element->content.push_back(document[position]);
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
            throw std::runtime_error(
                "XMLParserCPP::parseSpecialElement: incorrect element(3)");
          }
        break;
      }
    }

  switch(element->element_type)
    {
    case XMLElement::Comment:
      {
        for(auto it = element->content.begin(); it != element->content.end();)
          {
            if(*it >= 0 && *it <= 32)
              {
                element->content.erase(it);
              }
            else
              {
                if(*it == '-')
                  {
                    element->content.erase(it);
                  }
                else
                  {
                    break;
                  }
              }
          }
        while(element->content.size() > 0)
          {
            auto it = element->content.rbegin();
            if(*it >= 0 && *it <= 32)
              {
                element->content.pop_back();
              }
            else
              {
                if(*it == '-')
                  {
                    element->content.pop_back();
                  }
                else
                  {
                    break;
                  }
              }
          }
        break;
      }
    case XMLElement::SpecialElement:
      {
        for(auto it = element->content.begin(); it != element->content.end();)
          {
            if(*it >= 0 && *it <= 32)
              {
                element->content.erase(it);
              }
            else
              {
                break;
              }
          }
        while(element->content.size() > 0)
          {
            auto it = element->content.rbegin();
            if(*it >= 0 && *it <= 32)
              {
                element->content.pop_back();
              }
            else
              {
                break;
              }
          }
        break;
      }
    default:
      break;
    }
}

void
XMLParserCPP::parseElementContent(const std::string &document,
                                  size_t &position,
                                  std::shared_ptr<XMLElement> element)
{
  position++;
  XMLElement el;
  el.element_type = XMLElement::Type::ElementContent;
  bool stop = false;
  for(; position < document.size(); position++)
    {
      switch(document[position])
        {
        case '<':
          {
            stop = true;
            break;
          }
        default:
          {
            el.content.push_back(document[position]);
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
      throw std::runtime_error(
          "XMLParserCPP::parseElementContent: incorrect content");
    }
  for(auto it = el.content.begin(); it != el.content.end(); it++)
    {
      if((*it) < 0 || (*it) > 32)
        {
          element->elements.emplace_back(el);
          break;
        }
    }
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
          if(to_replace.size() > 0)
            {
              if(to_replace[0] == '#')
                {
                  to_replace.erase(to_replace.begin());
                  bool num = true;
                  for(auto it_tr = to_replace.begin();
                      it_tr != to_replace.end(); it_tr++)
                    {
                      if(*it_tr < 48 || *it_tr > 57)
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
