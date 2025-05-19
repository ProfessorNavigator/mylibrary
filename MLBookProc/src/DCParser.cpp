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
#include <DCParser.h>
#include <algorithm>

DCParser::DCParser(const std::shared_ptr<AuxFunc> &af) : XMLParser(af)
{
  this->af = af;
}

std::string
DCParser::dcTitle(const std::string &dc_file_content,
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
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }
  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcAuthor(const std::string &dc_file_content,
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
              std::copy(dc_file_content.begin() + it->content_start,
                        dc_file_content.begin() + it->content_end,
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
                             dc_file_content.begin() + it_m->content_start,
                             dc_file_content.begin() + it_m->content_end)
                         == "aut")
                        {
                          if(it->hasContent())
                            {
                              if(!result.empty())
                                {
                                  result += ", ";
                                }
                              std::copy(
                                  dc_file_content.begin() + it->content_start,
                                  dc_file_content.begin() + it->content_end,
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
                      std::copy(dc_file_content.begin() + it->content_start,
                                dc_file_content.begin() + it->content_end,
                                std::back_inserter(result));
                    }
                }
            }
        }
    }
  htmlSymbolsReplacement(result);

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
DCParser::dcGenre(const std::string &dc_file_content,
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
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcDate(const std::string &dc_file_content,
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
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcLanguage(const std::string &dc_file_content,
                     const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:language", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }
  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcPublisher(const std::string &dc_file_content,
                      const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:publisher", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {

          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcIdentifier(const std::string &dc_file_content,
                       const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:identifier", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcSource(const std::string &dc_file_content,
                   const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:source", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

std::string
DCParser::dcDescription(const std::string &dc_file_content,
                        const std::vector<XMLTag> &tgv)
{
  std::string result;

  std::vector<XMLTag> res;
  searchTag(tgv, "dc:description", res);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += " \n\n";
            }
          std::copy(dc_file_content.begin() + it->content_start,
                    dc_file_content.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}
