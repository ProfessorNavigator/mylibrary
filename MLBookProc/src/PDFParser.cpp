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
#include <PDFParser.h>
#include <algorithm>
#include <cstring>
#include <poppler-document.h>
#include <poppler-global.h>
#include <poppler-image.h>
#include <poppler-page-renderer.h>
#include <poppler-page.h>
#include <vector>

PDFParser::PDFParser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

BookParseEntry
PDFParser::pdf_parser(const std::string &file)
{
  if(file.empty())
    {
      throw MLException("PDFParser::pdf_parser: enpty file");
    }
  BookParseEntry bpe;

  std::shared_ptr<poppler::document> doc(poppler::document::load_from_raw_data(
      file.c_str(), file.size(), "", ""));

  if(doc)
    {
      std::vector<char> buf = doc->get_author().to_utf8();
      bpe.book_author = std::string(buf.begin(), buf.end());

      std::string::size_type n = 0;
      std::string find_str = "  ";
      for(;;)
        {
          n = bpe.book_author.find(find_str, n);
          if(n != std::string::npos)
            {
              bpe.book_author.erase(bpe.book_author.begin() + n);
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

      buf = doc->get_title().to_utf8();
      bpe.book_name = std::string(buf.begin(), buf.end());

      time_t creation_time;
#ifdef _OLDPOPPLER
      creation_time = doc->get_creation_date();
#else
      creation_time = doc->get_creation_date_t();
#endif
      bpe.book_date = af->time_t_to_date(creation_time);
    }
  else
    {
      throw MLException("PDFParser::pdf_parser: pdf file has not been opened");
    }

  return bpe;
}

std::shared_ptr<BookInfoEntry>
PDFParser::pdf_annotation_n_cover(const std::string &file, const double &x_dpi,
                                  const double &y_dpi)
{
  std::shared_ptr<BookInfoEntry> result = std::make_shared<BookInfoEntry>();

  std::shared_ptr<poppler::document> doc(poppler::document::load_from_raw_data(
      file.c_str(), file.size(), "", ""));

  if(doc)
    {
      std::vector<char> buf = doc->get_subject().to_utf8();
      std::copy(buf.begin(), buf.end(),
                std::back_inserter(result->annotation));
      if(doc->pages() > 0)
        {
          poppler::page *pg = doc->create_page(0);
          poppler::page_renderer pr;
          pr.set_image_format(poppler::image::format_rgb24);
          poppler::image image;
          if(x_dpi >= y_dpi)
            {
              image = pr.render_page(pg, x_dpi, x_dpi, -1, -1, -1, -1,
                                     poppler::rotate_0);
            }
          else
            {
              image = pr.render_page(pg, y_dpi, y_dpi, -1, -1, -1, -1,
                                     poppler::rotate_0);
            }
          result->cover.resize(image.bytes_per_row() * image.height());
          std::memcpy(result->cover.data(), image.const_data(),
                      result->cover.size());
          result->cover_type = BookInfoEntry::cover_types::rgb;
          result->bytes_per_row = image.bytes_per_row();
          delete pg;
        }
    }
  else
    {
      throw MLException(
          "PDFParser::pdf_annotation_n_cover: pdf file has not been opened");
    }

  return result;
}
