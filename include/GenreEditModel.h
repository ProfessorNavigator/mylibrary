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
#ifndef GENREEDITMODEL_H
#define GENREEDITMODEL_H

#include <BaseID.h>
#include <GenreBase.h>
#include <QAbstractItemModel>

class GenreEditModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  GenreEditModel(QObject *parent, const std::vector<UDBElement> &genres);

  virtual ~GenreEditModel();

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
  setGenre(const QModelIndex &index, const std::string &genre_code);

  void
  addGenre(const std::string &genre_code);

  void
  removeGenre(const QModelIndex &index);

  std::vector<UDBElement>
  getGenres();

private:
  std::vector<UDBElement> genres;
  GenreBase *genre_base;

  BaseID bid;
};

#endif // GENREEDITMODEL_H
