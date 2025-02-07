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

#ifndef DJVUPARSER_H
#define DJVUPARSER_H

#include <AuxFunc.h>
#include <BookInfoEntry.h>
#include <BookParseEntry.h>
#include <filesystem>
#include <libdjvu/ddjvuapi.h>
#include <memory>

class DJVUParser
{
public:
  DJVUParser(const std::shared_ptr<AuxFunc> &af);

  BookParseEntry
  djvu_parser(const std::filesystem::path &filepath);

  std::shared_ptr<BookInfoEntry>
  djvu_cover(const std::filesystem::path &filepath);

private:
  bool
  handle_djvu_msgs(const std::shared_ptr<ddjvu_context_t> &ctx,
                   const bool &wait);

  void
  getTag(const std::string &exp, const std::string &tag, std::string &line);

  std::shared_ptr<AuxFunc> af;
};

#endif // DJVUPARSER_H
