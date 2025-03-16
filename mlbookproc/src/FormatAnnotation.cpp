/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <FormatAnnotation.h>
#include <MLException.h>
#include <algorithm>
#include <iostream>

FormatAnnotation::FormatAnnotation(const std::shared_ptr<AuxFunc> &af)
    : XMLParser(af)
{
  formReplacementTable();
}

void
FormatAnnotation::remove_escape_sequences(std::string &annotation)
{
  annotation.erase(std::remove_if(annotation.begin(), annotation.end(),
                                  [](char &el) {
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
  try
    {
      std::vector<XMLTag> tgv = listAllTags(annotation);
      recursiveReplacement(tgv, annotation);
    }
  catch(MLException &e)
    {
      std::cout << "FormatAnnotation::replace_tags: " << e.what() << std::endl;
    }
}

void
FormatAnnotation::formReplacementTable()
{
  replace_tag tag;

  tag.tag_to_replace = "p";
  tag.begin_replacement = "  ";
  tag.end_replacement = "\n";
  replacement_table.push_back(tag);

  tag.tag_to_replace = "empty-line";
  tag.begin_replacement = "";
  tag.end_replacement = "\n\n  ";
  replacement_table.push_back(tag);

  tag.tag_to_replace = "sub";
  tag.begin_replacement = "<span rise=\"-5pt\">";
  tag.end_replacement = "</span>";
  replacement_table.push_back(tag);

  tag.tag_to_replace = "sup";
  tag.begin_replacement = "<span rise=\"5pt\">";
  tag.end_replacement = "</span>";
  replacement_table.push_back(tag);

  tag.tag_to_replace = "strong";
  tag.begin_replacement = "<span font_weight=\"bold\">";
  tag.end_replacement = "</span>";
  replacement_table.push_back(tag);

  tag.tag_to_replace = "emphasis";
  tag.begin_replacement = "<span font_style=\"italic\">";
  tag.end_replacement = "</span>";
  replacement_table.push_back(tag);

  tag.tag_to_replace = "strikethrough";
  tag.begin_replacement = "<span strikethrough=\"true\">";
  tag.end_replacement = "</span>";
  replacement_table.push_back(tag);
}

void
FormatAnnotation::final_cleaning(std::string &annotation)
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
  XMLParser::removeAllTags(annotation);
}

void
FormatAnnotation::replace_html(std::string &annotation,
                               const std::string &sstr,
                               const std::string &replacement)
{
  std::string::size_type n = 0;
  for(;;)
    {
      n = annotation.find(sstr, n);
      if(n != std::string::npos)
        {
          annotation.erase(n, sstr.size());
          annotation.insert(n, replacement);
        }
      else
        {
          break;
        }
    }
}

void
FormatAnnotation::recursiveReplacement(const std::vector<XMLTag> &tgv,
                                       std::string &annotation)
{
  for(auto it = tgv.rbegin(); it != tgv.rend(); it++)
    {
      std::string tag_id = it->tag_id;
      if(tag_id == "CDATA")
        {
          std::string repl = it->element;
          std::string find_str = "<![CDATA[";
          std::string::size_type n_cd = 0;
          while(repl.size() > 0)
            {
              n_cd = repl.find(find_str, n_cd);
              if(n_cd != std::string::npos)
                {
                  repl.erase(n_cd, find_str.size());
                }
              else
                {
                  break;
                }
            }
          find_str = "]]>";
          n_cd = 0;
          while(repl.size() > 0)
            {
              n_cd = repl.find(find_str, n_cd);
              if(n_cd != std::string::npos)
                {
                  repl.erase(n_cd, find_str.size());
                }
              else
                {
                  break;
                }
            }
          if(it->content_start != std::string::npos)
            {
              auto it_r
                  = annotation.erase(annotation.begin() + it->content_start
                                         - it->element.size() - 1,
                                     annotation.begin() + it->content_start);
              annotation.insert(it_r, repl.begin(), repl.end());
            }
          continue;
        }
      auto it_rpl
          = std::find_if(replacement_table.begin(), replacement_table.end(),
                         [tag_id](replace_tag &el) {
                           return tag_id == el.tag_to_replace;
                         });
      if(it->content_end != std::string::npos)
        {
          annotation.erase(annotation.begin() + it->content_end,
                           annotation.begin() + it->content_end
                               + it->tag_id.size() + 3);
          if(it_rpl != replacement_table.end())
            {
              annotation.insert(annotation.begin() + it->content_end,
                                it_rpl->end_replacement.begin(),
                                it_rpl->end_replacement.end());
            }
          else
            {
              annotation.insert(annotation.begin() + it->content_end, 32);
            }
        }

      recursiveReplacement(it->tag_list, annotation);

      if(it->content_start != std::string::npos)
        {
          std::string::size_type n = it->content_start - it->element.size();
          std::string repl;
          if(tag_id == "a")
            {
              repl = get_element_attribute(it->element, "l:href");
              if(repl.empty())
                {
                  repl = get_element_attribute(it->element, "href");
                }
              if(repl.empty())
                {
                  if(it->hasContent())
                    {
                      std::copy(annotation.begin() + it->content_start,
                                annotation.begin() + it->content_end,
                                std::back_inserter(repl));
                    }
                }
            }
          else
            {
              if(it_rpl != replacement_table.end())
                {
                  repl = it_rpl->begin_replacement;
                }
              else
                {
                  repl = " ";
                }
            }
          annotation.erase(annotation.begin() + n,
                           annotation.begin() + it->content_start);
          annotation.insert(annotation.begin() + n, repl.begin(), repl.end());
        }
    }
}
