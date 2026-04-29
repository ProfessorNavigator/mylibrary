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

#include <GenreView.h>
#include <MainWindowLeftWidget.h>
#include <QApplication>
#include <QCommonStyle>
#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QVariant>
#include <SearchProcWindow.h>
#include <fstream>
#include <iostream>

MainWindowLeftWidget::MainWindowLeftWidget(QWidget *parent, const Bases &bases)
    : QSplitter(parent)
{
  this->bases = bases;
  search_mtx = new std::mutex;

  createWidget();
}

MainWindowLeftWidget::~MainWindowLeftWidget()
{
  if(base_loading_thr)
    {
      if(base_loading_thr->joinable())
        {
          base_loading_thr->join();
        }
    }
  if(!current_collection_base_path.empty())
    {
      std::u8string u8str
          = current_collection_base_path.parent_path().filename().u8string();
      std::string cur_col(u8str.begin(), u8str.end());
      std::string str = QDir::homePath().toStdString();
      std::filesystem::path selcol = std::u8string(str.begin(), str.end());
      selcol /= std::filesystem::path(u8".cache")
                / std::filesystem::path(u8"MyLibrary")
                / std::filesystem::path(u8"ActiveCollection");
      std::filesystem::create_directories(selcol.parent_path());
      std::filesystem::remove_all(selcol);
      std::fstream f;
      f.open(selcol, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
        {
          f.write(cur_col.c_str(), cur_col.size());
          f.close();
        }
    }
  search_mtx->lock();
  search_mtx->unlock();
  delete search_mtx;
}

void
MainWindowLeftWidget::removeCollection(const QString &col_name)
{
  int index = collections->findText(col_name);
  if(index != -1)
    {
      if(collections->currentIndex() == index)
        {
          emit signalCollectionLoaded("0", "");
        }
      collections->removeItem(index);
    }
}

void
MainWindowLeftWidget::addCollection(const std::filesystem::path &base_path)
{
  QString str(base_path.parent_path().filename().u8string().c_str());
  QVariant var;
  var.setValue(base_path);
  collections->addItem(str, var);

  if(collections->model()->rowCount() == 1)
    {
      if(base_loading_thr)
        {
          if(base_loading_thr->joinable())
            {
              base_loading_thr->join();
            }
        }
      base_loading_thr = std::shared_ptr<std::thread>(new std::thread(
          [this, base_path]
            {
              emit signalCollectionLoaded(tr("loading"), tr("loading"));
              try
                {
                  bases.base_keeper->loadCollection(base_path);
                }
              catch(std::exception &er)
                {
                  std::cout << er.what() << std::endl;
                  emit signalCollectionLoaded(tr("error"), tr("error"));
                  return void();
                }
              size_t sz = bases.base_keeper->getBooksQuantity();
              QString str;
              str.setNum(sz);
              emit signalCollectionLoaded(
                  str, bases.base_keeper->getCollectionType().c_str());
            }));
    }
}

void
MainWindowLeftWidget::reloadCollection(const QString &col_name)
{
  if(collections->currentText() == col_name)
    {
      QVariant var = collections->currentData();
      if(var.isNull())
        {
          return void();
        }
      std::filesystem::path base_path = var.value<std::filesystem::path>();
      if(base_loading_thr)
        {
          if(base_loading_thr->joinable())
            {
              base_loading_thr->join();
            }
        }
      base_loading_thr = std::shared_ptr<std::thread>(new std::thread(
          [this, base_path]
            {
              emit signalCollectionLoaded(tr("loading"), tr("loading"));
              try
                {
                  bases.base_keeper->loadCollection(base_path);
                }
              catch(std::exception &er)
                {
                  std::cout << er.what() << std::endl;
                  emit signalCollectionLoaded(tr("error"), tr("error"));
                  return void();
                }
              size_t sz = bases.base_keeper->getBooksQuantity();
              QString str;
              str.setNum(sz);
              emit signalCollectionLoaded(
                  str, bases.base_keeper->getCollectionType().c_str());
            }));
    }
}

void
MainWindowLeftWidget::editBook(const UDBElement &book_search_result)
{
  bases.base_keeper->editBookEntry(book_search_result);
}

void
MainWindowLeftWidget::searchAuthorsBooks(
    const UDBElement &author_search_result, const bool &sep_window)
{
  if(!search_mtx->try_lock())
    {
      return void();
    }

  std::vector<UDBElement> request;
  UDBElement el;
  bid.setId(el, BaseID::Author);
  el.content = author_search_result.content;
  request.emplace_back(el);

  double coef = 1.0;

  SearchProcWindow *spw = new SearchProcWindow(this->window());
  spw->creatBookSearchWindow();
  connect(spw, &SearchProcWindow::signalCanceled, this,
          [this]
            {
              bases.base_keeper->cancelSearch();
            });
  spw->show();

  std::thread thr(
      [this, request, coef, spw, sep_window]
        {
          UDBase base
              = std::move(bases.base_keeper->searchBook(request, coef));
          emit spw->signalStartSorting();
          if(sep_window)
            {
              emit signalBookSearchSepWindow(base, spw,
                                             current_collection_base_path);
            }
          else
            {
              emit signalBookSearchFinished(base, spw,
                                            current_collection_base_path);
            }
          search_mtx->unlock();
        });
  thr.detach();
}

void
MainWindowLeftWidget::createWidget()
{
  this->setOrientation(Qt::Vertical);
  this->setChildrenCollapsible(false);

  this->addWidget(collectionSection());
  this->addWidget(searchSection());
  this->addWidget(operationsSection());
}

QWidget *
MainWindowLeftWidget::collectionSection()
{
  QWidget *result = new QWidget;

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Collection"));
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  collections = new QComboBox;
  collections->setObjectName("ComboBox");
  QGraphicsDropShadowEffect *shadow
      = new QGraphicsDropShadowEffect(collections);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  collections->setGraphicsEffect(shadow);
  collections->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  connect(collections, &QComboBox::currentIndexChanged, this,
          [this]
            {
              emit signalClearSearchResult();
            });
  v_box->addWidget(collections, 0, Qt::AlignHCenter);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  h_box->addStretch();

  lab = new QLabel;
  lab->setText(tr("Collection type:"));
  lab->setObjectName("Label");
  h_box->addWidget(lab, 0, Qt::AlignRight);

  QLabel *collection_type = new QLabel;
  collection_type->setObjectName("Label");
  font = collection_type->font();
  font.setItalic(true);
  collection_type->setFont(font);
  h_box->addWidget(collection_type, 0, Qt::AlignLeft);

  QFrame *line = new QFrame;
  line->setFrameShape(QFrame::VLine);
  h_box->addWidget(line);

  lab = new QLabel;
  lab->setText(tr("Books quantity:"));
  lab->setObjectName("Label");
  h_box->addWidget(lab, 0, Qt::AlignRight);

  lab = new QLabel;
  lab->setObjectName("Label");
  font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText("0");
  connect(this, &MainWindowLeftWidget::signalCollectionLoaded, lab,
          [this, lab, collection_type](const QString &books_quantity,
                                       const QString &coll_type)
            {
              lab->setText(books_quantity);
              if(coll_type == "native")
                {
                  collection_type->setText(tr("native"));
                }
              else if(coll_type == "legacy")
                {
                  collection_type->setText(tr("legacy"));
                }
              else
                {
                  collection_type->setText(coll_type);
                }
            });
  h_box->addWidget(lab, 0, Qt::AlignRight);

  h_box->addStretch();

  loadCollectionsList();
  qApp->processEvents();
  connect(collections, &QComboBox::currentIndexChanged, this,
          &MainWindowLeftWidget::selectCollection);
  setActiveCollection();

  return result;
}

QWidget *
MainWindowLeftWidget::searchSection()
{
  QWidget *result = new QWidget;

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  QLabel *lab = new QLabel;
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Search"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  lab = new QLabel;
  font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("Author"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  lab = new QLabel;
  lab->setText(tr("Surname:"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  surname = new QLineEdit;
  surname->setObjectName("LineEdit");
  connect(surname, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  v_box->addWidget(surname);

  lab = new QLabel;
  lab->setText(tr("First name:"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  first_name = new QLineEdit;
  first_name->setObjectName("LineEdit");
  connect(first_name, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  v_box->addWidget(first_name);

  lab = new QLabel;
  lab->setText(tr("Second name:"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  second_name = new QLineEdit;
  second_name->setObjectName("LineEdit");
  connect(second_name, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  v_box->addWidget(second_name);

  lab = new QLabel;
  lab->setText(tr("Nickname:"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  nickname = new QLineEdit;
  nickname->setObjectName("LineEdit");
  connect(nickname, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  v_box->addWidget(nickname);

  lab = new QLabel;
  font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(tr("Book"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  lab = new QLabel;
  lab->setText(tr("Book name:"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  book_name = new QLineEdit;
  book_name->setObjectName("LineEdit");
  connect(book_name, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  v_box->addWidget(book_name);

  QGridLayout *grid = new QGridLayout;
  v_box->addLayout(grid);

  lab = new QLabel;
  lab->setText(tr("Series:"));
  lab->setObjectName("Label");
  grid->addWidget(lab, 0, 0, 1, 1, Qt::AlignLeft);

  series = new QLineEdit;
  series->setObjectName("LineEdit");
  connect(series, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  grid->addWidget(series, 1, 0, 1, 1);

  lab = new QLabel;
  lab->setText(tr("Number:"));
  lab->setObjectName("Label");
  grid->addWidget(lab, 0, 1, 1, 1, Qt::AlignLeft);

  series_number = new QLineEdit;
  series_number->setObjectName("LineEdit");
  connect(series_number, &QLineEdit::returnPressed, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  grid->addWidget(series_number, 1, 1, 1, 1);

  lab = new QLabel;
  lab->setText(tr("Genre:"));
  lab->setObjectName("Label");
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  genre_button = new QPushButton;
  genre_button->setObjectName("ComboBox");
  QGraphicsDropShadowEffect *shadow
      = new QGraphicsDropShadowEffect(genre_button);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  genre_button->setGraphicsEffect(shadow);
  QString str = "<";
  str += tr("No");
  str += ">";
  genre_button->setText(str);
  QCommonStyle style;
  genre_button->setIcon(style.standardIcon(QStyle::SP_ArrowDown));
  h_box->addWidget(genre_button, 1);

  QWidget *widget = genrePopup();
  connect(genre_button, &QPushButton::clicked, widget,
          [widget, this]
            {
              widget->show();
              QPoint pos = genre_button->pos();
              pos = genre_button->parentWidget()->mapToGlobal(pos);
              pos.setY(pos.y() + genre_button->height());
              widget->move(pos);
            });

  QPushButton *clear_genre = new QPushButton;
  clear_genre->setObjectName("ClearButton");
  shadow = new QGraphicsDropShadowEffect(clear_genre);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  clear_genre->setGraphicsEffect(shadow);
  clear_genre->setText(tr("Clear"));
  connect(clear_genre, &QPushButton::clicked, genre_button,
          [this]
            {
              selected_genre.clear();
              QString str = "<";
              str += tr("No");
              str += ">";
              genre_button->setText(str);
            });
  h_box->addWidget(clear_genre, 0);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setText(tr("Coefficient of coincidence:"));
  lab->setObjectName("Label");
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  h_box->addStretch();

  QLabel *cc_val = new QLabel;
  cc_val->setText("75");
  cc_val->setObjectName("Label");
  h_box->addWidget(cc_val, 0, Qt::AlignRight);

  lab = new QLabel;
  lab->setText("%");
  lab->setObjectName("Label");
  h_box->addWidget(lab, 0, Qt::AlignRight);

  coef_coincidence = new QSlider;
  coef_coincidence->setObjectName("Slider");
  coef_coincidence->setMinimum(0);
  coef_coincidence->setMaximum(100);
  coef_coincidence->setValue(75);
  coef_coincidence->setOrientation(Qt::Horizontal);
  coef_coincidence->setTickPosition(QSlider::NoTicks);
  connect(coef_coincidence, &QSlider::sliderMoved, cc_val,
          [cc_val](int val)
            {
              QString str;
              str.setNum(val);
              cc_val->setText(str);
            });
  v_box->addWidget(coef_coincidence);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *search = new QPushButton;
  search->setText(tr("Search"));
  shadow = new QGraphicsDropShadowEffect(search);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  search->setGraphicsEffect(shadow);
  search->setObjectName("ApplyButton");
  connect(search, &QPushButton::clicked, this,
          [this]
            {
              searchBook(SearchType::Book);
            });
  h_box->addWidget(search, 0, Qt::AlignCenter);

  QPushButton *clear_all = new QPushButton;
  clear_all->setText(tr("Clear"));
  shadow = new QGraphicsDropShadowEffect(clear_all);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  clear_all->setGraphicsEffect(shadow);
  clear_all->setObjectName("ClearButton");
  connect(clear_all, &QPushButton::clicked, this,
          &MainWindowLeftWidget::clearSearchParameters);
  h_box->addWidget(clear_all, 0, Qt::AlignCenter);

  return result;
}

QWidget *
MainWindowLeftWidget::operationsSection()
{
  QWidget *result = new QWidget;

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *files = new QPushButton;
  files->setText(tr("Show collection files"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(files);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  files->setGraphicsEffect(shadow);
  files->setObjectName("ApplyButton");
  connect(files, &QPushButton::clicked, this,
          &MainWindowLeftWidget::showFiles);
  h_box->addWidget(files, 0, Qt::AlignCenter);

  QPushButton *authors = new QPushButton;
  authors->setText(tr("Show collection authors"));
  shadow = new QGraphicsDropShadowEffect(authors);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  authors->setGraphicsEffect(shadow);
  authors->setObjectName("ApplyButton");
  connect(authors, &QPushButton::clicked, this,
          &MainWindowLeftWidget::showAuthors);
  h_box->addWidget(authors, 0, Qt::AlignCenter);

  QPushButton *books_with_notes = new QPushButton;
  books_with_notes->setText(tr("Books with notes"));
  shadow = new QGraphicsDropShadowEffect(books_with_notes);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  books_with_notes->setGraphicsEffect(shadow);
  books_with_notes->setObjectName("ApplyButton");
  connect(books_with_notes, &QPushButton::clicked, this,
          [this]
            {
              searchBook(SearchType::BooksWithNotes);
            });
  v_box->addWidget(books_with_notes, 0, Qt::AlignCenter);

  return result;
}

void
MainWindowLeftWidget::loadCollectionsList()
{
  std::string str = QDir::homePath().toStdString();
  std::filesystem::path collections_path
      = std::u8string(str.begin(), str.end());
  collections_path /= std::filesystem::path(u8".local");
  collections_path /= std::filesystem::path(u8"share");
  collections_path /= std::filesystem::path(u8"MyLibrary");
  collections_path /= std::filesystem::path(u8"Collections");

  if(std::filesystem::exists(collections_path))
    {
      for(auto &dir_it : std::filesystem::directory_iterator(collections_path))
        {
          std::filesystem::path p = dir_it.path();
          if(std::filesystem::is_directory(p))
            {
              QString str(p.filename().u8string().c_str());
              p /= std::filesystem::path(u8"base");
              QVariant var;
              var.setValue(p);
              collections->addItem(str, var);
            }
        }
    }
}

void
MainWindowLeftWidget::setActiveCollection()
{
  std::string str = QDir::homePath().toStdString();
  std::filesystem::path selcol = std::u8string(str.begin(), str.end());
  selcol /= std::filesystem::path(u8".cache")
            / std::filesystem::path(u8"MyLibrary")
            / std::filesystem::path(u8"ActiveCollection");
  std::fstream f;
  f.open(selcol, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string col;
      f.seekg(0, std::ios_base::end);
      col.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(col.data(), col.size());
      f.close();
      int ind = collections->findText(col.c_str());
      if(ind == -1)
        {
          collections->setCurrentIndex(0);
          selectCollection(0);
        }
      else
        {
          collections->setCurrentIndex(ind);
          selectCollection(ind);
        }
    }
  else
    {
      collections->setCurrentIndex(0);
      selectCollection(0);
    }
}

void
MainWindowLeftWidget::selectCollection(int index)
{
  QVariant var = collections->itemData(index);
  if(var.isNull())
    {
      return void();
    }
  std::filesystem::path p = var.value<std::filesystem::path>();
  if(p == current_collection_base_path)
    {
      return void();
    }
  if(base_loading_thr)
    {
      if(base_loading_thr->joinable())
        {
          base_loading_thr->join();
        }
    }
  current_collection_base_path = p;
  base_loading_thr = std::shared_ptr<std::thread>(new std::thread(
      [this, p]
        {
          emit signalCollectionLoaded(tr("loading"), tr("loading"));
          try
            {
              bases.base_keeper->loadCollection(p);
            }
          catch(std::exception &er)
            {
              std::cout << er.what() << std::endl;
              emit signalCollectionLoaded(tr("error"), tr("error"));
              return void();
            }
          size_t sz = bases.base_keeper->getBooksQuantity();
          QString str;
          str.setNum(sz);
          emit signalCollectionLoaded(
              str, bases.base_keeper->getCollectionType().c_str());
        }));
}

QWidget *
MainWindowLeftWidget::genrePopup()
{
  QWidget *result = new QWidget(genre_button);
  result->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
  result->setObjectName("Menu");

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  GenreView *genre_view = new GenreView(nullptr, bases.genres_base);
  genre_view->setObjectName("Table");
  genre_view->viewport()->setObjectName("TableViewport");
  v_box->addWidget(genre_view);

  connect(genre_view, &GenreView::signalGenreSelected, genre_button,
          [this, result](const QModelIndex &index)
            {
              const UDBElement *el = reinterpret_cast<const UDBElement *>(
                  index.constInternalPointer());

              selected_genre = el->content;
              auto it = std::find_if(el->subelements.begin(),
                                     el->subelements.end(),
                                     [](const UDBElement &el)
                                       {
                                         return el.id == "translation";
                                       });
              if(it != el->subelements.end())
                {
                  QString str(it->content.c_str());
                  str += " ";
                  str += QString(selected_genre.c_str());
                  genre_button->setText(str);
                }
              else
                {
                  QString str(selected_genre.c_str());
                  genre_button->setText(str);
                }
              result->setVisible(false);
            });

  return result;
}

void
MainWindowLeftWidget::clearSearchParameters()
{
  surname->setText("");
  first_name->setText("");
  second_name->setText("");
  nickname->setText("");
  book_name->setText("");
  series->setText("");
  series_number->setText("");

  selected_genre.clear();
  QString str = "<";
  str += tr("No");
  str += ">";
  genre_button->setText(str);
  emit signalClearSearchResult();
}

void
MainWindowLeftWidget::searchBook(const SearchType &type)
{
  if(!search_mtx->try_lock())
    {
      return void();
    }

  switch(type)
    {
    case SearchType::Book:
      {
        std::vector<UDBElement> request;
        UDBElement el;
        bid.setId(el, BaseID::Author);
        QString str = surname->text();
        if(!str.isEmpty())
          {
            UDBElement sub;
            bid.setId(sub, BaseID::LastName);
            sub.content = str.toStdString();
            el.subelements.emplace_back(sub);
          }
        str = first_name->text();
        if(!str.isEmpty())
          {
            UDBElement sub;
            bid.setId(sub, BaseID::FirstName);
            sub.content = str.toStdString();
            el.subelements.emplace_back(sub);
          }
        str = second_name->text();
        if(!str.isEmpty())
          {
            UDBElement sub;
            bid.setId(sub, BaseID::MiddleName);
            sub.content = str.toStdString();
            el.subelements.emplace_back(sub);
          }
        str = nickname->text();
        if(!str.isEmpty())
          {
            UDBElement sub;
            bid.setId(sub, BaseID::Nickname);
            sub.content = str.toStdString();
            el.subelements.emplace_back(sub);
          }
        if(el.subelements.size() > 0)
          {
            request.emplace_back(el);
          }

        el = UDBElement();
        bid.setId(el, BaseID::BookTitle);
        el.content = book_name->text().toStdString();
        if(!el.content.empty())
          {
            request.emplace_back(el);
          }

        el = UDBElement();
        bid.setId(el, BaseID::Sequence);
        str = series->text();
        if(!str.isEmpty())
          {
            UDBElement sub;
            bid.setId(sub, BaseID::SequenceName);
            sub.content = str.toStdString();
            el.subelements.emplace_back(sub);

            str = series_number->text();
            if(!str.isEmpty())
              {
                sub = UDBElement();
                bid.setId(sub, BaseID::SequenceNumber);
                sub.content = str.toStdString();
                el.subelements.emplace_back(sub);
              }
          }
        if(el.subelements.size() > 0)
          {
            request.emplace_back(el);
          }

        if(!selected_genre.empty())
          {
            el = UDBElement();
            bid.setId(el, BaseID::Genre);
            el.content = selected_genre;
            request.emplace_back(el);
          }

        double coef = static_cast<double>(coef_coincidence->sliderPosition());
        coef /= 100.0;

        SearchProcWindow *spw = new SearchProcWindow(this->window());
        spw->creatBookSearchWindow();
        connect(spw, &SearchProcWindow::signalCanceled, this,
                [this]
                  {
                    bases.base_keeper->cancelSearch();
                  });
        spw->show();

        std::thread thr(
            [this, request, coef, spw]
              {
                UDBase base
                    = std::move(bases.base_keeper->searchBook(request, coef));
                emit spw->signalStartSorting();
                emit signalBookSearchFinished(base, spw,
                                              current_collection_base_path);
                search_mtx->unlock();
              });
        thr.detach();
        break;
      }
    case SearchType::BooksWithNotes:
      {
        SearchProcWindow *spw = new SearchProcWindow(this->window());
        spw->creatBookSearchWindow();
        connect(spw, &SearchProcWindow::signalCanceled, this,
                [this]
                  {
                    bases.base_keeper->cancelSearch();
                  });
        spw->show();

        std::thread thr(
            [this, spw]
              {
                UDBase base = std::move(
                    bases.base_keeper->searchBooksWithNotes(bases.notes));
                emit spw->signalStartSorting();
                emit signalBookSearchFinished(base, spw,
                                              current_collection_base_path);
                search_mtx->unlock();
              });
        thr.detach();
        break;
      }
    default:
      break;
    }
}

void
MainWindowLeftWidget::showFiles()
{
  if(!search_mtx->try_lock())
    {
      return void();
    }
  SearchProcWindow *spw = new SearchProcWindow(this->window());
  spw->creatBookSearchWindow();
  connect(spw, &SearchProcWindow::signalCanceled, this,
          [this]
            {
              bases.base_keeper->cancelSearch();
            });
  spw->show();

  std::thread thr(
      [this, spw]
        {
          UDBase base = std::move(bases.base_keeper->getAllFiles());
          emit spw->signalStartSorting();
          emit signalFilesSearchFinished(base, spw,
                                         current_collection_base_path);
          search_mtx->unlock();
        });
  thr.detach();
}

void
MainWindowLeftWidget::showAuthors()
{
  if(!search_mtx->try_lock())
    {
      return void();
    }
  SearchProcWindow *spw = new SearchProcWindow(this->window());
  spw->createAuthorSearchWindow();
  connect(spw, &SearchProcWindow::signalCanceled, this,
          [this]
            {
              bases.base_keeper->cancelSearch();
            });
  spw->show();

  std::thread thr(
      [this, spw]
        {
          UDBase base = std::move(bases.base_keeper->listAllAuthors());
          emit spw->signalStartSorting();
          emit signalAuthorsSearchFinished(base, spw,
                                           current_collection_base_path);
          search_mtx->unlock();
        });
  thr.detach();
}
