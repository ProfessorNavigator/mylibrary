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

#include <CollectionCreationProcWindow.h>
#include <MainWindow.h>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QVBoxLayout>
#include <StyledWindow.h>
#include <iostream>

CollectionCreationProcWindow::CollectionCreationProcWindow(
    QWidget *parent, const std::shared_ptr<MLBookProc> &mlbp,
    const std::filesystem::path &base_path,
    const std::vector<std::filesystem::path> &items, const int &threads)
    : QWidget(parent)
{
  cr_col = new CreateCollection(mlbp, threads);
  this->base_path = base_path;
  this->items = items;

  MainWindow *mw = dynamic_cast<MainWindow *>(parent);
  if(mw)
    {
      connect(this, &CollectionCreationProcWindow::signalCollectionCreated, mw,
              &MainWindow::signalCollectionCreated);
    }

  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(tr("Collection creation"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");

  canceled.store(false, std::memory_order_relaxed);
}

CollectionCreationProcWindow::~CollectionCreationProcWindow()
{
  if(cr_col_thread != nullptr)
    {
      if(cr_col_thread->joinable())
        {
          cr_col_thread->join();
        }
      delete cr_col_thread;
    }
  delete cr_col;
}

void
CollectionCreationProcWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Collection creation"));
  v_box->addWidget(lab, 0, Qt::AlignHCenter);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Files to parse:"));
  h_box->addWidget(lab, 0, Qt::AlignRight);

  files = new QLabel;
  files->setObjectName("Label");
  files->setText("0");
  h_box->addWidget(files, 0, Qt::AlignLeft);

  progress = new QProgressBar;
  progress->setObjectName("ProgressBar");
  progress->setMinimum(0);
  progress->setMaximum(100);
  v_box->addWidget(progress);

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
              canceled.store(true, std::memory_order_relaxed);
              cr_col->stopAll();
            });
  v_box->addWidget(cancel, 0, Qt::AlignCenter);
}

void
CollectionCreationProcWindow::startCreationProc()
{
  connect(this, &CollectionCreationProcWindow::signalSetFilesNum, files,
          [this](const size_t &num)
            {
              QString val;
              val.setNum(num);
              files->setText(val);
            });
  cr_col->signal_files_collecting
      = std::bind(&CollectionCreationProcWindow::signalSetFilesNum, this,
                  std::placeholders::_1);

  connect(this, &CollectionCreationProcWindow::signalSetProgress, progress,
          [this](double val, double max)
            {
              progress->setValue(static_cast<int>(((val / max) * 100.0)));
            });

  cr_col->signal_parsing_progress = [this](double val, double max)
    {
      emit signalSetProgress(val, max);
    };

  connect(this, &CollectionCreationProcWindow::signalFinished, this,
          &CollectionCreationProcWindow::slotFinished);

  cr_col_thread = new std::thread(
      [this]
        {
          try
            {
              cr_col->createCollection(items, base_path);
            }
          catch(std::exception &er)
            {
              std::cout << er.what() << std::endl;
              emit signalFinished(Result::Error, er.what());
              return void();
            }
          if(canceled.load(std::memory_order_relaxed))
            {
              emit signalFinished(
                  Result::Canceled,
                  base_path.parent_path().stem().u8string().c_str());
            }
          else
            {
              emit signalFinished(
                  Result::Success,
                  base_path.parent_path().stem().u8string().c_str());
            }
        });
}

void
CollectionCreationProcWindow::closeEvent(QCloseEvent *event)
{
  if(finished)
    {
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void
CollectionCreationProcWindow::slotFinished(const Result &variant,
                                           const QString &details)
{
  StyledWindow *window = new StyledWindow(this->parentWidget());
  window->setWindowModality(Qt::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

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
  lab->setText(details);
  h_box->addWidget(lab);

  h_box->addStretch();

  switch(variant)
    {
    case Result::Error:
      {
        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection creation error!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);
        break;
      }
    case Result::Success:
      {
        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection has been successfully created!"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);
        emit signalCollectionCreated(base_path);
        break;
      }
    case Result::Canceled:
      {
        lab = new QLabel;
        lab->setObjectName("Label");
        lab->setText(tr("Collection creation has been canceled"));
        v_box->addWidget(lab, 0, Qt::AlignCenter);
        break;
      }
    default:
      break;
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

  finished = true;

  this->close();
}

void
CollectionCreationProcWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
