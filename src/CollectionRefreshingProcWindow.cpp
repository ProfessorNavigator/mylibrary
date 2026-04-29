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

#include <CollectionRefreshingProcWindow.h>
#include <MainWindow.h>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>
#include <iostream>
#include <thread>

CollectionRefreshingProcWindow::CollectionRefreshingProcWindow(
    QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
    const std::filesystem::path &base_path, const int &threads,
    const bool &fast, const std::vector<std::filesystem::path> &refresh_files)
    : QWidget(parent)
{
  this->base_path = base_path;
  this->fast = fast;
  this->refresh_files = refresh_files;

  allow_close.store(false, std::memory_order_relaxed);
  canceled.store(Result::Success, std::memory_order_relaxed);

  refresh = new RefreshCollection(mlbp, threads);

  this->setWindowTitle(tr("Collection refreshing"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");

  showWindow();
}

CollectionRefreshingProcWindow::~CollectionRefreshingProcWindow()
{
  delete refresh;
}

void
CollectionRefreshingProcWindow::showWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Refreshing collection"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  lab = new QLabel;
  lab->setObjectName("Label");
  font = lab->font();
  font.setItalic(true);
  lab->setFont(font);
  lab->setText(base_path.parent_path().filename().u8string().c_str());
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Files to process:"));
  h_box->addWidget(lab);

  files = new QLabel;
  files->setObjectName("Label");
  files->setText("0");
  h_box->addWidget(files);

  h_box->addStretch();

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText("Hashing progress:");
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  hashing = new QProgressBar;
  hashing->setObjectName("ProgressBar");
  hashing->setMinimum(0);
  hashing->setMaximum(100);
  hashing->setTextVisible(true);
  v_box->addWidget(hashing);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Refreshing:"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  refreshing = new QProgressBar;
  refreshing->setObjectName("ProgressBar");
  refreshing->setMinimum(0);
  refreshing->setMaximum(100);
  refreshing->setTextVisible(true);
  v_box->addWidget(refreshing);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this,
          [this]
            {
              canceled.store(Result::Canceled, std::memory_order_relaxed);
              refresh->stopAll();
            });
  v_box->addWidget(cancel, 0, Qt::AlignCenter);

  this->show();

  startRefreshing();
}

void
CollectionRefreshingProcWindow::startRefreshing()
{
  connect(this, &CollectionRefreshingProcWindow::signalFilesCollecting, files,
          [this](const size_t &files_found)
            {
              QString str;
              str.setNum(files_found);
              files->setText(str);
            });

  refresh->signal_files_collecting = [this](const size_t &files_found)
    {
      emit signalFilesCollecting(files_found);
    };

  connect(this, &CollectionRefreshingProcWindow::signalParsingProcess,
          refreshing,
          [this](const double &processed, const double &total)
            {
              double progress = (processed / total) * 100.0;
              refreshing->setValue(static_cast<int>(progress));
            });
  refresh->signal_parsing_progress = [this](double processed, double total)
    {
      emit signalParsingProcess(processed, total);
    };

  connect(this, &CollectionRefreshingProcWindow::signalFileHashed, hashing,
          [this](const double &processed, const double &total)
            {
              double progress = (processed / total) * 100.0;
              hashing->setValue(static_cast<int>(progress));
            });
  refresh->signal_file_hashed = [this](double processed, double total)
    {
      emit signalFileHashed(processed, total);
    };

  connect(this, &CollectionRefreshingProcWindow::signalFinished, this,
          &CollectionRefreshingProcWindow::finishedDialog);

  std::thread thr(
      [this]
        {
          QString err;
          try
            {
              if(refresh_files.empty())
                {
                  refresh->refreshCollection(base_path, fast);
                }
              else
                {
                  refresh->addFilesAndDirs(base_path, refresh_files);
                }
            }
          catch(std::exception &er)
            {
              std::cout << er.what() << std::endl;
              canceled.store(Result::Error, std::memory_order_relaxed);
              err = er.what();
            }
          emit signalFinished(err);
        });
  thr.detach();
}

void
CollectionRefreshingProcWindow::closeEvent(QCloseEvent *event)
{
  if(allow_close.load(std::memory_order_relaxed))
    {
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void
CollectionRefreshingProcWindow::finishedDialog(const QString &err)
{
  StyledWindow *window = new StyledWindow(this->parentWidget());
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  switch(canceled.load(std::memory_order_relaxed))
    {
    case Result::Success:
      {
        QHBoxLayout *h_box = new QHBoxLayout;
        v_box->addLayout(h_box);

        h_box->addStretch();

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection") + ":");
        h_box->addWidget(lab);

        lab = new QLabel;
        QFont font = lab->font();
        font.setItalic(true);
        lab->setFont(font);
        lab->setObjectName("Label");
        lab->setText(base_path.parent_path().filename().u8string().c_str());
        h_box->addWidget(lab);

        h_box->addStretch();

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection has been successfully refreshed!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        MainWindow *mw = nullptr;
        QList<QWidget *> list = QApplication::topLevelWidgets();
        for(qsizetype i = 0; i < list.size(); i++)
          {
            mw = dynamic_cast<MainWindow *>(list[i]);
            if(mw != nullptr)
              {
                break;
              }
          }

        if(mw != nullptr)
          {
            emit mw->signalCollectionRefreshed(
                base_path.parent_path().filename().u8string().c_str());
          }

        break;
      }
    case Result::Canceled:
      {
        QHBoxLayout *h_box = new QHBoxLayout;
        v_box->addLayout(h_box);

        h_box->addStretch();

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection") + ":");
        h_box->addWidget(lab);

        lab = new QLabel;
        QFont font = lab->font();
        font.setItalic(true);
        lab->setFont(font);
        lab->setObjectName("Label");
        lab->setText(base_path.parent_path().filename().u8string().c_str());
        h_box->addWidget(lab);

        h_box->addStretch();

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection refreshing has been canceled!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        break;
      }
    case Result::Error:
      {
        QHBoxLayout *h_box = new QHBoxLayout;
        v_box->addLayout(h_box);

        h_box->addStretch();

        QLabel *lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection") + ":");
        h_box->addWidget(lab);

        lab = new QLabel;
        QFont font = lab->font();
        font.setItalic(true);
        lab->setFont(font);
        lab->setObjectName("Label");
        lab->setText(base_path.parent_path().filename().u8string().c_str());
        h_box->addWidget(lab);

        h_box->addStretch();

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection refreshing error!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(err);
        v_box->addWidget(lab, 0, Qt::AlignCenter);

        break;
      }
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

  allow_close.store(true, std::memory_order_relaxed);
  this->close();
}

void
CollectionRefreshingProcWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
