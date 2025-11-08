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

#include <XMLAlgorithms.h>
#include <algorithm>

XMLAlgorithms::XMLAlgorithms()
{
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement> &elements,
                             const std::string &element_name,
                             std::vector<XMLElement *> &result)
{
  XMLElement *end
      = const_cast<XMLElement *>(elements.data() + elements.size());
  for(XMLElement *it_el = const_cast<XMLElement *>(elements.data());
      it_el != end; it_el++)
    {
      if(it_el->element_name == element_name)
        {
          result.push_back(it_el);
        }
      searchElement(it_el->elements, element_name, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement> &elements,
                             const XMLElement::Type &element_type,
                             std::vector<XMLElement *> &result)
{
  XMLElement *end
      = const_cast<XMLElement *>(elements.data() + elements.size());
  for(XMLElement *it_el = const_cast<XMLElement *>(elements.data());
      it_el != end; it_el++)
    {
      if(it_el->element_type == element_type)
        {
          result.push_back(it_el);
        }
      searchElement(it_el->elements, element_type, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement> &elements,
                             const std::string &element_name,
                             const std::string &attribute_id,
                             std::vector<XMLElement *> &result)
{
  XMLElement *end
      = const_cast<XMLElement *>(elements.data() + elements.size());
  for(XMLElement *it_el = const_cast<XMLElement *>(elements.data());
      it_el != end; it_el++)
    {
      if(it_el->element_name == element_name)
        {
          auto it_attr
              = std::find_if(it_el->element_attributes.begin(),
                             it_el->element_attributes.end(),
                             [attribute_id](const XMLElementAttribute &el)
                               {
                                 return el.attribute_id == attribute_id;
                               });
          if(it_attr != it_el->element_attributes.end())
            {
              result.push_back(it_el);
            }
        }
      searchElement(it_el->elements, element_name, attribute_id, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement> &elements,
                             const std::string &element_name,
                             const std::string &attribute_id,
                             const std::string &attribute_value,
                             std::vector<XMLElement *> &result)
{
  XMLElement *end
      = const_cast<XMLElement *>(elements.data() + elements.size());
  for(XMLElement *it_el = const_cast<XMLElement *>(elements.data());
      it_el != end; it_el++)
    {
      if(it_el->element_name == element_name)
        {
          auto it_attr = std::find_if(
              it_el->element_attributes.begin(),
              it_el->element_attributes.end(),
              [attribute_id, attribute_value](const XMLElementAttribute &el)
                {
                  if(el.attribute_id == attribute_id)
                    {
                      if(el.attribute_value == attribute_value)
                        {
                          return true;
                        }
                    }
                  return false;
                });
          if(it_attr != it_el->element_attributes.end())
            {
              result.push_back(it_el);
            }
        }
      searchElement(it_el->elements, element_name, attribute_id,
                    attribute_value, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement *> &elements,
                             const std::string &element_name,
                             std::vector<XMLElement *> &result)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      XMLElement *el = *it_el;
      if(el->element_name == element_name)
        {
          result.push_back(el);
        }
      searchElement(el->elements, element_name, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement *> &elements,
                             const XMLElement::Type &element_type,
                             std::vector<XMLElement *> &result)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      XMLElement *el = *it_el;
      if(el->element_type == element_type)
        {
          result.push_back(el);
        }
      searchElement(el->elements, element_type, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement *> &elements,
                             const std::string &element_name,
                             const std::string &attribute_id,
                             std::vector<XMLElement *> &result)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      XMLElement *el = *it_el;
      if(el->element_name == element_name)
        {
          auto it = std::find_if(el->element_attributes.begin(),
                                 el->element_attributes.end(),
                                 [attribute_id](const XMLElementAttribute &el)
                                   {
                                     return el.attribute_id == attribute_id;
                                   });
          if(it != el->element_attributes.end())
            {
              result.push_back(el);
            }
        }
      searchElement(el->elements, element_name, attribute_id, result);
    }
}

void
XMLAlgorithms::searchElement(const std::vector<XMLElement *> &elements,
                             const std::string &element_name,
                             const std::string &attribute_id,
                             const std::string &attribute_value,
                             std::vector<XMLElement *> &result)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      XMLElement *el = *it_el;
      if(el->element_name == element_name)
        {
          auto it = std::find_if(
              el->element_attributes.begin(), el->element_attributes.end(),
              [attribute_id, attribute_value](const XMLElementAttribute &el)
                {
                  if(el.attribute_id == attribute_id)
                    {
                      if(el.attribute_value == attribute_value)
                        {
                          return true;
                        }
                    }
                  return false;
                });
          if(it != el->element_attributes.end())
            {
              result.push_back(el);
            }
        }
      searchElement(el->elements, element_name, attribute_id, attribute_value,
                    result);
    }
}

void
XMLAlgorithms::writeXML(const std::vector<XMLElement> &elements,
                        std::string &result)
{
  XMLAlgorithms algo;
  size_t tab_count = 0;
  algo.writeXMLRecursive(elements, result, tab_count);
  while(result.size() > 0)
    {
      if(*result.rbegin() == ' ')
        {
          result.pop_back();
        }
      else
        {
          break;
        }
    }
}

void
XMLAlgorithms::writeXML(const std::vector<XMLElement *> &elements,
                        std::string &result)
{
  XMLAlgorithms algo;
  size_t tab_count = 0;
  algo.writeXMLRecursive(elements, result, tab_count);
  while(result.size() > 0)
    {
      if(*result.rbegin() == ' ')
        {
          result.pop_back();
        }
      else
        {
          break;
        }
    }
}

void
XMLAlgorithms::writeXMLRecursive(const std::vector<XMLElement> &elements,
                                 std::string &result, size_t &tab_count)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      if(it_el->element_type != XMLElement::Type::ElementContent)
        {
          if(result.size() > 0)
            {
              switch(*result.rbegin())
                {
                case '>':
                  {
                    result.push_back('\n');
                  }
                case '\n':
                  {
                    for(size_t i = 0; i < tab_count; i++)
                      {
                        result.push_back(' ');
                      }
                    break;
                  }
                default:
                  break;
                }
            }
          result += "<";
        }
      switch(it_el->element_type)
        {
        case XMLElement::Type::ProgramControlElement:
          {
            result += "?";
            result += it_el->element_name;
            for(auto it_attr = it_el->element_attributes.begin();
                it_attr != it_el->element_attributes.end(); it_attr++)
              {
                result += " ";
                result += it_attr->attribute_id;
                result += "=\"";
                result += it_attr->attribute_value;
                result += "\"";
              }
            result += "?>";
            break;
          }
        case XMLElement::Type::CharData:
          {
            result += "![CDATA[";
            result += it_el->content;
            result += "]]>";
            break;
          }
        case XMLElement::Type::Comment:
          {
            result += "!-- ";
            result += it_el->content;
            result += " -->";
            break;
          }
        case XMLElement::Type::SpecialElement:
          {
            result += "!";
            result += it_el->content;
            result += ">";
            break;
          }
        case XMLElement::Type::OrdinaryElement:
          {
            result += it_el->element_name;
            for(auto it_attr = it_el->element_attributes.begin();
                it_attr != it_el->element_attributes.end(); it_attr++)
              {
                result += " ";
                result += it_attr->attribute_id;
                result += "=\"";
                copyAndReplaceProhibitedSymbols(it_attr->attribute_value,
                                                result);
                result += "\"";
              }
            if(it_el->empty)
              {
                result += "/>";
              }
            else
              {
                result += ">";
                if(it_el->elements.size() > 0)
                  {
                    tab_count++;
                    writeXMLRecursive(it_el->elements, result, tab_count);
                    tab_count--;
                  }
                if(result.size() > 0)
                  {
                    switch(*result.rbegin())
                      {
                      case '>':
                        {
                          result.push_back('\n');
                        }
                      case '\n':
                        {
                          for(size_t i = 0; i < tab_count; i++)
                            {
                              result.push_back(' ');
                            }
                          break;
                        }
                      default:
                        break;
                      }
                  }
                result += "</";
                result += it_el->element_name;
                result += ">";
              }
            break;
          }
        case XMLElement::Type::ElementContent:
          {
            copyAndReplaceProhibitedSymbols(it_el->content, result);
            break;
          }
        default:
          break;
        }
    }
}

void
XMLAlgorithms::writeXMLRecursive(const std::vector<XMLElement *> &elements,
                                 std::string &result, size_t &tab_count)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      XMLElement *el_ptr = *it_el;
      if(el_ptr->element_type != XMLElement::Type::ElementContent)
        {
          if(result.size() > 0)
            {
              switch(*result.rbegin())
                {
                case '>':
                  {
                    result.push_back('\n');
                  }
                case '\n':
                  {
                    for(size_t i = 0; i < tab_count; i++)
                      {
                        result.push_back(' ');
                      }
                    break;
                  }
                default:
                  break;
                }
            }
          result += "<";
        }
      switch(el_ptr->element_type)
        {
        case XMLElement::Type::ProgramControlElement:
          {
            result += "?";
            result += el_ptr->element_name;
            for(auto it_attr = el_ptr->element_attributes.begin();
                it_attr != el_ptr->element_attributes.end(); it_attr++)
              {
                result += " ";
                result += it_attr->attribute_id;
                result += "=\"";
                copyAndReplaceProhibitedSymbols(it_attr->attribute_value,
                                                result);
                result += "\"";
              }
            result += "?>";
            break;
          }
        case XMLElement::Type::CharData:
          {
            result += "![CDATA[";
            result += el_ptr->content;
            result += "]]>";
            break;
          }
        case XMLElement::Type::Comment:
          {
            result += "!-- ";
            result += el_ptr->content;
            result += " -->";
            break;
          }
        case XMLElement::Type::SpecialElement:
          {
            result += "!";
            result += el_ptr->content;
            result += ">";
            break;
          }
        case XMLElement::Type::OrdinaryElement:
          {
            result += el_ptr->element_name;
            for(auto it_attr = el_ptr->element_attributes.begin();
                it_attr != el_ptr->element_attributes.end(); it_attr++)
              {
                result += " ";
                result += it_attr->attribute_id;
                result += "=\"";
                result += it_attr->attribute_value;
                result += "\"";
              }
            if(el_ptr->empty)
              {
                result += "/>";
              }
            else
              {
                result += ">";
                if(el_ptr->elements.size() > 0)
                  {
                    tab_count++;
                    writeXMLRecursive(el_ptr->elements, result, tab_count);
                    tab_count--;
                  }
                if(result.size() > 0)
                  {
                    switch(*result.rbegin())
                      {
                      case '>':
                        {
                          result.push_back('\n');
                        }
                      case '\n':
                        {
                          for(size_t i = 0; i < tab_count; i++)
                            {
                              result.push_back(' ');
                            }
                          break;
                        }
                      default:
                        break;
                      }
                  }
                result += "</";
                result += el_ptr->element_name;
                result += ">";
              }
            break;
          }
        case XMLElement::Type::ElementContent:
          {
            copyAndReplaceProhibitedSymbols(el_ptr->content, result);
            break;
          }
        default:
          break;
        }
    }
}

void
XMLAlgorithms::copyAndReplaceProhibitedSymbols(const std::string &source,
                                               std::string &result)
{
  size_t limit = source.size();
  for(size_t i = 0; i < limit; i++)
    {
      switch(source[i])
        {
        case '<':
          {
            result += "&lt;";
            break;
          }
        case '>':
          {
            result += "&gt;";
            break;
          }
        case '&':
          {
            result += "&amp;";
            break;
          }
        case '\'':
          {
            result += "&apos;";
            break;
          }
        case '\"':
          {
            result += "&quot;";
            break;
          }
        default:
          {
            result.push_back(source[i]);
            break;
          }
        }
    }
}
