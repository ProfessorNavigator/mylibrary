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

#include <DublinCoreParser.h>
#include <XMLAlgorithms.h>
#include <algorithm>

DublinCoreParser::DublinCoreParser()
{
  xml_parser = new XMLParserCPP;
}

DublinCoreParser::~DublinCoreParser()
{
  delete xml_parser;
}

std::string
DublinCoreParser::dcTitle(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:title", res);
  std::vector<XMLElement *> res2;
  XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);

  std::string l_result;

  for(size_t i = 0; i < res2.size(); i++)
    {
      l_result = res2[i]->content;

      normalizeString(l_result);

      if(!result.empty() && !l_result.empty())
        {
          result += ". ";
        }
      result += l_result;
    }

  return result;
}

std::string
DublinCoreParser::dcAuthor(const std::vector<XMLElement> &elements)
{
  std::string result;

  result = getAuthor1(elements);
  if(result.empty())
    {
      result = getAuthor2(elements);
    }
  else
    {
      std::string l_res = getAuthor2(elements);
      if(!l_res.empty())
        {
          result += ", ";
          result += l_res;
        }
    }
  if(result.empty())
    {
      std::vector<XMLElement *> res;
      XMLAlgorithms::searchElement(elements, "dc:creator", res);
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res, XMLElement::ElementContent, res2);
      std::string l_result;
      for(size_t i = 0; i < res2.size(); i++)
        {
          l_result = res2[i]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcGenre(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:subject", res);

  std::string l_result;

  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcDate(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:date", res);

  std::string l_result;

  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcDescription(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:description", res);

  XMLAlgorithms::writeXML(res, result);

  return result;
}

std::string
DublinCoreParser::dcLanguage(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:language", res);

  std::string l_result;
  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcTranslator(const std::vector<XMLElement> &elements)
{
  std::string result;

  result = getTranslator1(elements);
  if(result.empty())
    {
      result = getTranslator2(elements);
    }
  else
    {
      std::string l_res = getTranslator2(elements);
      if(!l_res.empty())
        {
          result += ", ";
          result += l_res;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcPublisher(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:publisher", res);

  std::string l_result;
  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcIdentifier(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:identifier", res);

  std::string l_result;
  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::dcSource(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:source", res);

  std::string l_result;
  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::getAuthor1(const std::vector<XMLElement> &elements)
{
  std::string result;
  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "meta", "property", "role", res);
  res.erase(std::remove_if(res.begin(), res.end(),
                           [](const XMLElement *el)
                             {
                               std::vector<XMLElement *> res;
                               XMLAlgorithms::searchElement(
                                   el->elements, XMLElement::ElementContent,
                                   res);
                               for(size_t i = 0; i < res.size(); i++)
                                 {
                                   if(res[i]->content == "aut")
                                     {
                                       return false;
                                     }
                                 }
                               return true;
                             }),
            res.end());
  for(size_t i = 0; i < res.size(); i++)
    {
      auto it = std::find_if(res[i]->element_attributes.begin(),
                             res[i]->element_attributes.end(),
                             [](const XMLElementAttribute &el)
                               {
                                 return el.attribute_id == "refines";
                               });
      if(it != res[i]->element_attributes.end())
        {
          std::string attr_val = it->attribute_value;
          for(auto it_str = attr_val.begin(); it_str != attr_val.end();)
            {
              if(*it_str == '#')
                {
                  attr_val.erase(it_str);
                }
              else
                {
                  break;
                }
            }

          std::vector<XMLElement *> res2;
          XMLAlgorithms::searchElement(elements, "dc:creator", "id", attr_val,
                                       res2);

          std::string l_result;
          for(size_t j = 0; j < res2.size(); j++)
            {
              std::vector<XMLElement *> res3;
              XMLAlgorithms::searchElement(res2[j]->elements,
                                           XMLElement::ElementContent, res3);

              for(size_t k = 0; k < res3.size(); k++)
                {
                  l_result = res3[k]->content;

                  normalizeString(l_result);

                  if(!result.empty() && !l_result.empty())
                    {
                      result += ", ";
                    }
                  result += l_result;
                }
            }
        }
    }

  return result;
}

std::string
DublinCoreParser::getAuthor2(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:creator", res);
  std::string find_str(":role");
  res.erase(std::remove_if(res.begin(), res.end(),
                           [find_str](const XMLElement *el)
                             {
                               auto it = std::find_if(
                                   el->element_attributes.begin(),
                                   el->element_attributes.end(),
                                   [find_str](const XMLElementAttribute &el)
                                     {
                                       std::string::size_type n
                                           = el.attribute_id.find(find_str);
                                       return n != std::string::npos;
                                     });
                               if(it != el->element_attributes.end())
                                 {
                                   if(it->attribute_value == "aut")
                                     {
                                       return false;
                                     }
                                 }
                               return true;
                             }),
            res.end());

  std::string l_result;
  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);

      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty() && !l_result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

std::string
DublinCoreParser::getTranslator1(const std::vector<XMLElement> &elements)
{
  std::string result;
  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "meta", "property", "role", res);
  res.erase(std::remove_if(res.begin(), res.end(),
                           [](const XMLElement *el)
                             {
                               std::vector<XMLElement *> res;
                               XMLAlgorithms::searchElement(
                                   el->elements, XMLElement::ElementContent,
                                   res);
                               for(size_t i = 0; i < res.size(); i++)
                                 {
                                   if(res[i]->content == "trl")
                                     {
                                       return false;
                                     }
                                 }
                               return true;
                             }),
            res.end());
  for(size_t i = 0; i < res.size(); i++)
    {
      auto it = std::find_if(res[i]->element_attributes.begin(),
                             res[i]->element_attributes.end(),
                             [](const XMLElementAttribute &el)
                               {
                                 return el.attribute_id == "refines";
                               });
      if(it != res[i]->element_attributes.end())
        {
          std::string attr_val = it->attribute_value;
          for(auto it_str = attr_val.begin(); it_str != attr_val.end();)
            {
              if(*it_str == '#')
                {
                  attr_val.erase(it_str);
                }
              else
                {
                  break;
                }
            }

          std::vector<XMLElement *> res2;
          XMLAlgorithms::searchElement(elements, "dc:creator", "id", attr_val,
                                       res2);
          std::string l_result;
          for(size_t j = 0; j < res2.size(); j++)
            {
              std::vector<XMLElement *> res3;
              XMLAlgorithms::searchElement(res2[j]->elements,
                                           XMLElement::ElementContent, res3);
              for(size_t k = 0; k < res3.size(); k++)
                {
                  l_result = res3[k]->content;

                  normalizeString(l_result);

                  if(!result.empty() && !l_result.empty())
                    {
                      result += ", ";
                    }
                  result += l_result;
                }
            }
        }
    }

  return result;
}

std::string
DublinCoreParser::getTranslator2(const std::vector<XMLElement> &elements)
{
  std::string result;

  std::vector<XMLElement *> res;
  XMLAlgorithms::searchElement(elements, "dc:creator", res);
  std::string find_str(":role");
  res.erase(std::remove_if(res.begin(), res.end(),
                           [find_str](const XMLElement *el)
                             {
                               auto it = std::find_if(
                                   el->element_attributes.begin(),
                                   el->element_attributes.end(),
                                   [find_str](const XMLElementAttribute &el)
                                     {
                                       std::string::size_type n
                                           = el.attribute_id.find(find_str);
                                       return n != std::string::npos;
                                     });
                               if(it != el->element_attributes.end())
                                 {
                                   if(it->attribute_value == "trl")
                                     {
                                       return false;
                                     }
                                 }
                               return true;
                             }),
            res.end());

  std::string l_result;
  for(size_t i = 0; i < res.size(); i++)
    {
      std::vector<XMLElement *> res2;
      XMLAlgorithms::searchElement(res[i]->elements,
                                   XMLElement::ElementContent, res2);
      for(size_t j = 0; j < res2.size(); j++)
        {
          l_result = res2[j]->content;

          normalizeString(l_result);

          if(!result.empty())
            {
              result += ", ";
            }
          result += l_result;
        }
    }

  return result;
}

void
DublinCoreParser::normalizeString(std::string &str)
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
