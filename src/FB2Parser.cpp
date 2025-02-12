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

#include <FB2Parser.h>

FB2Parser::FB2Parser(const std::shared_ptr<AuxFunc> &af) : XMLParser(af)
{
  this->af = af;
}

BookParseEntry
FB2Parser::fb2_parser(const std::string &book)
{
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
      code_page = af->detect_encoding(book);
      loc_book = af->to_utf_8(book, code_page.c_str());
      result = fb2_description(loc_book);
      if(result.book_name.empty())
        {
          result = fb2_parser_alternative(book);
        }
    }

  return result;
}

std::string
FB2Parser::fb2_author(const std::vector<XMLTag> &authors, const int &variant)
{
  std::string elstr;
  for(auto it = authors.begin(); it != authors.end(); it++)
    {
      std::string author;
      std::vector<XMLTag> el;
      switch(variant)
        {
        case 2:
          {
            el = get_tag(it->content, "last-name", true);
            break;
          }
        default:
          {
            el = get_tag(it->content, "last-name");
            break;
          }
        }
      if(el.size() > 0)
        {
          author = el[0].content;
        }

      switch(variant)
        {
        case 2:
          {
            el = get_tag(it->content, "first-name", true);
            break;
          }
        default:
          {
            el = get_tag(it->content, "first-name");
            break;
          }
        }
      if(el.size() > 0)
        {
          if(author.size() > 0)
            {
              author = author + " " + el[0].content;
            }
          else
            {
              author = el[0].content;
            }
        }

      switch(variant)
        {
        case 2:
          {
            el = get_tag(it->content, "middle-name", true);
            break;
          }
        default:
          {
            el = get_tag(it->content, "middle-name");
            break;
          }
        }
      if(el.size() > 0)
        {
          if(author.size() > 0)
            {
              author = author + " " + el[0].content;
            }
          else
            {
              author = el[0].content;
            }
        }

      switch(variant)
        {
        case 2:
          {
            el = get_tag(it->content, "nickname", true);
            break;
          }
        default:
          {
            el = get_tag(it->content, "nickname");
            break;
          }
        }
      if(el.size() > 0)
        {
          if(author.size() > 0)
            {
              author = author + " aka " + el[0].content;
            }
          else
            {
              author = el[0].content;
            }
        }
      if(!elstr.empty())
        {
          if(!author.empty())
            {
              elstr = elstr + ", " + author;
            }
        }
      else
        {
          elstr = author;
        }
    }

  std::string search_str("&quot;");
  std::string::size_type n;
  std::string insert_str("'");
  for(;;)
    {
      n = elstr.find(search_str);
      if(n != std::string::npos)
        {
          elstr.erase(n, search_str.size());
          elstr.insert(n, insert_str);
        }
      else
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

  std::string search_str("&quot;");
  std::string::size_type n;
  std::string insert_str("\"");
  for(;;)
    {
      n = result.find(search_str);
      if(n != std::string::npos)
        {
          result.erase(n, search_str.size());
          result.insert(n, insert_str);
        }
      else
        {
          break;
        }
    }

  return result;
}

std::string
FB2Parser::fb2_genres(const std::vector<XMLTag> &genres)
{
  std::string result;

  for(auto it = genres.begin(); it != genres.end(); it++)
    {
      if(it->content.size() > 0)
        {
          if(result.size() > 0)
            {
              result = result + ", " + it->content;
            }
          else
            {
              result = it->content;
            }
        }
    }

  return result;
}

BookParseEntry
FB2Parser::fb2_description(const std::string &book)
{
  BookParseEntry result;
  std::vector<XMLTag> description = get_tag(book, "description");
  if(description.size() > 0)
    {
      std::vector<XMLTag> title_info
          = get_tag(description[0].content, "title-info");
      if(title_info.size() > 0)
        {
          std::vector<XMLTag> el = get_tag(title_info[0].content, "author");

          result.book_author = fb2_author(el, 1);

          el = get_tag(title_info[0].content, "book-title");
          if(el.size() > 0)
            {
              result.book_name = el[0].content;

              std::string search_str("&quot;");
              std::string::size_type n;
              std::string insert_str("\"");
              for(;;)
                {
                  n = result.book_name.find(search_str);
                  if(n != std::string::npos)
                    {
                      result.book_name.erase(n, search_str.size());
                      result.book_name.insert(n, insert_str);
                    }
                  else
                    {
                      break;
                    }
                }
            }

          el = get_tag(title_info[0].content, "sequence");
          if(el.size() > 0)
            {
              result.book_series = fb2_series(el);
            }

          el = get_tag(title_info[0].content, "genre");
          if(el.size() > 0)
            {
              result.book_genre = fb2_genres(el);
            }

          el = get_tag(title_info[0].content, "date");
          if(el.size() > 0)
            {
              result.book_date = el[0].content;
            }
        }
    }
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

  fb2_annotation_decode(loc_book, result->annotation, 1);
  if(result->annotation.empty())
    {
      code_page = af->detect_encoding(book);
      loc_book = af->to_utf_8(book, code_page.c_str());
      fb2_annotation_decode(loc_book, result->annotation, 1);
      if(result->annotation.empty())
        {
          fb2_annotation_decode(book, result->annotation, 2);
          fb2_cover(book, result->cover, 2);
          fb2_extra_info(book, *result, 2);
        }
      else
        {
          fb2_cover(loc_book, result->cover, 1);
          fb2_extra_info(loc_book, *result, 1);
        }
    }
  else
    {
      fb2_cover(loc_book, result->cover, 1);
      fb2_extra_info(loc_book, *result, 1);
    }

  if(!result->cover.empty())
    {
      result->cover_type = "base64";
    }

  return result;
}

BookParseEntry
FB2Parser::fb2_parser_alternative(const std::string &book)
{
  BookParseEntry result;
  std::vector<XMLTag> description = get_tag(book, "description", false);
  if(description.size() > 0)
    {
      std::vector<XMLTag> title_info
          = get_tag(description[0].content, "title-info");
      if(title_info.size() > 0)
        {
          std::vector<XMLTag> el
              = get_tag(title_info[0].content, "author", false);

          result.book_author = fb2_author(el, 2);

          el = get_tag(title_info[0].content, "book-title", true);
          if(el.size() > 0)
            {
              result.book_name = el[0].content;
            }

          el = get_tag(title_info[0].content, "sequence", true);
          if(el.size() > 0)
            {
              result.book_series = fb2_series(el);
            }

          el = get_tag(title_info[0].content, "genre", true);
          if(el.size() > 0)
            {
              result.book_genre = fb2_genres(el);
            }

          el = get_tag(title_info[0].content, "date", true);
          if(el.size() > 0)
            {
              result.book_date = el[0].content;
            }
        }
    }

  return result;
}

void
FB2Parser::fb2_annotation_decode(const std::string &book, std::string &result,
                                 const int &variant)
{
  std::vector<XMLTag> el;
  switch(variant)
    {
    case 1:
      {
        el = get_tag(book, "annotation");
        for(auto it = el.begin(); it != el.end(); it++)
          {
            if(result.empty())
              {
                result = it->content;
              }
            else
              {
                result = result + "\n\n" + it->content;
              }
          }
        break;
      }
    case 2:
      {
        el = get_tag(book, "annotation", true);
        for(auto it = el.begin(); it != el.end(); it++)
          {
            if(result.empty())
              {
                result = it->content;
              }
            else
              {
                result = result + "\n\n" + it->content;
              }
          }
        break;
      }
    default:
      break;
    }
}

void
FB2Parser::fb2_cover(const std::string &book, std::string &cover,
                     const int &variant)
{
  switch(variant)
    {
    case 1:
      {
        fb2_cover_main(book, cover);
        break;
      }
    case 2:
      {
        fb2_cover_fallback(book, cover);
        break;
      }
    default:
      break;
    }
}

void
FB2Parser::fb2_cover_main(const std::string &book, std::string &cover)
{
  std::vector<XMLTag> el;
  el = get_tag(book, "description");
  if(el.size() > 0)
    {
      std::string description = el[0].content;
      el = get_tag(el[0].content, "title-info");
      if(el.size() > 0)
        {
          el = get_tag(el[0].content, "coverpage");

          if(el.size() == 0)
            {
              el = get_tag(description, "src-title-info");
              if(el.size() > 0)
                {
                  el = get_tag(el[0].content, "coverpage");
                }
            }

          if(el.size() > 0)
            {
              el = get_tag(el[0].content, "image");
              if(el.size() > 0)
                {
                  std::string ref
                      = get_element_attribute(el[0].element, "l:href");
                  if(ref.empty())
                    {
                      ref = get_element_attribute(el[0].element, "href");
                    }
                  if(!ref.empty())
                    {
                      std::string sstr = "#";
                      std::string::size_type n = ref.find(sstr);
                      if(n != std::string::npos)
                        {
                          ref.erase(0, n + sstr.size());
                        }
                      el = get_tag(book, "binary");
                      for(auto it = el.begin(); it != el.end(); it++)
                        {
                          std::string id
                              = get_element_attribute(it->element, "id");
                          if(ref == id)
                            {
                              cover = it->content;
                              break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void
FB2Parser::fb2_cover_fallback(const std::string &book, std::string &cover)
{
  std::vector<XMLTag> el;
  el = get_tag(book, "description", false);
  if(el.size() > 0)
    {
      std::string description = el[0].content;
      el = get_tag(el[0].content, "title-info", false);
      if(el.size() > 0)
        {
          el = get_tag(el[0].content, "coverpage", false);

          if(el.size() == 0)
            {
              el = get_tag(description, "src-title-info", false);
              if(el.size() > 0)
                {
                  el = get_tag(el[0].content, "coverpage", false);
                }
            }

          if(el.size() > 0)
            {
              el = get_tag(el[0].content, "image", true);
              if(el.size() > 0)
                {
                  std::string ref
                      = get_element_attribute(el[0].element, "l:href");
                  if(ref.empty())
                    {
                      ref = get_element_attribute(el[0].element, "href");
                    }
                  if(!ref.empty())
                    {
                      std::string::size_type n = ref.find("#");
                      if(n != std::string::npos)
                        {
                          ref.erase(0, n + std::string("#").size());
                        }
                      el = get_tag(book, "binary", true);
                      for(auto it = el.begin(); it != el.end(); it++)
                        {
                          std::string id
                              = get_element_attribute(it->element, "id");
                          if(ref == id)
                            {
                              cover = it->content;
                              break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void
FB2Parser::fb2_extra_info(const std::string &book, BookInfoEntry &result,
                          const int &variant)
{
  switch(variant)
    {
    case 1:
      {
        fb2_extra_info_1(book, result);
        break;
      }
    case 2:
      {
        fb2_extra_info_2(book, result);
        break;
      }
    default:
      break;
    }
}

void
FB2Parser::fb2_extra_info_1(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> el;
  el = get_tag(book, "description");
  if(el.size() > 0)
    {
      std::string description = el[0].content;
      el = get_tag(el[0].content, "title-info");
      if(el.size() > 0)
        {
          std::vector<XMLTag> loc_el;
          loc_el = get_tag(el[0].content, "lang");
          if(loc_el.size() > 0)
            {
              for(auto it = loc_el.begin(); it != loc_el.end(); it++)
                {
                  if(result.language.empty())
                    {
                      result.language = it->content;
                    }
                  else if(!it->content.empty())
                    {
                      result.language = result.language + ", " + it->content;
                    }
                }
            }

          loc_el = get_tag(el[0].content, "src-lang");
          if(loc_el.size() > 0)
            {
              for(auto it = loc_el.begin(); it != loc_el.end(); it++)
                {
                  if(result.src_language.empty())
                    {
                      result.src_language = it->content;
                    }
                  else if(!it->content.empty())
                    {
                      result.src_language
                          = result.language + ", " + it->content;
                    }
                }
            }

          loc_el = get_tag(el[0].content, "translator");
          if(loc_el.size() > 0)
            {
              result.translator = fb2_author(loc_el, 1);
            }
        }

      fb2_publisher_info_1(description, result);
      fb2_electro_doc_info_1(description, result);
    }
}

void
FB2Parser::fb2_extra_info_2(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> el;
  el = get_tag(book, "description", false);
  if(el.size() > 0)
    {
      std::string description = el[0].content;
      el = get_tag(el[0].content, "title-info", false);
      if(el.size() > 0)
        {
          std::vector<XMLTag> loc_el;
          loc_el = get_tag(el[0].content, "lang", true);
          if(loc_el.size() > 0)
            {
              for(auto it = loc_el.begin(); it != loc_el.end(); it++)
                {
                  if(result.language.empty())
                    {
                      result.language = it->content;
                    }
                  else if(!it->content.empty())
                    {
                      result.language = result.language + ", " + it->content;
                    }
                }
            }

          loc_el = get_tag(el[0].content, "src-lang", true);
          if(loc_el.size() > 0)
            {
              for(auto it = loc_el.begin(); it != loc_el.end(); it++)
                {
                  if(result.src_language.empty())
                    {
                      result.src_language = it->content;
                    }
                  else if(!it->content.empty())
                    {
                      result.src_language
                          = result.language + ", " + it->content;
                    }
                }
            }

          loc_el = get_tag(el[0].content, "translator", false);
          if(loc_el.size() > 0)
            {
              result.translator = fb2_author(loc_el, 2);
            }
        }

      fb2_publisher_info_2(description, result);
      fb2_electro_doc_info_2(description, result);
    }
}

void
FB2Parser::fb2_publisher_info_1(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> el = get_tag(book, "publish-info");
  if(el.size() > 0)
    {
      std::vector<XMLTag> loc_el;
      loc_el = get_tag(el[0].content, "book-name");
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->book_name = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "publisher");
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->publisher = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "city");
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->city = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "year");
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->year = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "isbn");
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->isbn = loc_el[0].content;
            }
        }
    }
}

void
FB2Parser::fb2_publisher_info_2(const std::string &book, BookInfoEntry &result)
{
  std::vector<XMLTag> el = get_tag(book, "publish-info", false);
  if(el.size() > 0)
    {
      std::vector<XMLTag> loc_el;
      loc_el = get_tag(el[0].content, "book-name", true);
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->book_name = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "publisher", true);
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->publisher = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "city", true);
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->city = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "year", true);
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->year = loc_el[0].content;
            }
        }

      loc_el = get_tag(el[0].content, "isbn", true);
      if(loc_el.size() > 0)
        {
          if(!loc_el[0].content.empty())
            {
              result.paper->available = true;
              result.paper->isbn = loc_el[0].content;
            }
        }
    }
}

void
FB2Parser::fb2_electro_doc_info_1(const std::string &book,
                                  BookInfoEntry &result)
{
  std::vector<XMLTag> el = get_tag(book, "document-info");
  if(el.size() > 0)
    {
      std::vector<XMLTag> loc_el = get_tag(el[0].content, "author");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->author = fb2_author(loc_el, 1);
        }

      loc_el = get_tag(el[0].content, "program-used");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->program_used = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "date");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->date = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "src-url");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->src_url = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "src-ocr");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->src_ocr = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "id");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->id = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "version");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->version = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "history");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->history = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "publisher");
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->publisher = fb2_author(loc_el, 1);
        }
    }
}

void
FB2Parser::fb2_electro_doc_info_2(const std::string &book,
                                  BookInfoEntry &result)
{
  std::vector<XMLTag> el = get_tag(book, "document-info", false);
  if(el.size() > 0)
    {
      std::vector<XMLTag> loc_el = get_tag(el[0].content, "author", false);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->author = fb2_author(loc_el, 2);
        }

      loc_el = get_tag(el[0].content, "program-used", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->program_used = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "date", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->date = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "src-url", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->src_url = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "src-ocr", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->src_ocr = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "id", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->id = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "version", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->version = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "history", true);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->history = loc_el[0].content;
        }

      loc_el = get_tag(el[0].content, "publisher", false);
      if(loc_el.size() > 0)
        {
          result.electro->available = true;
          result.electro->publisher = fb2_author(loc_el, 2);
        }
    }
}
