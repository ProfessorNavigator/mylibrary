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

#include <EPUBParser.h>
#include <XMLTag.h>
#include <algorithm>

EPUBParser::EPUBParser(const std::shared_ptr<AuxFunc> &af)
    : XMLParser(af), LibArchive(af)
{
  this->af = af;
  dc = new DCParser(af);
}

EPUBParser::~EPUBParser()
{
  delete dc;
}

BookParseEntry
EPUBParser::epub_parser(const std::filesystem::path &filepath)
{
  BookParseEntry be;
  std::vector<ArchEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epub_get_root_file_address(filepath, filenames);
  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(it->filename == rootfile)
            {
              root_file_content = unpackByPositionStr(filepath, *it);
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
EPUBParser::epub_get_root_file_address(const std::filesystem::path &filepath,
                                       const std::vector<ArchEntry> &filenames)
{
  std::string buf;
  for(auto it = filenames.begin(); it != filenames.end(); it++)
    {
      if(it->filename == "META-INF/container.xml")
        {
          buf = unpackByPositionStr(filepath, *it);
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
      std::vector<XMLTag> tgv = get_tag(buf, "rootfiles");
      std::vector<XMLTag> res;
      searchTag(tgv, "rootfile", res);
      if(res.size() > 0)
        {
          content = get_element_attribute(res[0].element, "full-path");
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

  std::vector<XMLTag> tgv = listAllTags(buf);

  result.book_name = dc->dcTitle(buf, tgv);

  result.book_author = dc->dcAuthor(buf, tgv);

  result.book_genre = dc->dcGenre(buf, tgv);

  result.book_date = dc->dcDate(buf, tgv);

  return result;
}

std::shared_ptr<BookInfoEntry>
EPUBParser::epub_book_info(const std::filesystem::path &filepath)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();
  std::vector<ArchEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epub_get_root_file_address(filepath, filenames);
  std::string root_file_content;
  if(!rootfile.empty())
    {
      for(auto it = filenames.begin(); it != filenames.end(); it++)
        {
          if(it->filename == rootfile)
            {
              root_file_content = unpackByPositionStr(filepath, *it);
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
              result->cover = unpackByPositionStr(filepath, *it);
              result->cover_type = BookInfoEntry::cover_types::file;
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

  std::vector<XMLTag> tgv = listAllTags(root_file_content);

  result = dc->dcDescription(root_file_content, tgv);

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

  std::vector<XMLTag> tgv = listAllTags(buf);

  std::vector<XMLTag> res;
  searchTag(tgv, "meta", res);

  auto it_res = std::find_if(res.begin(), res.end(), [this](XMLTag &el) {
    return get_element_attribute(el.element, "name") == "cover";
  });
  if(it_res != res.end())
    {
      std::string cover_tag_id
          = get_element_attribute(it_res->element, "content");
      if(!cover_tag_id.empty())
        {
          res.clear();
          searchTag(tgv, "manifest", res);
          for(auto it = res.begin(); it != res.end(); it++)
            {
              std::vector<XMLTag> item_v;
              searchTag(it->tag_list, "item", item_v);
              it_res = std::find_if(item_v.begin(), item_v.end(),
                                    [this, cover_tag_id](XMLTag &el) {
                                      return get_element_attribute(el.element,
                                                                   "id")
                                             == cover_tag_id;
                                    });
              if(it_res != item_v.end())
                {
                  result = get_element_attribute(it_res->element, "href");
                  break;
                }
            }
        }
    }

  if(result.empty())
    {
      res.clear();
      searchTag(tgv, "manifest", res);

      for(auto it = res.begin(); it != res.end(); it++)
        {
          std::vector<XMLTag> item_v;
          searchTag(it->tag_list, "item", item_v);
          it_res
              = std::find_if(item_v.begin(), item_v.end(), [this](XMLTag &el) {
                  return get_element_attribute(el.element, "properties")
                         == "cover-image";
                });
          if(it_res != item_v.end())
            {
              result = get_element_attribute(it_res->element, "href");
              break;
            }
        }
      if(result.empty())
        {
          for(auto it = res.begin(); it != res.end(); it++)
            {
              std::vector<XMLTag> item_v;
              searchTag(it->tag_list, "item", item_v);
              std::string::size_type n;
              for(auto it_i = item_v.begin(); it_i != item_v.end(); it_i++)
                {
                  result = get_element_attribute(it_i->element, "media-type");
                  n = result.find("image");
                  if(n != std::string::npos)
                    {
                      result = get_element_attribute(it_i->element, "href");
                      break;
                    }
                  else
                    {
                      result.clear();
                    }
                }
              if(!result.empty())
                {
                  break;
                }
            }
        }
    }

  return result;
}

void
EPUBParser::epub_language(const std::string &root_file_content,
                          BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");
  result.language = dc->dcLanguage(root_file_content, tgv);
}

void
EPUBParser::epub_translator(const std::string &root_file_content,
                            BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = listAllTags(root_file_content);

  std::vector<XMLTag> meta;
  searchTag(tgv, "meta", meta);

  for(int i = 1; i <= 2; i++)
    {
      std::string tag_id;
      switch(i)
        {
        case 1:
          {
            tag_id = "dc:contributor";
            break;
          }
        case 2:
          {
            tag_id = "dc:creator";
            break;
          }
        default:
          break;
        }

      std::vector<XMLTag> res;
      searchTag(tgv, tag_id, res);
      for(auto it = res.begin(); it != res.end(); it++)
        {
          std::string attr = ":role";
          std::string::size_type n = it->element.find(attr);
          if(n != std::string::npos)
            {
              std::string::size_type n2 = it->element.rfind(" ", n);
              if(n2 != std::string::npos)
                {
                  attr = std::string(it->element.begin() + n2 + 1,
                                     it->element.begin() + n + attr.size());
                }
              else
                {
                  attr.clear();
                }
            }
          else
            {
              attr.clear();
            }
          std::string role;
          if(!attr.empty())
            {
              role = get_element_attribute(it->element, attr);
            }
          if(role == "trl")
            {
              if(it->hasContent())
                {
                  if(!result.translator.empty())
                    {
                      result.translator += ", ";
                    }
                  std::copy(root_file_content.begin() + it->content_start,
                            root_file_content.begin() + it->content_end,
                            std::back_inserter(result.translator));
                }
            }
          else
            {
              role = get_element_attribute(it->element, "id");
              if(!role.empty())
                {
                  role = "#" + role;
                  auto it_m = std::find_if(
                      meta.begin(), meta.end(), [role, this](XMLTag &el) {
                        if(role
                           == get_element_attribute(el.element, "refines"))
                          {
                            if(get_element_attribute(el.element, "property")
                               == "role")
                              {
                                return true;
                              }
                            else
                              {
                                return false;
                              }
                          }
                        else
                          {
                            return false;
                          }
                      });
                  if(it_m != meta.end())
                    {
                      if(it_m->hasContent())
                        {
                          if(std::string(root_file_content.begin()
                                             + it_m->content_start,
                                         root_file_content.begin()
                                             + it_m->content_end)
                             == "trl")
                            {
                              if(it->hasContent())
                                {
                                  if(!result.translator.empty())
                                    {
                                      result.translator += ", ";
                                    }
                                  std::copy(
                                      root_file_content.begin()
                                          + it->content_start,
                                      root_file_content.begin()
                                          + it->content_end,
                                      std::back_inserter(result.translator));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
  htmlSymbolsReplacement(result.translator);
}

void
EPUBParser::epub_publisher(const std::string &root_file_content,
                           BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");

  result.electro->publisher = dc->dcPublisher(root_file_content, tgv);
  if(!result.electro->publisher.empty())
    {
      result.electro->available = true;
    }
}

void
EPUBParser::epub_identifier(const std::string &root_file_content,
                            BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");
  result.electro->id = dc->dcIdentifier(root_file_content, tgv);
  if(!result.electro->id.empty())
    {
      result.electro->available = true;
    }
}

void
EPUBParser::epub_source(const std::string &root_file_content,
                        BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");

  result.electro->src_url = dc->dcSource(root_file_content, tgv);
  if(!result.electro->src_url.empty())
    {
      result.electro->available = true;
    }
}
