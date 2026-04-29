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
#ifndef COLLECTIONREFRESHINGPROCWINDOW_H
#define COLLECTIONREFRESHINGPROCWINDOW_H

#include <MLBookProc.h>
#include <QCloseEvent>
#include <QLabel>
#include <QPaintEvent>
#include <QProgressBar>
#include <QWidget>
#include <RefreshCollection.h>
#include <atomic>
#include <filesystem>

class CollectionRefreshingProcWindow : public QWidget
{
  Q_OBJECT
public:
  CollectionRefreshingProcWindow(
      QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
      const std::filesystem::path &base_path, const int &threads,
      const bool &fast,
      const std::vector<std::filesystem::path> &refresh_files
      = std::vector<std::filesystem::path>());

  virtual ~CollectionRefreshingProcWindow();

private:
  void
  showWindow();

  void
  startRefreshing();

  void
  closeEvent(QCloseEvent *event) override;

  void
  finishedDialog(const QString &err);

  void
  paintEvent(QPaintEvent *event) override;

  std::filesystem::path base_path;
  bool fast = true;
  std::vector<std::filesystem::path> refresh_files;

  std::atomic<bool> allow_close;

  RefreshCollection *refresh;

  QLabel *files;

  QProgressBar *hashing;
  QProgressBar *refreshing;

  enum Result
  {
    Success,
    Canceled,
    Error
  };

  std::atomic<Result> canceled;

signals:
  void
  signalFilesCollecting(const size_t &);

  void
  signalParsingProcess(const double &processed, const double &total);

  void
  signalFileHashed(const double &processed, const double &total);

  void
  signalFinished(const QString &error);
};

#endif // COLLECTIONREFRESHINGPROCWINDOW_H
