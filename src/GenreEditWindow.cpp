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

#include <GenreEditDelegate.h>
#include <GenreEditWindow.h>
#include <GenreView.h>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <TableView.h>

GenreEditWindow::GenreEditWindow(QWidget *parent) : QWidget(parent)
{
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowTitle(tr("Genres editor"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);
}

GenreEditWindow::~GenreEditWindow()
{
  if(model != nullptr)
    {
      model->deleteLater();
    }
}

void
GenreEditWindow::createWindow(const QModelIndex &index)
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  QFont font = lab->font();
  font.setBold(true);
  lab->setFont(font);
  lab->setText(tr("Genres editor"));
  v_box->addWidget(lab, 0, Qt::AlignCenter);

  std::vector<UDBElement> genres
      = index.data(Qt::EditRole).value<std::vector<UDBElement>>();

  model = new GenreEditModel(nullptr, genres);

  TableView *table = new TableView;
  connect(table, &TableView::signalResized, table,
          [table](const QSize &sz)
            {
              int total = sz.width();
              int col_width = total * 0.5;
              QHeaderView *hh = table->horizontalHeader();
              hh->resizeSection(0, col_width);
              hh->resizeSection(1, total - col_width);
            });
  QAbstractItemDelegate *prev_d = table->itemDelegate();
  GenreEditDelegate *delegate = new GenreEditDelegate;
  table->setItemDelegate(delegate);
  if(prev_d != nullptr)
    {
      prev_d->deleteLater();
    }
  QAbstractItemModel *prev = table->model();
  table->setModel(model);
  if(prev != nullptr)
    {
      prev->deleteLater();
    }
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
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

  QAction *add_act = new QAction(tr("Add genre"));
  connect(add_act, &QAction::triggered, this,
          &GenreEditWindow::addGenreDialog);
  table->addAction(add_act);
  act_list.push_back(add_act);

  QAction *remove_act = new QAction(tr("Remove"));
  connect(remove_act, &QAction::triggered, table,
          [table, this]
            {
              QModelIndex index = table->currentIndex();
              model->removeGenre(index);
            });
  table->addAction(remove_act);
  act_list.push_back(remove_act);

  act_list.shrink_to_fit();
  connect(table, &TableView::destroyed,
          [act_list, delegate]
            {
              delegate->deleteLater();
              for(size_t i = 0; i < act_list.size(); i++)
                {
                  delete act_list[i];
                }
            });

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *add = new QPushButton;
  add->setText(tr("Add genre"));
  connect(add, &QPushButton::clicked, add_act, &QAction::trigger);
  h_box->addWidget(add, 0, Qt::AlignCenter);

  QPushButton *remove = new QPushButton;
  remove->setText(tr("Remove selected"));
  connect(remove, &QPushButton::clicked, remove_act, &QAction::trigger);
  h_box->addWidget(remove, 0, Qt::AlignCenter);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *apply = new QPushButton;
  apply->setText(tr("Apply"));
  connect(apply, &QPushButton::clicked, this,
          [this]
            {
              applied = true;
              this->close();
            });
  h_box->addWidget(apply, 0, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  connect(cancel, &QPushButton::clicked, this, &GenreEditWindow::close);
  h_box->addWidget(cancel, 0, Qt::AlignCenter);

  QScreen *screen = this->parentWidget()->screen();
  QSize av_sz = screen->availableSize();
  av_sz.setHeight(av_sz.height() * 0.3);
  av_sz.setWidth(av_sz.width() * 0.3);
  this->resize(av_sz);
}

std::vector<UDBElement>
GenreEditWindow::getGenres()
{
  return model->getGenres();
}

void
GenreEditWindow::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Return)
    {
      applied = true;
    }
  QWidget::keyPressEvent(event);
}

void
GenreEditWindow::addGenreDialog()
{
  QWidget *window = new QWidget(this);
  window->setWindowFlags(Qt::Window);
  window->setAttribute(Qt::WA_DeleteOnClose);
  window->setWindowModality(Qt::WindowModal);

  QVBoxLayout *v_box = new QVBoxLayout;
  window->setLayout(v_box);

  std::shared_ptr<GenreBase> genre_base(new GenreBase);
  GenreView *genre_view = new GenreView(nullptr, genre_base);
  connect(genre_view, &GenreView::signalGenreSelected, window,
          [window, genre_view, this]
            {
              QModelIndex index = genre_view->currentIndex();
              if(index.isValid())
                {
                  const UDBElement *el = reinterpret_cast<const UDBElement *>(
                      index.constInternalPointer());
                  if(el != nullptr)
                    {
                      model->addGenre(el->content);
                    }
                }
              window->close();
            });
  v_box->addWidget(genre_view);

  QSize sz = this->size();
  sz.setWidth(window->sizeHint().width());
  window->resize(sz);

  window->show();
}
