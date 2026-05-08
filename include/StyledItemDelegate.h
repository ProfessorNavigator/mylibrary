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
#ifndef STYLEDITEMDELEGATE_H
#define STYLEDITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <SettingsManager.h>

class StyledItemDelegate : public QStyledItemDelegate
{
public:
  StyledItemDelegate(QObject *parent,
                     const std::shared_ptr<SettingsManager> &settings);

  virtual ~StyledItemDelegate();

  virtual void
  paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

protected:
  std::shared_ptr<SettingsManager> settings;
};

#endif // STYLEDITEMDELEGATE_H
