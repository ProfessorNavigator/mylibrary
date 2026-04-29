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
#ifndef SEARCHVIEWMODELAUTHORS_H
#define SEARCHVIEWMODELAUTHORS_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <QAbstractItemModel>
#include <QObject>
#include <UDBase.h>

class SearchViewModelAuthors : public QAbstractItemModel
{
  Q_OBJECT
public:
  SearchViewModelAuthors(QObject *parent, const UDBase &authors);

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

  QVariant
  headerData(int section, Qt::Orientation orientation,
             int role = Qt::DisplayRole) const override;

  void
  sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  void
  setFilter(const QString &filter);

  void
  removeFilter();

private:
  UDBase authors;

  UDBElement collection_info;

  bool sorted = false;
  Qt::SortOrder sort_order;

  UDBase filtered;
  bool filter_set = false;

  BaseID bid;
};

#endif // SEARCHVIEWMODELAUTHORS_H
