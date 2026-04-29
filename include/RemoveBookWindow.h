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
#ifndef REMOVEBOOKWINDOW_H
#define REMOVEBOOKWINDOW_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QWidget>
#include <RemoveBook.h>
#include <UDBElement.h>
#include <atomic>

class RemoveBookWindow : public QWidget
{
  Q_OBJECT
public:
  RemoveBookWindow(QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~RemoveBookWindow();

  void
  showWindow(const UDBElement &book_search_result,
             const std::filesystem::path &base_path);

signals:
  void
  signalBookRemoved(const UDBElement &book_search_result);

private:
  void
  removeProcWindow(const UDBElement &book_search_result,
                   const std::filesystem::path &base_path);

  void
  closeEvent(QCloseEvent *event) override;

  enum ErrorType
  {
    Error,
    Success
  };

  void
  finalDialog(const ErrorType &er, const QString &er_text,
              const UDBElement &book_search_result,
              const std::filesystem::path &base_path);

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;

  BaseID bid;

  std::atomic<bool> allow_destroy;

  RemoveBook *rmb;

signals:
  void
  signalProgress(double processed, double total);

  void
  signalCompleted(const ErrorType &er, const QString &er_text,
                  const UDBElement &book_search_result,
                  const std::filesystem::path &base_path);
};

#endif // REMOVEBOOKWINDOW_H
