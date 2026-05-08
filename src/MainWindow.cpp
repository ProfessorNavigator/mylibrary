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

#include <AboutWindow.h>
#include <AddBooksWindow.h>
#include <BookMarksWindow.h>
#include <CollectionBaseExportWindow.h>
#include <CollectionBaseImportWindow.h>
#include <CreateCollectionWindow.h>
#include <CreateInpxCollectionWindow.h>
#include <MainWindow.h>
#include <MainWindowLeftWidget.h>
#include <MainWindowRightWidget.h>
#include <NotesManagerWindow.h>
#include <PluginsWindow.h>
#include <QApplication>
#include <QDir>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QScreen>
#include <RefreshCollectionWindow.h>
#include <RemoveCollectionWindow.h>
#include <SettingsWindow.h>
#include <filesystem>
#include <fstream>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  bases.mlbp = MLBookProc::create();
  bases.genres_base = std::make_shared<GenreBase>();
  bases.bookmarks = std::make_shared<BookmarksKeeper>(bases.mlbp);
  bases.base_keeper = std::make_shared<BaseKeeper>(bases.mlbp);
  std::string str = QDir::homePath().toStdString();
  std::filesystem::path p = std::u8string(str.begin(), str.end());
  p /= std::u8string(u8".local");
  p /= std::u8string(u8"share");
  p /= std::u8string(u8"MyLibrary");
  p /= std::u8string(u8"BookMarks");
  p /= std::u8string(u8"bookmarks");
  try
    {
      bases.bookmarks->loadBookmarksBase(p);
    }
  catch(std::exception &er)
    {
      std::cout << er.what() << std::endl;
    }

  bases.notes = std::make_shared<NotesKeeper>(bases.mlbp);
  p = std::u8string(str.begin(), str.end());
  p /= std::u8string(u8".local");
  p /= std::u8string(u8"share");
  p /= std::u8string(u8"MyLibrary");
  p /= std::u8string(u8"Notes");
  p /= std::u8string(u8"notes_base");
  try
    {
      bases.notes->loadNotesBase(p);
    }
  catch(std::exception &er)
    {
      std::cout << er.what() << std::endl;
    }

  settings = std::make_shared<SettingsManager>();

  plugins = std::make_shared<PluginManager>(bases);

  this->setObjectName("MainWindow");

  createWindow();
}

MainWindow::~MainWindow()
{
}

void
MainWindow::showEvent(QShowEvent *event)
{
  event->accept();
  loadSizes();
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
  saveSizes();
  event->accept();
}

void
MainWindow::loadSizes()
{
  QSize mw_size;
  mw_size.setHeight(0);
  mw_size.setWidth(0);
  std::string str = QDir::homePath().toStdString();
  std::filesystem::path szpath = std::u8string(str.begin(), str.end());
  szpath /= std::filesystem::path(u8".cache")
            / std::filesystem::path(u8"MyLibrary")
            / std::filesystem::path(u8"main_window_sizes");

  QList<int> mw_splitter_szs;
  std::fstream f;
  f.open(szpath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string str;
      f.seekg(0, std::ios_base::end);
      str.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(str.data(), str.size());
      f.close();

      int32_t val;
      size_t sz_32 = sizeof(val);
      size_t rb = 0;
      size_t str_sz = str.size();
      int count = 0;
      while(rb + sz_32 <= str_sz)
        {
          std::memcpy(&val, &str[rb], sz_32);
          switch(count)
            {
            case 0:
              {
                mw_size.setHeight(static_cast<int>(val));
                break;
              }
            case 1:
              {
                mw_size.setWidth(static_cast<int>(val));
                break;
              }
            default:
              {
                mw_splitter_szs.append(val);
                break;
              }
            }
          rb += sz_32;
          count++;
        }
    }
  QScreen *screen = QGuiApplication::primaryScreen();
  int screen_width = screen->availableSize().width();
  int screen_height = screen->availableSize().height();
  if(mw_size.height() > 0 && mw_size.width() > 0)
    {
      if(mw_size.height() >= screen_height && mw_size.width() >= screen_width)
        {
          this->showMaximized();
        }
      else
        {
          this->resize(mw_size);
        }
    }
  else
    {
      mw_size.setWidth(screen_width * 0.7);
      mw_size.setHeight(screen_height * 0.7);
      this->resize(mw_size);
    }
  if(mw_splitter_szs.size() > 0)
    {
      QWidget *wdg = this->centralWidget();
      QSplitter *cw = dynamic_cast<QSplitter *>(wdg);
      if(cw)
        {
          cw->setSizes(mw_splitter_szs);
        }
    }
}

void
MainWindow::saveSizes()
{
  std::string str = QDir::homePath().toStdString();
  std::filesystem::path szpath = std::u8string(str.begin(), str.end());
  szpath /= std::filesystem::path(u8".cache")
            / std::filesystem::path(u8"MyLibrary")
            / std::filesystem::path(u8"main_window_sizes");
  std::filesystem::create_directories(szpath.parent_path());
  std::filesystem::remove_all(szpath);

  QSize sz = this->size();
  QList<int> sp_sz;
  QWidget *wdg = this->centralWidget();
  QSplitter *spl = dynamic_cast<QSplitter *>(wdg);
  if(spl)
    {
      sp_sz = spl->sizes();
    }
  std::fstream f;
  f.open(szpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      int32_t val = static_cast<int32_t>(sz.height());
      size_t sz_32 = sizeof(val);
      f.write(reinterpret_cast<char *>(&val), sz_32);

      val = static_cast<int32_t>(sz.width());
      f.write(reinterpret_cast<char *>(&val), sz_32);

      for(qsizetype i = 0; i < sp_sz.size(); i++)
        {
          val = static_cast<int32_t>(sp_sz[i]);
          f.write(reinterpret_cast<char *>(&val), sz_32);
        }
      f.close();
    }
}

void
MainWindow::createWindow()
{
  createMainMenu();

  QSplitter *central_widget = new QSplitter;
  this->setCentralWidget(central_widget);

  MainWindowLeftWidget *left_widget = new MainWindowLeftWidget(nullptr, bases);
  central_widget->insertWidget(1, left_widget);

  MainWindowRightWidget *right_widget
      = new MainWindowRightWidget(central_widget, bases, settings);
  central_widget->insertWidget(2, right_widget);

  connect(this, &MainWindow::signalCollectionRemoved, left_widget,
          &MainWindowLeftWidget::removeCollection);
  connect(this, &MainWindow::signalCollectionCreated, left_widget,
          &MainWindowLeftWidget::addCollection);
  connect(left_widget, &MainWindowLeftWidget::signalBookSearchFinished,
          right_widget, &MainWindowRightWidget::setBookSearchResult);
  connect(left_widget, &MainWindowLeftWidget::signalFilesSearchFinished,
          right_widget, &MainWindowRightWidget::setFilesSearchResult);
  connect(left_widget, &MainWindowLeftWidget::signalAuthorsSearchFinished,
          right_widget, &MainWindowRightWidget::setAuthorsSearchResult);
  connect(left_widget, &MainWindowLeftWidget::signalClearSearchResult,
          right_widget, &MainWindowRightWidget::clearSearchResult);
  connect(this, &MainWindow::signalCollectionRefreshed, left_widget,
          &MainWindowLeftWidget::reloadCollection);
  connect(right_widget, &MainWindowRightWidget::signalEditBook, left_widget,
          &MainWindowLeftWidget::editBook);
  connect(right_widget, &MainWindowRightWidget::signalSearchAuthorsBooks,
          left_widget, &MainWindowLeftWidget::searchAuthorsBooks);
  connect(left_widget, &MainWindowLeftWidget::signalBookSearchSepWindow,
          right_widget,
          [right_widget](const UDBase &base, QWidget *spw,
                         const std::filesystem::path &collection_base_path)
            {
              emit right_widget->signalShowBooksWindow(base, spw,
                                                       collection_base_path);
            });
}

void
MainWindow::createMainMenu()
{
  QMenuBar *main_menu = this->menuBar();
  main_menu->setObjectName("MenuBar");

  std::vector<QAction *> list;

  QMenu *menu = main_menu->addMenu(tr("Actions"));
  menu->setObjectName("Menu");
  QAction *action = new QAction;
  action->setText(tr("Create collection"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              CreateCollectionWindow *ccw
                  = new CreateCollectionWindow(this, bases.mlbp, settings);
              ccw->createWindow();
              ccw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction;
  action->setText(tr("Create inpx collection"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              CreateInpxCollectionWindow *inpx
                  = new CreateInpxCollectionWindow(this, bases.mlbp);
              inpx->createWindow();
              inpx->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction;
  action->setText(tr("Remove collection"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              RemoveCollectionWindow *rcw = new RemoveCollectionWindow(this);
              rcw->createWindow();
              connect(rcw, &RemoveCollectionWindow::signalCollectionRemoved,
                      this, &MainWindow::signalCollectionRemoved);
              rcw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction;
  action->setText(tr("Refresh collection"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              RefreshCollectionWindow *rcw
                  = new RefreshCollectionWindow(this, bases.mlbp);
              rcw->createWindow();
              rcw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction;
  action->setText(tr("Add files and/or directories"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              AddBooksWindow *abw
                  = new AddBooksWindow(this, bases.mlbp, settings);
              abw->showWindow();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("Export current collection base (native only)"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              if(bases.base_keeper->getCollectionType() != "native")
                {
                  return void();
                }
              CollectionBaseExportWindow *cbev
                  = new CollectionBaseExportWindow(this, bases.base_keeper);
              cbev->showWindow();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("Import collection base"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              CollectionBaseImportWindow *cbiw
                  = new CollectionBaseImportWindow(this);
              cbiw->createWindow();
              connect(cbiw,
                      &CollectionBaseImportWindow::signalCollectionImported,
                      this,
                      [this](const std::filesystem::path &base_path)
                        {
                          emit signalCollectionCreated(base_path);
                        });
              cbiw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("Reload current collection"));
  connect(action, &QAction ::triggered, this,
          []
            {
              QWidgetList list = qApp->allWidgets();
              for(qsizetype i = 0; i < list.size(); i++)
                {
                  MainWindowLeftWidget *mwlw
                      = dynamic_cast<MainWindowLeftWidget *>(list[i]);
                  if(mwlw != nullptr)
                    {
                      mwlw->reloadCurrentCollection();
                    }
                }
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("Bookmarks"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              BookMarksWindow *bmw = new BookMarksWindow(
                  this, bases.mlbp, bases.bookmarks, settings);
              bmw->createWindow();
              bmw->show();
            });
  main_menu->addAction(action);
  list.push_back(action);

  menu = main_menu->addMenu(tr("Instruments"));
  menu->setObjectName("Menu");

  action = new QAction(tr("Notes manager"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              NotesManagerWindow *nmw = new NotesManagerWindow(
                  this, bases.mlbp, bases.notes, settings);
              nmw->createWindow();
              nmw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("Plugins manager"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              PluginsWindow *pw = new PluginsWindow(this, plugins);
              pw->createWindow();
              pw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("Settings"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              SettingsWindow *sw = new SettingsWindow(this, settings);
              sw->createWindow();
              sw->show();
            });
  main_menu->addAction(action);
  list.push_back(action);

  menu = main_menu->addMenu(tr("About"));
  menu->setObjectName("Menu");

  action = new QAction(tr("About MyLibrary"));
  connect(action, &QAction::triggered, this,
          [this]
            {
              AboutWindow *aw = new AboutWindow(this, bases.mlbp);
              aw->createWindow();
              aw->show();
            });
  menu->addAction(action);
  list.push_back(action);

  action = new QAction(tr("About Qt"));
  connect(action, &QAction::triggered, this,
          []
            {
              qApp->aboutQt();
            });
  menu->addAction(action);
  list.push_back(action);

  list.shrink_to_fit();
  connect(main_menu, &QMenuBar::destroyed,
          [list]
            {
              for(size_t i = 0; i < list.size(); i++)
                {
                  delete list[i];
                }
            });
}
