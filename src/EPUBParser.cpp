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

#include <EPUBParser.h>
#include <XMLTag.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>

EPUBParser::EPUBParser(const std::shared_ptr<AuxFunc> &af) :
    XMLParser(af)
{
  this->af = af;
}

EPUBParser::~EPUBParser()
{

}

BookParseEntry
EPUBParser::epub_parser(const std::filesystem::path &filepath)
{
  BookParseEntry be;
  std::vector<ZipFileEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epub_get_root_file_address(filepath,
						    filenames);
  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
	{
	  if(it->filename == rootfile)
	    {
	      root_file_content = unpackByPosition(filepath, *it);
	    }
	}
    }

  if(!root_file_content.empty())
    {
      be = epub_parse_root_file(root_file_content);
    }

  return be;
}

std::string
EPUBParser::epub_get_root_file_address(
    const std::filesystem::path &filepath,
    const std::vector<ZipFileEntry> &filenames)
{
  std::string buf;
  for(auto it = filenames.begin(); it != filenames.end(); it++)
    {
      if(it->filename == "META-INF/container.xml")
	{
	  buf = unpackByPosition(filepath, *it);
	  break;
	}
    }

  std::string content;
  if(buf.size() > 0)
    {
      std::string code_page = get_book_encoding(buf);
      if(code_page.empty())
	{
	  code_page = af->detect_encoding(buf);
	}
      buf = af->to_utf_8(buf, code_page.c_str());
      std::vector<XMLTag> el = get_tag(buf, "rootfiles");
      if(el.size() > 0)
	{

	  el = get_tag(el[0].content, "rootfile");
	  if(el.size() > 0)
	    {
	      content = get_element_attribute(el[0].element,
					      "full-path");
	    }
	}
    }
  return content;
}

BookParseEntry
EPUBParser::epub_parse_root_file(const std::string &root_file_content)
{
  BookParseEntry result;
  std::string code_page = get_book_encoding(root_file_content);
  std::string buf;
  if(code_page.empty())
    {
      code_page = af->detect_encoding(root_file_content);
    }
  buf = af->to_utf_8(root_file_content, code_page.c_str());
  std::vector<XMLTag> el = get_tag(buf, "dc:title");
  if(el.size() > 0)
    {
      result.book_name = el[0].content;
    }

  std::vector<std::string> auth = epub_get_tags_by_role(
      root_file_content, "dc:creator", "aut", false);
  for(auto it = auth.begin(); it != auth.end(); it++)
    {
      if(result.book_author.size() > 0)
	{
	  result.book_author = result.book_author + ", " + *it;
	}
      else
	{
	  result.book_author = *it;
	}
    }

  el = get_tag(buf, "dc:date");
  if(el.size() > 0)
    {
      result.book_date = el[0].content;
    }

  el = get_tag(buf, "dc:subject");
  for(auto it = el.begin(); it != el.end(); it++)
    {
      if(result.book_genre.size() > 0)
	{
	  result.book_genre = result.book_genre + ", " + it->content;
	}
      else
	{
	  result.book_genre = it->content;
	}
    }

  return result;
}

std::shared_ptr<BookInfoEntry>
EPUBParser::epub_book_info(const std::filesystem::path &filepath)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<
      BookInfoEntry>();
  std::vector<ZipFileEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epub_get_root_file_address(filepath,
						    filenames);
  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
	{
	  if(it->filename == rootfile)
	    {
	      root_file_content = unpackByPosition(filepath, *it);
	    }
	}
    }

  if(!root_file_content.empty())
    {
      result->annotation = epub_annotation(root_file_content);
      epub_language(root_file_content, *result);
      epub_translator(root_file_content, *result);
      epub_publisher(root_file_content, *result);
      epub_identifier(root_file_content, *result);
      epub_source(root_file_content, *result);
      std::string cover_fnm = epub_cover_address(root_file_content);
      std::filesystem::path root = std::filesystem::u8path(rootfile);
      std::string root_path = root.parent_path().u8string();
      if(!root_path.empty())
	{
	  cover_fnm = root_path + "/" + cover_fnm;
	}
      for(auto it = filenames.begin(); it != filenames.end(); it++)
	{
	  if(it->filename == cover_fnm)
	    {
	      result->cover = unpackByPosition(filepath, *it);
	      result->cover_type = "file";
	      break;
	    }
	}
    }

  return result;
}

std::string
EPUBParser::epub_annotation(const std::string &root_file_content)
{
  std::string result;

  std::string code_page = get_book_encoding(root_file_content);
  std::string buf;
  if(code_page.empty())
    {
      code_page = af->detect_encoding(root_file_content);
    }
  buf = af->to_utf_8(root_file_content, code_page.c_str());
  std::vector<XMLTag> el = get_tag(buf, "dc:description");
  if(el.size() > 0)
    {
      result = el[0].content;
    }

  return result;
}

std::string
EPUBParser::epub_cover_address(const std::string &root_file_content)
{
  std::string result;

  std::string code_page = get_book_encoding(root_file_content);
  std::string buf;
  if(code_page.empty())
    {
      code_page = af->detect_encoding(root_file_content);
    }
  buf = af->to_utf_8(root_file_content, code_page.c_str());

  std::vector<XMLTag> el = get_tag(buf, "metadata");
  if(el.size() > 0)
    {
      el = get_tag(el[0].content, "meta");
      std::string cover_tag_id;
      for(auto it = el.begin(); it != el.end(); it++)
	{
	  std::string id = get_element_attribute(it->element, "name");
	  if(id == "cover")
	    {
	      cover_tag_id = get_element_attribute(it->element,
						   "content");
	      break;
	    }
	}
      if(!cover_tag_id.empty())
	{
	  el = get_tag(buf, "manifest");
	  if(el.size() > 0)
	    {
	      el = get_tag(el[0].content, "item");
	      for(auto it = el.begin(); it != el.end(); it++)
		{
		  if(cover_tag_id
		      == get_element_attribute(it->element, "id"))
		    {
		      result = get_element_attribute(it->element,
						     "href");
		      break;
		    }
		}

	    }
	}
    }

  if(result.empty())
    {
      el = get_tag(buf, "manifest");
      if(el.size() > 0)
	{
	  el = get_tag(el[0].content, "item");
	  for(auto it = el.begin(); it != el.end(); it++)
	    {
	      if(get_element_attribute(it->element, "properties")
		  == "cover-image")
		{
		  result = get_element_attribute(it->element, "href");
		  break;
		}
	    }
	}
    }

  if(result.empty())
    {
      el = get_tag(buf, "manifest");
      if(el.size() > 0)
	{
	  el = get_tag(el[0].content, "item");
	  std::string ch;
	  std::string::size_type n;
	  for(auto it = el.begin(); it != el.end(); it++)
	    {
	      ch = get_element_attribute(it->element, "media-type");
	      n = ch.find("image");
	      if(n != std::string::npos)
		{
		  result = get_element_attribute(it->element, "href");
		  break;
		}
	    }
	}
    }

  return result;
}

bool
EPUBParser::epub_find_role_in_meta(XMLTag &tag, const std::string &id,
				   const std::string &role)
{
  bool result = false;

  std::string refines = get_element_attribute(tag.element, "refines");
  if(refines == std::string("#" + id))
    {
      std::string prop = get_element_attribute(tag.element,
					       "property");
      if(prop == "role" && tag.content == role)
	{
	  result = true;
	}
    }

  return result;
}

void
EPUBParser::epub_language(const std::string &root_file_content,
			  BookInfoEntry &result)
{
  std::vector<XMLTag> el;
  el = get_tag(root_file_content, "metadata");
  if(el.size() > 0)
    {
      el = get_tag(el[0].content, "dc:language");
      for(auto it = el.begin(); it != el.end(); it++)
	{
	  if(!it->content.empty())
	    {
	      if(result.language.empty())
		{
		  result.language = it->content;
		}
	      else
		{
		  result.language = result.language + ", "
		      + it->content;
		}
	    }
	}
    }
}

void
EPUBParser::epub_translator(const std::string &root_file_content,
			    BookInfoEntry &result)
{
  std::vector<std::string> trl;
  for(int i = 1; i <= 2; i++)
    {
      switch(i)
	{
	case 1:
	  {
	    trl = epub_get_tags_by_role(root_file_content,
					"dc:contributor", "trl",
					true);
	    break;
	  }
	case 2:
	  {
	    trl = epub_get_tags_by_role(root_file_content,
					"dc:creator", "trl", true);
	    break;
	  }
	default:
	  {
	    trl.clear();
	    break;
	  }
	}
      for(auto it = trl.begin(); it != trl.end(); it++)
	{
	  if(result.translator.size() > 0)
	    {
	      result.translator = result.translator + ", " + *it;
	    }
	  else
	    {
	      result.translator = *it;
	    }
	}
    }
}

std::vector<std::string>
EPUBParser::epub_get_tags_by_role(
    const std::string &root_file_content, const std::string &tag_val,
    const std::string &role_input, const bool &exact_search)
{
  std::vector<std::string> result;

  std::vector<XMLTag> meta;
  meta = get_tag(root_file_content, "meta");
  std::vector<XMLTag> el = get_tag(root_file_content, tag_val);
  std::string id;
  std::string role;
  for(auto it = el.begin(); it != el.end(); it++)
    {
      role = get_element_attribute(it->element, "role");
      if(role.empty())
	{
	  id = get_element_attribute(it->element, "id");
	  if(!id.empty())
	    {
	      auto itmeta = std::find_if(
		  meta.begin(),
		  meta.end(),
		  std::bind(&EPUBParser::epub_find_role_in_meta, this,
			    std::placeholders::_1, id, role_input));
	      if(itmeta != meta.end())
		{
		  role = role_input;
		}
	    }
	  else
	    {
	      if(!exact_search)
		{
		  role = role_input;
		}
	    }
	}
      if(role == role_input)
	{
	  if(!it->content.empty())
	    {
	      result.push_back(it->content);
	    }
	}
    }

  return result;
}

void
EPUBParser::epub_publisher(const std::string &root_file_content,
			   BookInfoEntry &result)
{
  std::vector<XMLTag> el;
  el = get_tag(root_file_content, "metadata");
  if(el.size() > 0)
    {
      el = get_tag(el[0].content, "dc:publisher");
      for(auto it = el.begin(); it != el.end(); it++)
	{
	  if(!it->content.empty())
	    {
	      result.electro->available = true;
	      if(result.electro->publisher.empty())
		{
		  result.electro->publisher = it->content;
		}
	      else
		{
		  result.electro->publisher = result.electro
		      ->publisher + ", " + it->content;
		}
	    }
	}
    }
}

void
EPUBParser::epub_identifier(const std::string &root_file_content,
			    BookInfoEntry &result)
{
  std::vector<XMLTag> el;
  el = get_tag(root_file_content, "metadata");
  if(el.size() > 0)
    {
      el = get_tag(el[0].content, "dc:identifier");

      for(auto it = el.begin(); it != el.end(); it++)
	{
	  if(!it->content.empty())
	    {
	      result.electro->available = true;
	      if(result.electro->id.empty())
		{
		  result.electro->id = it->content;
		}
	      else
		{
		  result.electro->id = result.electro->id + ", "
		      + it->content;
		}
	    }
	}
    }
}

void
EPUBParser::epub_source(const std::string &root_file_content,
			BookInfoEntry &result)
{
  std::vector<XMLTag> el;
  el = get_tag(root_file_content, "metadata");
  if(el.size() > 0)
    {
      el = get_tag(el[0].content, "dc:source");
      for(auto it = el.begin(); it != el.end(); it++)
	{
	  if(!it->content.empty())
	    {
	      result.electro->available = true;
	      if(result.electro->src_url.empty())
		{
		  result.electro->src_url = it->content;
		}
	      else
		{
		  result.electro->src_url = result.electro
		      ->src_url + ", " + it->content;
		}
	    }
	}
    }
}
