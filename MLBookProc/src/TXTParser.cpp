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

#include <TXTParser.h>
#include <XMLTextEncoding.h>
#include <chrono>
#include <fstream>

TXTParser::TXTParser(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
}

UDBElement
TXTParser::parseBook(const std::filesystem::path &file_path)
{
  UDBElement result;
  bid.setId(result, BaseID::Book);
  result.subelements.reserve(2);

  UDBElement el;
  bid.setId(el, BaseID::BookTitle);
  std::u8string u8str = file_path.stem().u8string();
  el.content = std::string(u8str.begin(), u8str.end());
  result.subelements.emplace_back(el);

  el = UDBElement();
  bid.setId(el, BaseID::Date);
  std::filesystem::file_time_type mod_tm_fl
      = std::filesystem::last_write_time(file_path);
  std::chrono::time_point<std::chrono::system_clock> mod_tm
      = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          mod_tm_fl - std::filesystem::file_time_type::clock::now()
          + std::chrono::system_clock::now());
  time_t tmt = std::chrono::system_clock::to_time_t(mod_tm);
  el.content = mlbp->timeToDate(tmt);
  result.subelements.emplace_back(el);

  return result;
}

UDBase
TXTParser::getBookInfo(const std::filesystem::path &file_path)
{
  UDBase result;

  std::filesystem::file_time_type mod_tm_fl
      = std::filesystem::last_write_time(file_path);
  std::chrono::time_point<std::chrono::system_clock> mod_tm
      = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          mod_tm_fl - std::filesystem::file_time_type::clock::now()
          + std::chrono::system_clock::now());

  time_t tm_t = std::chrono::system_clock::to_time_t(mod_tm);

  UDBElement el;
  bid.setId(el, BaseID::EbookDate);
  el.content = mlbp->timeToDate(tm_t);
  if(!el.content.empty())
    {
      result.addElement(el);
    }

  std::string buf;
  std::fstream f;
  f.open(file_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.seekg(0, std::ios_base::end);
      buf.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();
    }
  else
    {
      return result;
    }

  if(buf.size() > 0)
    {
      std::vector<std::string> enc
          = XMLTextEncoding::detectStringEncoding(buf);
      el = UDBElement();
      bid.setId(el, BaseID::CoverPage);
      if(enc.size() > 0)
        {
          XMLTextEncoding::convertToEncoding(buf, el.content, enc[0], "UTF-8");
        }
      else
        {
          el.content = buf;
        }
      if(!el.content.empty())
        {
          std::string ext = mlbp->getExtension(file_path);
          ext = mlbp->stringToLower(ext);
          UDBElement type;
          bid.setId(type, BaseID::CoverType);
          if(ext == ".txt")
            {
              type.content = "txt";
            }
          else
            {
              type.content = "md";
            }
          el.subelements.emplace_back(type);
          result.addElement(el);
        }
    }

  return result;
}
