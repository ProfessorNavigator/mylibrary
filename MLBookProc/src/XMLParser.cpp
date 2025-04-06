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

#include <MLException.h>
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

  std::vector<XMLTag> tgv = listAllTags(book);
  searchTag(tgv, tag_id, result);

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
      while(!result.empty())
        {
          n = result.find(find_str, n);
          if(n != std::string::npos)
            {
              result.erase(n, find_str.size());
              result.insert(result.begin() + n, replace);
            }
          else
            {
              break;
            }
        }
    }

  return result;
}

std::vector<XMLTag>
XMLParser::listAllTags(const std::string &book)
{
  std::vector<XMLTag> result;
  if(book.empty())
    {
      return result;
    }
  std::string find_str = "?>";
  std::string::size_type n = book.find(find_str);
  if(n != std::string::npos)
    {
      n += find_str.size();
    }
  else
    {
      n = 0;
    }

  find_str = "<";

  std::string::size_type n_end = book.rfind(">");

  for(;;)
    {
      n = book.find(find_str, n);
      if(n != std::string::npos)
        {
          XMLTag tag;
          std::string::size_type end;
          tag_type tg_tp;
          tag.element = tagElement(book, n, end, tg_tp);
          n = end;
          tagId(tag);
          if(tg_tp == tag_type::has_content)
            {
              if(n + 1 < book.size())
                {
                  n++;
                  tag.content_start = n;
                  tagContent(book, n, n_end, tag, end);
                  n = end;
                }
              else
                {
                  result.emplace_back(tag);
                  break;
                }
            }
          else
            {
              tag.content_start = n + 1;
            }
          result.emplace_back(tag);
        }
      else
        {
          break;
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
XMLParser::htmlSymbolsReplacement(std::string &book)
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

void
XMLParser::removeAllTags(std::string &book)
{
  std::string find_str = "<";
  std::string::size_type n = 0;
  std::string cdata_s = "<![CDATA[";
  std::string cdata_e = "]]>";
  std::string comm_s = "<!--";
  std::string comm_e = "-->";
  while(book.size() > 0)
    {
      n = book.find(find_str, n);
      if(n != std::string::npos)
        {
          if(n + cdata_s.size() < book.size())
            {
              if(book.substr(n, cdata_s.size()) == cdata_s)
                {
                  book.erase(book.begin() + n,
                             book.begin() + n + cdata_s.size());
                  std::string::size_type n2 = book.find(cdata_e, n);
                  if(n2 != std::string::npos)
                    {
                      book.erase(book.begin() + n2,
                                 book.begin() + n2 + cdata_e.size());
                      n = book.find(find_str, n);
                      if(n == std::string::npos)
                        {
                          break;
                        }
                    }
                  else
                    {
                      book.erase(book.begin() + n, book.end());
                      break;
                    }
                }
              else if(book.substr(n, comm_s.size()) == comm_s)
                {
                  std::string::size_type n2 = book.find(comm_e, n);
                  if(n2 != std::string::npos)
                    {
                      book.erase(book.begin() + n,
                                 book.begin() + n2 + comm_e.size());
                      continue;
                    }
                  else
                    {
                      book.erase(book.begin() + n, book.end());
                      break;
                    }
                }
            }
          std::string::size_type n2 = std::string::npos;
          bool d_quot = false;
          bool s_quot = false;
          bool stop = false;
          for(std::string::size_type i = n; i < book.size(); i++)
            {
              switch(book[i])
                {
                case 34:
                  {
                    if(!s_quot)
                      {
                        d_quot = !d_quot;
                      }
                    break;
                  }
                case 39:
                  {
                    if(!d_quot)
                      {
                        s_quot = !s_quot;
                      }
                    break;
                  }
                case 62:
                  {
                    if(!s_quot && !d_quot)
                      {
                        n2 = i + 1;
                        stop = true;
                      }
                    break;
                  }
                default:
                  {
                    break;
                  }
                }
              if(stop)
                {
                  break;
                }
            }
          if(n2 != std::string::npos)
            {
              book.erase(book.begin() + n, book.begin() + n2);
            }
          else
            {
              book.erase(book.begin(), book.end());
              break;
            }
        }
      else
        {
          break;
        }
    }
}

std::string
XMLParser::tagElement(const std::string &book,
                      const std::string::size_type &start,
                      std::string::size_type &end, tag_type &tg_type)
{
  tg_type = tag_type::has_content;
  {
    std::string find_str = "<![CDATA[";
    if(start + find_str.size() < book.size())
      {
        if(book.substr(start, find_str.size()) == find_str)
          {
            find_str = "]]>";
            end = book.find(find_str, start);
            if(end != std::string::npos)
              {
                end += find_str.size();
                tg_type = tag_type::spec_tag;
                return book.substr(start, end - start);
              }
            else
              {
                throw MLException(
                    "XMLParser::tagElement: cannot find CDATA end");
              }
          }
      }
  }

  bool stop = false;
  bool s_quot = false;
  bool d_quot = false;

  for(end = start; end < book.size(); end++)
    {
      char val = book[end];
      switch(val)
        {
        case 33:
          {
            if(end == start + 1)
              {
                tg_type = tag_type::spec_tag;
              }
            break;
          }
        case 34:
          {
            if(tg_type != tag_type::spec_tag)
              {
                if(!s_quot)
                  {
                    d_quot = !d_quot;
                  }
              }
            break;
          }
        case 39:
          {
            if(tg_type != tag_type::spec_tag)
              {
                if(!d_quot)
                  {
                    s_quot = !s_quot;
                  }
              }
            break;
          }
        case 47:
          {
            if(!s_quot && !d_quot)
              {
                if(end == start + 1)
                  {
                    tg_type = tag_type::end_tag;
                  }
                else
                  {
                    tg_type = tag_type::single;
                  }
              }
            break;
          }
        case 62:
          {
            if(!s_quot && !d_quot)
              {
                stop = true;
              }
            break;
          }
        default:
          {
            break;
          }
        }

      if(stop)
        {
          break;
        }
    }

  std::string result = book.substr(start, end + 1 - start);

  return result;
}

void
XMLParser::tagContent(const std::string &book,
                      const std::string::size_type &start,
                      const std::string::size_type &book_end, XMLTag &tag,
                      std::string::size_type &tag_end)
{
  std::string find_str = "<";
  std::string::size_type n = start;
  bool stop = false;

  while(n < book_end)
    {
      n = book.find(find_str, n);
      if(n != std::string::npos)
        {
          tag_type tg_tp;
          XMLTag ltg;
          ltg.element = tagElement(book, n, tag_end, tg_tp);
          n = tag_end;
          switch(tg_tp)
            {
            case tag_type::end_tag:
              {
                stop = true;
                tag.content_end = tag_end + 1 - ltg.element.size();
                break;
              }
            case tag_type::spec_tag:
            case tag_type::single:
              {
                tagId(ltg);
                ltg.content_start = tag_end + 1;
                tag.tag_list.emplace_back(ltg);
                break;
              }
            case tag_type::has_content:
              {
                tagId(ltg);
                if(n + 1 < book.size())
                  {
                    n++;
                    ltg.content_start = n;
                    tagContent(book, n, book_end, ltg, tag_end);
                    n = tag_end;
                    tag.tag_list.emplace_back(ltg);
                  }
                else
                  {
                    stop = true;
                  }
                break;
              }
            default:
              {
                stop = true;
                break;
              }
            }
        }
      else
        {
          throw MLException("XMLParser::tagContent: cannot find tag end");
        }
      if(stop)
        {
          break;
        }
    }
}

void
XMLParser::tagId(XMLTag &tag)
{
  std::string find_str = "<!--";
  if(tag.element.substr(0, find_str.size()) == find_str)
    {
      tag.tag_id = "!--";
      return void();
    }
  find_str = "<![CDATA[";
  if(tag.element.substr(0, find_str.size()) == find_str)
    {
      tag.tag_id = "CDATA";
      return void();
    }

  find_str = " ";
  std::string::size_type n = tag.element.find(find_str);
  if(n != std::string::npos)
    {
      tag.tag_id = tag.element.substr(0, n);
      tag.tag_id.erase(std::remove_if(tag.tag_id.begin(), tag.tag_id.end(),
                                      [](char &el) {
                                        switch(el)
                                          {
                                          case 0 ... 32:
                                          case 47:
                                          case 60:
                                            {
                                              return true;
                                            }
                                          default:
                                            return false;
                                          }
                                      }),
                       tag.tag_id.end());
    }
  else
    {
      tag.tag_id = tag.element;
      tag.tag_id.erase(std::remove_if(tag.tag_id.begin(), tag.tag_id.end(),
                                      [](char &el) {
                                        switch(el)
                                          {
                                          case 0 ... 32:
                                          case 47:
                                          case 60:
                                          case 62:
                                            {
                                              return true;
                                            }
                                          default:
                                            return false;
                                          }
                                      }),
                       tag.tag_id.end());
    }
}
