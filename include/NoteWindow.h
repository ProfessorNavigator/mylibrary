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
#ifndef NOTEWINDOW_H
#define NOTEWINDOW_H

#include <BaseID.h>
#include <NotesKeeper.h>
#include <QPaintEvent>
#include <QTextEdit>
#include <QWidget>

class NoteWindow : public QWidget
{
  Q_OBJECT
public:
  NoteWindow(QWidget *parent, const UDBElement &book_search_result,
             const std::shared_ptr<NotesKeeper> &notes);

  void
  createWindow();

private:
  void
  saveDialog();

  void
  paintEvent(QPaintEvent *event) override;

  UDBElement book_search_result;
  std::shared_ptr<NotesKeeper> notes;

  QTextEdit *note_txt;

  BaseID bid;
};

#endif // NOTEWINDOW_H
