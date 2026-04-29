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
#ifndef CREATECOLLECTIONWINDOW_H
#define CREATECOLLECTIONWINDOW_H

#include <FilesModel.h>
#include <MLBookProc.h>
#include <QLineEdit>
#include <QPaintEvent>
#include <QWidget>

class CreateCollectionWindow : public QWidget
{
  Q_OBJECT
public:
  CreateCollectionWindow(QWidget *parent,
                         const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~CreateCollectionWindow();

  void
  createWindow();

private:
  void
  filesAddDialog();

  void
  directoriesAddDialog();

  void
  checkInput();

  enum Error
  {
    CollectionNameEmpty,
    CollectionExists
  };

  void
  errorDialog(const Error &er);

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;

  QLineEdit *collection_name;
  QLineEdit *threads;

  FilesModel *model = nullptr;
};

#endif // CREATECOLLECTIONWINDOW_H
