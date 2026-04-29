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

#include <FilesModel.h>
#include <algorithm>

FilesModel::FilesModel(QObject *parent) : QAbstractItemModel(parent)
{
}

QModelIndex
FilesModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  if(parent.isValid())
    {
      return result;
    }

  size_t l_row = static_cast<size_t>(row);
  if(l_row >= base.size())
    {
      return result;
    }

  const std::filesystem::path *ptr = base.data() + l_row;

  result = createIndex(row, column, ptr);

  return result;
}

QModelIndex
FilesModel::parent(const QModelIndex &index) const
{
  Q_UNUSED(index);

  return QModelIndex();
}

int
FilesModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }
  return static_cast<int>(base.size());
}

int
FilesModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }
  return 1;
}

QVariant
FilesModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        std::vector<std::filesystem::path>::const_iterator it
            = base.begin() + index.row();
        QString str = it->u8string().c_str();
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

  return result;
}

Qt::ItemFlags
FilesModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags result
      = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;

  return result;
}

QVariant
FilesModel::headerData(int section, Qt::Orientation orientation,
                       int role) const
{
  QVariant result;

  switch(orientation)
    {
    case Qt::Horizontal:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              QString str = tr("Files or directories");
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
    case Qt::Vertical:
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
FilesModel::addItem(const std::filesystem::path &item)
{
  auto it = std::find(base.begin(), base.end(), item);
  if(it != base.end())
    {
      return void();
    }
  emit beginInsertRows(QModelIndex(), static_cast<int>(base.size()),
                       static_cast<int>(base.size()));
  base.push_back(item);
  emit endInsertRows();
}

void
FilesModel::removeItem(const QModelIndex &index)
{
  if(!index.isValid())
    {
      return void();
    }
  emit beginRemoveRows(QModelIndex(), index.row(), index.row());
  base.erase(base.begin() + index.row());
  emit endRemoveRows();
}

std::vector<std::filesystem::path>
FilesModel::getBase()
{
  return base;
}
