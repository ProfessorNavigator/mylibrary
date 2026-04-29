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
#ifndef GENREEDITWINDOW_H
#define GENREEDITWINDOW_H

#include <GenreEditModel.h>
#include <QKeyEvent>
#include <QWidget>

class GenreEditWindow : public QWidget
{
  Q_OBJECT
public:
  GenreEditWindow(QWidget *parent = nullptr);

  virtual ~GenreEditWindow();

  void
  createWindow(const QModelIndex &index);

  std::vector<UDBElement>
  getGenres();

  bool applied = false;

private:
  void
  keyPressEvent(QKeyEvent *event) override;

  void
  addGenreDialog();

  GenreEditModel *model = nullptr;
};

#endif // GENREEDITWINDOW_H
