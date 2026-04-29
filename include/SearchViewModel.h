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
#ifndef SEARCHVIEWMODEL_H
#define SEARCHVIEWMODEL_H

#include <BaseID.h>
#include <GenreBase.h>
#include <MLBookProc.h>
#include <QAbstractItemModel>
#include <QObject>
#include <SearchViewModelItem.h>
#include <UDBase.h>

class SearchViewModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  SearchViewModel(QObject *parent, const UDBase &base,
                  const std::shared_ptr<GenreBase> &genre_base,
                  const std::shared_ptr<MLBookProc> &mlbp);

  QModelIndex
  index(int row, int column,
        const QModelIndex &parent = QModelIndex()) const override;

  QModelIndex
  parent(const QModelIndex &index) const override;

  int
  rowCount(const QModelIndex &parent = QModelIndex()) const override;

  int
  columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant
  data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags
  flags(const QModelIndex &index) const override;

  QVariant
  headerData(int section, Qt::Orientation orientation,
             int role = Qt::DisplayRole) const override;

  void
  sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  Qt::ItemFlags
  getCollectionEditable();

  void
  setFilter(const QString &filter, const int &column);

  void
  removeFilter();

  void
  removeBook(const UDBElement &book_search_result);

  void
  setAuthors(const QModelIndex &index, const std::vector<UDBElement> &authors);

  void
  setBookTitle(const QModelIndex &index, const QString &title);

  void
  setSeries(const QModelIndex &index, const std::vector<UDBElement> &series);

  void
  setGenres(const QModelIndex &index, const std::vector<UDBElement> &genres);

  void
  setDate(const QModelIndex &index, const QString &date);

signals:
  void
  signalEditBook(const UDBElement &book_search_result);

private:
  QVariant
  authorData(SearchViewModelItem *item, const int &role) const;

  QVariant
  bookData(SearchViewModelItem *item, const int &role) const;

  QVariant
  seriesData(SearchViewModelItem *item, const int &role) const;

  QVariant
  genreData(SearchViewModelItem *item, const int &role) const;

  QVariant
  dateData(SearchViewModelItem *item, const int &role) const;

  void
  authorFilter(const QString &filter);

  void
  bookFilter(const QString &filter);

  void
  seriesFilter(const QString &filter);

  void
  genreFilter(const QString &filter);

  void
  dateFilter(const QString &filter);

  std::string
  getRarPath(const UDBElement &path);

  bool
  rarRemove(const UDBElement &path, const std::string &rar_path);

  std::vector<SearchViewModelItem> base;

  UDBElement collection_info;

  Qt::ItemFlags editable = Qt::NoItemFlags;

  std::shared_ptr<GenreBase> genre_base;

  std::shared_ptr<MLBookProc> mlbp;

  std::vector<SearchViewModelItem> filtered;
  bool filter_enabled = false;

  std::tuple<int, Qt::SortOrder> current_sort;

  BaseID bid;
};

#endif // SEARCHVIEWMODEL_H
