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

#include <NotesManagerModel.h>
#include <algorithm>

NotesManagerModel::NotesManagerModel(QObject *parent,
                                     const std::shared_ptr<NotesKeeper> &notes)
    : QAbstractItemModel(parent)
{
  this->notes = notes;
}

QModelIndex
NotesManagerModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;
  if(parent.isValid())
    {
      return result;
    }

  std::vector<UDBElement> *raw_base = notes->getRawBase();
  if(static_cast<size_t>(row) >= raw_base->size())
    {
      return result;
    }

  UDBElement *el = raw_base->data() + row;
  result = createIndex(row, column, el);

  return result;
}

QModelIndex
NotesManagerModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
NotesManagerModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return static_cast<int>(notes->baseSize());
}

int
NotesManagerModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return 3;
}

QVariant
NotesManagerModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  const UDBElement *note
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(note == nullptr)
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
              auto it = std::find_if(note->subelements.begin(),
                                     note->subelements.end(),
                                     [this](const UDBElement &el)
                                       {
                                         return bid.getId(el) == BaseID::File;
                                       });
              if(it != note->subelements.end())
                {
                  QString str = it->content.c_str();
                  result = QVariant(str);
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
    case 1:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              auto it = std::find_if(
                  note->subelements.begin(), note->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it != note->subelements.end())
                {
                  QString str = it->content.c_str();
                  for(;;)
                    {
                      auto it_p = std::find_if(
                          it->subelements.begin(), it->subelements.end(),
                          [this](const UDBElement &el)
                            {
                              return bid.getId(el) == BaseID::PathInFile;
                            });
                      if(it_p == it->subelements.end())
                        {
                          break;
                        }
                      else
                        {
                          str += "/";
                          str += it_p->content.c_str();
                          it = it_p;
                        }
                    }
                  result = QVariant(str);
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
    case 2:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              auto it = std::find_if(
                  note->subelements.begin(), note->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::BookNoteFile;
                    });
              if(it != note->subelements.end())
                {
                  QString str = it->content.c_str();
                  result = QVariant(str);
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
    default:
      break;
    }

  return result;
}

QVariant
NotesManagerModel::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
  QVariant result;

  switch(orientation)
    {
    case Qt::Orientation::Horizontal:
      {
        switch(section)
          {
          case 0:
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
          case 1:
            {
              switch(role)
                {
                case Qt::DisplayRole:
                  {
                    QString str(tr("Path in file (if any)"));
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
          case 2:
            {
              switch(role)
                {
                case Qt::DisplayRole:
                  {
                    QString str(tr("Notes file path"));
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
NotesManagerModel::removeNote(const QModelIndex &index)
{
  size_t row = static_cast<size_t>(index.row());
  if(row >= notes->baseSize())
    {
      return void();
    }

  const UDBElement *note
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(note == nullptr)
    {
      return void();
    }

  beginRemoveRows(QModelIndex(), index.row(), index.row());
  notes->removeNote(*note);
  endRemoveRows();
}
