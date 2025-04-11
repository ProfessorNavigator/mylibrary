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

#include <DJVUParser.h>
#include <chrono>
#include <iostream>
#include <libdjvu/miniexp.h>
#include <string>

#ifdef USE_OPENMP
#include <unistd.h>
#endif

#ifndef USE_OPENMP
#include <thread>
#endif

DJVUParser::DJVUParser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

BookParseEntry
DJVUParser::djvu_parser(const std::filesystem::path &filepath)
{
  BookParseEntry bpe;

  bpe.book_name = filepath.stem().u8string();
  std::filesystem::file_time_type cr
      = std::filesystem::last_write_time(filepath);

  auto sctp
      = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          cr - std::filesystem::file_time_type::clock::now()
          + std::chrono::system_clock::now());
  time_t tt = std::chrono::system_clock::to_time_t(sctp);

  bpe.book_date = af->time_t_to_date(tt);

  std::shared_ptr<ddjvu_context_t> context = af->getDJVUContext();

  if(context)
    {
      std::shared_ptr<ddjvu_document_t> doc(
          ddjvu_document_create_by_filename_utf8(
              context.get(), filepath.u8string().c_str(), true),
          [](ddjvu_document_t *doc) {
            ddjvu_document_release(doc);
          });

      if(doc)
        {
          if(!handleDJVUmsgs(context, doc))
            {
              return bpe;
            }

          miniexp_t r;

          while((r = ddjvu_document_get_anno(doc.get(), true))
                == miniexp_dummy)
            {
#ifdef USE_OPENMP
              usleep(10000);
#endif
#ifndef USE_OPENMP
              std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
            }
          if(r)
            {
              if(r != miniexp_dummy)
                {
                  miniexp_t exp = miniexp_pname(r, 0);
                  const char *val = miniexp_to_str(exp);
                  if(val)
                    {
                      std::string exp_str(val);
                      getTag(exp_str, "author", bpe.book_author);
                      if(bpe.book_author.empty())
                        {
                          getTag(exp_str, "Author", bpe.book_author);
                        }
                      std::string::size_type n = 0;
                      std::string find_str = "  ";
                      for(;;)
                        {
                          n = bpe.book_author.find(find_str, n);
                          if(n != std::string::npos)
                            {
                              bpe.book_author.erase(bpe.book_author.begin()
                                                    + n);
                            }
                          else
                            {
                              break;
                            }
                        }

                      bool stop = false;

                      while(bpe.book_author.size() > 0)
                        {
                          switch(*bpe.book_author.rbegin())
                            {
                            case 0 ... 32:
                              {
                                bpe.book_author.pop_back();
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
                      while(bpe.book_author.size() > 0)
                        {
                          switch(*bpe.book_author.begin())
                            {
                            case 0 ... 32:
                              {
                                bpe.book_author.erase(bpe.book_author.begin());
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
                      std::string tmp;
                      getTag(exp_str, "title", tmp);
                      if(tmp.empty())
                        {
                          getTag(exp_str, "Title", tmp);
                        }
                      if(!tmp.empty())
                        {
                          bpe.book_name = tmp;
                        }
                      getTag(exp_str, "series", bpe.book_series);
                      if(bpe.book_series.empty())
                        {
                          getTag(exp_str, "Series", bpe.book_series);
                        }
                      tmp.clear();
                      getTag(exp_str, "year", tmp);
                      if(tmp.empty())
                        {
                          getTag(exp_str, "Year", tmp);
                        }
                      if(!tmp.empty())
                        {
                          bpe.book_date = tmp;
                        }
                    }
                }
              ddjvu_miniexp_release(doc.get(), r);
            }
        }
      else
        {
          std::cout << "DJVUParser::djvu_parser: document has not been opened"
                    << std::endl;
        }
    }
  else
    {
      std::cout << "DJVUParser::djvu_parser: context has not been created"
                << std::endl;
    }

  return bpe;
}

std::shared_ptr<BookInfoEntry>
DJVUParser::djvu_book_info(const std::filesystem::path &filepath)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();
  std::shared_ptr<ddjvu_context_t> context = af->getDJVUContext();
  if(context)
    {
      std::shared_ptr<ddjvu_document_t> doc(
          ddjvu_document_create_by_filename_utf8(
              context.get(), filepath.u8string().c_str(), true),
          [](ddjvu_document_t *doc) {
            ddjvu_document_release(doc);
          });

      if(doc)
        {
          if(!handleDJVUmsgs(context, doc))
            {
              return result;
            }

          miniexp_t r;

          while((r = ddjvu_document_get_anno(doc.get(), true))
                == miniexp_dummy)
            {
              handleDJVUmsgs(context, doc);
            }
          if(r)
            {
              if(r != miniexp_dummy)
                {
                  miniexp_t exp = miniexp_pname(r, 0);
                  const char *val = miniexp_to_str(exp);
                  if(val)
                    {
                      std::string exp_str(val);
                      getTag(exp_str, "annote", result->annotation);
                    }
                }
              ddjvu_miniexp_release(doc.get(), r);
            }

          std::shared_ptr<ddjvu_page_t> page(
              ddjvu_page_create_by_pageno(doc.get(), 0), [](ddjvu_page_t *p) {
                ddjvu_page_release(p);
              });

          if(page)
            {
              while(!ddjvu_page_decoding_done(page.get()))
                {
                  if(!handleDJVUmsgs(context, doc))
                    {
                      return result;
                    }
                }
              int iw = ddjvu_page_get_width(page.get());
              int ih = ddjvu_page_get_height(page.get());

              ddjvu_rect_t prect;
              ddjvu_rect_t rrect;
              prect.x = 0;
              prect.y = 0;
              prect.w = iw;
              prect.h = ih;
              rrect = prect;

              unsigned int bitmask[4];
              bitmask[0] = 255;
              bitmask[1] = 65280;
              bitmask[2] = 16711680;
              bitmask[3] = 4278190080;

              std::shared_ptr<ddjvu_format_t> fmt(
                  ddjvu_format_create(DDJVU_FORMAT_RGBMASK32, 4, bitmask),
                  [](ddjvu_format_t *fmt) {
                    ddjvu_format_release(fmt);
                  });

              if(fmt)
                {
                  ddjvu_format_set_row_order(fmt.get(), 1);
                  int rowsize = rrect.w * 4;
                  result->cover.resize(rowsize * rrect.h);
                  if(ddjvu_page_render(page.get(), DDJVU_RENDER_COLOR, &prect,
                                       &rrect, fmt.get(), rowsize,
                                       &result->cover[0]))
                    {
                      result->cover_type = BookInfoEntry::cover_types::rgba;
                      result->bytes_per_row = rowsize;
                    }
                  else
                    {
                      std::cout << "DJVUParser::djvu_cover: DJVU render error"
                                << std::endl;
                      result->cover.clear();
                    }
                }
              else
                {
                  std::cout << "DJVUParser::djvu_cover: error on format set"
                            << std::endl;
                }
            }
          else
            {
              std::cout << "DJVUParser::djvu_cover: page has not been opened"
                        << std::endl;
            }
        }
      else
        {
          std::cout << "DJVUParser::djvu_cover: document has not been opened"
                    << std::endl;
        }
    }
  else
    {
      std::cout << "DJVUParser::djvu_cover: context has not been created"
                << std::endl;
    }

  return result;
}

bool
DJVUParser::handleDJVUmsgs(const std::shared_ptr<ddjvu_context_t> &ctx,
                           const std::shared_ptr<ddjvu_document_t> &doc)
{
  bool result = true;

  const ddjvu_message_t *msg;

  while((msg = ddjvu_message_peek(ctx.get())))
    {
      if(doc.get() == msg->m_any.document)
        {
          switch(msg->m_any.tag)
            {
            case DDJVU_ERROR:
              {
                if(msg->m_error.message)
                  {
                    std::cout << "DJVUParser error: " << msg->m_error.message
                              << std::endl;
                  }
                result = false;
                break;
              }
            default:
              {
                break;
              }
            }
          ddjvu_message_pop(ctx.get());
        }
      else
        {
#ifdef USE_OPENMP
          usleep(10000);
#endif
#ifndef USE_OPENMP
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
        }
    }

  return result;
}

void
DJVUParser::getTag(const std::string &exp, const std::string &tag,
                   std::string &line)
{
  std::string::size_type n = exp.find(tag);
  if(n != std::string::npos && exp.size() > n + tag.size())
    {
      n += tag.size();
      std::string find_str = "\"";
      n = exp.find(find_str, n);
      if(n != std::string::npos && exp.size() > n + find_str.size())
        {
          n += find_str.size();
          std::string::size_type n2 = exp.find(find_str, n);
          if(n2 != std::string::npos)
            {
              line = exp.substr(n, n2 - n);
            }
        }
    }
}
