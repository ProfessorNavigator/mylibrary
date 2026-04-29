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

#include <ColorButton.h>
#include <QVBoxLayout>

ColorButton::ColorButton()
{
  createButton();
}

void
ColorButton::setBakcroundColor(const QString &style)
{
  QString str = "background-color: " + style;
  background_w->setStyleSheet(str);
  current_style = style;
}

QString
ColorButton::getBackGroundColor()
{
  return current_style;
}

void
ColorButton::createButton()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  background_w = new QWidget;
  v_box->addWidget(background_w);
}
