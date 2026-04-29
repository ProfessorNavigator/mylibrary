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

#include <TableView.h>

TableView::TableView(QWidget *parent) : QTableView(parent)
{
}

void
TableView::resizeEvent(QResizeEvent *event)
{
  QTableView::resizeEvent(event);

  emit signalResized(event->size());
}

void
TableView::showEvent(QShowEvent *event)
{
  QTableView::showEvent(event);
  emit signalShowed();
}

void
TableView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
    {
      QPoint pos = event->globalPosition().toPoint();
      emit signalLeftMouseButton(pos);
    }
  QTableView::mousePressEvent(event);
}