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

#include <PDFParser.h>
#include <algorithm>
#include <poppler-document.h>
#include <poppler-page-renderer.h>
#include <poppler-page.h>
#include <stdexcept>

PDFParser::PDFParser(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
}

UDBElement
PDFParser::parseBook(const std::string &book_content)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);

  std::unique_ptr<poppler::document> doc(poppler::document::load_from_raw_data(
      book_content.c_str(), static_cast<int>(book_content.size())));
  if(doc.get() == nullptr)
    {
      throw std::runtime_error("PDFParser::parseBook: document is null");
    }

  std::vector<char> buf = doc->get_author().to_utf8();
  if(buf.size() > 0)
    {
      UDBElement el;
      bid.setId(el, BaseID::Author);
      el.content.reserve(buf.size());
      std::copy(buf.begin(), buf.end(), std::back_inserter(el.content));
      normalizeString(el.content);
      if(!el.content.empty())
        {
          result.subelements.emplace_back(el);
        }
    }

  buf = doc->get_title().to_utf8();
  if(buf.size() > 0)
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      el.content.reserve(buf.size());
      std::copy(buf.begin(), buf.end(), std::back_inserter(el.content));
      normalizeString(el.content);
      if(!el.content.empty())
        {
          result.subelements.emplace_back(el);
        }
    }
  time_t creation_time;
#ifdef _OLDPOPPLER
  creation_time = doc->get_creation_date();
#else
  creation_time = doc->get_creation_date_t();
#endif

  UDBElement el;
  bid.setId(el, BaseID::Date);
  el.content = mlbp->timeToDate(creation_time);
  if(!el.content.empty())
    {
      result.subelements.emplace_back(el);
    }

  return result;
}

UDBase
PDFParser::getBookInfo(const std::string &book_content)
{
  UDBase result;
  std::unique_ptr<poppler::document> doc(poppler::document::load_from_raw_data(
      book_content.c_str(), static_cast<int>(book_content.size())));
  if(doc.get() == nullptr)
    {
      throw std::runtime_error("PDFParser::getBookInfo: document is null");
    }

  std::vector<char> buf = doc->get_subject().to_utf8();
  UDBElement el;
  bid.setId(el, BaseID::Annotation);
  std::copy(buf.begin(), buf.end(), std::back_inserter(el.content));
  normalizeString(el.content);
  if(!el.content.empty())
    {
      result.addElement(el);
    }

  if(doc->pages() > 0 && poppler::page_renderer::can_render())
    {
      std::unique_ptr<poppler::page> page(doc->create_page(0));
      if(page.get() != nullptr)
        {
          std::unique_ptr<poppler::page_renderer> renderer(
              new poppler::page_renderer);
          renderer->set_image_format(poppler::image::format_argb32);
          poppler::image image = renderer->render_page(
              page.get(), horizontal_dpi, vertical_dpi);
          char *data = image.data();
          int lim = image.bytes_per_row() * image.height();
          UDBElement el;
          bid.setId(el, BaseID::CoverPage);
          el.content.reserve(lim);
          for(int i = 0; i < lim; i++)
            {
              el.content.push_back(data[i]);
            }
          if(!el.content.empty())
            {
              UDBElement type;
              bid.setId(type, BaseID::CoverType);
              type.content = "ARGB";

              UDBElement val;
              bid.setId(val, BaseID::CoverHeight);
              int v = image.height();
              char *ptr = reinterpret_cast<char *>(&v);
              val.content.resize(sizeof(v));
              for(size_t i = 0; i < val.content.size(); i++)
                {
                  val.content[i] = ptr[i];
                }
              type.subelements.emplace_back(val);

              val = UDBElement();
              bid.setId(val, BaseID::CoverWidth);
              v = image.width();
              val.content.resize(sizeof(v));
              for(size_t i = 0; i < val.content.size(); i++)
                {
                  val.content[i] = ptr[i];
                }
              type.subelements.emplace_back(val);

              el.subelements.emplace_back(type);
              result.addElement(el);
            }
        }
    }

  buf = doc->get_keywords().to_utf8();
  el = UDBElement();
  bid.setId(el, BaseID::Keywords);
  std::copy(buf.begin(), buf.end(), std::back_inserter(el.content));
  normalizeString(el.content);
  if(!el.content.empty())
    {
      result.addElement(el);
    }

  el = UDBElement();
  bid.setId(el, BaseID::EbookID);
  if(doc->get_pdf_id(&el.content, nullptr))
    {
      if(!el.content.empty())
        {
          result.addElement(el);
        }
    }

  result.shrinkToFit();

  return result;
}

void
PDFParser::setDPI(const double &horizontal_dpi, const double &vertical_dpi)
{
  this->horizontal_dpi = horizontal_dpi;
  this->vertical_dpi = vertical_dpi;
}

double
PDFParser::getHorizontalDPI()
{
  return horizontal_dpi;
}

double
PDFParser::getVerticalDPI()
{
  return vertical_dpi;
}

void
PDFParser::normalizeString(std::string &str)
{
  for(auto it = str.begin(); it != str.end();)
    {
      char val = *it;
      if(val >= 0 && val <= 32)
        {
          str.erase(it);
        }
      else
        {
          break;
        }
    }
  while(str.size() > 0)
    {
      char val = *str.rbegin();
      if(val >= 0 && val <= 32)
        {
          str.pop_back();
        }
      else
        {
          break;
        }
    }

  std::string find_str("  ");
  std::string::size_type n = 0;
  for(;;)
    {
      n = str.find(find_str, n);
      if(n != std::string::npos)
        {
          str.erase(str.begin() + n);
        }
      else
        {
          break;
        }
    }
}
