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
#ifndef SEARCHVIEWMODELITEM_H
#define SEARCHVIEWMODELITEM_H

#include <BaseID.h>
#include <GenreBase.h>
#include <QString>
#include <UDBElement.h>
#include <memory>

class SearchViewModelItem
{
public:
  SearchViewModelItem();

  SearchViewModelItem(const UDBElement &book_search_result,
                      const std::shared_ptr<GenreBase> &genre_base);

  SearchViewModelItem(const SearchViewModelItem &other);

  SearchViewModelItem(SearchViewModelItem &&other);

  void
  refresh();

  SearchViewModelItem &
  operator=(const SearchViewModelItem &other);

  SearchViewModelItem &
  operator=(SearchViewModelItem &&other);

  QString authors;
  QString book_title;
  QString series;
  QString genres;
  QString date;
  UDBElement book_search_result;

private:
  void
  getAuthors(const std::vector<UDBElement> &book, BaseID &bid);

  void
  getBookTitle(const std::vector<UDBElement> &book, BaseID &bid);

  void
  getSeries(const std::vector<UDBElement> &book, BaseID &bid);

  void
  getGenres(const std::vector<UDBElement> &book, BaseID &bid);

  void
  getDate(const std::vector<UDBElement> &book, BaseID &bid);

  std::shared_ptr<GenreBase> genre_base;
};

#endif // SEARCHVIEWMODELITEM_H
