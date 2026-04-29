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
#ifndef NOTESMANAGERWINDOW_H
#define NOTESMANAGERWINDOW_H

#include <MLBookProc.h>
#include <NotesKeeper.h>
#include <NotesManagerModel.h>
#include <QPaintEvent>
#include <QWidget>

class NotesManagerWindow : public QWidget
{
  Q_OBJECT
public:
  NotesManagerWindow(QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
                     const std::shared_ptr<NotesKeeper> &notes);

  virtual ~NotesManagerWindow();

  void
  createWindow();

private:
  void
  removeDialog(const QModelIndex &index);

  void
  saveDialog(const QModelIndex &index);

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;
  std::shared_ptr<NotesKeeper> notes;

  NotesManagerModel *model = nullptr;
};

#endif // NOTESMANAGERWINDOW_H
