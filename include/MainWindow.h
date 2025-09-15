/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <AuxFunc.h>
#include <BookMarks.h>
#include <LeftGrid.h>
#include <NotesKeeper.h>
#include <RightGrid.h>
#include <gtkmm-4.0/gtkmm/applicationwindow.h>
#include <gtkmm-4.0/gtkmm/paned.h>
#include <gtkmm-4.0/gtkmm/popovermenubar.h>
#include <memory>
#include <string>

#ifdef USE_PLUGINS
#include <PluginsKeeper.h>
#endif

class MainWindow : public Gtk::ApplicationWindow
{
public:
  MainWindow(const std::shared_ptr<AuxFunc> &af);

  virtual ~MainWindow();

private:
  void
  formMainWindow();

  void
  createMainMenuActionGroup();

  Gtk::PopoverMenuBar *
  createMainMenu();

  void
  setMainWindowSizes();

  bool
  mainWindowCloseFunc();

  void
  collectionRemoveSlot(const std::string &filename);

  std::string
  getCurrentCollectionName();

  void
  aboutDialog();

  std::string
  loadStyles();

  std::shared_ptr<AuxFunc> af;

  Gtk::Paned *main_pane = nullptr;

  LeftGrid *lg = nullptr;

  RightGrid *rg = nullptr;

  std::shared_ptr<BookMarks> bookmarks;
  std::shared_ptr<NotesKeeper> notes;

  std::filesystem::path mlbookproc_docs_path;
  std::filesystem::path mlpluginifc_docs_path;

  std::filesystem::path temp_background_path;

#ifdef USE_PLUGINS
  std::shared_ptr<PluginsKeeper> plugins_keeper;
#endif
};

#endif // MAINWINDOW_H
