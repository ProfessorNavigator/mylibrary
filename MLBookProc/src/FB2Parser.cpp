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

#include <FB2Parser.h>
#include <MLException.h>
#include <algorithm>

FB2Parser::FB2Parser(const std::shared_ptr<AuxFunc> &af) : XMLParser(af)
{
  this->af = af;
}

BookParseEntry
FB2Parser::fb2_parser(const std::string &book)
{
  if(book.empty())
    {
      throw MLException("FB2Parser::fb2_parser: book is empty");
    }

  BookParseEntry result;

  std::string code_page = get_book_encoding(book);
  std::string loc_book;
  if(code_page.empty())
    {
      code_page = af->detect_encoding(book);
    }
  loc_book = af->to_utf_8(book, code_page.c_str());

  result = fb2_description(loc_book);

  if(result.book_name.empty())
    {
      loc_book = book;
      loc_book.erase(
          std::remove_if(loc_book.begin(), loc_book.end(), [](char &el) {
            switch(el)
              {
              case 0 ... 8:
              case 10 ... 31:
                return true;
              default:
                return false;
              }
          }));
      code_page = af->detect_encoding(loc_book);
      loc_book = af->to_utf_8(book, code_page.c_str());
      result = fb2_description(loc_book);
    }

  return result;
}

std::string
FB2Parser::fb2_author(const std::string &book,
                      const std::vector<XMLTag> &author)
{
  std::string elstr;
  for(auto it = author.begin(); it != author.end(); it++)
    {
      std::string auth;
      auto it_t = std::find_if(it->tag_list.begin(), it->tag_list.end(),
                               [](const XMLTag &el) {
                                 return el.tag_id == "last-name";
                               });
      if(it_t != it->tag_list.end())
        {
          if(it_t->hasContent())
            {
              std::copy(book.begin() + it_t->content_start,
                        book.begin() + it_t->content_end,
                        std::back_inserter(auth));
            }
        }
      it_t = std::find_if(it->tag_list.begin(), it->tag_list.end(),
                          [](const XMLTag &el) {
                            return el.tag_id == "first-name";
                          });
      if(it_t != it->tag_list.end())
        {
          if(it_t->hasContent())
            {
              if(!auth.empty())
                {
                  auth += " ";
                }
              std::copy(book.begin() + it_t->content_start,
                        book.begin() + it_t->content_end,
                        std::back_inserter(auth));
            }
        }

      it_t = std::find_if(it->tag_list.begin(), it->tag_list.end(),
                          [](const XMLTag &el) {
                            return el.tag_id == "middle-name";
                          });
      if(it_t != it->tag_list.end())
        {
          if(it_t->hasContent())
            {
              if(!auth.empty())
                {
                  auth += " ";
                }
              std::copy(book.begin() + it_t->content_start,
                        book.begin() + it_t->content_end,
                        std::back_inserter(auth));
            }
        }

      it_t = std::find_if(it->tag_list.begin(), it->tag_list.end(),
                          [](const XMLTag &el) {
                            return el.tag_id == "nickname";
                          });
      if(it_t != it->tag_list.end())
        {
          if(it_t->hasContent())
            {
              if(!auth.empty())
                {
                  auth += " aka ";
                }
              std::copy(book.begin() + it_t->content_start,
                        book.begin() + it_t->content_end,
                        std::back_inserter(auth));
            }
        }

      if(elstr.empty())
        {
          elstr = auth;
        }
      else
        {
          elstr += ", " + auth;
        }
    }

  htmlSymbolsReplacement(elstr);

  std::string::size_type n = 0;
  std::string find_str = "  ";
  for(;;)
    {
      n = elstr.find(find_str, n);
      if(n != std::string::npos)
        {
          elstr.erase(elstr.begin() + n);
        }
      else
        {
          break;
        }
    }
  bool stop = false;

  while(elstr.size() > 0)
    {
      switch(*elstr.rbegin())
        {
        case 0 ... 32:
          {
            elstr.pop_back();
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
  while(elstr.size() > 0)
    {
      switch(*elstr.begin())
        {
        case 0 ... 32:
          {
            elstr.erase(elstr.begin());
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

  return elstr;
}

std::string
FB2Parser::fb2_series(const std::vector<XMLTag> &sequence)
{
  std::string result;

  for(auto it = sequence.begin(); it != sequence.end(); it++)
    {
      std::string seq = it->element;
      std::string name = get_element_attribute(seq, "name");
      std::string number = get_element_attribute(seq, "number");
      if(!number.empty())
        {
          name = name + " " + number;
        }
      if(result.size() > 0)
        {
          result = result + ", " + name;
        }
      else
        {
          result = name;
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

std::string
FB2Parser::fb2_genres(const std::string &book,
                      const std::vector<XMLTag> &genres)
{
  std::string result;

  for(auto it = genres.begin(); it != genres.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }

  htmlSymbolsReplacement(result);

  return result;
}

BookParseEntry
FB2Parser::fb2_description(const std::string &book)
{
  BookParseEntry result;
  std::vector<XMLTag> tgv = get_tag(book, "title-info");

  std::vector<XMLTag> res;
  searchTag(tgv, "author", res);
  result.book_author = fb2_author(book, res);

  res.clear();
  searchTag(tgv, "book-title", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.book_name.empty())
            {
              result.book_name += ". ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.book_name));
        }
    }
  htmlSymbolsReplacement(result.book_name);

  res.clear();
  searchTag(tgv, "sequence", res);
  result.book_series = fb2_series(res);

  res.clear();
  searchTag(tgv, "genre", res);
  result.book_genre = fb2_genres(book, res);

  res.clear();
  searchTag(tgv, "date", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.book_date.empty())
            {
              result.book_date += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.book_date));
        }
    }
  htmlSymbolsReplacement(result.book_date);

  return result;
}

std::shared_ptr<BookInfoEntry>
FB2Parser::fb2_book_info(const std::string &book)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();

  std::string code_page = get_book_encoding(book);
  std::string loc_book;
  if(code_page.empty())
    {
      code_page = af->detect_encoding(book);
    }
  loc_book = af->to_utf_8(book, code_page.c_str());

  fb2_annotation_decode(loc_book, result->annotation);
  result->cover_type = fb2_cover(loc_book, result->cover);
  fb2_extra_info(loc_book, *result);

  return result;
}

void
FB2Parser::fb2_annotation_decode(const std::string &book, std::string &result)
{
  std::vector<XMLTag> tgv = get_tag(book, "annotation");
  for(auto it = tgv.begin(); it != tgv.end(); it++)
    {
      if(it->hasContent())
        {
          if(!result.empty())
            {
              result += "\n\n";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result));
        }
    }
  htmlSymbolsReplacement(result);
}

BookInfoEntry::cover_types
FB2Parser::fb2_cover(const std::string &book, std::string &cover)
{
  BookInfoEntry::cover_types result = BookInfoEntry::cover_types::error;
  std::vector<XMLTag> tgv = get_tag(book, "title-info");
  if(tgv.size() > 0)
    {
      auto it = std::find_if(tgv.begin()->tag_list.begin(),
                             tgv.begin()->tag_list.end(), [](XMLTag &el) {
                               return el.tag_id == "coverpage";
                             });
      if(it != tgv.begin()->tag_list.end())
        {
          if(it->tag_list.size() > 0)
            {
              std::string attr
                  = get_element_attribute(it->tag_list[0].element, "l:href");
              if(attr.empty())
                {
                  attr
                      = get_element_attribute(it->tag_list[0].element, "href");
                }
              attr.erase(std::remove_if(attr.begin(), attr.end(),
                                        [](char &el) {
                                          return el == '#';
                                        }),
                         attr.end());
              tgv = get_tag(book, "binary");
              it = std::find_if(
                  tgv.begin(), tgv.end(), [attr, this](XMLTag &el) {
                    return attr == get_element_attribute(el.element, "id");
                  });
              if(it != tgv.end())
                {
                  if(it->hasContent())
                    {
                      std::copy(book.begin() + it->content_start,
                                book.begin() + it->content_end,
                                std::back_inserter(cover));
                      result = BookInfoEntry::cover_types::base64;
                    }
                }
            }
        }
    }
  if(cover.empty())
    {
      tgv = get_tag(book, "src-title-info");
      if(tgv.size() > 0)
        {
          auto it = std::find_if(tgv.begin()->tag_list.begin(),
                                 tgv.begin()->tag_list.end(), [](XMLTag &el) {
                                   return el.tag_id == "coverpage";
                                 });
          if(it != tgv.begin()->tag_list.end())
            {
              if(it->tag_list.size() > 0)
                {
                  std::string attr = get_element_attribute(
                      it->tag_list[0].element, "l:href");
                  if(attr.empty())
                    {
                      attr = get_element_attribute(it->tag_list[0].element,
                                                   "href");
                    }
                  attr.erase(std::remove_if(attr.begin(), attr.end(),
                                            [](char &el) {
                                              return el == '#';
                                            }),
                             attr.end());
                  tgv = get_tag(book, "binary");
                  it = std::find_if(
                      tgv.begin(), tgv.end(), [attr, this](XMLTag &el) {
                        return attr == get_element_attribute(el.element, "id");
                      });
                  if(it != tgv.end())
                    {
                      if(it->hasContent())
                        {
                          std::copy(book.begin() + it->content_start,
                                    book.begin() + it->content_end,
                                    std::back_inserter(cover));
                          result = BookInfoEntry::cover_types::base64;
                        }
                    }
                }
            }
        }
    }
  if(cover.empty())
    {
      tgv = get_tag(book, "body");
      if(tgv.size() > 0)
        {
          XMLTag tag = *tgv.begin();
          if(tag.hasContent())
            {
              cover = std::string(book.begin() + tag.content_start,
                                  book.begin() + tag.content_end);
              result = BookInfoEntry::cover_types::text;
              htmlSymbolsReplacement(cover);
            }
        }
    }
  return result;
}

void
FB2Parser::fb2_extra_info(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> tgv;
  tgv = get_tag(book, "title-info");
  if(tgv.size() > 0)
    {
      std::vector<XMLTag> res;
      searchTag(tgv, "lang", res);
      for(auto it = res.begin(); it != res.end(); it++)
        {
          if(it->hasContent())
            {
              if(!result.language.empty())
                {
                  result.language += ", ";
                }
              std::copy(book.begin() + it->content_start,
                        book.begin() + it->content_end,
                        std::back_inserter(result.language));
            }
        }
      htmlSymbolsReplacement(result.language);

      res.clear();
      searchTag(tgv, "src-lang", res);
      for(auto it = res.begin(); it != res.end(); it++)
        {
          if(it->hasContent())
            {
              if(!result.src_language.empty())
                {
                  result.src_language += ", ";
                }
              std::copy(book.begin() + it->content_start,
                        book.begin() + it->content_end,
                        std::back_inserter(result.src_language));
            }
        }
      htmlSymbolsReplacement(result.src_language);

      res.clear();
      searchTag(tgv, "translator", res);
      result.translator = fb2_author(book, res);

      fb2_publisher_info(book, result);
      fb2_electro_doc_info(book, result);
    }
}

void
FB2Parser::fb2_publisher_info(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(book, "publish-info");

  std::vector<XMLTag> res;
  searchTag(tgv, "book-name", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.paper->available = true;
          if(!result.paper->book_name.empty())
            {
              result.paper->book_name += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.paper->book_name));
        }
    }
  htmlSymbolsReplacement(result.paper->book_name);

  res.clear();
  searchTag(tgv, "publisher", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.paper->available = true;
          if(!result.paper->publisher.empty())
            {
              result.paper->publisher += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.paper->publisher));
        }
    }
  htmlSymbolsReplacement(result.paper->publisher);

  res.clear();
  searchTag(tgv, "city", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.paper->available = true;
          if(!result.paper->city.empty())
            {
              result.paper->city += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.paper->city));
        }
    }
  htmlSymbolsReplacement(result.paper->city);

  res.clear();
  searchTag(tgv, "year", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.paper->available = true;
          if(!result.paper->year.empty())
            {
              result.paper->year += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.paper->year));
        }
    }
  htmlSymbolsReplacement(result.paper->year);

  res.clear();
  searchTag(tgv, "isbn", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.paper->available = true;
          if(!result.paper->isbn.empty())
            {
              result.paper->isbn += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.paper->isbn));
        }
    }
  htmlSymbolsReplacement(result.paper->isbn);
}

void
FB2Parser::fb2_electro_doc_info(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> tgv = get_tag(book, "document-info");

  std::vector<XMLTag> res;
  searchTag(tgv, "author", res);
  if(res.size() > 0)
    {
      result.electro->available = true;
      result.electro->author = fb2_author(book, res);
    }

  res.clear();
  searchTag(tgv, "program-used", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->program_used.empty())
            {
              result.electro->program_used += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->program_used));
        }
    }
  htmlSymbolsReplacement(result.electro->program_used);

  res.clear();
  searchTag(tgv, "date", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->date.empty())
            {
              result.electro->date += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->date));
        }
    }
  htmlSymbolsReplacement(result.electro->date);

  res.clear();
  searchTag(tgv, "src-url", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->src_url.empty())
            {
              result.electro->src_url += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->src_url));
        }
    }
  htmlSymbolsReplacement(result.electro->src_url);

  res.clear();
  searchTag(tgv, "src-ocr", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->src_ocr.empty())
            {
              result.electro->src_ocr += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->src_ocr));
        }
    }
  htmlSymbolsReplacement(result.electro->src_ocr);

  res.clear();
  searchTag(tgv, "id", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->id.empty())
            {
              result.electro->id += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->id));
        }
    }
  htmlSymbolsReplacement(result.electro->id);

  res.clear();
  searchTag(tgv, "version", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->version.empty())
            {
              result.electro->version += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->version));
        }
    }
  htmlSymbolsReplacement(result.electro->version);

  res.clear();
  searchTag(tgv, "history", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->hasContent())
        {
          result.electro->available = true;
          if(!result.electro->history.empty())
            {
              result.electro->history += ", ";
            }
          std::copy(book.begin() + it->content_start,
                    book.begin() + it->content_end,
                    std::back_inserter(result.electro->history));
        }
    }
  htmlSymbolsReplacement(result.electro->history);

  res.clear();
  searchTag(tgv, "publisher", res);
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->tag_list.size() > 0)
        {
          fb2_elctor_publisher(book, it->tag_list, result);
        }
      else
        {
          if(it->hasContent())
            {
              result.electro->available = true;
              if(!result.electro->publisher.empty())
                {
                  result.electro->publisher += ", ";
                }
              std::copy(book.begin() + it->content_start,
                        book.begin() + it->content_end,
                        std::back_inserter(result.electro->publisher));
            }
        }
    }
  htmlSymbolsReplacement(result.electro->publisher);
}

void
FB2Parser::fb2_elctor_publisher(const std::string &book,
                                const std::vector<XMLTag> &tgv,
                                BookInfoEntry &result)
{
  recursiveGetTags(book, tgv, result.electro->publisher);
  htmlSymbolsReplacement(result.electro->publisher);
}

void
FB2Parser::recursiveGetTags(const std::string &book,
                            const std::vector<XMLTag> &tgv,
                            std::string &result)
{
  for(auto it = tgv.begin(); it != tgv.end(); it++)
    {
      if(it->tag_list.size() > 0)
        {
          recursiveGetTags(book, it->tag_list, result);
        }
      else
        {
          if(it->hasContent())
            {
              if(!result.empty())
                {
                  result += " ";
                }
              std::copy(book.begin() + it->content_start,
                        book.begin() + it->content_end,
                        std::back_inserter(result));
            }
        }
    }
}
