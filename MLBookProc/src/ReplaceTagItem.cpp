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
#include <ReplaceTagItem.h>

ReplaceTagItem::ReplaceTagItem()
{
}

ReplaceTagItem::ReplaceTagItem(const ReplaceTagItem &other)
{
  tag_to_replace = other.tag_to_replace;
  begin_replacement = other.begin_replacement;
  end_replacement = other.end_replacement;
}

ReplaceTagItem::ReplaceTagItem(ReplaceTagItem &&other)
{
  tag_to_replace = std::move(other.tag_to_replace);
  begin_replacement = std::move(other.begin_replacement);
  end_replacement = std::move(other.end_replacement);
}

ReplaceTagItem &
ReplaceTagItem::operator=(const ReplaceTagItem &other)
{
  if(this != &other)
    {
      tag_to_replace = other.tag_to_replace;
      begin_replacement = other.begin_replacement;
      end_replacement = other.end_replacement;
    }
  return *this;
}

ReplaceTagItem &
ReplaceTagItem::operator=(ReplaceTagItem &&other)
{
  if(this != &other)
    {
      tag_to_replace = std::move(other.tag_to_replace);
      begin_replacement = std::move(other.begin_replacement);
      end_replacement = std::move(other.end_replacement);
    }
  return *this;
}
