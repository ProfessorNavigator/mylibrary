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

#include <AuthorsModel.h>

AuthorsModel::AuthorsModel(QObject *parent,
                           const std::vector<UDBElement> &authors)
    : QAbstractItemModel(parent)
{
  this->authors = authors;
}

QModelIndex
AuthorsModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  if(parent.isValid())
    {
      return result;
    }

  size_t l_row = static_cast<size_t>(row);
  if(l_row >= authors.size())
    {
      return result;
    }

  UDBElement *el = const_cast<UDBElement *>(authors.data() + row);
  switch(column)
    {
    case 0:
      {
        if(el->content.empty())
          {
            UDBElement *res;
            UDBElement *end = el->subelements.data() + el->subelements.size();
            for(res = el->subelements.data(); res != end; res++)
              {
                if(bid.getId(*res) == BaseID::LastName)
                  {
                    break;
                  }
              }
            if(res != end)
              {
                result = createIndex(row, column, res);
              }
            else
              {
                result = createIndex(row, column);
              }
          }
        else
          {
            result = createIndex(row, column, el);
          }
        break;
      }
    case 1:
      {
        UDBElement *res;
        UDBElement *end = el->subelements.data() + el->subelements.size();
        for(res = el->subelements.data(); res != end; res++)
          {
            if(bid.getId(*res) == BaseID::FirstName)
              {
                break;
              }
          }
        if(res != end)
          {
            result = createIndex(row, column, res);
          }
        else
          {
            result = createIndex(row, column);
          }
        break;
      }
    case 2:
      {
        UDBElement *res;
        UDBElement *end = el->subelements.data() + el->subelements.size();
        for(res = el->subelements.data(); res != end; res++)
          {
            if(bid.getId(*res) == BaseID::MiddleName)
              {
                break;
              }
          }
        if(res != end)
          {
            result = createIndex(row, column, res);
          }
        else
          {
            result = createIndex(row, column);
          }
        break;
      }
    case 3:
      {
        UDBElement *res;
        UDBElement *end = el->subelements.data() + el->subelements.size();
        for(res = el->subelements.data(); res != end; res++)
          {
            if(bid.getId(*res) == BaseID::Nickname)
              {
                break;
              }
          }
        if(res != end)
          {
            result = createIndex(row, column, res);
          }
        else
          {
            result = createIndex(row, column);
          }
        break;
      }
    case 4:
      {
        UDBElement *res;
        UDBElement *end = el->subelements.data() + el->subelements.size();
        for(res = el->subelements.data(); res != end; res++)
          {
            if(bid.getId(*res) == BaseID::HomePage)
              {
                break;
              }
          }
        if(res != end)
          {
            result = createIndex(row, column, res);
          }
        else
          {
            result = createIndex(row, column);
          }
        break;
      }
    case 5:
      {
        UDBElement *res;
        UDBElement *end = el->subelements.data() + el->subelements.size();
        for(res = el->subelements.data(); res != end; res++)
          {
            if(bid.getId(*res) == BaseID::EMail)
              {
                break;
              }
          }
        if(res != end)
          {
            result = createIndex(row, column, res);
          }
        else
          {
            result = createIndex(row, column);
          }
        break;
      }
    case 6:
      {
        UDBElement *res;
        UDBElement *end = el->subelements.data() + el->subelements.size();
        for(res = el->subelements.data(); res != end; res++)
          {
            if(bid.getId(*res) == BaseID::AuthorID)
              {
                break;
              }
          }
        if(res != end)
          {
            result = createIndex(row, column, res);
          }
        else
          {
            result = createIndex(row, column);
          }
        break;
      }
    default:
      break;
    }

  return result;
}

QModelIndex
AuthorsModel::parent(const QModelIndex &index) const
{
  Q_UNUSED(index);
  return QModelIndex();
}

int
AuthorsModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }
  else
    {
      return static_cast<int>(authors.size());
    }
}

int
AuthorsModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }
  else
    {
      return 7;
    }
}

QVariant
AuthorsModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  if(!index.isValid())
    {
      return result;
    }

  switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      {
        const UDBElement *el = reinterpret_cast<const UDBElement *>(
            index.constInternalPointer());
        QString str;
        if(el != nullptr)
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

  return result;
}

Qt::ItemFlags
AuthorsModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags result = Qt::NoItemFlags;

  if(!index.isValid())
    {
      return result;
    }
  result = editable | Qt::ItemIsSelectable | Qt::ItemIsEnabled
           | Qt::ItemNeverHasChildren;

  return result;
}

QVariant
AuthorsModel::headerData(int section, Qt::Orientation orientation,
                         int role) const
{
  QVariant result;

  switch(orientation)
    {
    case Qt::Vertical:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              QString str = QString::number(section + 1);
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
    case Qt::Horizontal:
      {
        switch(role)
          {
          case Qt::DisplayRole:
            {
              switch(section)
                {
                case 0:
                  {
                    QString str(tr("Surname or full name"));
                    result = QVariant(str);
                    break;
                  }
                case 1:
                  {
                    QString str(tr("Name"));
                    result = QVariant(str);
                    break;
                  }
                case 2:
                  {
                    QString str(tr("Second name"));
                    result = QVariant(str);
                    break;
                  }
                case 3:
                  {
                    QString str(tr("Nickname"));
                    result = QVariant(str);
                    break;
                  }
                case 4:
                  {
                    QString str(tr("Home page"));
                    result = QVariant(str);
                    break;
                  }
                case 5:
                  {
                    QString str(tr("e-mail"));
                    result = QVariant(str);
                    break;
                  }
                case 6:
                  {
                    QString str(tr("ID"));
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
    default:
      break;
    }

  return result;
}

bool
AuthorsModel::setData(const QModelIndex &index, const QVariant &value,
                      int role)
{
  bool result = false;

  if(role != Qt::EditRole)
    {
      return result;
    }

  auto it = authors.begin() + index.row();
  if(it < authors.end())
    {
      switch(index.column())
        {
        case 0:
          {
            it->content.clear();
            QString str = value.toString();
            auto it_auth
                = std::find_if(it->subelements.begin(), it->subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::LastName;
                                 });
            if(it_auth != it->subelements.end())
              {
                it_auth->content = str.toStdString();
              }
            else
              {
                UDBElement el;
                bid.setId(el, BaseID::LastName);
                el.content = str.toStdString();
                it->subelements.emplace_back(el);
              }
            result = true;
            emit dataChanged(index, index);
            break;
          }
        case 1:
          {
            QString str = value.toString();
            auto it_auth
                = std::find_if(it->subelements.begin(), it->subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::FirstName;
                                 });
            if(it_auth != it->subelements.end())
              {
                it_auth->content = str.toStdString();
              }
            else
              {
                UDBElement el;
                bid.setId(el, BaseID::FirstName);
                el.content = str.toStdString();
                it->subelements.emplace_back(el);
              }
            result = true;
            emit dataChanged(index, index);
            break;
          }
        case 2:
          {
            QString str = value.toString();
            auto it_auth
                = std::find_if(it->subelements.begin(), it->subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::MiddleName;
                                 });
            if(it_auth != it->subelements.end())
              {
                it_auth->content = str.toStdString();
              }
            else
              {
                UDBElement el;
                bid.setId(el, BaseID::MiddleName);
                el.content = str.toStdString();
                it->subelements.emplace_back(el);
              }
            result = true;
            emit dataChanged(index, index);
            break;
          }
        case 3:
          {
            QString str = value.toString();
            auto it_auth
                = std::find_if(it->subelements.begin(), it->subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::Nickname;
                                 });
            if(it_auth != it->subelements.end())
              {
                it_auth->content = str.toStdString();
              }
            else
              {
                UDBElement el;
                bid.setId(el, BaseID::Nickname);
                el.content = str.toStdString();
                it->subelements.emplace_back(el);
              }
            result = true;
            emit dataChanged(index, index);
            break;
          }
        default:
          break;
        }
    }

  return result;
}

void
AuthorsModel::setEditable(const bool &editable)
{
  if(editable)
    {
      this->editable = Qt::ItemIsEditable;
    }
  else
    {
      this->editable = Qt::NoItemFlags;
    }
}

std::vector<UDBElement>
AuthorsModel::getAuthors() const
{
  return authors;
}

void
AuthorsModel::addAuthor(const UDBElement &author)
{
  beginInsertRows(QModelIndex(), rowCount(), rowCount());

  authors.push_back(author);

  endInsertRows();
}

void
AuthorsModel::removeAuthor(const QModelIndex &index)
{
  if(!index.isValid() || static_cast<size_t>(index.row()) >= authors.size())
    {
      return void();
    }
  beginRemoveRows(QModelIndex(), index.row(), index.row());

  authors.erase(authors.begin() + index.row());

  endRemoveRows();
}
