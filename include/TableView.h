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
#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QResizeEvent>
#include <QTableView>
#include <QWidget>

class TableView : public QTableView
{
  Q_OBJECT
public:
  TableView(QWidget *parent = nullptr);

signals:
  void
  signalResized(const QSize &new_size);

  void
  signalShowed();

  void
  signalLeftMouseButton(const QPoint &global);

private:
  void
  resizeEvent(QResizeEvent *event) override;

  void
  showEvent(QShowEvent *event) override;

  void
  mousePressEvent(QMouseEvent *event) override;
};

#endif // TABLEVIEW_H
