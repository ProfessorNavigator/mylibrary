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

#include <XMLParser.h>
#include <algorithm>

XMLParser::XMLParser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

std::vector<XMLTag>
XMLParser::get_tag(const std::string &book, const std::string &tag_id)
{
  std::vector<XMLTag> result;

  if(book.size() < tag_id.size())
    {
      return result;
    }
  bool s_quot = false;
  bool d_quot = false;
  const char *ptr = book.data();
  size_t lim = book.size();
  std::string find_str = "<" + tag_id;
  bool tag_s = false;
  XMLTag tag;
  tag.tag_id = tag_id;
  bool checked = false;
  bool found = false;
  for(size_t i = 0; i < lim; i++)
    {
      switch(ptr[i])
        {
        case 32:
          {
            if(tag_s)
              {
                if(!checked)
                  {
                    if(find_str == tag.element)
                      {
                        found = true;
                      }
                    checked = true;
                  }
                tag.element.push_back(ptr[i]);
              }
            break;
          }
        case 34:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
                if(!s_quot)
                  {
                    d_quot = !d_quot;
                  }
              }
            break;
          }
        case 39:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
                if(!d_quot)
                  {
                    s_quot = !s_quot;
                  }
              }
            break;
          }
        case 60:
          {
            if(!s_quot && !d_quot)
              {
                tag_s = true;
                checked = false;
              }
            if(tag_s)
              {
                tag.content_start = i;
                tag.element.push_back(ptr[i]);
              }
            break;
          }
        case 62:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
              }
            if(!s_quot && !d_quot)
              {
                tag.content_start += tag.element.size();
                if(!found)
                  {
                    if(find_str + ">" == tag.element)
                      {
                        found = true;
                      }
                  }
                tag_s = false;
                if(found)
                  {
                    if(*(tag.element.end() - 2) != '/')
                      {
                        std::string f_str = "</" + tag_id + ">";
                        std::string::size_type n = book.find(f_str, i);
                        if(n != std::string::npos)
                          {
                            tag.content_end = n;
                            tag.tag_list
                                = listAllTags(book, tag.content_start, n);
                            i = n + f_str.size() - 1;
                          }
                        else
                          {
                            f_str = "<" + tag_id + "/>";
                            n = book.find(f_str, i);
                            if(n != std::string::npos)
                              {
                                tag.content_end = n;
                                tag.tag_list
                                    = listAllTags(book, tag.content_start, n);
                                i = n + f_str.size() - 1;
                              }
                          }
                      }
                    af->html_to_utf8(tag.element);
                    result.push_back(tag);
                  }
                tag.element.clear();
                tag.tag_list.clear();
                tag.content_start = std::string::npos;
                tag.content_end = std::string::npos;
                found = false;
                checked = false;
              }
            break;
          }
        default:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
              }

            break;
          }
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

  std::string::size_type n = book.find("<?xml");
  if(n != std::string::npos)
    {
      std::string::size_type n2 = book.find("?>", n);
      if(n2 != std::string::npos)
        {
          result = get_element_attribute(
              std::string(book.begin() + n, book.begin() + n2), "encoding");
        }
    }
  if(result.empty())
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
  if(element.size() < 2)
    {
      return result;
    }
  std::string::size_type n = element.find(" ");
  if(n != std::string::npos)
    {
      bool s_quot = false;
      bool d_quot = false;
      bool found = false;
      std::string attr;
      bool g_break = false;
      for(auto it = element.begin() + n; it != element.end() - 1; it++)
        {
          char l = *it;
          switch(l)
            {
            case 0 ... 8:
            case 10 ... 31:
              {
                break;
              }
            case 9:
            case 32:
              {
                if(s_quot || d_quot)
                  {
                    if(found)
                      {
                        result.push_back(l);
                      }
                  }
                break;
              }
            case 34:
              {
                if(!s_quot)
                  {
                    d_quot = !d_quot;
                    if(found && !d_quot)
                      {
                        g_break = true;
                      }
                  }
                else
                  {
                    if(found)
                      {
                        result.push_back(l);
                      }
                  }
                break;
              }
            case 39:
              {
                if(!d_quot)
                  {
                    s_quot = !s_quot;
                    if(found && !s_quot)
                      {
                        g_break = true;
                      }
                  }
                else
                  {
                    if(found)
                      {
                        result.push_back(l);
                      }
                  }
                break;
              }
            case 61:
              {
                if(found)
                  {
                    if(s_quot || d_quot)
                      {
                        result.push_back(l);
                      }
                  }
                else
                  {
                    if(!s_quot && !d_quot)
                      {
                        if(attr == attr_name)
                          {
                            found = true;
                          }
                        attr.clear();
                      }
                  }
                break;
              }
            default:
              {
                if(found)
                  {
                    if(s_quot || d_quot)
                      {
                        result.push_back(l);
                      }
                  }
                else
                  {
                    if(!d_quot && !s_quot)
                      {
                        attr.push_back(l);
                      }
                  }
                break;
              }
            }
          if(g_break)
            {
              break;
            }
        }
    }
  std::string find_str;
  char replace;
  for(int i = 1; i <= 5; i++)
    {
      switch(i)
        {
        case 1:
          {
            find_str = "&lt;";
            replace = '<';
            break;
          }
        case 2:
          {
            find_str = "&gt;";
            replace = '>';
            break;
          }
        case 3:
          {
            find_str = "&amp;";
            replace = '&';
            break;
          }
        case 4:
          {
            find_str = "&apos;";
            replace = '\'';
            break;
          }
        case 5:
          {
            find_str = "&quot;";
            replace = '\"';
            break;
          }
        }
      n = 0;
      while(n != std::string::npos)
        {
          n = result.find(find_str, n);
          if(n != std::string::npos)
            {
              result.erase(n, find_str.size());
              result.insert(result.begin() + n, replace);
            }
        }
    }

  return result;
}

std::vector<XMLTag>
XMLParser::listAllTags(const std::string &book,
                       const std::string::size_type &offset,
                       const std::string::size_type &lim)
{
  std::vector<XMLTag> result;

  bool s_quot = false;
  bool d_quot = false;
  const char *ptr = book.data();
  size_t llim = lim;
  if(llim == 0)
    {
      llim = book.size();
    }
  bool tag_s = false;
  XMLTag tag;
  bool checked = false;

  for(size_t i = offset; i < llim; i++)
    {
      switch(ptr[i])
        {
        case 32:
          {
            if(tag_s)
              {
                if(!checked)
                  {
                    tag.tag_id = std::string(tag.element.begin() + 1,
                                             tag.element.end());
                    checked = true;
                  }
                tag.element.push_back(ptr[i]);
              }
            break;
          }
        case 34:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
                if(!s_quot)
                  {
                    d_quot = !d_quot;
                  }
              }
            break;
          }
        case 39:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
                if(!d_quot)
                  {
                    s_quot = !s_quot;
                  }
              }
            break;
          }
        case 60:
          {
            if(!s_quot && !d_quot)
              {
                tag_s = true;
                checked = false;
              }
            if(tag_s)
              {
                tag.content_start = i;
                tag.element.push_back(ptr[i]);
              }
            break;
          }
        case 62:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
              }
            if(!s_quot && !d_quot)
              {
                tag.content_start += tag.element.size();
                if(tag.tag_id.empty())
                  {
                    std::for_each(tag.element.begin(), tag.element.end(),
                                  [&tag](char &el) {
                                    switch(el)
                                      {
                                      case 0 ... 32:
                                      case 47:
                                      case 60:
                                      case 62:
                                        {
                                          break;
                                        }
                                      default:
                                        {
                                          tag.tag_id.push_back(el);
                                          break;
                                        }
                                      }
                                  });
                  }
                tag_s = false;
                if(*(tag.element.end() - 2) != '/')
                  {
                    std::string f_str = "</" + tag.tag_id + ">";
                    std::string::size_type n = book.find(f_str, i);
                    if(n != std::string::npos)
                      {
                        tag.content_end = n;
                        tag.tag_list = listAllTags(book, tag.content_start, n);
                        i = n + f_str.size() - 1;
                      }
                    else
                      {
                        f_str = "<" + tag.tag_id + "/>";
                        n = book.find(f_str, i);
                        if(n != std::string::npos)
                          {
                            tag.content_end = n;
                            tag.tag_list
                                = listAllTags(book, tag.content_start, n);
                            i = n + f_str.size() - 1;
                          }
                      }
                  }
                af->html_to_utf8(tag.element);
                if(tag.element.size() > 2)
                  {
                    if(std::string(tag.element.begin(),
                                   tag.element.begin() + 2)
                       != "</")
                      {
                        result.push_back(tag);
                      }
                  }
                tag.element.clear();
                tag.tag_id.clear();
                tag.tag_list.clear();
                tag.content_start = std::string::npos;
                tag.content_end = std::string::npos;
                checked = false;
              }
            break;
          }
        default:
          {
            if(tag_s)
              {
                tag.element.push_back(ptr[i]);
              }

            break;
          }
        }
    }

  return result;
}

void
XMLParser::searchTag(const std::vector<XMLTag> &list,
                     const std::string &tag_id, std::vector<XMLTag> &result)
{
  for(auto it = list.begin(); it != list.end(); it++)
    {
      if(it->tag_id == tag_id)
        {
          result.push_back(*it);
        }
      else
        {
          searchTag(it->tag_list, tag_id, result);
        }
    }
}

void
XMLParser::htmlSybolsReplacement(std::string &book)
{
  for(int i = 1; i <= 5; i++)
    {
      std::string find_str;
      char replacement;
      switch(i)
        {
        case 1:
          {
            find_str = "&lt;";
            replacement = '<';
            break;
          }
        case 2:
          {
            find_str = "&gt;";
            replacement = '>';
            break;
          }
        case 3:
          {
            find_str = "&amp;";
            replacement = '&';
            break;
          }
        case 4:
          {
            find_str = "&apos;";
            replacement = '\'';
            break;
          }
        case 5:
          {
            find_str = "&quot;";
            replacement = '"';
            break;
          }
        default:
          {
            break;
          }
        }

      std::string::size_type n = 0;
      for(;;)
        {
          n = book.find(find_str);
          if(n != std::string::npos)
            {
              book.erase(n, find_str.size());
              book.insert(book.begin() + n, replacement);
            }
          else
            {
              break;
            }
        }
    }
  af->html_to_utf8(book);
}
