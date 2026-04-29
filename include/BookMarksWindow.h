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
#ifndef BOOKMARKSWINDOW_H
#define BOOKMARKSWINDOW_H

#include <BookInfo.h>
#include <BookMarksModel.h>
#include <BookmarksKeeper.h>
#include <FormatAnnotation.h>
#include <OpenBook.h>
#include <QPaintEvent>
#include <QWidget>
#include <TableView.h>
#include <condition_variable>
#include <mutex>

class BookMarksWindow : public QWidget
{
  Q_OBJECT
public:
  BookMarksWindow(QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
                  const std::shared_ptr<BookmarksKeeper> &bookmarks);

  virtual ~BookMarksWindow();

  void
  createWindow();

private:
  void
  openBookCallback(const std::filesystem::path &p);

  void
  formTagReplacementTable(
      std::vector<ReplaceTagItem> &replacement_table,
      std::vector<std::tuple<std::string, std::string>> &symbols_replacement);

  void
  bookmarkRemoveDialog(const QModelIndex &index);

  void
  bookSaveDialog(TableView *table);

  void
  saveBookFunc(const QString &save_path, const UDBElement &bsr);

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;
  std::shared_ptr<BookmarksKeeper> bookmarks;

  BookMarksModel *model = nullptr;
  std::shared_ptr<GenreBase> genre_base;

  std::filesystem::path book_open_dir;

  OpenBook *open_book;

  std::shared_ptr<FormatAnnotation> format_annot;

  BookInfo *book_info;

  int thr_num = 0;
  std::mutex thr_num_mtx;
  std::condition_variable thr_num_var;
};

#endif // BOOKMARKSWINDOW_H
