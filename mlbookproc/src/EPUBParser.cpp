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

#include <EPUBParser.h>
#include <XMLTag.h>
#include <algorithm>

EPUBParser::EPUBParser(const std::shared_ptr<AuxFunc> &af) : XMLParser(af)
{
  this->af = af;
}

BookParseEntry
EPUBParser::epub_parser(const std::filesystem::path &filepath)
{
  BookParseEntry be;
  std::vector<ZipFileEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epub_get_root_file_address(filepath, filenames);
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

  result.book_name = epubTitle(buf, tgv);

  result.book_author = epubAuthor(buf, tgv);

  result.book_genre = epubGenre(buf, tgv);

  result.book_date = epubDate(buf, tgv);

  return result;
}

std::string
EPUBParser::epubTitle(const std::string &root_file_content,
                      const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:title", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ". ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }
  htmlSybolsReplacement(result);

  return result;
}

std::string
EPUBParser::epubAuthor(const std::string &root_file_content,
                       const std::vector<XMLTag> &tgv)
{
  std::string result;
  std::vector<XMLTag> res;
  searchTag(tgv, "dc:creator", res);
  std::vector<XMLTag> meta;
  searchTag(tgv, "meta", meta);
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
      if(role == "aut")
        {
          if(it->hasContent())
            {
              if(!result.empty())
                {
                  result += ", ";
                }
              std::copy(root_file_content.begin() + it->content_start,
                        root_file_content.begin() + it->content_end,
                        std::back_inserter(result));
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
                    if(role == get_element_attribute(el.element, "refines"))
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
                      if(std::string(
                             root_file_content.begin() + it_m->content_start,
                             root_file_content.begin() + it_m->content_end)
                         == "aut")
                        {
                          if(it->hasContent())
                            {
                              if(!result.empty())
                                {
                                  result += ", ";
                                }
                              std::copy(root_file_content.begin()
                                            + it->content_start,
                                        root_file_content.begin()
                                            + it->content_end,
                                        std::back_inserter(result));
                            }
                        }
                    }
                }
            }
          else
            {
              if(it->element.size() == it->tag_id.size() + 2)
                {
                  if(it->hasContent())
                    {
                      if(!result.empty())
                        {
                          result += ", ";
                        }
                      std::copy(root_file_content.begin() + it->content_start,
                                root_file_content.begin() + it->content_end,
                                std::back_inserter(result));
                    }
                }
            }
        }
    }
  htmlSybolsReplacement(result);

  std::string::size_type n = 0;
  std::string find_str = "  ";
  for(;;)
    {
      n = result.find(find_str, n);
      if(n != std::string::npos)
        {
          result.erase(result.begin() + n);
        }
      else
        {
          break;
        }
    }

  bool stop = false;

  while(result.size() > 0)
    {
      switch(*result.rbegin())
        {
        case 0 ... 32:
          {
            result.pop_back();
            break;
          }
        default:
          {
            stop = true;
            break;
          }
        }
      if(stop)
        {
          break;
        }
    }

  stop = false;
  while(result.size() > 0)
    {
      switch(*result.begin())
        {
        case 0 ... 32:
          {
            result.erase(result.begin());
            break;
          }
        default:
          {
            stop = true;
            break;
          }
        }
      if(stop)
        {
          break;
        }
    }

  return result;
}

std::string
EPUBParser::epubGenre(const std::string &root_file_content,
                      const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:subject", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSybolsReplacement(result);

  return result;
}

std::string
EPUBParser::epubDate(const std::string &root_file_content,
                     const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:date", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSybolsReplacement(result);

  return result;
}

std::shared_ptr<BookInfoEntry>
EPUBParser::epub_book_info(const std::filesystem::path &filepath)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();
  std::vector<ZipFileEntry> filenames;
  fileNames(filepath, filenames);
  std::string rootfile = epub_get_root_file_address(filepath, filenames);
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
  std::vector<XMLTag> tgv = get_tag(buf, "dc:description");

  for(auto it = tgv.begin(); it != tgv.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += " \n\n";
            }
          std::copy(buf.begin() + it->content_start,
                    buf.begin() + it->content_end, std::back_inserter(result));
        }
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
  std::vector<XMLTag> res;
  searchTag(tgv, "dc:language", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.language.empty())
            {
              result.language += ", ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result.language));
        }
    }
  htmlSybolsReplacement(result.language);
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
  htmlSybolsReplacement(result.translator);
}

void
EPUBParser::epub_publisher(const std::string &root_file_content,
                           BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:publisher", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->publisher.empty())
            {
              result.electro->publisher += ", ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result.electro->publisher));
        }
    }

  htmlSybolsReplacement(result.electro->publisher);
}

void
EPUBParser::epub_identifier(const std::string &root_file_content,
                            BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:identifier", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.electro->id.empty())
            {
              result.electro->id += ", ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result.electro->id));
        }
    }

  htmlSybolsReplacement(result.electro->id);
}

void
EPUBParser::epub_source(const std::string &root_file_content,
                        BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(root_file_content, "metadata");

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:source", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.electro->src_url.empty())
            {
              result.electro->src_url += ", ";
            }
          std::copy(root_file_content.begin() + it->content_start,
                    root_file_content.begin() + it->content_end,
                    std::back_inserter(result.electro->src_url));
        }
    }

  htmlSybolsReplacement(result.electro->src_url);
}
