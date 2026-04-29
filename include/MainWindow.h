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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Bases.h>
#include <PluginManager.h>
#include <QCloseEvent>
#include <QMainWindow>
#include <QShowEvent>
#include <QSplitter>
#include <SettingsManager.h>
#include <filesystem>
#include <memory>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);

  virtual ~MainWindow();

signals:
  void
  signalCollectionCreated(const std::filesystem::path &base_path);

  void
  signalCollectionRefreshed(const QString &col_name);

private:
  void
  showEvent(QShowEvent *event) override;

  void
  closeEvent(QCloseEvent *event) override;

  void
  loadSizes();

  void
  saveSizes();

  void
  createWindow();

  void
  createMainMenu();

  Bases bases;

  std::shared_ptr<SettingsManager> settings;

  std::shared_ptr<PluginManager> plugins;

signals:
  void
  signalCollectionRemoved(const QString &col_name);
};
#endif // MAINWINDOW_H
