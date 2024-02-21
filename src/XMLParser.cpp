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

#include <XMLParser.h>
#include <iostream>
#include <iterator>

XMLParser::XMLParser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

XMLParser::~XMLParser()
{

}

std::vector<XMLTag>
XMLParser::get_tag(const std::string &book, const std::string &tag_id)
{
  std::vector<XMLTag> result;

  if(book.empty())
    {
      return result;
    }

  std::string::size_type n = 0;
  std::string::size_type n_end;
  for(;;)
    {
      n = book.find("<", n);
      if(n != std::string::npos)
	{
	  n_end = book.find(">", n);
	  if(n_end != std::string::npos)
	    {
	      XMLTag vl;
	      vl.element = std::string(book.begin() + n,
				       book.begin() + n_end + 1);
	      XMLTagId id = get_tag_id(vl.element);
	      if(id.tag_id == tag_id)
		{
		  if(!id.end_tag)
		    {
		      std::string::size_type n_fin = n_end;
		      std::string::size_type n_fin_end;
		      std::string loc_tag;
		      for(;;)
			{
			  n_fin = book.find("<", n_fin);
			  if(n_fin != std::string::npos)
			    {
			      n_fin_end = book.find(">", n_fin);
			      if(n_fin_end != std::string::npos)
				{
				  loc_tag = std::string(
				      book.begin() + n_fin,
				      book.begin() + n_fin_end + 1);
				  XMLTagId l_id = get_tag_id(loc_tag);
				  if(l_id.tag_id == tag_id
				      && l_id.end_tag)
				    {
				      vl.content = std::string(
					  book.begin() + n_end + 1,
					  book.begin() + n_fin);
				      af->html_to_utf8(vl.content);
				      break;
				    }
				  else
				    {
				      n_fin = n_fin_end;
				    }
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
		      result.emplace_back(vl);
		    }
		  else if(id.single_tag)
		    {
		      result.emplace_back(vl);
		    }
		}
	      n = n_end;
	    }
	  else
	    {
	      std::cout << "XMLParser::get_tag parsing error"
		  << std::endl;
	      break;
	    }
	}
      else
	{
	  break;
	}
    }

  return result;
}

std::string
XMLParser::get_book_encoding(const std::string &book)
{
  std::string result;

  if(book.empty())
    {
      return result;
    }

  std::string::size_type n = book.find("<");
  if(n != std::string::npos)
    {
      std::string::size_type n2 = book.find(">", n);
      if(n2 != std::string::npos)
	{
	  result = std::string(book.begin() + n, book.begin() + n2);
	  std::string code_page = af->detect_encoding(result);
	  result = af->to_utf_8(result, code_page.c_str());
	  for(auto it = result.begin(); it != result.end();)
	    {
	      if(*it == 0)
		{
		  result.erase(it);
		}
	      else
		{
		  it++;
		}
	    }
	  result = get_element_attribute(result, "encoding");
	}
    }
  else
    {
      result = af->detect_encoding(book);
    }

  return result;
}

std::string
XMLParser::get_element_attribute(const std::string &element,
				 const std::string &attr_name)
{
  std::string result;

  if(element.empty() || attr_name.empty())
    {
      return result;
    }

  std::string::size_type n = element.find(attr_name);
  if(n != std::string::npos)
    {
      std::string::size_type n2 = element.find("\"", n);
      if(n2 != std::string::npos)
	{
	  n2 = n2 + std::string("\"").size();
	  std::string::size_type n3 = element.find("\"", n2);
	  if(n3 != std::string::npos)
	    {
	      result = std::string(element.begin() + n2,
				   element.begin() + n3);
	    }
	}
      else
	{
	  n2 = element.find("'", n);
	  if(n2 != std::string::npos)
	    {
	      n2 = n2 + std::string("'").size();
	      std::string::size_type n3 = element.find("'", n2);
	      if(n3 != std::string::npos)
		{
		  result = std::string(element.begin() + n2,
				       element.begin() + n3);
		}
	    }
	}
    }

  return result;
}

std::vector<XMLTag>
XMLParser::get_tag(const std::string &book, const std::string &tag_id,
		   const bool &content_decode)
{
  std::vector<XMLTag> result;

  if(book.empty())
    {
      return result;
    }

  std::string::size_type n = 0;
  std::string::size_type n_end;
  std::string code_page;
  for(;;)
    {
      n = book.find("<", n);
      if(n != std::string::npos)
	{
	  n_end = book.find(">", n);
	  if(n_end != std::string::npos)
	    {
	      XMLTag vl;
	      vl.element = std::string(book.begin() + n,
				       book.begin() + n_end + 1);
	      code_page = af->detect_encoding(vl.element);
	      vl.element = af->to_utf_8(vl.element,
					code_page.c_str());
	      XMLTagId id = get_tag_id(vl.element);
	      if(id.tag_id == tag_id)
		{
		  if(!id.end_tag)
		    {
		      std::string::size_type n_fin = n_end;
		      std::string::size_type n_fin_end;
		      std::string loc_tag;
		      for(;;)
			{
			  n_fin = book.find("<", n_fin);
			  if(n_fin != std::string::npos)
			    {
			      n_fin_end = book.find(">", n_fin);
			      if(n_fin_end != std::string::npos)
				{
				  loc_tag = std::string(
				      book.begin() + n_fin,
				      book.begin() + n_fin_end + 1);
				  code_page = af->detect_encoding(
				      loc_tag);
				  loc_tag = af->to_utf_8(
				      loc_tag, code_page.c_str());
				  XMLTagId l_id = get_tag_id(loc_tag);
				  if(l_id.tag_id == tag_id
				      && l_id.end_tag)
				    {
				      vl.content = std::string(
					  book.begin() + n_end + 1,
					  book.begin() + n_fin);
				      if(content_decode)
					{
					  code_page = af
					      ->detect_encoding(
					      vl.content);
					  vl.content = af->to_utf_8(
					      vl.content,
					      code_page.c_str());
					}
				      af->html_to_utf8(vl.content);
				      break;
				    }
				  else
				    {
				      n_fin = n_fin_end;
				    }
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
		      result.emplace_back(vl);
		    }
		  else if(id.single_tag)
		    {
		      result.emplace_back(vl);
		    }
		}
	      n = n_end;
	    }
	  else
	    {
	      std::cout << "XMLParser::get_tag(2) parsing error"
		  << std::endl;
	      break;
	    }
	}
      else
	{
	  break;
	}
    }

  return result;
}

XMLTagId
XMLParser::get_tag_id(const std::string &tag)
{
  std::string loc_tag(tag);
  XMLTagId id;

  if(tag.empty())
    {
      return id;
    }
  std::string::size_type tag_single = loc_tag.find("/>");
  if(tag_single != std::string::npos)
    {
      id.end_tag = true;
      id.single_tag = true;
    }
  else
    {
      tag_single = loc_tag.find("</");
      if(tag_single != std::string::npos)
	{
	  id.end_tag = true;
	}
    }

  for(auto it = loc_tag.begin(); it != loc_tag.end();)
    {
      char ch = *it;
      if(ch == ' ' || ch == '<' || ch == '/')
	{
	  loc_tag.erase(it);
	}
      else
	{
	  break;
	}
    }

  std::string::size_type tag_e;
  tag_e = loc_tag.find(" ");
  if(tag_e != std::string::npos)
    {
      loc_tag = loc_tag.substr(0, tag_e);
    }
  else
    {
      tag_e = loc_tag.find("/>");
      if(tag_e != std::string::npos)
	{
	  loc_tag = loc_tag.substr(0, tag_e);
	}
      else
	{
	  tag_e = loc_tag.find(">");
	  if(tag_e != std::string::npos)
	    {
	      loc_tag = loc_tag.substr(0, tag_e);
	    }
	}
    }

  id.tag_id = loc_tag;

  return id;
}
