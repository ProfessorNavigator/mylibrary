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

#include <GenreModel.h>
#include <algorithm>

GenreModel::GenreModel(QObject *parent,
                       const std::shared_ptr<GenreBase> &genre_base)
    : QAbstractItemModel(parent)
{
  base = genre_base;
}

QModelIndex
GenreModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  Q_UNUSED(column);

  std::vector<UDBElement> *raw_base = base->getRawBase();
  if(parent.isValid())
    {
      const UDBElement *el = reinterpret_cast<const UDBElement *>(
          parent.constInternalPointer());
      if(el == nullptr)
        {
          return result;
        }
      if(el->id != "group")
        {
          return result;
        }
      int count = 0;
      const UDBElement *sub = el->subelements.data();
      for(size_t i = 0; i < el->subelements.size(); i++)
        {
          if((sub + i)->id == "genre")
            {
              if(count == row)
                {
                  result = createIndex(row, column, (sub + i));
                  break;
                }
              count++;
            }
        }
    }
  else
    {
      if(static_cast<size_t>(row) >= raw_base->size())
        {
          return result;
        }
      UDBElement *it = raw_base->data() + row;
      result = createIndex(row, column, it);
    }

  return result;
}

QModelIndex
GenreModel::parent(const QModelIndex &index) const
{
  QModelIndex result;

  if(index.isValid())
    {
      std::vector<UDBElement> *raw_base = base->getRawBase();

      const UDBElement *el
          = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
      if(el == nullptr)
        {
          return result;
        }
      if(el->id == "group")
        {
          return result;
        }

      int count = 0;
      UDBElement *it = raw_base->data();
      for(size_t i = 0; i < raw_base->size(); i++)
        {
          UDBElement *sub = (it + i)->subelements.data();
          for(size_t j = 0; j < (it + i)->subelements.size(); j++)
            {
              if((sub + j) == el)
                {
                  result = createIndex(count, 0, (it + i));
                  return result;
                }
            }
          count++;
        }
    }

  return result;
}

int
GenreModel::rowCount(const QModelIndex &parent) const
{
  int result = 0;

  if(parent.isValid())
    {
      std::vector<UDBElement> *raw = base->getRawBase();
      size_t row = static_cast<size_t>(parent.row());
      if(row < raw->size())
        {
          for(auto it = raw->at(row).subelements.begin();
              it != raw->at(row).subelements.end(); it++)
            {
              if(it->id == "genre")
                {
                  result++;
                }
            }
        }
    }
  else
    {
      result = static_cast<int>(base->baseSize());
    }

  return result;
}

int
GenreModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  return 1;
}

QVariant
GenreModel::data(const QModelIndex &index, int role) const
{
  QVariant result;
  if(!index.isValid())
    {
      return result;
    }

  switch(role)
    {
    case Qt::DisplayRole:
      {
        const UDBElement *el = reinterpret_cast<const UDBElement *>(
            index.constInternalPointer());
        auto it = std::find_if(el->subelements.begin(), el->subelements.end(),
                               [](const UDBElement &el)
                                 {
                                   return el.id == "translation";
                                 });
        if(it != el->subelements.end())
          {
            QString str(it->content.c_str());
            if(el->id == "genre")
              {
                str += " ";
                str += QString(el->content.c_str());
              }
            result = QVariant(str);
          }
        else
          {
            QString str(el->content.c_str());
            result = QVariant(str);
          }
        break;
      }
    default:
      break;
    }

  return result;
}

Qt::ItemFlags
GenreModel::flags(const QModelIndex &index) const
{
  if(!index.isValid())
    {
      return Qt::NoItemFlags;
    }
  Qt::ItemFlags result = Qt::ItemIsEnabled;
  const UDBElement *el
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(el != nullptr)
    {
      if(el->id == "genre")
        {
          result = result | Qt::ItemNeverHasChildren;
        }
    }

  return result;
}
