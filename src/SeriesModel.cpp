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

#include <SeriesModel.h>

SeriesModel::SeriesModel(QObject *parent,
                         const std::vector<UDBElement> &series)
    : QAbstractItemModel(parent)
{
  this->series = series;
}

QModelIndex
SeriesModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  if(parent.isValid())
    {
      return result;
    }

  if(static_cast<size_t>(row) >= series.size() || column >= 2)
    {
      return result;
    }

  const UDBElement *el = series.data() + row;

  result = createIndex(row, column, el);

  return result;
}

QModelIndex
SeriesModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
SeriesModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return static_cast<int>(series.size());
}

int
SeriesModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return 2;
}

QVariant
SeriesModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  const UDBElement *el
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());

  switch(index.column())
    {
    case 0:
      {
        auto it
            = std::find_if(el->subelements.begin(), el->subelements.end(),
                           [this](const UDBElement &el)
                             {
                               return bid.getId(el) == BaseID::SequenceName;
                             });
        if(it == el->subelements.end() && el->content.empty())
          {
            break;
          }

        switch(role)
          {
          case Qt::DisplayRole:
          case Qt::EditRole:
            {
              QString str;
              if(it != el->subelements.end())
                {
                  str = it->content.c_str();
                }
              else
                {
                  str = el->content.c_str();
                }
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
        auto it
            = std::find_if(el->subelements.begin(), el->subelements.end(),
                           [this](const UDBElement &el)
                             {
                               return bid.getId(el) == BaseID::SequenceNumber;
                             });
        if(it == el->subelements.end())
          {
            break;
          }
        switch(role)
          {
          case Qt::DisplayRole:
          case Qt::EditRole:
            {
              QString str = it->content.c_str();
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
SeriesModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags result
      = Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;

  return result;
}

QVariant
SeriesModel::headerData(int section, Qt::Orientation orientation,
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
                    QString str(tr("Seiries"));
                    result = QVariant(str);
                    break;
                  }
                case 1:
                  {
                    QString str(tr("Number"));
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

bool
SeriesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool result = false;

  UDBElement *el = const_cast<UDBElement *>(
      reinterpret_cast<const UDBElement *>(index.constInternalPointer()));

  if(el == nullptr)
    {
      return result;
    }

  if(role != Qt::EditRole)
    {
      return result;
    }

  switch(index.column())
    {
    case 0:
      {
        QString str = value.toString();
        auto it
            = std::find_if(el->subelements.begin(), el->subelements.end(),
                           [this](const UDBElement &el)
                             {
                               return bid.getId(el) == BaseID::SequenceName;
                             });
        if(it == el->subelements.end())
          {
            if(!str.isEmpty())
              {
                UDBElement nm;
                bid.setId(nm, BaseID::SequenceName);
                nm.content = str.toStdString();

                el->subelements.emplace_back(nm);
                result = true;
              }
          }
        else
          {
            it->content = str.toStdString();
            result = true;
          }
        break;
      }
    case 1:
      {
        QString str = value.toString();
        auto it
            = std::find_if(el->subelements.begin(), el->subelements.end(),
                           [this](const UDBElement &el)
                             {
                               return bid.getId(el) == BaseID::SequenceNumber;
                             });
        if(it == el->subelements.end())
          {
            if(!str.isEmpty())
              {
                UDBElement nm;
                bid.setId(nm, BaseID::SequenceNumber);
                nm.content = str.toStdString();

                el->subelements.emplace_back(nm);
                result = true;
              }
          }
        else
          {
            it->content = str.toStdString();
            result = true;
          }
        break;
      }
    default:
      break;
    }

  if(result)
    {
      emit dataChanged(index, index);
    }

  return result;
}

void
SeriesModel::addSeries(const UDBElement &series)
{
  beginInsertRows(QModelIndex(), rowCount(), rowCount());

  this->series.push_back(series);

  endInsertRows();
}

void
SeriesModel::removeSeries(const QModelIndex &index)
{
  if(!index.isValid())
    {
      return void();
    }
  if(static_cast<size_t>(index.row()) >= series.size())
    {
      return void();
    }
  beginRemoveRows(QModelIndex(), index.row(), index.row());
  series.erase(series.begin() + index.row());
  endRemoveRows();
}

std::vector<UDBElement>
SeriesModel::getSeries()
{
  return series;
}
