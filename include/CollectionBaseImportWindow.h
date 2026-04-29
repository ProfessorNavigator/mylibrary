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
#ifndef COLLECTIONBASEIMPORTWINDOW_H
#define COLLECTIONBASEIMPORTWINDOW_H

#include <BaseID.h>
#include <QLabel>
#include <QLineEdit>
#include <QPaintEvent>
#include <QWidget>
#include <filesystem>

class CollectionBaseImportWindow : public QWidget
{
  Q_OBJECT
public:
  CollectionBaseImportWindow(QWidget *parent);

  void
  createWindow();

signals:
  void
  signalCollectionImported(const std::filesystem::path &base_path);

private:
  enum OpenDialogType
  {
    BaseSrc,
    Anchor
  };

  void
  openDialog(const OpenDialogType &type);

  void
  checkResult();

  enum ErrorType
  {
    ColNameEmpty,
    CollectionExists,
    AnchorPathEmpty,
    AnchorNotExists,
    AnchorNamesNotEqual,
    ImportError,
    Success
  };

  void
  errorDialog(const ErrorType &er, const QString &txt = QString());

  void
  paintEvent(QPaintEvent *event) override;

  QLineEdit *collection_name;
  QLineEdit *src_path;
  QLineEdit *anchor_path;

  QLabel *anchor_fnm;

  BaseID::ID anchor_object_type;
};

#endif // COLLECTIONBASEIMPORTWINDOW_H
