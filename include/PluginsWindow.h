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
#ifndef PLUGINSWINDOW_H
#define PLUGINSWINDOW_H

#include <PluginManager.h>
#include <QPaintEvent>
#include <QWidget>

class PluginsWindow : public QWidget
{
  Q_OBJECT
public:
  PluginsWindow(QWidget *parent,
                const std::shared_ptr<PluginManager> &plugins);

  void
  createWindow();

private:
  QWidget *
  formPluginWidget(const std::shared_ptr<Plugin> &plugin);

  void
  removeConfirmationDialog(const std::shared_ptr<Plugin> &plugin,
                           QWidget *plugin_widget);

  void
  pluginAddDialog();

  void
  paintEvent(QPaintEvent *event) override;

  std::shared_ptr<PluginManager> plugins;

signals:
  void
  signalRemovePlugin(QWidget *plugin_widget);

  void
  signalAddPlugin(const std::shared_ptr<Plugin> &plugin);
};

#endif // PLUGINSWINDOW_H
