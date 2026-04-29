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
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <TableView.h>
#include <SeriesEditWindow.h>
#include <StyledWindow.h>

SeriesEditWindow::SeriesEditWindow(QWidget *parent) : QWidget(parent)
{
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Series editor"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);

  this->setObjectName("Window");
}

SeriesEditWindow::~SeriesEditWindow()
{
  if(model != nullptr)
    {
      model->deleteLater();
    }
}

void
SeriesEditWindow::createWindow(const QModelIndex &index)
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Series editor"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  std::vector<UDBElement> series
      = index.data(Qt::EditRole).value<std::vector<UDBElement>>();

  model = new SeriesModel(nullptr, series);

  TableView *table = new TableView;
  table->setObjectName("Table");
  table->viewport()->setObjectName("TableViewport");
  connect(table, &TableView::signalResized, table,
          [table](const QSize &sz)
            {
              QHeaderView *hh = table->horizontalHeader();
              int total = sz.width();
              int col_sz = total * 0.7;
              hh->resizeSection(0, col_sz);
              hh->resizeSection(1, total - col_sz);
            });
  QAbstractItemModel *prev = table->model();
  table->setModel(model);
  if(prev != nullptr)
    {
      prev->deleteLater();
    }
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(table, &TableView::customContextMenuRequested,
          [table](const QPoint &pos)
            {
              QModelIndex index = table->indexAt(pos);
              table->setCurrentIndex(index);
              if(index.isValid())
                {
                  QMenu *menu = new QMenu(table);
                  menu->setAttribute(Qt::WA_DeleteOnClose);
                  menu->setObjectName("Menu");
                  menu->addActions(table->actions());
                  menu->popup(table->viewport()->mapToGlobal(pos));
                }
            });
  v_box->addWidget(table);

  QAction *edit_act = new QAction(tr("Edit"));
  connect(edit_act, &QAction::triggered, table,
          [table]
            {
              QModelIndex index = table->currentIndex();
              if(index.isValid())
                {
                  table->edit(index);
                }
            });
  table->addAction(edit_act);

  std::vector<QAction *> act_list;
  act_list.push_back(edit_act);

  QAction *add_act = new QAction(tr("Add series"));
  connect(add_act, &QAction::triggered, this,
          &SeriesEditWindow::addSeriesDialog);
  table->addAction(add_act);
  act_list.push_back(add_act);

  QAction *remove_act = new QAction(tr("Remove"));
  connect(remove_act, &QAction::triggered, table,
          [table, this]
            {
              QModelIndex index = table->currentIndex();
              model->removeSeries(index);
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

  QPushButton *add = new QPushButton;
  add->setText(tr("Add series"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(add);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add->setGraphicsEffect(shadow);
  add->setObjectName("ApplyButton");
  connect(add, &QPushButton::clicked, add_act, &QAction::trigger);
  h_box->addWidget(add, 0, Qt::AlignCenter);

  QPushButton *remove_selected = new QPushButton;
  remove_selected->setText(tr("Remove selected"));
  shadow = new QGraphicsDropShadowEffect(remove_selected);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  remove_selected->setGraphicsEffect(shadow);
  remove_selected->setObjectName("ClearButton");
  connect(remove_selected, &QPushButton::clicked, remove_act,
          &QAction::trigger);
  h_box->addWidget(remove_selected, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *apply = new QPushButton;
  apply->setText(tr("Apply"));
  shadow = new QGraphicsDropShadowEffect(apply);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  apply->setGraphicsEffect(shadow);
  apply->setObjectName("ApplyButton");
  connect(apply, &QPushButton::clicked, this,
          [this]
            {
              applied = true;
              this->close();
            });
  h_box->addWidget(apply, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, this, &SeriesEditWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(av_sz.height() * 0.3);
  av_sz.setWidth(av_sz.width() * 0.3);
  this->resize(av_sz);
}

std::vector<UDBElement>
SeriesEditWindow::getSeries()
{
  return model->getSeries();
}

void
SeriesEditWindow::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Return)
    {
      applied = true;
    }
  QWidget::keyPressEvent(event);
}

void
SeriesEditWindow::addSeriesDialog()
{
  StyledWindow *window = new StyledWindow(this);
  window->setWindowTitle(tr("Series editor"));
  window->setWindowModality(Qt::WindowModality::WindowModal);
  window->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Series name:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QLineEdit *name = new QLineEdit;
  name->setObjectName("LineEdit");
  v_box->addWidget(name);

  lab = new QLabel;
  lab->setObjectName("Label");
  lab->setText(tr("Number:"));
  v_box->addWidget(lab, 0, Qt::AlignLeft);

  QLineEdit *number = new QLineEdit;
  number->setObjectName("LineEdit");
  v_box->addWidget(number);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *add = new QPushButton;
  add->setText(tr("Add"));
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(add);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  add->setGraphicsEffect(shadow);
  add->setObjectName("ApplyButton");
  connect(add, &QPushButton::clicked, model,
          [this, name, number, window]
            {
              QString txt = name->text();
              if(txt.isEmpty())
                {
                  return void();
                }
              UDBElement series;
              BaseID bid;
              bid.setId(series, BaseID::Sequence);

              UDBElement el;
              bid.setId(el, BaseID::SequenceName);
              el.content = txt.toStdString();
              series.subelements.emplace_back(el);

              txt = number->text();
              if(!txt.isEmpty())
                {
                  el = UDBElement();
                  bid.setId(el, BaseID::SequenceNumber);
                  el.content = txt.toStdString();
                  series.subelements.emplace_back(el);
                }
              model->addSeries(series);

              window->close();
            });
  h_box->addWidget(add, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  shadow = new QGraphicsDropShadowEffect(cancel);
  shadow->setBlurRadius(12);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 120));
  cancel->setGraphicsEffect(shadow);
  cancel->setObjectName("CancelButton");
  connect(cancel, &QPushButton::clicked, window, &QWidget::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  window->show();
}

void
SeriesEditWindow::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
