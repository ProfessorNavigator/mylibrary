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
#ifndef COVERWIDGET_H
#define COVERWIDGET_H

#include <BaseID.h>
#include <FormatAnnotation.h>
#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QWidget>
#include <UDBElement.h>
#include <memory>

class CoverWidget : public QWidget
{
  Q_OBJECT
public:
  CoverWidget(QWidget *parent,
              const std::shared_ptr<FormatAnnotation> &format_annotation);

  virtual ~CoverWidget();

  void
  setCover(const UDBElement &cover_obj);

  void
  clearCover();

private:
  void
  paintEvent(QPaintEvent *event) override;

  void
  mousePressEvent(QMouseEvent *event) override;

  void
  saveImageDialog();

  void
  saveImage(const QString &result);

  void
  errorDialog(const std::string &er);

  std::shared_ptr<FormatAnnotation> format_annotation;

  QImage original_image;
  QImage cover;

  QSize current_size;

  bool need_byte_conversion = false;

  std::vector<QAction *> act_list;

  BaseID bid;
};

#endif // COVERWIDGET_H
