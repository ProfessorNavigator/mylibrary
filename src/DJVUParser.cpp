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

#include <DJVUParser.h>
#include <chrono>
#include <iostream>
#include <string>

DJVUParser::DJVUParser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

DJVUParser::~DJVUParser()
{

}

BookParseEntry
DJVUParser::djvu_parser(const std::filesystem::path &filepath)
{
  BookParseEntry bpe;

  bpe.book_name = filepath.stem().u8string();
  std::filesystem::file_time_type cr =
      std::filesystem::last_write_time(filepath);

  auto sctp = std::chrono::time_point_cast<
      std::chrono::system_clock::duration>(
      cr - std::filesystem::file_time_type::clock::now()
	  + std::chrono::system_clock::now());
  time_t tt = std::chrono::system_clock::to_time_t(sctp);

  bpe.book_date = af->time_t_to_date(tt);

  return bpe;
}

std::shared_ptr<BookInfoEntry>
DJVUParser::djvu_cover(const std::filesystem::path &filepath)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<
      BookInfoEntry>();

  std::shared_ptr<ddjvu_context_t> context(
      ddjvu_context_create("MyLibrary"), []
      (ddjvu_context_t *c)
	{
	  ddjvu_context_release(c);
	});
  if(context)
    {
      if(!handle_djvu_msgs(context, false))
	{
	  return result;
	}

      std::shared_ptr<ddjvu_document_t> doc(
	  ddjvu_document_create_by_filename_utf8(
	      context.get(), filepath.u8string().c_str(), true),
	  []
	  (ddjvu_document_t *doc)
	    {
	      ddjvu_document_release(doc);
	    });

      if(doc)
	{
	  if(!handle_djvu_msgs(context, true))
	    {
	      return result;
	    }

	  std::shared_ptr<ddjvu_page_t> page(
	      ddjvu_page_create_by_pageno(doc.get(), 0), []
	      (ddjvu_page_t *p)
		{
		  ddjvu_page_release(p);
		});

	  if(page)
	    {
	      while(!ddjvu_page_decoding_done(page.get()))
		{
		  if(!handle_djvu_msgs(context, true))
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
		  ddjvu_format_create(DDJVU_FORMAT_RGBMASK32, 4,
				      bitmask),
		  []
		  (ddjvu_format_t *fmt)
		    {
		      ddjvu_format_release(fmt);
		    });

	      if(fmt)
		{
		  ddjvu_format_set_row_order(fmt.get(), 1);
		  int rowsize = rrect.w * 4;
		  result->cover.resize(rowsize * rrect.h);
		  if(ddjvu_page_render(page.get(), DDJVU_RENDER_COLOR,
				       &prect, &rrect, fmt.get(),
				       rowsize, &result->cover[0]))
		    {
		      result->cover_type = "rgba";
		      result->bytes_per_row = rowsize;
		    }
		  else
		    {
		      std::cout
			  << "DJVUParser::djvu_cover: DJVU render error"
			  << std::endl;
		      result->cover.clear();
		    }
		}
	      else
		{
		  std::cout
		      << "DJVUParser::djvu_cover: error on format set"
		      << std::endl;
		}
	    }
	  else
	    {
	      std::cout
		  << "DJVUParser::djvu_cover: page has not been opened"
		  << std::endl;
	    }
	}
      else
	{
	  std::cout
	      << "DJVUParser::djvu_cover: document has not been opened"
	      << std::endl;
	}
    }
  else
    {
      std::cout << "DJVUParser::djvu_cover: has not been created"
	  << std::endl;
    }

  return result;
}

bool
DJVUParser::handle_djvu_msgs(
    const std::shared_ptr<ddjvu_context_t> &ctx, const bool &wait)
{
  bool result = true;

  const ddjvu_message_t *msg;

  if(wait)
    {
      ddjvu_message_wait(ctx.get());
    }
  while((msg = ddjvu_message_peek(ctx.get())))
    {
      switch(msg->m_any.tag)
	{
	case DDJVU_ERROR:
	  {
	    const char *str = msg->m_error.message;
	    if(str)
	      {
		std::cout << "DJVUParser error: " << str << std::endl;
	      }
	    result = false;
	    break;
	  }
	default:
	  break;
	}
      ddjvu_message_pop(ctx.get());
    }

  return result;
}
