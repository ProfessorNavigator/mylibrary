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
#ifndef COLLECTIONCREATIONPROCWINDOW_H
#define COLLECTIONCREATIONPROCWINDOW_H

#include <CreateCollection.h>
#include <MLBookProc.h>
#include <QCloseEvent>
#include <QLabel>
#include <QPaintEvent>
#include <QProgressBar>
#include <QWidget>
#include <atomic>
#include <filesystem>
#include <thread>

class CollectionCreationProcWindow : public QWidget
{
  Q_OBJECT
public:
  CollectionCreationProcWindow(QWidget *parent,
                               const std::shared_ptr<MLBookProc> &mlbp,
                               const std::filesystem::path &base_path,
                               const std::vector<std::filesystem::path> &items,
                               const int &threads);

  virtual ~CollectionCreationProcWindow();

  void
  createWindow();

  void
  startCreationProc();

signals:
  void
  signalCollectionCreated(const std::filesystem::path &base_path);

private:
  void
  closeEvent(QCloseEvent *event) override;

  enum Result
  {
    Error,
    Success,
    Canceled
  };

  void
  slotFinished(const Result &variant, const QString &details);

  void
  paintEvent(QPaintEvent *event) override;

  std::filesystem::path base_path;
  std::vector<std::filesystem::path> items;

  CreateCollection *cr_col;  

  QLabel *files;
  QProgressBar *progress; 

  bool finished = false;

  std::atomic<bool> canceled;  

  std::thread *cr_col_thread = nullptr;

signals:
  void
  signalSetFilesNum(const size_t &num);

  void
  signalSetProgress(double val, double max);

  void
  signalFinished(const Result &variant, const QString &details);
};

#endif // COLLECTIONCREATIONPROCWINDOW_H
