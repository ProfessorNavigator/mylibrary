/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <algorithm>
#include <iostream>
#include <iterator>

FormatAnnotation::FormatAnnotation(const std::shared_ptr<AuxFunc> &af) : XMLParser(
    af)
{
  formReplacementTable();
}

FormatAnnotation::~FormatAnnotation()
{

}

void
FormatAnnotation::remove_escape_sequences(std::string &annotation)
{
  annotation.erase(std::remove_if(annotation.begin(), annotation.end(), []
  (auto &el)
    {
      if(el == '\a' || el == '\b' || el == '\f' || el == '\n' || el == '\r'
	  || el == '\t' || el == '\v')
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }),
		   annotation.end());
  for(auto it = annotation.begin(); it != annotation.end();)
    {
      if(*it == ' ')
	{
	  annotation.erase(it);
	}
      else
	{
	  break;
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
  std::string::size_type n_start = 0;
  std::string::size_type n_end = 0;
  std::string sstr1 = "<";
  std::string sstr2 = ">";
  for(;;)
    {
      n_start = annotation.find(sstr1, n_start);
      if(n_start != std::string::npos)
	{
	  n_end = annotation.find(sstr2, n_start);
	  if(n_end != std::string::npos)
	    {
	      std::string tag(annotation.begin() + n_start,
			      annotation.begin() + n_end + 1);
	      annotation.erase(annotation.begin() + n_start,
			       annotation.begin() + n_end + 1);
	      tag_replacement_process(tag, annotation, n_start);
	    }
	  else
	    {
	      break;
	    }
	}
      else
	{
	  break;
	}
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
FormatAnnotation::tag_replacement_process(const std::string &tag,
					  std::string &annotation,
					  std::string::size_type &n)
{
  XMLTagId id = get_tag_id(tag);
  if(id.tag_id == "a")
    {
      std::string ref = get_element_attribute(tag, "href");
      if(!ref.empty())
	{
	  std::string::size_type n_end = annotation.find("</a", n);
	  if(n_end != std::string::npos)
	    {
	      annotation.erase(annotation.begin() + n,
			       annotation.begin() + n_end);
	      annotation.insert(n, ref);
	      return void();
	    }
	  else
	    {
	      n_end = annotation.find("<a/", n);
	      if(n_end != std::string::npos)
		{
		  annotation.erase(annotation.begin() + n,
				   annotation.begin() + n_end);
		  annotation.insert(n, ref);
		  return void();
		}
	    }
	}
    }

  auto it = std::find_if(replacement_table.begin(), replacement_table.end(),
			 [id]
			 (auto &el)
			   {
			     return el.tag_to_replace == id.tag_id;
			   });
  if(it != replacement_table.end())
    {
      if(!id.end_tag)
	{
	  annotation.insert(n, it->begin_replacement);
	  n = n + it->begin_replacement.size();
	}
      else
	{
	  annotation.insert(n, it->end_replacement);
	  n = n + it->end_replacement.size();
	}
    }
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
}
