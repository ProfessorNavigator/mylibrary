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
#include <SearchViewModelFiles.h>
#include <algorithm>

SearchViewModelFiles::SearchViewModelFiles(
    QObject *parent, const UDBase &files,
    const std::shared_ptr<MLBookProc> &mlbp)
    : QAbstractItemModel(parent)
{
  this->files = files;
  this->mlbp = mlbp;

  std::vector<UDBElement> *raw_base = this->files.getRawBase();
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
SearchViewModelFiles::index(int row, int column,
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
      raw_base = files.getRawBase();
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
SearchViewModelFiles::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
SearchViewModelFiles::rowCount(const QModelIndex &parent) const
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
      return static_cast<int>(files.baseSize());
    }
}

int
SearchViewModelFiles::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }
  return 1;
}

QVariant
SearchViewModelFiles::data(const QModelIndex &index, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        const UDBElement *file = reinterpret_cast<const UDBElement *>(
            index.constInternalPointer());
        if(file == nullptr)
          {
            break;
          }
        QString str = file->content.c_str();
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        Qt::Alignment align = Qt::AlignVCenter | Qt::AlignLeft;
        result = QVariant(align);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
SearchViewModelFiles::headerData(int section, Qt::Orientation orientation,
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
              QString str(tr("File path"));
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
SearchViewModelFiles::sort(int column, Qt::SortOrder order)
{
  beginResetModel();
  std::vector<UDBElement> *raw_base;
  if(filter_set)
    {
      raw_base = filtered.getRawBase();
    }
  else
    {
      raw_base = files.getRawBase();
    }
  if(sorted)
    {
      if(this->order != order)
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
      alg.parallelSort(raw_base->begin(), raw_base->end(),
                       [order](const UDBElement &el1, const UDBElement &el2)
                         {
                           if(order == Qt::DescendingOrder)
                             {
                               return el1.content <= el2.content;
                             }
                           else
                             {
                               return el1.content >= el2.content;
                             }
                         });
      sorted = true;
    }
  this->order = order;
  endResetModel();
}

void
SearchViewModelFiles::setFilter(const QString &filter)
{
  beginResetModel();
  filtered = files;
  std::string fltr = filter.toLower().toStdString();
  filtered.removeElements(
      [this, fltr](const UDBElement &el)
        {
          if(bid.getId(el) != BaseID::File)
            {
              return true;
            }

          std::string fp = mlbp->stringToLower(el.content);
          std::string::size_type n = fp.find(fltr);
          if(n != std::string::npos)
            {
              return false;
            }
          return true;
        });
  filter_set = true;
  endResetModel();
}

void
SearchViewModelFiles::removeFilter()
{
  beginResetModel();
  filter_set = false;
  filtered.clearBase();
  endResetModel();
}

UDBElement
SearchViewModelFiles::getCollectionInfo()
{
  return collection_info;
}
