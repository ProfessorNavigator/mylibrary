/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <ByteOrder.h>
#include <DJVUParser.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <libdjvu/miniexp.h>
#include <syncstream>
#include <thread>

DJVUParser::DJVUParser(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
}

UDBElement
DJVUParser::parseBook(const std::string &book_content)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);

  UDBElement el;
  bid.setId(el, BaseID::BookTitle);
  result.subelements.emplace_back(el);

  el = UDBElement();
  bid.setId(el, BaseID::Date);
  result.subelements.emplace_back(el);

  std::shared_ptr<DJVUContext> ctx = mlbp->getDJVUContext();
  if(!ctx.operator bool())
    {
      std::osyncstream(std::cout)
          << "DJVUParser::parseBook: context object is null!" << std::endl;
      return result;
    }

  std::shared_ptr<ddjvu_document_t> doc(
      ddjvu_document_create(ctx->context, nullptr, false),
      [](ddjvu_document_t *doc)
        {
          ddjvu_document_release(doc);
        });
  if(!doc.operator bool())
    {
      std::osyncstream(std::cout)
          << "DJVUParser::parseBook: document object is null!" << std::endl;
      return result;
    }

  if(!setBookContentToStream(ctx, doc, book_content))
    {
      return result;
    }

  if(!waitDocumentInfo(ctx, doc))
    {
      return result;
    }

  miniexp_t r;
  std::chrono::milliseconds mlsc(5);
  while((r = ddjvu_document_get_anno(doc.get(), true)) == miniexp_dummy)
    {
      std::this_thread::sleep_for(mlsc);
    }
  if(r)
    {
      if(r != miniexp_nil)
        {
          miniexp_t exp = miniexp_pname(r, 0);
          const char *val = miniexp_to_str(exp);
          if(val)
            {
              el = UDBElement();
              bid.setId(el, BaseID::Author);
              std::string exp_str(val);
              getTag(exp_str, "author", el.content);
              if(el.content.empty())
                {
                  getTag(exp_str, "Author", el.content);
                }
              std::string::size_type n = 0;
              std::string find_str = "  ";
              for(;;)
                {
                  n = el.content.find(find_str, n);
                  if(n != std::string::npos)
                    {
                      el.content.erase(el.content.begin() + n);
                    }
                  else
                    {
                      break;
                    }
                }

              bool stop = false;

              while(el.content.size() > 0)
                {
                  switch(*el.content.rbegin())
                    {
                    case 0 ... 32:
                      {
                        el.content.pop_back();
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
              while(el.content.size() > 0)
                {
                  switch(*el.content.begin())
                    {
                    case 0 ... 32:
                      {
                        el.content.erase(el.content.begin());
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

              result.subelements.emplace_back(el);

              auto it_res = std::find_if(
                  result.subelements.begin(), result.subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::BookTitle;
                    });
              if(it_res == result.subelements.end())
                {
                  return result;
                }

              std::string tmp;
              getTag(exp_str, "title", tmp);
              if(tmp.empty())
                {
                  getTag(exp_str, "Title", tmp);
                }
              if(!tmp.empty())
                {
                  it_res->content = tmp;
                }

              el = UDBElement();
              bid.setId(el, BaseID::Sequence);
              getTag(exp_str, "series", el.content);
              if(el.content.empty())
                {
                  getTag(exp_str, "Series", el.content);
                }
              result.subelements.emplace_back(el);

              it_res = std::find_if(result.subelements.begin(),
                                    result.subelements.end(),
                                    [this](const UDBElement &el)
                                      {
                                        return bid.getId(el) == BaseID::Date;
                                      });
              if(it_res == result.subelements.end())
                {
                  return result;
                }

              tmp.clear();
              getTag(exp_str, "year", tmp);
              if(tmp.empty())
                {
                  getTag(exp_str, "Year", tmp);
                }
              if(!tmp.empty())
                {
                  it_res->content = tmp;
                }
            }
        }
    }

  return result;
}

UDBase
DJVUParser::getBookInfo(const std::string &book_content)
{
  UDBase result;

  std::shared_ptr<DJVUContext> ctx = mlbp->getDJVUContext();
  if(!ctx.operator bool())
    {
      std::osyncstream(std::cout)
          << "DJVUParser::getBookInfo: context object is null!" << std::endl;
      return result;
    }

  std::shared_ptr<ddjvu_document_t> doc(
      ddjvu_document_create(ctx->context, nullptr, false),
      [](ddjvu_document_t *doc)
        {
          ddjvu_document_release(doc);
        });
  if(!doc.operator bool())
    {
      std::osyncstream(std::cout)
          << "DJVUParser::getBookInfo: document object is null!" << std::endl;
      return result;
    }

  if(!setBookContentToStream(ctx, doc, book_content))
    {
      return result;
    }

  if(!waitDocumentInfo(ctx, doc))
    {
      return result;
    }

  miniexp_t r;
  std::chrono::milliseconds mlsc(5);
  while((r = ddjvu_document_get_anno(doc.get(), true)) == miniexp_dummy)
    {
      std::this_thread::sleep_for(mlsc);
    }
  if(r != nullptr)
    {
      if(r != miniexp_nil)
        {
          miniexp_t exp = miniexp_pname(r, 0);
          const char *val = miniexp_to_str(exp);
          if(val)
            {
              std::string exp_str(val);
              UDBElement el;
              bid.setId(el, BaseID::Annotation);
              getTag(exp_str, "annote", el.content);
              if(!el.content.empty())
                {
                  result.addElement(el);
                }

              el = UDBElement();
              bid.setId(el, BaseID::Keywords);
              getTag(exp_str, "Keywords", el.content);
              if(!el.content.empty())
                {
                  result.addElement(el);
                }

              el = UDBElement();
              bid.setId(el, BaseID::DjvuPublisher);
              getTag(exp_str, "publisher", el.content);
              if(!el.content.empty())
                {
                  result.addElement(el);
                }

              el = UDBElement();
              bid.setId(el, BaseID::EbookProgramUsed);
              getTag(exp_str, "software", el.content);
              if(!el.content.empty())
                {
                  result.addElement(el);
                }
            }
        }
    }

  std::shared_ptr<ddjvu_page_t> page = getFirstPage(ctx, doc);
  if(!page.operator bool())
    {
      return result;
    }

  int iw = ddjvu_page_get_width(page.get());
  int ih = ddjvu_page_get_height(page.get());

  ddjvu_rect_t page_rect;
  page_rect.x = 0;
  page_rect.y = 0;
  page_rect.w = iw;
  page_rect.h = ih;

  std::unique_ptr<uint32_t, std::function<void(uint32_t *)>> bitmask(
      new uint32_t[4],
      [](uint32_t *ptr)
        {
          delete[] ptr;
        });
  uint32_t val = 0x000000ff;
  ByteOrder bo;
  bo.setLittle(val);
  *(bitmask.get()) = bo;

  val = 0x0000ff00;
  bo.setLittle(val);
  *(bitmask.get() + 1) = bo;

  val = 0x00ff0000;
  bo.setLittle(val);
  *(bitmask.get() + 2) = bo;

  val = 0xff000000;
  bo.setLittle(val);
  *(bitmask.get() + 3) = bo;

  std::shared_ptr<ddjvu_format_t> fmt(
      ddjvu_format_create(DDJVU_FORMAT_RGBMASK32, 4, bitmask.get()),
      [](ddjvu_format_t *fmt)
        {
          ddjvu_format_release(fmt);
        });

  if(!fmt.operator bool())
    {
      std::osyncstream(std::cout)
          << "DJVUParser::getBookInfo: format object is null!" << std::endl;
    }

  ddjvu_format_set_row_order(fmt.get(), 1);
  int rowsize = page_rect.w * 4;
  UDBElement el;
  bid.setId(el, BaseID::CoverPage);
  el.content.resize(rowsize * page_rect.h);
  if(ddjvu_page_render(page.get(), DDJVU_RENDER_COLOR, &page_rect, &page_rect,
                       fmt.get(), rowsize, el.content.data()))
    {
      UDBElement type;
      bid.setId(type, BaseID::CoverType);
      type.content = "RGB";

      UDBElement val;
      bid.setId(val, BaseID::CoverHeight);
      val.content.resize(sizeof(page_rect.h));
      char *ptr = reinterpret_cast<char *>(&page_rect.h);
      for(size_t i = 0; i < val.content.size(); i++)
        {
          val.content[i] = ptr[i];
        }
      type.subelements.emplace_back(val);

      val = UDBElement();
      bid.setId(val, BaseID::CoverWidth);
      val.content.resize(sizeof(page_rect.w));
      ptr = reinterpret_cast<char *>(&page_rect.w);
      for(size_t i = 0; i < val.content.size(); i++)
        {
          val.content[i] = ptr[i];
        }
      type.subelements.emplace_back(val);

      el.subelements.emplace_back(type);

      result.addElement(el);
    }
  else
    {
      std::osyncstream(std::cout)
          << "DJVUParser::getBookInfo: DJVU render error" << std::endl;
    }

  return result;
}

bool
DJVUParser::setBookContentToStream(
    const std::shared_ptr<DJVUContext> &ctx,
    const std::shared_ptr<ddjvu_document_t> &doc,
    const std::string &book_content)
{
  ddjvu_message_t *msg = nullptr;

  std::unique_lock<std::mutex> ullock(ctx->context_mtx);
  ctx->context_var.wait_for(
      ullock, std::chrono::seconds(5),
      [ctx, doc, &msg]
        {
          msg = ddjvu_message_peek(ctx->context);
          if(msg == nullptr)
            {
              return false;
            }
          if(msg->m_any.document != doc.get())
            {
              msg = nullptr;
              return false;
            }
          switch(msg->m_any.tag)
            {
            case DDJVU_ERROR:
              {
                std::osyncstream(std::cout)
                    << "DJVUParser::setBookContentToStream error: "
                    << msg->m_error.message
                    << " function: " << msg->m_error.function << std::endl;
                ddjvu_message_pop(ctx->context);
                msg = nullptr;
                break;
              }
            case DDJVU_NEWSTREAM:
              {
                break;
              }
            default:
              {
                ddjvu_message_pop(ctx->context);
                msg = nullptr;
                return false;
              }
            }
          return true;
        });

  if(msg == nullptr)
    {
      return false;
    }

  ddjvu_stream_write(msg->m_any.document, msg->m_newstream.streamid,
                     book_content.c_str(),
                     static_cast<unsigned long>(book_content.size()));
  ddjvu_stream_close(msg->m_any.document, msg->m_newstream.streamid, false);

  ddjvu_message_pop(ctx->context);

  return true;
}

bool
DJVUParser::waitDocumentInfo(const std::shared_ptr<DJVUContext> &ctx,
                             const std::shared_ptr<ddjvu_document_t> &doc)
{
  ddjvu_message_t *msg = nullptr;

  std::unique_lock<std::mutex> ullock(ctx->context_mtx);
  ctx->context_var.wait_for(
      ullock, std::chrono::seconds(5),
      [ctx, doc, &msg]
        {
          msg = ddjvu_message_peek(ctx->context);
          if(msg == nullptr)
            {
              return false;
            }
          if(msg->m_any.document != doc.get())
            {
              msg = nullptr;
              return false;
            }
          switch(msg->m_any.tag)
            {
            case DDJVU_ERROR:
              {
                std::osyncstream(std::cout)
                    << "DJVUParser::waitDocumentInfo error: "
                    << msg->m_error.message
                    << " function: " << msg->m_error.function << std::endl;
                ddjvu_message_pop(ctx->context);
                msg = nullptr;
                break;
              }
            case DDJVU_DOCINFO:
              {
                ddjvu_message_pop(ctx->context);
                break;
              }
            default:
              {
                ddjvu_message_pop(ctx->context);
                msg = nullptr;
                return false;
              }
            }
          return true;
        });

  if(msg == nullptr)
    {
      return false;
    }
  else
    {
      return true;
    }
}

std::shared_ptr<ddjvu_page_t>
DJVUParser::getFirstPage(const std::shared_ptr<DJVUContext> &ctx,
                         const std::shared_ptr<ddjvu_document_t> &doc)
{
  std::shared_ptr<ddjvu_page_t> result(
      ddjvu_page_create_by_pageno(doc.get(), 0),
      [](ddjvu_page_t *p)
        {
          ddjvu_page_release(p);
        });
  if(!result.operator bool())
    {
      return result;
    }

  ddjvu_message_t *msg = nullptr;
  std::unique_lock<std::mutex> ullock(ctx->context_mtx);
  ctx->context_var.wait_for(
      ullock, std::chrono::seconds(5),
      [ctx, doc, &msg, result]
        {
          msg = ddjvu_message_peek(ctx->context);
          if(msg == nullptr)
            {
              return false;
            }
          if(msg->m_any.document != doc.get())
            {
              msg = nullptr;
              return false;
            }
          switch(msg->m_any.tag)
            {
            case DDJVU_ERROR:
              {
                std::osyncstream(std::cout)
                    << "DJVUParser::getFirstPage error: "
                    << msg->m_error.message
                    << " function: " << msg->m_error.function << std::endl;
                ddjvu_message_pop(ctx->context);
                msg = nullptr;
                break;
              }
            default:
              {
                if(!ddjvu_page_decoding_done(result.get()))
                  {
                    ddjvu_message_pop(ctx->context);
                    msg = nullptr;
                    return false;
                  }
                else
                  {
                    ddjvu_message_pop(ctx->context);
                  }
                break;
              }
            }

          return true;
        });
  if(msg == nullptr)
    {
      result.reset();
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
