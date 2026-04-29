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
#include <GenreEditModel.h>
#include <GenreView.h>
#include <QVBoxLayout>
#include <StyledWindow.h>

GenreEditDelegate::GenreEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *
GenreEditDelegate::createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
  StyledWindow *result = new StyledWindow(parent->window());
  result->setWindowModality(Qt::WindowModal);
  result->setObjectName("Window");

  QVBoxLayout *v_box = new QVBoxLayout;
  result->setLayout(v_box);

  std::shared_ptr<GenreBase> genre_base(new GenreBase);
  GenreView *genre_view = new GenreView(nullptr, genre_base);
  genre_view->setObjectName("Table");
  genre_view->viewport()->setObjectName("TableViewport");
  connect(genre_view, &GenreView::signalGenreSelected, result,
          &QWidget::close);
  v_box->addWidget(genre_view);

  return result;
}

void
GenreEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  QLayout *layout = editor->layout();
  for(int i = 0; i < layout->count(); i++)
    {
      QLayoutItem *item = layout->itemAt(i);
      if(item != nullptr)
        {
          GenreView *genre_view = dynamic_cast<GenreView *>(item->widget());
          if(genre_view != nullptr)
            {
              QModelIndex sel = genre_view->currentIndex();
              if(sel.isValid())
                {
                  const UDBElement *el = reinterpret_cast<const UDBElement *>(
                      sel.constInternalPointer());
                  if(el != nullptr)
                    {
                      GenreEditModel *gem
                          = dynamic_cast<GenreEditModel *>(model);
                      if(gem != nullptr)
                        {
                          gem->setGenre(index, el->content);
                        }
                    }
                }
              break;
            }
        }
    }
}

void
GenreEditDelegate::updateEditorGeometry(QWidget *editor,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
  QSize av_sz = editor->parentWidget()->window()->size();
  av_sz.setWidth(editor->sizeHint().width());
  editor->resize(av_sz);
}
