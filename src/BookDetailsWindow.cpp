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

#include <AuthorsModel.h>
#include <BookDetailsWindow.h>
#include <QClipboard>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <TableView.h>

BookDetailsWindow::BookDetailsWindow(
    QWidget *parent, const UDBase &info, const UDBElement &book_search_result,
    const std::shared_ptr<FormatAnnotation> &format_annotation,
    const std::shared_ptr<GenreBase> &genre_base)
    : QWidget(parent)
{
  this->info = info;
  this->book_search_result = book_search_result;
  this->format_annotation = format_annotation;
  this->genre_base = genre_base;

  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Book info"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
BookDetailsWindow::createWindow()
{
  QScreen *screen = this->parentWidget()->window()->screen();
  QSize sz = screen->availableSize();
  sz.setWidth(sz.width() * 0.7);
  sz.setHeight(sz.height() * 0.7);
  this->resize(sz);

  QHBoxLayout *h_box = new QHBoxLayout;
  this->setLayout(h_box);

  QScrollArea *scroll = new QScrollArea;
  scroll->setObjectName("ScrollArea");
  scroll->setFrameShape(QFrame::NoFrame);
  h_box->addWidget(scroll, 1);

  QWidget *scroll_widget = new QWidget;
  scroll_widget->setObjectName("ScrollAreaViewport");

  QVBoxLayout *v_box = new QVBoxLayout;
  scroll_widget->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Book info"));
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  v_box->addLayout(bookSection());

  QLayout *layout = srcBookSection();
  if(layout != nullptr)
    {
      QFrame *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      v_box->addWidget(line);

      v_box->addLayout(layout);
    }

  layout = ebookSection();
  if(layout != nullptr)
    {
      QFrame *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      v_box->addWidget(line);

      v_box->addLayout(layout);
    }

  layout = paperSection();
  if(layout != nullptr)
    {
      QFrame *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      v_box->addWidget(line);

      v_box->addLayout(layout);
    }

  layout = customInfoSection();
  if(layout != nullptr)
    {
      QFrame *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      v_box->addWidget(line);

      v_box->addLayout(layout);
    }

  layout = fileInfoSection();
  if(layout != nullptr)
    {
      QFrame *line = new QFrame;
      line->setFrameShape(QFrame::HLine);
      v_box->addWidget(line);

      v_box->addLayout(layout);
    }

  v_box->addStretch();

  scroll->setWidget(scroll_widget);
  scroll->setWidgetResizable(true);

  std::vector<UDBElement> *raw_base = info.getRawBase();
  std::vector<UDBElement>::iterator it
      = std::find_if(raw_base->begin(), raw_base->end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::CoverPage;
                       });
  if(it != raw_base->end())
    {
      cover_el = *it;
      cover = new CoverWidget(nullptr, format_annotation);
      h_box->addWidget(cover, 1);
    }
}

void
BookDetailsWindow::showEvent(QShowEvent *event)
{
  if(cover != nullptr)
    {
      cover->setCover(cover_el);
      cover->update();
    }
  QWidget::showEvent(event);
}

QLayout *
BookDetailsWindow::bookSection()
{
  QGridLayout *result = new QGridLayout;
  int row = 0;

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("Book"));
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  std::vector<UDBElement>::iterator it
      = std::find_if(book_search_result.subelements.begin(),
                     book_search_result.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Book;
                       });
  if(it != book_search_result.subelements.end())
    {
      bookTitle(result, it->subelements, row, BaseID::BookTitle);
      bookAuthor(result, it->subelements, row, BaseID::Author);
      bookSequence(result, it->subelements, row, BaseID::Sequence);
      bookGenre(result, it->subelements, row, BaseID::Genre);
      bookDate(result, it->subelements, row, BaseID::Date);
    }
  addField(result, row, BaseID::Keywords, tr("Keywords"));
  addField(result, row, BaseID::Language, tr("Language"));
  addField(result, row, BaseID::SourceLanguage, tr("Source language"));
  bookTranslator(result, row, BaseID::Translator);

  result->setColumnStretch(0, 0);
  result->setColumnStretch(1, 1);

  return result;
}

void
BookDetailsWindow::bookTitle(QLayout *layout,
                             const std::vector<UDBElement> &book, int &row,
                             const BaseID::ID &search)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  QString str;
  for(auto it_b = book.begin(); it_b != book.end(); it_b++)
    {
      if(bid.getId(*it_b) != search)
        {
          continue;
        }
      if(!str.isEmpty())
        {
          str += ", ";
        }
      str += it_b->content;
    }
  if(!str.isEmpty())
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Book title") + ":");
      result->addWidget(lab, row, 0, 1, 1);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(str);
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      result->addWidget(lab, row, 1, 1, 1);
      row++;
    }
}

void
BookDetailsWindow::bookAuthor(QLayout *layout,
                              const std::vector<UDBElement> &book, int &row,
                              const BaseID::ID &search)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  std::vector<UDBElement> authors;
  for(auto it = book.begin(); it != book.end(); it++)
    {
      if(bid.getId(*it) == search)
        {
          authors.push_back(*it);
        }
    }
  if(authors.size() == 0)
    {
      return void();
    }
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Authors") + ":");
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignLeft);
  row++;

  TableView *srt = new TableView;
  srt->setObjectName("Table");
  srt->viewport()->setObjectName("TableViewport");
  QAbstractItemModel *prev = srt->model();
  AuthorsModel *model = new AuthorsModel(nullptr, authors);
  connect(srt, &TableView::destroyed, model,
          [model]
            {
              delete model;
            });
  srt->setModel(model);
  delete prev;
  srt->resizeRowsToContents();
  result->addWidget(srt, row, 0, 1, 2);
  row++;

  srt->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(srt, &TableView::customContextMenuRequested,
          [srt](const QPoint &pos)
            {
              srt->setCurrentIndex(srt->indexAt(pos));
              QList<QAction *> actions = srt->actions();
              QMenu *menu = new QMenu(srt);
              menu->setAttribute(Qt::WA_DeleteOnClose);
              menu->setObjectName("Menu");

              menu->addActions(actions);

              menu->popup(srt->viewport()->mapToGlobal(pos));
            });
  QAction *act = new QAction;
  act->setText(tr("Copy"));
  srt->addAction(act);
  connect(act, &QAction::triggered, srt,
          [srt]
            {
              QModelIndex index = srt->currentIndex();
              if(index.isValid())
                {
                  QString str = index.data(Qt::DisplayRole).toString();
                  if(!str.isEmpty())
                    {
                      QClipboard *clpb = QGuiApplication::clipboard();
                      clpb->setText(str);
                    }
                }
            });
  connect(srt, &TableView::destroyed, act,
          [act]
            {
              delete act;
            });

  connect(srt, &TableView::signalShowed, srt,
          [srt]
            {
              QHeaderView *vh = srt->verticalHeader();
              int sz = 0;
              for(int i = 0; i < vh->count(); i++)
                {
                  sz += vh->sectionSize(i);
                }
              QHeaderView *hh = srt->horizontalHeader();
              sz += hh->height();
              QScrollBar *h_scrl = srt->horizontalScrollBar();
              if(h_scrl->isVisible())
                {
                  sz += h_scrl->height();
                }

              sz += srt->frameWidth() * 2;

              if(sz < srt->height())
                {
                  srt->setMaximumHeight(sz);
                }
            });

  QHeaderView *hh = srt->horizontalHeader();

  for(int i = 0; i < hh->count(); i++)
    {
      hh->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
}

void
BookDetailsWindow::bookSequence(QLayout *layout,
                                const std::vector<UDBElement> &book, int &row,
                                const BaseID::ID &search)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  QString str;
  for(auto it_b = book.begin(); it_b != book.end(); it_b++)
    {
      if(bid.getId(*it_b) != BaseID::Sequence)
        {
          continue;
        }
      if(it_b->content.empty())
        {
          std::vector<UDBElement>::const_iterator it = std::find_if(
              it_b->subelements.begin(), it_b->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::SequenceName;
                });
          if(it != it_b->subelements.end())
            {
              QString seq_str = it->content.c_str();
              if(seq_str.isEmpty())
                {
                  continue;
                }
              it = std::find_if(
                  it_b->subelements.begin(), it_b->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::SequenceNumber;
                    });
              if(it != it_b->subelements.end())
                {
                  if(!it->content.empty())
                    {
                      seq_str += " ";
                      seq_str += it->content.c_str();
                    }
                }
              if(!str.isEmpty())
                {
                  str += ", ";
                }
              str += seq_str;
            }
        }
      else
        {
          if(!str.isEmpty())
            {
              str += ", ";
            }
          str += it_b->content.c_str();
        }
    }

  if(!str.isEmpty())
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Series") + ":");
      result->addWidget(lab, row, 0, 1, 1);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      lab->setText(str);
      result->addWidget(lab, row, 1, 1, 1);

      row++;
    }
}

void
BookDetailsWindow::bookGenre(QLayout *layout,
                             const std::vector<UDBElement> &book, int &row,
                             const BaseID::ID &search)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  QString str;
  for(auto it_b = book.begin(); it_b != book.end(); it_b++)
    {
      if(bid.getId(*it_b) != search)
        {
          continue;
        }
      QString genre_str = genre_base->getTranslationByGenreCode(it_b->content);
      if(genre_str.isEmpty())
        {
          genre_str = it_b->content.c_str();
        }
      if(!genre_str.isEmpty())
        {
          if(!str.isEmpty())
            {
              str += ", ";
            }
          str += genre_str;
        }
    }
  if(!str.isEmpty())
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Genre") + ":");
      result->addWidget(lab, row, 0, 1, 1);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      lab->setText(str);
      result->addWidget(lab, row, 1, 1, 1);

      row++;
    }
}

void
BookDetailsWindow::bookDate(QLayout *layout,
                            const std::vector<UDBElement> &book, int &row,
                            const BaseID::ID &search)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  QString str;
  for(auto it_b = book.begin(); it_b != book.end(); it_b++)
    {
      if(bid.getId(*it_b) != BaseID::Date)
        {
          continue;
        }
      if(!it_b->content.empty())
        {
          if(!str.isEmpty())
            {
              str += ", ";
            }
          str += it_b->content.c_str();
        }
    }

  if(!str.isEmpty())
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Date") + ":");
      result->addWidget(lab, row, 0, 1, 1);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      lab->setText(str);
      result->addWidget(lab, row, 1, 1, 1);

      row++;
    }
}

void
BookDetailsWindow::addField(QLayout *layout, int &row,
                            const BaseID::ID &search, const QString &name)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  std::vector<UDBElement> res = info.searchElementV(
      [search, this](const UDBElement &el)
        {
          return bid.getId(el) == search;
        });
  QString str;
  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->content.empty())
        {
          continue;
        }
      if(!str.isEmpty())
        {
          str += ", ";
        }
      str += it->content.c_str();
    }

  if(!str.isEmpty())
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(name + ":");
      result->addWidget(lab, row, 0, 1, 1);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      lab->setText(str);
      result->addWidget(lab, row, 1, 1, 1);

      row++;
    }
}

void
BookDetailsWindow::bookTranslator(QLayout *layout, int &row,
                                  const BaseID::ID &search)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  std::vector<UDBElement> authors = info.searchElementV(
      [search, this](const UDBElement &el)
        {
          return bid.getId(el) == BaseID::Translator;
        });
  if(authors.size() == 0)
    {
      return void();
    }
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Translators") + ":");
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignLeft);
  row++;

  TableView *srt = new TableView;
  srt->setObjectName("Table");
  srt->viewport()->setObjectName("TableViewport");
  QAbstractItemModel *prev = srt->model();
  AuthorsModel *model = new AuthorsModel(nullptr, authors);
  connect(srt, &TableView::destroyed, model,
          [model]
            {
              delete model;
            });
  srt->setModel(model);
  delete prev;
  srt->resizeRowsToContents();
  result->addWidget(srt, row, 0, 1, 2);
  row++;

  srt->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(srt, &TableView::customContextMenuRequested,
          [srt](const QPoint &pos)
            {
              srt->setCurrentIndex(srt->indexAt(pos));
              QList<QAction *> actions = srt->actions();
              QMenu *menu = new QMenu(srt);
              menu->setAttribute(Qt::WA_DeleteOnClose);
              menu->setObjectName("Menu");

              menu->addActions(actions);

              menu->popup(srt->viewport()->mapToGlobal(pos));
            });
  QAction *act = new QAction;
  act->setText(tr("Copy"));
  srt->addAction(act);
  connect(act, &QAction::triggered, srt,
          [srt]
            {
              QModelIndex index = srt->currentIndex();
              if(index.isValid())
                {
                  QString str = index.data(Qt::DisplayRole).toString();
                  if(!str.isEmpty())
                    {
                      QClipboard *clpb = QGuiApplication::clipboard();
                      clpb->setText(str);
                    }
                }
            });
  connect(srt, &TableView::destroyed, act,
          [act]
            {
              delete act;
            });

  connect(srt, &TableView::signalShowed, srt,
          [srt]
            {
              QHeaderView *vh = srt->verticalHeader();
              int sz = 0;
              for(int i = 0; i < vh->count(); i++)
                {
                  sz += vh->sectionSize(i);
                }
              QHeaderView *hh = srt->horizontalHeader();
              sz += hh->height();
              QScrollBar *h_scrl = srt->horizontalScrollBar();
              if(h_scrl->isVisible())
                {
                  sz += h_scrl->height();
                }

              sz += srt->frameWidth() * 2;

              if(sz < srt->height())
                {
                  srt->setMaximumHeight(sz);
                }
            });

  QHeaderView *hh = srt->horizontalHeader();

  for(int i = 0; i < hh->count(); i++)
    {
      hh->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
}

QLayout *
BookDetailsWindow::srcBookSection()
{
  QGridLayout *result = new QGridLayout;
  int row = 0;

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("Original book"));
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  bookTitle(result, *info.getRawBase(), row, BaseID::SourceBookTitle);
  bookAuthor(result, *info.getRawBase(), row, BaseID::SourceBookAuthor);
  bookSequence(result, *info.getRawBase(), row, BaseID::SourceBookSequence);
  bookGenre(result, *info.getRawBase(), row, BaseID::SourceBookGenre);
  bookDate(result, *info.getRawBase(), row, BaseID::SourceBookDate);
  addField(result, row, BaseID::SourceBookKeywords, tr("Keywords"));
  addField(result, row, BaseID::SourceBookLanguage, tr("Language"));
  addField(result, row, BaseID::SourceBookSourceLanguage,
           tr("Source language"));
  bookTranslator(result, row, BaseID::SourceBookTranslator);
  addField(result, row, BaseID::SourceBookDublinCore, tr("Source"));

  result->setColumnStretch(0, 0);
  result->setColumnStretch(1, 1);

  if(row <= 1)
    {
      delete result;
      result = nullptr;
    }

  return result;
}

QLayout *
BookDetailsWindow::ebookSection()
{
  QGridLayout *result = new QGridLayout;
  int row = 0;

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("E-Book"));
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  bookAuthor(result, *info.getRawBase(), row, BaseID::EbookAuthor);
  addField(result, row, BaseID::EbookProgramUsed, tr("Program"));
  bookDate(result, *info.getRawBase(), row, BaseID::EbookDate);
  addField(result, row, BaseID::EbookSourceUrl, tr("Source URL"));
  addField(result, row, BaseID::EbookSourceOCR, tr("Original E-Book author"));
  addField(result, row, BaseID::EbookID, tr("ID"));
  addField(result, row, BaseID::EbookVersion, tr("Version"));
  ebookHistory(result, row);
  ebookPublisher(result, row);
  addField(result, row, BaseID::DjvuPublisher, tr("Publisher"));

  result->setColumnStretch(0, 0);
  result->setColumnStretch(1, 1);

  if(row <= 1)
    {
      delete result;
      result = nullptr;
    }

  return result;
}

void
BookDetailsWindow::ebookHistory(QLayout *layout, int &row)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  if(result == nullptr)
    {
      return void();
    }

  std::vector<UDBElement> res = info.searchElementV(
      [this](const UDBElement &el)
        {
          return bid.getId(el) == BaseID::EbookHistory;
        });

  if(res.size() == 0)
    {
      return void();
    }

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("History") + ":");
  result->addWidget(lab, row, 0, 1, 1);

  for(auto it = res.begin(); it != res.end(); it++)
    {
      if(it->content.empty())
        {
          continue;
        }
      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      lab->setText(it->content.c_str());
      result->addWidget(lab, row, 1, 1, 1);

      row++;
    }
}

void
BookDetailsWindow::ebookPublisher(QLayout *layout, int &row)
{
  QGridLayout *result = dynamic_cast<QGridLayout *>(layout);

  std::vector<UDBElement> authors = info.searchElementV(
      [this](const UDBElement &el)
        {
          return bid.getId(el) == BaseID::EbookPublisher;
        });
  if(authors.size() == 0)
    {
      return void();
    }
  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Publishers") + ":");
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignLeft);
  row++;

  TableView *srt = new TableView;
  srt->setObjectName("Table");
  srt->viewport()->setObjectName("TableViewport");
  QAbstractItemModel *prev = srt->model();
  AuthorsModel *model = new AuthorsModel(nullptr, authors);
  connect(srt, &TableView::destroyed, model,
          [model]
            {
              delete model;
            });
  srt->setModel(model);
  delete prev;
  srt->resizeRowsToContents();
  result->addWidget(srt, row, 0, 1, 2);
  row++;

  connect(srt, &TableView::signalShowed, srt,
          [srt]
            {
              QHeaderView *vh = srt->verticalHeader();
              int sz = 0;
              for(int i = 0; i < vh->count(); i++)
                {
                  sz += vh->sectionSize(i);
                }
              QHeaderView *hh = srt->horizontalHeader();
              sz += hh->height();
              QScrollBar *h_scrl = srt->horizontalScrollBar();
              if(h_scrl->isVisible())
                {
                  sz += h_scrl->height();
                }

              sz += srt->frameWidth() * 2;

              if(sz < srt->height())
                {
                  srt->setMaximumHeight(sz);
                }
            });

  QHeaderView *hh = srt->horizontalHeader();

  for(int i = 0; i < hh->count(); i++)
    {
      hh->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
}

QLayout *
BookDetailsWindow::paperSection()
{
  QGridLayout *result = new QGridLayout;
  int row = 0;

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("Paper book"));
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  addField(result, row, BaseID::PaperBookName, tr("Book name"));
  addField(result, row, BaseID::PaperBookPublisher, tr("Publisher"));
  addField(result, row, BaseID::PaperBookCity, tr("City"));
  addField(result, row, BaseID::PaperBookYear, tr("Year"));
  addField(result, row, BaseID::PaperBookISBN, "ISBN");
  bookSequence(result, *info.getRawBase(), row, BaseID::PaperBookSequence);

  result->setColumnStretch(0, 0);
  result->setColumnStretch(1, 1);

  if(row <= 1)
    {
      delete result;
      result = nullptr;
    }

  return result;
}

QLayout *
BookDetailsWindow::customInfoSection()
{
  QGridLayout *result = new QGridLayout;
  int row = 0;

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("Custom info"));
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  std::vector<UDBElement> res = info.searchElementV(
      [this](const UDBElement &el)
        {
          return bid.getId(el) == BaseID::CustomInfo;
        });

  if(res.size() > 0)
    {
      QTextBrowser *text = new QTextBrowser;
      for(auto it = res.begin(); it != res.end(); it++)
        {
          text->insertHtml(it->content.c_str());
          text->insertHtml("<br><br>");
        }
      result->addWidget(text, row, 0, 1, 2);
      row++;

      text->moveCursor(QTextCursor::Start);

      text->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(text, &QTextBrowser::customContextMenuRequested,
              [text](const QPoint &pos)
                {
                  QMenu *menu = text->createStandardContextMenu(pos);
                  menu->setAttribute(Qt::WA_DeleteOnClose);
                  menu->setObjectName("Menu");
                  menu->popup(text->viewport()->mapToGlobal(pos));
                });
      text->setTextInteractionFlags(
          Qt::TextInteractionFlag::TextBrowserInteraction);
    }

  result->setColumnStretch(0, 0);
  result->setColumnStretch(1, 1);

  if(row <= 1)
    {
      delete result;
      result = nullptr;
    }

  return result;
}

QLayout *
BookDetailsWindow::fileInfoSection()
{
  QGridLayout *result = new QGridLayout;
  int row = 0;

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("File info"));
  result->addWidget(lab, row, 0, 1, 2, Qt::AlignCenter);
  row++;

  std::vector<UDBElement>::iterator it
      = std::find_if(book_search_result.subelements.begin(),
                     book_search_result.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::File;
                       });
  if(it != book_search_result.subelements.end())
    {
      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("File path") + ":");
      result->addWidget(lab, row, 0, 1, 1, Qt::AlignLeft);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(it->content.c_str());
      lab->setWordWrap(true);
      lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard);
      result->addWidget(lab, row, 1, 1, 1);
      row++;
    }

  it = std::find_if(book_search_result.subelements.begin(),
                    book_search_result.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Book;
                      });
  if(it != book_search_result.subelements.end())
    {
      std::vector<QString> str;
      std::vector<UDBElement> *prev = &it->subelements;
      for(;;)
        {
          it = std::find_if(prev->begin(), prev->end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::PathInFile;
                              });
          if(it == prev->end())
            {
              break;
            }
          if(it->content.empty())
            {
              break;
            }

          str.push_back(QString("\"") + it->content.c_str() + "\"");
          prev = &it->subelements;
        }
      if(str.size() > 0)
        {
          lab = new QLabel;
          lab->setObjectName("Label");
          lab->setText(tr("Path in file") + ":");
          result->addWidget(lab, row, 0, 1, 1, Qt::AlignLeft);

          for(auto it_str = str.begin(); it_str != str.end(); it_str++)
            {
              lab = new QLabel;
              lab->setObjectName("Label");
              lab->setText(*it_str);
              lab->setWordWrap(true);
              lab->setTextInteractionFlags(Qt::TextSelectableByMouse
                                           | Qt::TextSelectableByKeyboard);
              result->addWidget(lab, row, 1, 1, 1);
              row++;
            }
        }
    }

  result->setColumnStretch(0, 0);
  result->setColumnStretch(1, 1);

  if(row <= 1)
    {
      delete result;
      result = nullptr;
    }

  return result;
}

void
BookDetailsWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
