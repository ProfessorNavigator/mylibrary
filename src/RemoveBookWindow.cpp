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

#include <MainWindow.h>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStyleOption>
#include <QVBoxLayout>
#include <RemoveBookWindow.h>
#include <StyledWindow.h>
#include <algorithm>
#include <thread>

RemoveBookWindow::RemoveBookWindow(QWidget *parent,
                                   const std::shared_ptr<MLBookProc> &mlbp)
    : QWidget{ parent }
{
  this->mlbp = mlbp;
  allow_destroy.store(true, std::memory_order_relaxed);

  rmb = new RemoveBook(mlbp);

  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

RemoveBookWindow::~RemoveBookWindow()
{
  delete rmb;
}

void
RemoveBookWindow::showWindow(const UDBElement &book_search_result,
                             const std::filesystem::path &base_path)
{
  StyledWindow *window = new StyledWindow(this->parentWidget());
  window->setWindowModality(Qt::WindowModality::WindowModal);
  window->setObjectName("Window");

  connect(window, &QWidget::destroyed, this,
          [this]
            {
              if(allow_destroy.load(std::memory_order_relaxed))
                {
                  this->deleteLater();
                }
            });

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box, 0);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Book to remove:"));
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == book_search_result.subelements.end())
    {
      window->deleteLater();
      return void();
    }

  auto it_tit
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::BookTitle;
                       });
  if(it_tit == it_book->subelements.end())
    {
      window->deleteLater();
      return void();
    }

  lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(it_tit->content.c_str());
  h_box->addWidget(lab, 0, Qt::AlignLeft);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Note: if book is in archive, archive will be repacked"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(
      tr("Note: if book is in rar archive, whole archive will be removed"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);  

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Continue?"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box, 0);

  QPushButton *yes = new QPushButton;
  yes->setText(tr("Yes"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(yes);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  yes->setGraphicsEffect(shadow);
  yes->setObjectName("ApplyButton");
  connect(yes, &QPushButton::clicked, this,
          [this, book_search_result, base_path, window]
            {
              removeProcWindow(book_search_result, base_path);
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
RemoveBookWindow::removeProcWindow(const UDBElement &book_search_result,
                                   const std::filesystem::path &base_path)
{
  allow_destroy.store(false, std::memory_order_relaxed);

  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Removing..."));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  QProgressBar *progr = new QProgressBar;
  progr->setObjectName("ProgressBar");
  progr->setMinimum(0);
  progr->setMaximum(0);
  v_box->addWidget(progr);

  connect(this, &RemoveBookWindow::signalProgress, progr,
          [progr](double processed, double total)
            {
              if(progr->maximum() == 0)
                {
                  progr->setMaximum(100);
                }
              double val = processed * 100.0 / total;
              progr->setValue(static_cast<int>(val));
            });

  this->show();

  rmb->signal_parsing_progress = [this](double processed, double total)
    {
      emit signalProgress(processed, total);
    };

  connect(this, &RemoveBookWindow::signalCompleted, this,
          &RemoveBookWindow::finalDialog);

  std::thread thr(
      [this, base_path, book_search_result]
        {
          ErrorType e_t = ErrorType::Success;
          QString text;
          try
            {
              rmb->removeBook(base_path, book_search_result);
            }
          catch(std::exception &er)
            {
              text = er.what();
              e_t = ErrorType::Error;
            }
          allow_destroy.store(true, std::memory_order_relaxed);

          emit signalCompleted(e_t, text, book_search_result, base_path);
        });
  thr.detach();
}

void
RemoveBookWindow::closeEvent(QCloseEvent *event)
{
  if(allow_destroy.load(std::memory_order_relaxed))
    {
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void
RemoveBookWindow::finalDialog(const ErrorType &er, const QString &er_text,
                              const UDBElement &book_search_result,
                              const std::filesystem::path &base_path)
{
  StyledWindow *window = new StyledWindow(this->parentWidget());
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  if(er == ErrorType::Error)
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Error on book removing!"));
      v_box->addWidget(lab, 0, Qt::AlignCenter);

      lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(er_text);
      v_box->addWidget(lab, 0, Qt::AlignCenter);
    }
  else
    {
      QLabel *lab = new QLabel;
      lab->setObjectName("Label");
      lab->setText(tr("Book has been removed"));
      v_box->addWidget(lab, 0, Qt::AlignCenter);
      emit signalBookRemoved(book_search_result);
    }

  QPushButton *close = new QPushButton;
  close->setText(tr("Close"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(close);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  close->setGraphicsEffect(shadow);
  close->setObjectName("ApplyButton");
  connect(close, &QPushButton::clicked, window, &StyledWindow::close);
  v_box->addWidget(close, 0, Qt::AlignCenter);

  window->show();

  QWidgetList list = QApplication::topLevelWidgets();
  for(qsizetype i = 0; i < list.size(); i++)
    {
      MainWindow *mw = dynamic_cast<MainWindow *>(list[i]);
      if(mw != nullptr)
        {
          std::u8string u8str = base_path.parent_path().filename().u8string();
          std::string str(u8str.begin(), u8str.end());
          emit mw->signalCollectionRefreshed(str.c_str());
          break;
        }
    }

  this->close();
}

void
RemoveBookWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
