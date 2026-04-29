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
#include <QHeaderView>

GenreView::GenreView(QWidget *parent,
                     const std::shared_ptr<GenreBase> &genre_base)
    : QTreeView(parent)
{
  model = new GenreModel(nullptr, genre_base);
  this->setModel(model);
  QHeaderView *header = this->header();
  header->setVisible(false);
}

GenreView::~GenreView()
{
  delete model;
}

void
GenreView::mousePressEvent(QMouseEvent *event)
{
  QTreeView::mousePressEvent(event);
  QModelIndex index = this->indexAt(event->position().toPoint());
  if(index.isValid())
    {
      const UDBElement *el
          = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
      if(el == nullptr)
        {
          return void();
        }
      if(el->id == "genre")
        {
          emit signalGenreSelected(index);
        }
    }
}
