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
#ifndef SERIESEDITWINDOW_H
#define SERIESEDITWINDOW_H

#include <QKeyEvent>
#include <QPaintEvent>
#include <QWidget>
#include <SeriesModel.h>

class SeriesEditWindow : public QWidget
{
  Q_OBJECT
public:
  SeriesEditWindow(QWidget *parent = nullptr);

  virtual ~SeriesEditWindow();

  void
  createWindow(const QModelIndex &index);

  std::vector<UDBElement>
  getSeries();

  bool applied = false;

private:
  void
  keyPressEvent(QKeyEvent *event) override;

  void
  addSeriesDialog();

  void
  paintEvent(QPaintEvent *event) override;

  SeriesModel *model = nullptr;
};

#endif // SERIESEDITWINDOW_H
