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
#ifndef SEARCHRESULTITEMDELEGATE_H
#define SEARCHRESULTITEMDELEGATE_H

#include <QObject>
#include <QPainter>
#include <SettingsManager.h>
#include <StyledItemDelegate.h>

class SearchResultItemDelegate : public StyledItemDelegate
{
public:
  SearchResultItemDelegate(QObject *obj,
                           const std::shared_ptr<SettingsManager> &settings);

  QWidget *
  createEditor(QWidget *parent, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

  void
  setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void
  setModelData(QWidget *editor, QAbstractItemModel *model,
               const QModelIndex &index) const override;

  void
  destroyEditor(QWidget *editor, const QModelIndex &index) const override;
};

#endif // SEARCHRESULTITEMDELEGATE_H
