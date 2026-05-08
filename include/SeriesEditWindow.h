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
#include <SettingsManager.h>

class SeriesEditWindow : public QWidget
{
  Q_OBJECT
public:
  SeriesEditWindow(QWidget *parent,
                   const std::shared_ptr<SettingsManager> &settings);

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

  std::shared_ptr<SettingsManager> settings;

  SeriesModel *model = nullptr;
};

#endif // SERIESEDITWINDOW_H
