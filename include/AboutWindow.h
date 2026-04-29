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
#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <MLBookProc.h>
#include <QPaintEvent>
#include <QWidget>

class AboutWindow : public QWidget
{
  Q_OBJECT
public:
  AboutWindow(QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp);

  void
  createWindow();

private:
  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;
};

#endif // ABOUTWINDOW_H
