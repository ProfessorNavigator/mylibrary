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

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>
#include <QVBoxLayout>
#include <SearchProcWindow.h>

SearchProcWindow::SearchProcWindow(QWidget *parent) : QWidget(parent)
{
  close_window.store(false, std::memory_order_relaxed);

  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
SearchProcWindow::creatBookSearchWindow()
{
  this->setWindowTitle(tr("Book search"));

  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Searching..."));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  connect(this, &SearchProcWindow::signalStartSorting, lab,
          [lab]
            {
              lab->setText(tr("Sorting..."));
            });

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this,
          [lab, cancel, this]
            {
              lab->setText(tr("Canceling..."));
              cancel->setVisible(false);
              emit signalCanceled();
            });
  v_box->addWidget(cancel, 0, Qt::AlignCenter);
}

void
SearchProcWindow::createAuthorSearchWindow()
{
  this->setWindowTitle(tr("Authors search"));

  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Searching..."));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  connect(this, &SearchProcWindow::signalStartSorting, lab,
          [lab]
            {
              lab->setText(tr("Sorting..."));
            });

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this,
          [lab, cancel, this]
            {
              lab->setText(tr("Canceling..."));
              cancel->setVisible(false);
              emit signalCanceled();
            });
  v_box->addWidget(cancel, 0, Qt::AlignCenter);
}

void
SearchProcWindow::createBaseLoadingWindow()
{
  this->setWindowTitle(tr("Collection loading"));

  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Loading collection..."));
  v_box->addWidget(lab, 0, Qt::AlignCenter);
}

void
SearchProcWindow::createCopyingWindow()
{
  this->setWindowTitle(tr("Copying"));

  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Copying..."));
  v_box->addWidget(lab, 0, Qt::AlignCenter);
}

void
SearchProcWindow::allowClose(const bool &allow)
{
  close_window.store(allow, std::memory_order_relaxed);
}

void
SearchProcWindow::closeEvent(QCloseEvent *event)
{
  if(close_window.load(std::memory_order_relaxed))
    {
      event->accept();
      QWidget::closeEvent(event);
    }
  else
    {
      event->ignore();
    }
}

void
SearchProcWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
