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
#ifndef ADDBOOKSWINDOW_H
#define ADDBOOKSWINDOW_H

#include <FilesModel.h>
#include <MLBookProc.h>
#include <QComboBox>
#include <QLineEdit>
#include <QPaintEvent>
#include <QWidget>
#include <SettingsManager.h>

class AddBooksWindow : public QWidget
{
  Q_OBJECT
public:
  AddBooksWindow(QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
                 const std::shared_ptr<SettingsManager> &settings);

  virtual ~AddBooksWindow();

  void
  showWindow();

private:
  void
  addBooks();

  void
  confirmationDialog(const std::filesystem::path &base_path);

  void
  filesAddDialog();

  void
  directoriesAddDialog();

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<MLBookProc> mlbp;
  std::shared_ptr<SettingsManager> settings;

  QComboBox *collections;
  QLineEdit *threads;

  FilesModel *model = nullptr;
};

#endif // ADDBOOKSWINDOW_H
