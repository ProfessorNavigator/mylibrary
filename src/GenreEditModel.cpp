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

#include <GenreEditModel.h>

GenreEditModel::GenreEditModel(QObject *parent,
                               const std::vector<UDBElement> &genres)
    : QAbstractItemModel(parent)
{
  this->genres = genres;
  genre_base = new GenreBase;
}

GenreEditModel::~GenreEditModel()
{
  delete genre_base;
}

QModelIndex
GenreEditModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  if(parent.isValid())
    {
      return result;
    }

  if(static_cast<size_t>(row) >= genres.size())
    {
      return result;
    }

  const UDBElement *el = genres.data() + row;

  result = createIndex(row, column, el);

  return result;
}

QModelIndex
GenreEditModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
GenreEditModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return static_cast<int>(genres.size());
}

int
GenreEditModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return 2;
}

QVariant
GenreEditModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  const UDBElement *el
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(el == nullptr)
    {
      return result;
    }

  switch(index.column())
    {
    case 0:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              QString str = genre_base->getTranslationByGenreCode(el->content);
              result = QVariant(str);
              break;
            }
          case Qt::TextAlignmentRole:
            {
              result = QVariant(Qt::AlignCenter);
              break;
            }
          default:
            break;
          }
        break;
      }
    case 1:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              QString str = el->content.c_str();
              result = QVariant(str);
              break;
            }
          case Qt::TextAlignmentRole:
            {
              result = QVariant(Qt::AlignCenter);
              break;
            }
          default:
            break;
          }
        break;
      }
    default:
      break;
    }

  return result;
}

Qt::ItemFlags
GenreEditModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  if(index.column() == 1)
    {
      result |= Qt::ItemIsEditable;
    }

  return result;
}

QVariant
GenreEditModel::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  QVariant result;

  switch(orientation)
    {
    case Qt::Orientation::Horizontal:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              switch(section)
                {
                case 0:
                  {
                    QString str(tr("Genre name"));
                    result = QVariant(str);
                    break;
                  }
                case 1:
                  {
                    QString str(tr("Genre code"));
                    result = QVariant(str);
                    break;
                  }
                default:
                  break;
                }
              break;
            }
          case Qt::TextAlignmentRole:
            {
              result = QVariant(Qt::AlignCenter);
              break;
            }
          default:
            break;
          }
        break;
      }
    case Qt::Orientation::Vertical:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              QString str;
              str.setNum(section + 1);
              result = QVariant(str);
              break;
            }
          case Qt::TextAlignmentRole:
            {
              result = QVariant(Qt::AlignCenter);
              break;
            }
          default:
            break;
          }
        break;
      }
    default:
      break;
    }

  return result;
}

void
GenreEditModel::setGenre(const QModelIndex &index,
                         const std::string &genre_code)
{
  if(genre_code.empty())
    {
      return void();
    }
  UDBElement *el = const_cast<UDBElement *>(
      reinterpret_cast<const UDBElement *>(index.constInternalPointer()));
  if(el != nullptr)
    {
      el->content = genre_code;
      emit dataChanged(index, index);
    }
}

void
GenreEditModel::addGenre(const std::string &genre_code)
{
  UDBElement el;
  bid.setId(el, BaseID::Genre);
  el.content = genre_code;

  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  genres.emplace_back(el);
  endInsertRows();
}

void
GenreEditModel::removeGenre(const QModelIndex &index)
{
  if(!index.isValid())
    {
      return void();
    }

  if(static_cast<size_t>(index.row()) >= genres.size())
    {
      return void();
    }

  beginRemoveRows(QModelIndex(), index.row(), index.row());
  genres.erase(genres.begin() + index.row());
  endRemoveRows();
}

std::vector<UDBElement>
GenreEditModel::getGenres()
{
  return genres;
}
