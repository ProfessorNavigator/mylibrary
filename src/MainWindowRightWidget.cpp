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

#include <BookDetailsWindow.h>
#include <MainWindowRightWidget.h>
#include <NoteWindow.h>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QScrollBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>
#include <RemoveBookWindow.h>
#include <SearchProcWindow.h>
#include <SearchResultItemDelegate.h>
#include <SearchViewModel.h>
#include <SearchViewModelAuthors.h>
#include <SearchViewModelFiles.h>
#include <atomic>
#include <iostream>
#include <thread>

MainWindowRightWidget::MainWindowRightWidget(QWidget *parent,
                                             const Bases &bases)
    : QWidget(parent)
{
  this->bases = bases;
  book_info = new BookInfo(bases.mlbp);
  open_book = new OpenBook(bases.mlbp);

  book_open_dir
      = bases.mlbp->tempDirPath() / std::filesystem::path(u8"MyLibrary");

  format_annotation = std::make_shared<FormatAnnotation>(bases.mlbp);
  std::vector<ReplaceTagItem> rptbl;
  std::vector<std::tuple<std::string, std::string>> symrpl;
  formTagReplacementTable(rptbl, symrpl);
  format_annotation->setTagReplacementTable(rptbl, symrpl);

  createWidget();
}

MainWindowRightWidget::~MainWindowRightWidget()
{
  std::unique_lock<std::mutex> ullock(thr_num_mtx);
  thr_num_var.wait(ullock,
                   [this]
                     {
                       return thr_num <= 0;
                     });
  delete book_info;
  delete open_book;
  std::filesystem::remove_all(book_open_dir);
}

void
MainWindowRightWidget::setBookSearchResult(
    const UDBase &search_result, QWidget *spw,
    const std::filesystem::path &collection_base_path)
{
  if(search_result_resize)
    {
      disconnect(search_result_resize);
    }
  current_type = SearchResultType::Books;
  search_view->setSortingEnabled(false);

  this->collection_base_path = collection_base_path;

  QAbstractItemDelegate *delegate = search_view->itemDelegate();
  SearchResultItemDelegate *search_delegate = new SearchResultItemDelegate;
  search_view->setItemDelegate(search_delegate);
  delete delegate;

  connect(search_view, &TableView::destroyed, search_delegate,
          [search_delegate]
            {
              delete search_delegate;
            });

  QAbstractItemModel *prev_model = search_view->model();
  SearchViewModel *model = new SearchViewModel(nullptr, search_result,
                                               bases.genres_base, bases.mlbp);
  SearchProcWindow *spw_l = dynamic_cast<SearchProcWindow *>(spw);
  if(spw_l != nullptr)
    {
      connect(model, &SearchViewModel::modelReset, spw_l,
              [spw_l]
                {
                  spw_l->allowClose(true);
                  spw_l->close();
                });
    }

  connect(model, &SearchViewModel::signalEditBook, this,
          [this](const UDBElement &el)
            {
              emit signalEditBook(el);
            });

  search_view->setModel(model);
  connect(search_view, &TableView::destroyed, model,
          [model]
            {
              delete model;
            });
  delete prev_model;

  search_view_width = search_view->width();

  search_view->setSortingEnabled(true);
  resizeColumns();

  search_result_resize
      = connect(search_view, &TableView::signalResized, this,
                [this](const QSize &sz)
                  {
                    search_view_width = sz.width();
                    resizeColumns();
                  });

  setFiltersBook();

  filter_string->setText("");

  QScrollBar *v_bar = search_view->verticalScrollBar();
  v_bar->setValue(v_bar->minimum());

  annotation->setText("");
  cover->clearCover();

  QList<QAction *> actions = operations->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      operations->removeAction(actions[i]);
    }

  actions = search_view->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      search_view->removeAction(actions[i]);
      delete actions[i];
    }

  actions = bookActions(model->getCollectionEditable());
  search_view->addActions(actions);
  operations->addActions(actions);
  operations->update();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      QAction *act = actions[i];
      connect(search_view, &TableView::destroyed, act,
              [act]
                {
                  delete act;
                });
    }
  if(search_result_doubleclicked)
    {
      disconnect(search_result_doubleclicked);
    }
  search_result_doubleclicked = connect(
      search_view, &TableView::doubleClicked, this,
      [this](const QModelIndex &index)
        {
          if(index.isValid())
            {
              const SearchViewModelItem *el
                  = reinterpret_cast<const SearchViewModelItem *>(
                      index.constInternalPointer());
              if(el != nullptr)
                {
                  std::filesystem::remove_all(book_open_dir);
                  try
                    {
                      open_book->openBook(
                          el->book_search_result, book_open_dir,
                          std::bind(&MainWindowRightWidget::openBookCallback,
                                    this, std::placeholders::_1));
                    }
                  catch(std::exception &er)
                    {
                      std::cout
                          << "MainWindowRightWidget::setBookSearchResult: \""
                          << er.what() << "\"" << std::endl;
                    }
                }
            }
        });

  if(search_result_singleclicked)
    {
      disconnect(search_result_singleclicked);
    }
  search_result_singleclicked
      = connect(search_view, &TableView::clicked, this,
                &MainWindowRightWidget::getBookInfo);

  search_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void
MainWindowRightWidget::setFilesSearchResult(
    const UDBase &search_result, QWidget *spw,
    const std::filesystem::path &collection_base_path)
{
  if(search_result_resize)
    {
      disconnect(search_result_resize);
    }
  search_view->setSortingEnabled(false);
  current_type = SearchResultType::Files;
  this->collection_base_path = collection_base_path;

  QAbstractItemDelegate *delegate = search_view->itemDelegate();
  QStyledItemDelegate *search_delegate = new QStyledItemDelegate;
  search_view->setItemDelegate(search_delegate);
  delete delegate;

  connect(search_view, &TableView::destroyed, search_delegate,
          [search_delegate]
            {
              delete search_delegate;
            });

  QAbstractItemModel *prev_model = search_view->model();
  SearchViewModelFiles *model
      = new SearchViewModelFiles(nullptr, search_result, bases.mlbp);
  SearchProcWindow *spw_l = dynamic_cast<SearchProcWindow *>(spw);
  if(spw_l != nullptr)
    {
      connect(model, &SearchViewModelFiles::modelReset, spw_l,
              [spw_l]
                {
                  spw_l->allowClose(true);
                  spw_l->close();
                });
    }
  search_view->setModel(model);
  connect(search_view, &TableView::destroyed, model,
          [model]
            {
              delete model;
            });
  delete prev_model;

  search_view_width = search_view->width();
  search_view->setSortingEnabled(true);
  resizeColumns();

  search_result_resize
      = connect(search_view, &TableView::signalResized, this,
                [this](const QSize &sz)
                  {
                    search_view_width = sz.width();
                    resizeColumns();
                  });

  setFiltersFiles();

  filter_string->setText("");

  QScrollBar *v_bar = search_view->verticalScrollBar();
  v_bar->setValue(v_bar->minimum());

  annotation->setText("");
  cover->clearCover();

  QList<QAction *> actions = operations->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      operations->removeAction(actions[i]);
    }

  actions = search_view->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      search_view->removeAction(actions[i]);
      delete actions[i];
    }

  actions = fileActions();
  search_view->addActions(actions);
  operations->addActions(actions);
  operations->update();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      QAction *act = actions[i];
      connect(search_view, &TableView::destroyed, act,
              [act]
                {
                  delete act;
                });
    }
  if(search_result_doubleclicked)
    {
      disconnect(search_result_doubleclicked);
    }

  if(search_result_singleclicked)
    {
      disconnect(search_result_singleclicked);
    }
}

void
MainWindowRightWidget::setAuthorsSearchResult(
    const UDBase &search_result, QWidget *spw,
    const std::filesystem::path &collection_base_path)
{
  if(search_result_resize)
    {
      disconnect(search_result_resize);
    }
  search_view->setSortingEnabled(false);
  current_type = SearchResultType::Authors;
  this->collection_base_path = collection_base_path;

  QAbstractItemDelegate *delegate = search_view->itemDelegate();
  QStyledItemDelegate *search_delegate = new QStyledItemDelegate;
  search_view->setItemDelegate(search_delegate);
  delete delegate;

  connect(search_view, &TableView::destroyed, search_delegate,
          [search_delegate]
            {
              delete search_delegate;
            });

  QAbstractItemModel *prev_model = search_view->model();
  SearchViewModelAuthors *model
      = new SearchViewModelAuthors(nullptr, search_result);

  SearchProcWindow *spw_l = dynamic_cast<SearchProcWindow *>(spw);
  if(spw_l != nullptr)
    {
      connect(model, &SearchViewModelFiles::modelReset, spw_l,
              [spw_l]
                {
                  spw_l->allowClose(true);
                  spw_l->close();
                });
    }
  search_view->setModel(model);
  connect(search_view, &TableView::destroyed, model,
          [model]
            {
              delete model;
            });
  delete prev_model;

  search_view_width = search_view->width();
  search_view->setSortingEnabled(true);
  resizeColumns();

  search_result_resize
      = connect(search_view, &TableView::signalResized, this,
                [this](const QSize &sz)
                  {
                    search_view_width = sz.width();
                    resizeColumns();
                  });

  setFiltersAuthors();

  filter_string->setText("");

  QScrollBar *v_bar = search_view->verticalScrollBar();
  v_bar->setValue(v_bar->minimum());

  annotation->setText("");
  cover->clearCover();

  QList<QAction *> actions = operations->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      operations->removeAction(actions[i]);
    }

  actions = search_view->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      search_view->removeAction(actions[i]);
      delete actions[i];
    }

  actions = authorActions();

  search_view->addActions(actions);
  operations->addActions(actions);
  operations->update();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      QAction *act = actions[i];
      connect(search_view, &TableView::destroyed, act,
              [act]
                {
                  delete act;
                });
    }
  if(search_result_doubleclicked)
    {
      disconnect(search_result_doubleclicked);
    }

  if(search_result_singleclicked)
    {
      disconnect(search_result_singleclicked);
    }
}

void
MainWindowRightWidget::createWidget()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  search_view = new TableView;
  search_view->setObjectName("Table");
  search_view->viewport()->setObjectName("TableViewport");
  search_view_width = search_view->sizeHint().width();
  search_view->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(search_view, &TableView::customContextMenuRequested,
          [this](const QPoint &pos)
            {
              QModelIndex index = search_view->indexAt(pos);
              search_view->setCurrentIndex(index);
              if(index.isValid())
                {
                  QMenu *menu = new QMenu(search_view);
                  menu->setObjectName("Menu");
                  menu->setAttribute(Qt::WA_DeleteOnClose);

                  menu->addActions(search_view->actions());

                  menu->popup(search_view->viewport()->mapToGlobal(pos));
                }
            });
  connect(search_view, &TableView::signalLeftMouseButton, this,
          [this](const QPoint &glob)
            {
              QPoint pos = search_view->viewport()->mapFromGlobal(glob);
              search_view->setCurrentIndex(search_view->indexAt(pos));
            });
  v_box->addWidget(search_view, 70);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  operations = new QPushButton;
  operations->setText(tr("Operations"));
  QGraphicsDropShadowEffect *shadow
      = new QGraphicsDropShadowEffect(operations);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  operations->setGraphicsEffect(shadow);
  operations->setObjectName("ComboBox");
  connect(operations, &QPushButton::clicked, this,
          [this]
            {
              QList<QAction *> actions = operations->actions();
              if(actions.size() > 0)
                {
                  QMenu *menu = new QMenu(operations);
                  menu->setObjectName("Menu");
                  menu->setAttribute(Qt::WA_DeleteOnClose);

                  menu->addActions(actions);

                  QPoint pos = operations->pos();
                  pos = operations->parentWidget()->mapToGlobal(pos);
                  pos.setY(pos.y() + operations->height());

                  menu->popup(pos);
                }
            });
  h_box->addWidget(operations, 0, Qt::AlignCenter);

  QFrame *separator = new QFrame;
  separator->setFrameShape(QFrame::VLine);
  separator->setFrameShadow(QFrame::Sunken);
  h_box->addWidget(separator);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Filter") + QString(":"));
  h_box->addWidget(lab);

  filter_type = new QComboBox;
  shadow = new QGraphicsDropShadowEffect(filter_type);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  filter_type->setGraphicsEffect(shadow);
  filter_type->setObjectName("ComboBox");
  setFiltersBook();
  h_box->addWidget(filter_type);

  filter_string = new QLineEdit;
  filter_string->setObjectName("LineEdit");
  connect(filter_string, &QLineEdit::editingFinished, this,
          &MainWindowRightWidget::setFilter);
  h_box->addWidget(filter_string);

  QPushButton *apply_filter = new QPushButton;
  shadow = new QGraphicsDropShadowEffect(apply_filter);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  apply_filter->setGraphicsEffect(shadow);
  apply_filter->setObjectName("ApplyButton");
  apply_filter->setText(tr("Apply filter"));
  connect(apply_filter, &QPushButton::clicked, this,
          &MainWindowRightWidget::setFilter);
  h_box->addWidget(apply_filter);

  QPushButton *remove_filter = new QPushButton;
  shadow = new QGraphicsDropShadowEffect(remove_filter);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove_filter->setGraphicsEffect(shadow);
  remove_filter->setObjectName("ClearButton");
  remove_filter->setText(tr("Remove filter"));
  connect(remove_filter, &QPushButton::clicked, this,
          &MainWindowRightWidget::removeFilter);
  h_box->addWidget(remove_filter);

  h_box->addStretch();

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box, 25);

  annotation = new QTextBrowser;
  annotation->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(annotation, &QTextBrowser::customContextMenuRequested, this,
          [this](const QPoint &pos)
            {
              QMenu *menu = annotation->createStandardContextMenu();
              if(menu != nullptr)
                {
                  menu->setAttribute(Qt::WA_DeleteOnClose);
                  menu->setObjectName("Menu");
                  menu->popup(annotation->viewport()->mapToGlobal(pos));
                }
            });
  annotation->setObjectName("TextEdit");
  annotation->setTextInteractionFlags(Qt::TextBrowserInteraction);
  annotation->setOpenExternalLinks(true);
  h_box->addWidget(annotation, 85);

  cover = new CoverWidget(nullptr, format_annotation);
  h_box->addWidget(cover, 15);

  connect(this, &MainWindowRightWidget::signalShowBooks, this,
          &MainWindowRightWidget::setBookSearchResult);

  connect(this, &MainWindowRightWidget::signalShowBooksWindow, this,
          [this](const UDBase &search_result, QWidget *spw,
                 const std::filesystem::path &collection_base_path)
            {
              MainWindowRightWidget *win
                  = new MainWindowRightWidget(this->window(), bases);
              win->setWindowFlag(Qt::Window, true);
              win->setObjectName("Window");
              win->setAttribute(Qt::WA_DeleteOnClose);
              win->resize(this->size());
              win->show();
              win->setBookSearchResult(search_result, spw,
                                       collection_base_path);
            });
}

void
MainWindowRightWidget::resizeColumns()
{
  QAbstractItemModel *model = search_view->model();
  if(model == nullptr)
    {
      return void();
    }
  int columns = model->columnCount();
  QHeaderView *hh = search_view->horizontalHeader();
  int width = search_view_width;
  switch(current_type)
    {
    case SearchResultType::Books:
      {
        int w = 0;
        for(int i = 0; i < columns; i++)
          {
            switch(i)
              {
              case 0:
                {
                  int val = width * 0.3;
                  hh->resizeSection(i, val);
                  w += val;
                  break;
                }
              case 1:
                {
                  int val = width * 0.3;
                  hh->resizeSection(i, val);
                  w += val;
                  break;
                }
              case 2:
                {
                  int val = width * 0.15;
                  hh->resizeSection(i, val);
                  w += val;
                  break;
                }
              case 3:
                {
                  int val = width * 0.15;
                  hh->resizeSection(i, val);
                  w += val;
                  break;
                }
              case 4:
                {
                  int val = width - w;
                  hh->resizeSection(i, val);
                  break;
                }
              default:
                break;
              }
          }
        break;
      }
    case SearchResultType::Files:
    case SearchResultType::Authors:
      {
        hh->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
        break;
      }
    default:
      break;
    }
  search_view->resizeRowsToContents();
}

void
MainWindowRightWidget::setFiltersBook()
{
  filter_type->clear();
  filter_type->addItem(tr("Author"), QVariant(int(0)));
  filter_type->addItem(tr("Book"), QVariant(int(1)));
  filter_type->addItem(tr("Series"), QVariant(int(2)));
  filter_type->addItem(tr("Genre"), QVariant(int(3)));
  filter_type->addItem(tr("Date"), QVariant(int(4)));
}

void
MainWindowRightWidget::setFiltersFiles()
{
  filter_type->clear();
  filter_type->addItem(tr("File path"), QVariant(int(0)));
}

void
MainWindowRightWidget::setFiltersAuthors()
{
  filter_type->clear();
  filter_type->addItem(tr("Author"), QVariant(int(0)));
}

void
MainWindowRightWidget::setFilter()
{
  switch(current_type)
    {
    case SearchResultType::Books:
      {
        SearchViewModel *sv_model
            = dynamic_cast<SearchViewModel *>(search_view->model());
        if(sv_model != nullptr)
          {
            QString filter = filter_string->text();
            int val = filter_type->currentData().toInt();
            sv_model->setFilter(filter, val);
          }
        break;
      }
    case SearchResultType::Files:
      {
        SearchViewModelFiles *sv_model
            = dynamic_cast<SearchViewModelFiles *>(search_view->model());
        if(sv_model != nullptr)
          {
            QString filter = filter_string->text();
            sv_model->setFilter(filter);
          }
        break;
      }
    case SearchResultType::Authors:
      {
        SearchViewModelAuthors *sv_model
            = dynamic_cast<SearchViewModelAuthors *>(search_view->model());
        if(sv_model != nullptr)
          {
            QString filter = filter_string->text();
            sv_model->setFilter(filter);
          }
        break;
      }
    default:
      break;
    }
}

void
MainWindowRightWidget::removeFilter()
{
  filter_string->setText("");
  switch(current_type)
    {
    case SearchResultType::Books:
      {
        SearchViewModel *sv_model
            = dynamic_cast<SearchViewModel *>(search_view->model());
        if(sv_model != nullptr)
          {
            sv_model->removeFilter();
          }
        break;
      }
    case SearchResultType::Files:
      {
        SearchViewModelFiles *sv_model
            = dynamic_cast<SearchViewModelFiles *>(search_view->model());
        if(sv_model != nullptr)
          {
            sv_model->removeFilter();
          }
        break;
      }
    case SearchResultType::Authors:
      {
        SearchViewModelAuthors *sv_model
            = dynamic_cast<SearchViewModelAuthors *>(search_view->model());
        if(sv_model != nullptr)
          {
            sv_model->removeFilter();
          }
        break;
      }
    default:
      break;
    }
}

QList<QAction *>
MainWindowRightWidget::bookActions(const Qt::ItemFlags &editable)
{
  QList<QAction *> result;

  QAction *act = new QAction(tr("Open"));
  connect(
      act, &QAction::triggered, this,
      [this]
        {
          QModelIndex index = search_view->currentIndex();
          if(index.isValid())
            {
              const SearchViewModelItem *el
                  = reinterpret_cast<const SearchViewModelItem *>(
                      index.constInternalPointer());
              if(el != nullptr)
                {
                  std::filesystem::remove_all(book_open_dir);
                  try
                    {
                      open_book->openBook(
                          el->book_search_result, book_open_dir,
                          std::bind(&MainWindowRightWidget::openBookCallback,
                                    this, std::placeholders::_1));
                    }
                  catch(std::exception &er)
                    {
                      std::cout << "MainWindowRightWidget::bookActions: \""
                                << er.what() << "\"" << std::endl;
                    }
                }
            }
        });
  result.append(act);

  act = new QAction(tr("Book info"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              QModelIndex index = search_view->currentIndex();
              if(index.isValid())
                {
                  const SearchViewModelItem *el
                      = reinterpret_cast<const SearchViewModelItem *>(
                          index.constInternalPointer());
                  if(el != nullptr)
                    {
                      UDBase info;
                      try
                        {
                          info
                              = book_info->getBookInfo(el->book_search_result);
                        }
                      catch(std::exception &er)
                        {
                          std::cout << "MainWindowRightWidget::bookActions: \""
                                    << er.what() << "\"" << std::endl;
                          return void();
                        }

                      BookDetailsWindow *bdw = new BookDetailsWindow(
                          this->window(), info, el->book_search_result,
                          format_annotation, bases.genres_base);
                      bdw->createWindow();
                      bdw->show();
                    }
                }
            });
  result.append(act);

  act = new QAction(tr("Add to bookmarks"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              QModelIndex index = search_view->currentIndex();
              if(index.isValid())
                {
                  const SearchViewModelItem *bsr
                      = reinterpret_cast<const SearchViewModelItem *>(
                          index.constInternalPointer());
                  if(bsr != nullptr)
                    {
                      UDBElement book_search_result = bsr->book_search_result;
                      addBookToBookMarks(book_search_result);
                    }
                }
            });
  result.append(act);

  act = new QAction(tr("Save as..."));
  connect(act, &QAction::triggered, this,
          &MainWindowRightWidget::bookSaveDialog);
  result.append(act);

  act = new QAction(tr("Notes"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              QModelIndex index = search_view->currentIndex();
              if(!index.isValid())
                {
                  return void();
                }
              const SearchViewModelItem *bsr
                  = reinterpret_cast<const SearchViewModelItem *>(
                      index.constInternalPointer());
              if(bsr == nullptr)
                {
                  return void();
                }
              NoteWindow *nw = new NoteWindow(
                  this->window(), bsr->book_search_result, bases.notes);
              nw->createWindow();
              nw->show();
            });
  result.append(act);

  if(editable & Qt::ItemIsEditable)
    {
      act = new QAction(tr("Edit"));
      connect(act, &QAction::triggered, this,
              [this]
                {
                  QModelIndex index = search_view->currentIndex();
                  if(index.isValid())
                    {
                      search_view->edit(index);
                    }
                });
      result.append(act);

      act = new QAction(tr("Remove book"));
      connect(act, &QAction::triggered, this,
              [this]
                {
                  QModelIndex index = search_view->currentIndex();
                  if(index.isValid())
                    {
                      const SearchViewModelItem *el
                          = reinterpret_cast<const SearchViewModelItem *>(
                              index.constInternalPointer());
                      if(el != nullptr)
                        {
                          UDBElement to_remove(el->book_search_result);
                          RemoveBookWindow *rbw = new RemoveBookWindow(
                              this->window(), bases.mlbp);
                          connect(rbw, &RemoveBookWindow::signalBookRemoved,
                                  this,
                                  [this](const UDBElement &el)
                                    {
                                      SearchViewModel *mdl
                                          = dynamic_cast<SearchViewModel *>(
                                              search_view->model());
                                      if(mdl != nullptr)
                                        {
                                          mdl->removeBook(el);
                                        }
                                    });
                          rbw->showWindow(to_remove, collection_base_path);
                        }
                    }
                });
      result.append(act);
    }

  return result;
}

QList<QAction *>
MainWindowRightWidget::fileActions()
{
  QList<QAction *> result;

  QAction *act = new QAction(tr("Show books"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              showBooks(false);
            });
  result.append(act);

  act = new QAction(tr("Show books in separate window"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              showBooks(true);
            });
  result.append(act);

  return result;
}

QList<QAction *>
MainWindowRightWidget::authorActions()
{
  QList<QAction *> result;

  QAction *act = new QAction(tr("Show books"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              QModelIndex index = search_view->currentIndex();
              if(!index.isValid())
                {
                  return void();
                }
              const UDBElement *el_ptr = reinterpret_cast<const UDBElement *>(
                  index.constInternalPointer());
              if(el_ptr == nullptr)
                {
                  return void();
                }

              UDBElement asr = *el_ptr;
              emit signalSearchAuthorsBooks(asr, false);
            });
  result.append(act);

  act = new QAction(tr("Show books in separate window"));
  connect(act, &QAction::triggered, this,
          [this]
            {
              QModelIndex index = search_view->currentIndex();
              if(!index.isValid())
                {
                  return void();
                }
              const UDBElement *el_ptr = reinterpret_cast<const UDBElement *>(
                  index.constInternalPointer());
              if(el_ptr == nullptr)
                {
                  return void();
                }

              UDBElement asr = *el_ptr;
              emit signalSearchAuthorsBooks(asr, true);
            });
  result.append(act);

  return result;
}

void
MainWindowRightWidget::getBookInfo(const QModelIndex &index)
{
  annotation->setText("");
  cover->clearCover();
  if(!index.isValid())
    {
      return void();
    }
  const SearchViewModelItem *el
      = reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer());

  QScreen *screen = this->window()->windowHandle()->screen();
  book_info->setDPI(static_cast<double>(screen->physicalDotsPerInchX()),
                    static_cast<double>(screen->physicalDotsPerInchY()));

  UDBase info;
  try
    {
      info = std::move(book_info->getBookInfo(el->book_search_result));
    }
  catch(std::exception &er)
    {
      std::cout << "MainWindowRightWidget::getBookInfo: \"" << er.what()
                << "\"" << std::endl;
      return void();
    }

  std::vector<UDBElement> *raw_base = info.getRawBase();
  std::string::size_type n;
  std::string find_str("l:href");
  std::string find_str2("<br><br><br>");
  std::string ins_start("<annotation>");
  std::string ins_end("</annotation>");
  for(auto it = raw_base->begin(); it != raw_base->end(); it++)
    {
      if(bid.getId(*it) == BaseID::Annotation)
        {
          format_annotation->replaceTags(it->content);
          it->content.insert(it->content.begin(), ins_start.begin(),
                             ins_start.end());
          std::copy(ins_end.begin(), ins_end.end(),
                    std::back_inserter(it->content));
          format_annotation->replaceTags(it->content);
          format_annotation->removeEscapeSequences(it->content);
          format_annotation->finalCleaning(it->content);
          n = 0;
          for(;;)
            {
              n = it->content.find(find_str, n);
              if(n == std::string::npos)
                {
                  break;
                }
              it->content.erase(it->content.begin() + n,
                                it->content.begin() + n + 2);
            }
          n = 0;
          for(;;)
            {
              n = it->content.find(find_str2, n);
              if(n == std::string::npos)
                {
                  break;
                }
              it->content.erase(it->content.begin() + n,
                                it->content.begin() + n
                                    + find_str2.size() / 3);
            }
          annotation->setHtml(it->content.c_str());
          annotation->setAlignment(Qt::AlignJustify);
        }
      else if(bid.getId(*it) == BaseID::CoverPage)
        {
          cover->setCover(*it);
          cover->update();
        }
    }
}

void
MainWindowRightWidget::formTagReplacementTable(
    std::vector<ReplaceTagItem> &replacement_table,
    std::vector<std::tuple<std::string, std::string>> &symbols_replacement)
{
  ReplaceTagItem tag;

  tag.tag_to_replace = "p";
  tag.begin_replacement = "";
  tag.end_replacement = "<br><br>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "empty-line";
  tag.begin_replacement = "";
  tag.end_replacement = "<br>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "sub";
  tag.begin_replacement
      = "<span style=\"vertical-align: sub; font-size: smaller;\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "sup";
  tag.begin_replacement
      = "<span style=\"vertical-align: super; font-size: smaller;\">";
  tag.end_replacement = "</span>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "strong";
  tag.begin_replacement = "<b>";
  tag.end_replacement = "</b>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "emphasis";
  tag.begin_replacement = "<em>";
  tag.end_replacement = "</em>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "strikethrough";
  tag.begin_replacement = "<s>";
  tag.end_replacement = "</s>";
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "a";
  tag.replace = false;
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "span";
  tag.replace = false;
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "br";
  tag.replace = false;
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "b";
  tag.replace = false;
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "em";
  tag.replace = false;
  replacement_table.emplace_back(tag);

  tag.tag_to_replace = "s";
  tag.replace = false;
  replacement_table.emplace_back(tag);

  replacement_table.shrink_to_fit();

  symbols_replacement.push_back(std::make_tuple("<", "&lt;"));
  symbols_replacement.push_back(std::make_tuple(">", "&gt;"));

  symbols_replacement.shrink_to_fit();
}

void
MainWindowRightWidget::openBookCallback(const std::filesystem::path &p)
{
  QString str(p.u8string().c_str());
  QDesktopServices::openUrl(QUrl::fromLocalFile(str));
}

void
MainWindowRightWidget::bookSaveDialog()
{
  QModelIndex index = search_view->currentIndex();
  if(!index.isValid())
    {
      return void();
    }

  const SearchViewModelItem *bsr
      = reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer());
  if(bsr == nullptr)
    {
      return void();
    }

  auto it_book = std::find_if(bsr->book_search_result.subelements.begin(),
                              bsr->book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == bsr->book_search_result.subelements.end())
    {
      return void();
    }

  QString suffix;
  QString file_name;
  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  if(it_path == it_book->subelements.end())
    {
      auto it_fl = std::find_if(bsr->book_search_result.subelements.begin(),
                                bsr->book_search_result.subelements.end(),
                                [this](const UDBElement &el)
                                  {
                                    return bid.getId(el) == BaseID::File;
                                  });
      if(it_fl == bsr->book_search_result.subelements.end())
        {
          return void();
        }
      suffix = bases.mlbp->getExtension(it_fl->content).c_str();
      std::filesystem::path p
          = std::u8string(it_fl->content.begin(), it_fl->content.end());
      std::u8string u8str = p.stem().u8string();
      file_name = std::string(u8str.begin(), u8str.end()).c_str();
    }
  else
    {
      for(;;)
        {
          auto it_p = std::find_if(
              it_path->subelements.begin(), it_path->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::PathInFile;
                });
          if(it_p == it_path->subelements.end())
            {
              suffix = bases.mlbp->getExtension(it_path->content).c_str();
              std::filesystem::path p = std::u8string(it_path->content.begin(),
                                                      it_path->content.end());
              std::u8string u8str = p.stem().u8string();
              file_name = std::string(u8str.begin(), u8str.end()).c_str();
              break;
            }
          else
            {
              it_path = it_p;
            }
        }
    }

  QFileDialog *fd = new QFileDialog(this->window());
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptSave);
  fd->setNameFilter(QString("*") + suffix);
  fd->setDefaultSuffix(suffix);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::AnyFile);
  fd->selectFile(file_name);

  connect(fd, &QFileDialog::fileSelected, this,
          [bsr, this](const QString &file)
            {
              UDBElement lbsr = bsr->book_search_result;
              std::lock_guard<std::mutex> lglock(thr_num_mtx);
              thr_num++;
              std::thread thr(
                  [this, file, lbsr]
                    {
                      saveBookFunc(file, lbsr);
                      std::lock_guard<std::mutex> lglock(thr_num_mtx);
                      thr_num--;
                      thr_num_var.notify_all();
                    });
              thr.detach();
            });

  fd->show();
}

void
MainWindowRightWidget::saveBookFunc(const QString &save_path,
                                    const UDBElement &bsr)
{
  std::string str = save_path.toStdString();
  std::filesystem::path sp = std::u8string(str.begin(), str.end());

  std::shared_ptr<std::filesystem::path> tmp_path(
      new std::filesystem::path,
      [](std::filesystem::path *p)
        {
          std::filesystem::remove_all(*p);
          delete p;
        });
  *tmp_path = bases.mlbp->tempDirPath() / bases.mlbp->randomFileName();

  std::function<void(const std::filesystem::path &p)> call_back
      = [sp](const std::filesystem::path &p)
    {
      std::filesystem::remove_all(sp);
      std::error_code ec;
      std::filesystem::copy(p, sp, ec);
      if(ec)
        {
          std::cout << "MainWindowRightWidget::saveBookFunc: \""
                    << ec.message() << "\"" << std::endl;
        }
    };

  try
    {
      open_book->openBook(bsr, *tmp_path, call_back);
    }
  catch(std::exception &er)
    {
      std::cout << "MainWindowRightWidget::saveBookFunc: \"" << er.what()
                << "\"" << std::endl;
    }
}

void
MainWindowRightWidget::addBookToBookMarks(const UDBElement &book_search_result)
{
  QString er_msg;
  try
    {
      bases.bookmarks->addToBookmarks(book_search_result);
    }
  catch(std::exception &er)
    {
      er_msg = er.what();
    }

  QMessageBox *msg = new QMessageBox(this->window());
  msg->setAttribute(Qt::WA_DeleteOnClose);
  msg->setWindowModality(Qt::WindowModal);

  if(er_msg.isEmpty())
    {
      msg->setIcon(QMessageBox::Information);
      msg->setText(tr("Book has been successfully added to bookmarks"));
    }
  else
    {
      msg->setIcon(QMessageBox::Critical);
      msg->setText(tr("Error!"));
      msg->setDetailedText(er_msg);
    }

  msg->show();
}

void
MainWindowRightWidget::showBooks(const bool &separate_window)
{
  QModelIndex index = search_view->currentIndex();
  if(!index.isValid())
    {
      return void();
    }
  const UDBElement *el_ptr
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(el_ptr == nullptr)
    {
      return void();
    }
  UDBElement el = *el_ptr;

  SearchViewModelFiles *svmdf
      = dynamic_cast<SearchViewModelFiles *>(search_view->model());
  if(svmdf == nullptr)
    {
      return void();
    }

  UDBElement col_info = svmdf->getCollectionInfo();

  std::shared_ptr<std::atomic<bool>> stop(new std::atomic<bool>);
  stop->store(false, std::memory_order_relaxed);
  SearchProcWindow *spw = new SearchProcWindow(this->window());
  spw->creatBookSearchWindow();
  connect(spw, &SearchProcWindow::signalCanceled, this,
          [this, stop]
            {
              stop->store(true, std::memory_order_relaxed);
            });
  spw->show();

  std::thread thr(
      [spw, this, stop, el, col_info, separate_window]
        {
          UDBase result;
          result.addElement(col_info);

          UDBElement fl;
          fl.id = el.id;
          fl.content = el.content;

          for(auto it = el.subelements.begin(); it != el.subelements.end();
              it++)
            {
              if(stop->load(std::memory_order_relaxed))
                {
                  break;
                }
              if(bid.getId(*it) != BaseID::Book)
                {
                  continue;
                }
              UDBElement bsr;
              bid.setId(bsr, BaseID::BookSearchResult);
              bsr.subelements.push_back(fl);
              bsr.subelements.push_back(*it);
              result.addElement(bsr);
            }

          if(stop->load(std::memory_order_relaxed))
            {
              result.clearBase();
              result.addElement(col_info);
            }
          if(separate_window)
            {
              emit signalShowBooksWindow(result, spw, collection_base_path);
            }
          else
            {
              emit signalShowBooks(result, spw, collection_base_path);
            }
        });
  thr.detach();
}

void
MainWindowRightWidget::paintEvent(QPaintEvent *event)
{
  Qt::WindowFlags flags = this->windowFlags() & Qt::Window;
  if(flags == 0)
    {
      QWidget::paintEvent(event);
    }
  else
    {
      QStyleOption opt;
      opt.initFrom(this);
      QPainter p(this);
      style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
}

void
MainWindowRightWidget::clearSearchResult()
{
  QAbstractItemModel *model = search_view->model();
  search_view->setModel(nullptr);
  delete model;

  annotation->setText("");
  cover->clearCover();
  QList<QAction *> actions = operations->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      operations->removeAction(actions[i]);
    }
  operations->update();

  actions = search_view->actions();
  for(qsizetype i = 0; i < actions.size(); i++)
    {
      search_view->removeAction(actions[i]);
      delete actions[i];
    }
}
