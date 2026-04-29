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
#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H

#include <BaseID.h>
#include <BookmarksKeeper.h>
#include <GenreBase.h>
#include <QAbstractItemModel>

class BookMarksModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  BookMarksModel(QObject *parent,
                 const std::shared_ptr<BookmarksKeeper> &bookmarks,
                 const std::shared_ptr<GenreBase> &genre_base);

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
  removeBookmark(const QModelIndex &index);

private:
  QVariant
  authorData(const std::vector<UDBElement> &book, int role) const;

  QVariant
  bookData(const std::vector<UDBElement> &book, int role) const;

  QVariant
  seriesData(const std::vector<UDBElement> &book, int role) const;

  QVariant
  genreData(const std::vector<UDBElement> &book, int role) const;

  QVariant
  dateData(const std::vector<UDBElement> &book, int role) const;

  std::shared_ptr<BookmarksKeeper> bookmarks;

  std::shared_ptr<GenreBase> genre_base;

  BaseID bid;
};

#endif // BOOKMARKSMODEL_H
