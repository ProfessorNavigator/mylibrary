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

#include <TXTParser.h>
#include <chrono>
#include <fstream>

TXTParser::TXTParser(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

BookParseEntry
TXTParser::txtParser(const std::filesystem::path &txt_path)
{
  BookParseEntry bpe;

  bpe.book_name = txt_path.stem().u8string();
  std::filesystem::file_time_type mod_tm_fl
      = std::filesystem::last_write_time(txt_path);
  std::chrono::time_point<std::chrono::system_clock> mod_tm
      = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          mod_tm_fl - std::filesystem::file_time_type::clock::now()
          + std::chrono::system_clock::now());

  time_t tm_t = std::chrono::system_clock::to_time_t(mod_tm);

  bpe.book_date = af->time_t_to_date(tm_t);

  return bpe;
}

std::shared_ptr<BookInfoEntry>
TXTParser::txtBookInfo(const std::filesystem::path &txt_path)
{
  std::shared_ptr<BookInfoEntry> bie = std::make_shared<BookInfoEntry>();

  std::filesystem::file_time_type mod_tm_fl
      = std::filesystem::last_write_time(txt_path);
  std::chrono::time_point<std::chrono::system_clock> mod_tm
      = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          mod_tm_fl - std::filesystem::file_time_type::clock::now()
          + std::chrono::system_clock::now());

  time_t tm_t = std::chrono::system_clock::to_time_t(mod_tm);

  bie->electro->date = af->time_t_to_date(tm_t);
  bie->electro->available = true;

  std::fstream f;
  f.open(txt_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.seekg(0, std::ios_base::end);
      bie->cover.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(bie->cover.data(), bie->cover.size());
      f.close();
    }
  else
    {
      return bie;
    }

  if(bie->cover.size() > 0)
    {
      std::string enc = af->detectEncoding(bie->cover);
      bie->cover = af->toUTF8(bie->cover, enc.c_str());
      bie->cover_type = BookInfoEntry::cover_types::text;
    }

  return bie;
}
