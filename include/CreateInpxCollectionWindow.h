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
#ifndef CREATEINPXCOLLECTIONWINDOW_H
#define CREATEINPXCOLLECTIONWINDOW_H

#include <MLBookProc.h>
#include <QLineEdit>
#include <QPaintEvent>
#include <QShowEvent>
#include <QWidget>

class CreateInpxCollectionWindow : public QWidget
{
  Q_OBJECT
public:
  CreateInpxCollectionWindow(QWidget *parent,
                             const std::shared_ptr<MLBookProc> &mlbp);

  void
  createWindow();

private:
  void
  showEvent(QShowEvent *event) override;

  void
  openInpxDialog();

  void
  checkInput();

  enum Error
  {
    CollectionNameEmpty,
    CollectionExists,
    InpxEmpty,
    IncorrectInpx,
    CreationError,
    Success
  };

  void
  errorDialog(const Error &er, const QString &det = QString());

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;

  QLineEdit *collection_name;
  QLineEdit *inpx_path;
};

#endif // CREATEINPXCOLLECTIONWINDOW_H
