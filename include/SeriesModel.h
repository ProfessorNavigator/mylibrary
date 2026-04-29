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
#ifndef SERIESMODEL_H
#define SERIESMODEL_H

#include <BaseID.h>
#include <QAbstractItemModel>
#include <UDBElement.h>

class SeriesModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  SeriesModel(QObject *parent, const std::vector<UDBElement> &series);

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

  bool
  setData(const QModelIndex &index, const QVariant &value,
          int role = Qt::EditRole) override;

  void
  addSeries(const UDBElement &series);

  void
  removeSeries(const QModelIndex &index);

  std::vector<UDBElement>
  getSeries();

private:
  std::vector<UDBElement> series;

  BaseID bid;
};

#endif // SERIESMODEL_H
