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

#include <AuthorEditWindow.h>
#include <GenreEditWindow.h>
#include <QFontMetrics>
#include <QLineEdit>
#include <SearchResultItemDelegate.h>
#include <SearchViewModel.h>
#include <SeriesEditWindow.h>

SearchResultItemDelegate::SearchResultItemDelegate(
    QObject *obj, const std::shared_ptr<SettingsManager> &settings)
    : StyledItemDelegate(obj, settings)
{
}

QWidget *
SearchResultItemDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
  QWidget *result = nullptr;
  switch(index.column())
    {
    case 0:
      {
        result = new AuthorEditWindow(parent->window(), settings);
        break;
      }
    case 1:
    case 4:
      {
        result = new QLineEdit(parent);
        result->setObjectName("LineEdit");
        result->resize(option.decorationSize);
        break;
      }
    case 2:
      {
        result = new SeriesEditWindow(parent->window(), settings);
        break;
      }
    case 3:
      {
        result = new GenreEditWindow(parent->window());
        break;
      }
    default:
      break;
    }

  return result;
}

void
SearchResultItemDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const
{
  switch(index.column())
    {
    case 0:
      {
        AuthorEditWindow *aew = dynamic_cast<AuthorEditWindow *>(editor);
        if(aew != nullptr)
          {
            aew->createWindow(index);
          }
        break;
      }
    case 1:
    case 4:
      {
        QLineEdit *w = dynamic_cast<QLineEdit *>(editor);
        if(w != nullptr)
          {
            w->setText(index.data(Qt::EditRole).toString());
          }
        break;
      }
    case 2:
      {
        SeriesEditWindow *sew = dynamic_cast<SeriesEditWindow *>(editor);
        if(sew != nullptr)
          {
            sew->createWindow(index);
          }
        break;
      }
    case 3:
      {
        GenreEditWindow *gew = dynamic_cast<GenreEditWindow *>(editor);
        if(gew != nullptr)
          {
            gew->createWindow(index);
          }
        break;
      }
    default:
      {
        break;
      }
    }
}

void
SearchResultItemDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
  switch(index.column())
    {
    case 0:
      {
        AuthorEditWindow *aew = dynamic_cast<AuthorEditWindow *>(editor);
        SearchViewModel *s_model = dynamic_cast<SearchViewModel *>(model);
        if(aew != nullptr && s_model != nullptr)
          {
            if(aew->applied)
              {
                std::vector<UDBElement> authors = aew->getAuthors();
                s_model->setAuthors(index, authors);
              }
          }
        break;
      }
    case 1:
      {
        QLineEdit *w = dynamic_cast<QLineEdit *>(editor);
        SearchViewModel *s_model = dynamic_cast<SearchViewModel *>(model);
        if(w != nullptr && s_model != nullptr)
          {
            QString title = w->text();
            s_model->setBookTitle(index, title);
          }
        break;
      }
    case 2:
      {
        SeriesEditWindow *sew = dynamic_cast<SeriesEditWindow *>(editor);
        SearchViewModel *s_model = dynamic_cast<SearchViewModel *>(model);
        if(sew != nullptr && s_model != nullptr)
          {
            if(sew->applied)
              {
                std::vector<UDBElement> series = sew->getSeries();
                s_model->setSeries(index, series);
              }
          }
        break;
      }
    case 3:
      {
        GenreEditWindow *gew = dynamic_cast<GenreEditWindow *>(editor);
        SearchViewModel *s_model = dynamic_cast<SearchViewModel *>(model);
        if(gew != nullptr && s_model != nullptr)
          {
            if(gew->applied)
              {
                std::vector<UDBElement> genres = gew->getGenres();
                s_model->setGenres(index, genres);
              }
          }
        break;
      }
    case 4:
      {
        QLineEdit *w = dynamic_cast<QLineEdit *>(editor);
        SearchViewModel *s_model = dynamic_cast<SearchViewModel *>(model);
        if(w != nullptr && s_model != nullptr)
          {
            QString date = w->text();
            s_model->setDate(index, date);
          }
        break;
      }
    default:
      {
        break;
      }
    }
}

void
SearchResultItemDelegate::destroyEditor(QWidget *editor,
                                        const QModelIndex &index) const
{
  editor->deleteLater();
}
