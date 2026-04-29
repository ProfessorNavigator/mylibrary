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
#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QObject>
#include <QPushButton>

class ColorButton : public QPushButton
{
  Q_OBJECT
public:
  ColorButton();

  void
  setBakcroundColor(const QString &style);

  QString
  getBackGroundColor();

private:
  void
  createButton();

  QWidget *background_w;

  QString current_style;
};

#endif // COLORBUTTON_H
