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
#include <BookMarksWindow.h>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <StyledItemDelegate.h>
#include <StyledWindow.h>
#include <iostream>

BookMarksWindow::BookMarksWindow(
    QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
    const std::shared_ptr<BookmarksKeeper> &bookmarks,
    const std::shared_ptr<SettingsManager> &settings)
    : QWidget(parent)
{
  this->mlbp = mlbp;
  this->bookmarks = bookmarks;
  this->settings = settings;
  genre_base = std::make_shared<GenreBase>();

  book_open_dir = mlbp->tempDirPath() / std::filesystem::path(u8"MyLibrary");

  open_book = new OpenBook(mlbp);

  format_annot = std::make_shared<FormatAnnotation>(mlbp);

  book_info = new BookInfo(mlbp);

  std::vector<ReplaceTagItem> replacement_table;
  std::vector<std::tuple<std::string, std::string>> symbols_replacement;
  formTagReplacementTable(replacement_table, symbols_replacement);
  format_annot->setTagReplacementTable(replacement_table, symbols_replacement);

  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Bookmarks"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

BookMarksWindow::~BookMarksWindow()
{
  std::unique_lock<std::mutex> ullock(thr_num_mtx);
  thr_num_var.wait(ullock,
                   [this]
                     {
                       return thr_num <= 0;
                     });

  if(model != nullptr)
    {
      model->deleteLater();
    }
  delete open_book;
  delete book_info;
}

void
BookMarksWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Bookmarks"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  model = new BookMarksModel(nullptr, bookmarks, genre_base);

  TableView *table = new TableView;
  table->setObjectName("Table");
  table->viewport()->setObjectName("TableViewport");
  QAbstractItemDelegate *delegate = table->itemDelegate();
  StyledItemDelegate *item_delegate = new StyledItemDelegate(table, settings);
  table->setItemDelegate(item_delegate);
  delete delegate;
  connect(table, &TableView::signalResized, table,
          [table](const QSize &sz)
            {
              int total = sz.width();
              QHeaderView *hh = table->horizontalHeader();
              for(int i = 0; i < 5; i++)
                {
                  switch(i)
                    {
                    case 0:
                    case 1:
                      {
                        int ssz = sz.width() * 0.3;
                        hh->resizeSection(i, ssz);
                        total -= ssz;
                        break;
                      }
                    case 2:
                    case 3:
                      {
                        int ssz = sz.width() * 0.15;
                        hh->resizeSection(i, ssz);
                        total -= ssz;
                        break;
                      }
                    default:
                      {
                        hh->resizeSection(i, total);
                        break;
                      }
                    }
                }
            });
  QAbstractItemModel *prev = table->model();
  table->setModel(model);
  if(prev != nullptr)
    {
      prev->deleteLater();
    }
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  v_box->addWidget(table);

  connect(table, &TableView::customContextMenuRequested, table,
          [table](const QPoint &pos)
            {
              QModelIndex index = table->indexAt(pos);
              table->setCurrentIndex(index);

              QMenu *menu = new QMenu(table);
              menu->setObjectName("Menu");
              menu->setAttribute(Qt::WA_DeleteOnClose);

              menu->addActions(table->actions());

              menu->popup(table->viewport()->mapToGlobal(pos));
            });

  connect(table, &TableView::signalLeftMouseButton, table,
          [table](const QPoint &glob_pos)
            {
              QPoint pos = table->viewport()->mapFromGlobal(glob_pos);
              table->setCurrentIndex(table->indexAt(pos));
            });

  QAction *open_act = new QAction(tr("Open"));
  connect(open_act, &QAction::triggered, this,
          [this, table]
            {
              QModelIndex index = table->currentIndex();
              if(!index.isValid())
                {
                  return void();
                }
              const UDBElement *bm = reinterpret_cast<const UDBElement *>(
                  index.constInternalPointer());
              if(bm == nullptr)
                {
                  return void();
                }

              std::filesystem::remove_all(book_open_dir);

              try
                {
                  open_book->openBook(
                      *bm, book_open_dir,
                      std::bind(&BookMarksWindow::openBookCallback, this,
                                std::placeholders::_1));
                }
              catch(std::exception &er)
                {
                  std::cout << "BookMarksWindow::createWindow: \"" << er.what()
                            << "\"" << std::endl;
                }
            });
  table->addAction(open_act);

  std::vector<QAction *> act_list;
  act_list.push_back(open_act);

  QAction *info_act = new QAction(tr("Book info"));
  connect(info_act, &QAction::triggered, table,
          [this, table]
            {
              QModelIndex index = table->currentIndex();
              if(!index.isValid())
                {
                  return void();
                }
              const UDBElement *bm = reinterpret_cast<const UDBElement *>(
                  index.constInternalPointer());
              if(bm == nullptr)
                {
                  return void();
                }
              QScreen *screen = this->screen();
              book_info->setDPI(
                  static_cast<double>(screen->physicalDotsPerInchX()),
                  static_cast<double>(screen->physicalDotsPerInchY()));
              try
                {
                  UDBase info = book_info->getBookInfo(*bm);

                  BookDetailsWindow *bdw = new BookDetailsWindow(
                      this, info, *bm, format_annot, genre_base, settings);
                  bdw->createWindow();
                  bdw->show();
                }
              catch(std::exception &er)
                {
                  std::cout << er.what() << std::endl;
                }
            });
  table->addAction(info_act);
  act_list.push_back(info_act);

  QAction *save_act = new QAction(tr("Save as..."));
  connect(save_act, &QAction::triggered, table,
          [table, this]
            {
              bookSaveDialog(table);
            });
  table->addAction(save_act);
  act_list.push_back(save_act);

  QAction *remove_act = new QAction(tr("Remove from bookmarks"));
  connect(remove_act, &QAction::triggered, table,
          [table, this]
            {
              QModelIndex index = table->currentIndex();
              bookmarkRemoveDialog(index);
            });
  table->addAction(remove_act);
  act_list.push_back(remove_act);

  act_list.shrink_to_fit();
  connect(table, &TableView::destroyed,
          [act_list]
            {
              for(size_t i = 0; i < act_list.size(); i++)
                {
                  delete act_list[i];
                }
            });

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *open = new QPushButton;
  open->setText(tr("Open selected"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(open);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  open->setGraphicsEffect(shadow);
  open->setObjectName("ApplyButton");
  connect(open, &QPushButton::clicked, open_act, &QAction::trigger);
  h_box->addWidget(open, 0, Qt::AlignCenter);

  QPushButton *info = new QPushButton;
  info->setText(tr("Book info"));
  shadow = new QGraphicsDropShadowEffect(info);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  info->setGraphicsEffect(shadow);
  info->setObjectName("ApplyButton");
  connect(info, &QPushButton::clicked, info_act, &QAction::trigger);
  h_box->addWidget(info, 0, Qt::AlignCenter);

  QPushButton *save = new QPushButton;
  save->setText(tr("Save selected as..."));
  shadow = new QGraphicsDropShadowEffect(save);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  save->setGraphicsEffect(shadow);
  save->setObjectName("ApplyButton");
  connect(save, &QPushButton::clicked, save_act, &QAction::trigger);
  h_box->addWidget(save, 0, Qt::AlignCenter);

  QPushButton *remove = new QPushButton;
  remove->setText(tr("Remove selected"));
  shadow = new QGraphicsDropShadowEffect(remove);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove->setGraphicsEffect(shadow);
  remove->setObjectName("ClearButton");
  connect(remove, &QPushButton::clicked, remove_act, &QAction::trigger);
  h_box->addWidget(remove, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(av_sz.height() * 0.5);
  av_sz.setWidth(av_sz.width() * 0.5);
  this->resize(av_sz);
}

void
BookMarksWindow::openBookCallback(const std::filesystem::path &p)
{
  QString str(p.u8string().c_str());
  QDesktopServices::openUrl(QUrl::fromLocalFile(str));
}

void
BookMarksWindow::formTagReplacementTable(
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
BookMarksWindow::bookmarkRemoveDialog(const QModelIndex &index)
{
  if(!index.isValid())
    {
      return void();
    }
  const UDBElement *bm
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(bm == nullptr)
    {
      return void();
    }

  BaseID bid;
  auto it_book = std::find_if(bm->subelements.begin(), bm->subelements.end(),
                              [bid](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == bm->subelements.end())
    {
      return void();
    }

  auto it_tit
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [bid](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::BookTitle;
                       });
  if(it_tit == it_book->subelements.end())
    {
      return void();
    }

  StyledWindow *window = new StyledWindow(this);
  window->setObjectName("Window");
  window->setWindowModality(Qt::WindowModality::WindowModal);

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Bookmark to remove:"));
  h_box->addWidget(lab, 0, Qt::AlignRight);

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(it_tit->content.c_str());
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Are you sure?"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *yes = new QPushButton;
  yes->setText(tr("Yes"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(yes);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  yes->setGraphicsEffect(shadow);
  yes->setObjectName("ApplyButton");
  connect(yes, &QPushButton::clicked, this,
          [this, index, window]
            {
              model->removeBookmark(index);
              window->close();
            });
  h_box->addWidget(yes, 0, Qt::AlignCenter);

  QPushButton *no = new QPushButton;
  no->setText(tr("No"));
  shadow = new QGraphicsDropShadowEffect(no);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  no->setGraphicsEffect(shadow);
  no->setObjectName("CancelButton");
  connect(no, &QPushButton::clicked, window, &QWidget::close);
  h_box->addWidget(no, 0, Qt::AlignCenter);

  window->show();
}

void
BookMarksWindow::bookSaveDialog(TableView *table)
{
  QModelIndex index = table->currentIndex();
  if(!index.isValid())
    {
      return void();
    }

  const UDBElement *book_search_result
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(book_search_result == nullptr)
    {
      return void();
    }
  BaseID bid;
  auto it_book = std::find_if(book_search_result->subelements.begin(),
                              book_search_result->subelements.end(),
                              [bid](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == book_search_result->subelements.end())
    {
      return void();
    }

  QString suffix;
  QString file_name;
  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [bid](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  if(it_path == it_book->subelements.end())
    {
      auto it_fl = std::find_if(book_search_result->subelements.begin(),
                                book_search_result->subelements.end(),
                                [bid](const UDBElement &el)
                                  {
                                    return bid.getId(el) == BaseID::File;
                                  });
      if(it_fl == book_search_result->subelements.end())
        {
          return void();
        }
      suffix = mlbp->getExtension(it_fl->content).c_str();
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
              [bid](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::PathInFile;
                });
          if(it_p == it_path->subelements.end())
            {
              suffix = mlbp->getExtension(it_path->content).c_str();
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
          [book_search_result, this](const QString &file)
            {
              UDBElement bsr = *book_search_result;
              std::lock_guard<std::mutex> lglock(thr_num_mtx);
              thr_num++;
              std::thread thr(
                  [this, file, bsr]
                    {
                      saveBookFunc(file, bsr);
                      std::lock_guard<std::mutex> lglock(thr_num_mtx);
                      thr_num--;
                      thr_num_var.notify_all();
                    });
              thr.detach();
            });

  fd->show();
}

void
BookMarksWindow::saveBookFunc(const QString &save_path, const UDBElement &bsr)
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
  *tmp_path = mlbp->tempDirPath() / mlbp->randomFileName();

  std::function<void(const std::filesystem::path &p)> call_back
      = [sp](const std::filesystem::path &p)
    {
      std::filesystem::remove_all(sp);
      std::error_code ec;
      std::filesystem::copy(p, sp, ec);
      if(ec)
        {
          std::cout << "BookMarksWindow::saveBookFunc: \"" << ec.message()
                    << "\"" << std::endl;
        }
    };

  try
    {
      open_book->openBook(bsr, *tmp_path, call_back);
    }
  catch(std::exception &er)
    {
      std::cout << "BookMarksWindow::saveBookFunc: \"" << er.what() << "\""
                << std::endl;
    }
}

void
BookMarksWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
