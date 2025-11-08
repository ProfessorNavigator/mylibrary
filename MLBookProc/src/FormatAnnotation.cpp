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

#include <FormatAnnotation.h>
#include <algorithm>
#include <iostream>

FormatAnnotation::FormatAnnotation(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  xml_parser = new XMLParserCPP;
}

FormatAnnotation::~FormatAnnotation()
{
  delete xml_parser;
}

void
FormatAnnotation::remove_escape_sequences(std::string &annotation)
{
  removeEscapeSequences(annotation);
}

void
FormatAnnotation::removeEscapeSequences(std::string &annotation)
{
  annotation.erase(std::remove_if(annotation.begin(), annotation.end(),
                                  [](char &el)
                                    {
                                      switch(el)
                                        {
                                        case 0 ... 8:
                                        case 11 ... 31:
                                          {
                                            return true;
                                          }
                                        default:
                                          return false;
                                        }
                                    }),
                   annotation.end());

  for(auto it = annotation.begin(); it != annotation.end(); it++)
    {
      if(*it == 9)
        {
          *it = 32;
        }
    }

  std::string::size_type n = 0;
  std::string sstr = "  ";
  for(;;)
    {
      n = annotation.find(sstr, n);
      if(n != std::string::npos)
        {
          annotation.erase(annotation.begin() + n);
        }
      else
        {
          break;
        }
    }
}

void
FormatAnnotation::replace_tags(std::string &annotation)
{
  replaceTags(annotation);
}

void
FormatAnnotation::replaceTags(std::string &annotation)
{
  if(annotation.empty())
    {
      return void();
    }
  std::vector<XMLElement> elements;
  try
    {
      elements = xml_parser->parseDocument(annotation);
      annotation.clear();
    }
  catch(std::exception &e)
    {
      std::cout << "FormatAnnotation::replaceTags: \"" << e.what() << "\""
                << std::endl;
    }
  if(annotation.empty())
    {
      recursiveReplacement(elements, annotation);
    }
}

void
FormatAnnotation::final_cleaning(std::string &annotation)
{
  finalCleaning(annotation);
}

void
FormatAnnotation::finalCleaning(std::string &annotation)
{
  std::string::size_type n = 0;
  std::string sstr = "   ";
  for(;;)
    {
      n = annotation.find(sstr, n);
      if(n != std::string::npos)
        {
          annotation.erase(annotation.begin() + n);
        }
      else
        {
          break;
        }
    }
  for(auto it = annotation.begin(); it != annotation.end();)
    {
      if(*it == '\n')
        {
          annotation.erase(it);
        }
      else
        {
          break;
        }
    }
  bool g_break = false;
  while(annotation.size() > 0)
    {
      switch(*annotation.rbegin())
        {
        case 0 ... 32:
          {
            annotation.pop_back();
            break;
          }
        default:
          {
            g_break = true;
            break;
          }
        }
      if(g_break)
        {
          break;
        }
    }
  sstr = "\n\n\n";
  n = 0;
  for(;;)
    {
      n = annotation.find(sstr, n);
      if(n != std::string::npos)
        {
          annotation.erase(annotation.begin() + n);
        }
      else
        {
          break;
        }
    }
}

void
FormatAnnotation::removeAllTags(std::string &annotation)
{
  if(annotation.empty())
    {
      return void();
    }
  std::vector<XMLElement> elements;
  try
    {
      elements = xml_parser->parseDocument(annotation);
      annotation.clear();
    }
  catch(std::exception &e)
    {
      std::cout << "FormatAnnotation::removeAllTags: \"" << e.what() << "\""
                << std::endl;
    }
  if(annotation.empty())
    {
      recursiveTagRemoving(elements, annotation);
    }
}

void
FormatAnnotation::setTagReplacementTable(
    const std::vector<ReplaceTagItem> &replacement_table,
    const std::vector<std::tuple<std::string, std::string>>
        &symbols_replacement)
{
  this->replacement_table = replacement_table;
  this->symbols_replacement = symbols_replacement;
}

void
FormatAnnotation::recursiveReplacement(const std::vector<XMLElement> &elements,
                                       std::string &annotation)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      auto it_rpl
          = std::find_if(replacement_table.begin(), replacement_table.end(),
                         [it_el](const ReplaceTagItem &el)
                           {
                             return el.tag_to_replace == it_el->element_name;
                           });
      if(it_rpl != replacement_table.end())
        {
          annotation += it_rpl->begin_replacement;
        }
      if(it_el->element_type == XMLElement::ElementContent
         || it_el->element_type == XMLElement::CharData)
        {
          annotation += replaceSymbols(it_el->content);
        }
      else
        {
          recursiveReplacement(it_el->elements, annotation);
        }
      if(it_rpl != replacement_table.end())
        {
          annotation += it_rpl->end_replacement;
        }
    }
}

void
FormatAnnotation::recursiveTagRemoving(const std::vector<XMLElement> &elements,
                                       std::string &annotation)
{
  for(auto it_el = elements.begin(); it_el != elements.end(); it_el++)
    {
      if(it_el->element_type == XMLElement::ElementContent
         || it_el->element_type == XMLElement::CharData)
        {
          annotation += replaceSymbols(it_el->content);
        }
      else
        {
          recursiveTagRemoving(it_el->elements, annotation);
        }
    }
}

std::string
FormatAnnotation::replaceSymbols(const std::string &source)
{
  std::string result = source;

  std::string::size_type n;
  for(auto it = symbols_replacement.begin(); it != symbols_replacement.end();
      it++)
    {
      n = 0;
      for(;;)
        {
          n = result.find(std::get<0>(*it), n);
          if(n != std::string::npos)
            {
              result.replace(n, std::get<0>(*it).size(), std::get<1>(*it), 0,
                             std::get<1>(*it).size());
            }
          else
            {
              break;
            }
          n += std::get<1>(*it).size();
        }
    }

  return result;
}
