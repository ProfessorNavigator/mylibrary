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

#include <CoverWindow.h>
#include <QLabel>
#include <QMargins>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>

CoverWindow::CoverWindow(QWidget *parent, const QImage &cover)
    : QWidget(parent)
{
  this->cover = cover;

  this->setWindowTitle(tr("Cover"));
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

void
CoverWindow::createWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  QMargins margins;
  margins.setBottom(0);
  margins.setTop(0);
  margins.setLeft(0);
  margins.setRight(0);
  v_box->setContentsMargins(margins);
  this->setLayout(v_box);

  QLabel *cover_widget = new QLabel;
  cover_widget->setMinimumSize(cover.size());
  cover_widget->setAlignment(Qt::AlignCenter);
  cover_widget->setPixmap(QPixmap::fromImage(cover));

  QScrollArea *scrl = new QScrollArea;
  scrl->setObjectName("CoverWindowScrollArea");
  scrl->setFrameStyle(QFrame::NoFrame);
  scrl->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrl->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrl->setWidgetResizable(true);
  scrl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
  scrl->setWidget(cover_widget);
  v_box->addWidget(scrl);

  QSize win_sz;
  win_sz.setWidth(cover.width());
  win_sz.setHeight(cover.height());

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  if(win_sz.height() > av_sz.height())
    {
      win_sz.setHeight(av_sz.height());
    }
  if(win_sz.width() > av_sz.width())
    {
      win_sz.setWidth(av_sz.width());
    }

  this->resize(win_sz);
}

void
CoverWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}