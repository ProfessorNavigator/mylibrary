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

#include <QPainter>
#include <StyledItemDelegate.h>

StyledItemDelegate::StyledItemDelegate(
    QObject *parent, const std::shared_ptr<SettingsManager> &settings)
    : QStyledItemDelegate{ parent }
{
  this->settings = settings;
}

StyledItemDelegate::~StyledItemDelegate()
{
}

void
StyledItemDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  if(opt.state & QStyle::State_Selected)
    {
      QColor color = settings->stringToColor(
          settings
              ->getStyleAttributeValue("Table", "selection-background-color")
              .toStdString());
      painter->fillRect(opt.rect, color);
      color = settings->stringToColor(
          settings->getStyleAttributeValue("Table", "selection-color")
              .toStdString());
      opt.palette.setColor(QPalette::Text, Qt::white);
    }
  QStyledItemDelegate::paint(painter, opt, index);
}
