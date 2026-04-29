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

#include <Algorithm.h>
#include <SearchViewModelAuthors.h>
#include <algorithm>

SearchViewModelAuthors::SearchViewModelAuthors(QObject *parent,
                                               const UDBase &authors)
    : QAbstractItemModel(parent)
{
  this->authors = authors;
  std::vector<UDBElement> *raw_base = this->authors.getRawBase();
  auto it = std::find_if(raw_base->begin(), raw_base->end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::CollectionInfo;
                           });
  if(it != raw_base->end())
    {
      collection_info = *it;
      raw_base->erase(it);
    }
}

QModelIndex
SearchViewModelAuthors::index(int row, int column,
                              const QModelIndex &parent) const
{
  QModelIndex result;
  if(parent.isValid())
    {
      return result;
    }

  std::vector<UDBElement> *raw_base;
  if(filter_set)
    {
      raw_base = filtered.getRawBase();
    }
  else
    {
      raw_base = authors.getRawBase();
    }
  if(static_cast<size_t>(row) >= raw_base->size())
    {
      return result;
    }
  const UDBElement *el = raw_base->data() + row;

  result = createIndex(row, column, el);

  return result;
}

QModelIndex
SearchViewModelAuthors::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
SearchViewModelAuthors::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  if(filter_set)
    {
      return static_cast<int>(filtered.baseSize());
    }
  else
    {
      return static_cast<int>(authors.baseSize());
    }
}

int
SearchViewModelAuthors::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return 1;
}

QVariant
SearchViewModelAuthors::data(const QModelIndex &index, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        const UDBElement *el = reinterpret_cast<const UDBElement *>(
            index.constInternalPointer());
        if(el == nullptr)
          {
            break;
          }
        QString str = el->content.c_str();
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        Qt::Alignment al = Qt::AlignVCenter | Qt::AlignLeft;
        result = QVariant(al);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
SearchViewModelAuthors::headerData(int section, Qt::Orientation orientation,
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
              QString str(tr("Author"));
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
              result = QVariant(Qt::AlignHCenter);
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
SearchViewModelAuthors::sort(int column, Qt::SortOrder order)
{
  beginResetModel();
  std::vector<UDBElement> *raw_base;
  if(filter_set)
    {
      raw_base = filtered.getRawBase();
    }
  else
    {
      raw_base = authors.getRawBase();
    }
  if(sorted)
    {
      if(sort_order != order)
        {
          UDBElement *start = raw_base->data();
          UDBElement *end = raw_base->data() + raw_base->size() - 1;
          while(end > start)
            {
              std::swap(*start, *end);
              start++;
              end--;
            }
        }
    }
  else
    {
      Algorithm alg;
      alg.parallelSort(
          raw_base->begin(), raw_base->end(),
          [this, order](const UDBElement &el1, const UDBElement &el2)
            {
              QString str1(el1.content.c_str());
              QString str2(el2.content.c_str());
              int res = QString::localeAwareCompare(str1.toLower(),
                                                    str2.toLower());
              if(order == Qt::DescendingOrder)
                {
                  return res <= 0;
                }
              else
                {
                  return res >= 0;
                }
            });
      sorted = true;
    }
  sort_order = order;
  endResetModel();
}

void
SearchViewModelAuthors::setFilter(const QString &filter)
{
  beginResetModel();

  filtered.clearBase();

  std::vector<UDBElement> *raw_base = authors.getRawBase();
  for(auto it = raw_base->begin(); it != raw_base->end(); it++)
    {
      QString str(it->content.c_str());
      if(str.contains(filter, Qt::CaseInsensitive))
        {
          filtered.addElement(*it);
        }
    }
  sorted = false;
  filter_set = true;
  endResetModel();
}

void
SearchViewModelAuthors::removeFilter()
{
  beginResetModel();
  filtered.clearBase();
  filtered.shrinkToFit();
  filter_set = false;
  sorted = false;
  endResetModel();
}
