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
#ifndef AUTHOREDITWINDOW_H
#define AUTHOREDITWINDOW_H

#include <AuthorsModel.h>
#include <BaseID.h>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPaintEvent>
#include <QWidget>
#include <SettingsManager.h>

class AuthorEditWindow : public QWidget
{
  Q_OBJECT
public:
  AuthorEditWindow(QWidget *parent,
                   const std::shared_ptr<SettingsManager> &settings);

  virtual ~AuthorEditWindow();

  void
  createWindow(const QModelIndex &index);

  std::vector<UDBElement>
  getAuthors();

  bool applied = false;

private:
  void
  addAuthorDialog();

  void
  keyPressEvent(QKeyEvent *event) override;

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<SettingsManager> settings;

  QModelIndex index;

  AuthorsModel *model = nullptr;

  BaseID bid;
};

#endif // AUTHOREDITWINDOW_H
