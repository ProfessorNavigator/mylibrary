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

#include <BookMarksModel.h>

BookMarksModel::BookMarksModel(
    QObject *parent, const std::shared_ptr<BookmarksKeeper> &bookmarks,
    const std::shared_ptr<GenreBase> &genre_base)
    : QAbstractItemModel(parent)
{
  this->bookmarks = bookmarks;
  this->genre_base = genre_base;
}

QModelIndex
BookMarksModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  if(parent.isValid())
    {
      return result;
    }

  std::shared_lock shlock(*bookmarks->getRawMutex());
  std::vector<UDBElement> *raw_base = bookmarks->getRawBase();
  if(static_cast<size_t>(row) >= raw_base->size())
    {
      return result;
    }
  const UDBElement *el = raw_base->data() + row;
  result = createIndex(row, column, el);

  return result;
}

QModelIndex
BookMarksModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
BookMarksModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return static_cast<int>(bookmarks->baseSize());
}

int
BookMarksModel::columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }

  return 5;
}

QVariant
BookMarksModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  const UDBElement *book_mark
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(book_mark == nullptr)
    {
      return result;
    }

  auto it_b = std::find_if(book_mark->subelements.begin(),
                           book_mark->subelements.end(),
                           [this](const UDBElement &el)
                             {
                               return bid.getId(el) == BaseID::Book;
                             });
  if(it_b == book_mark->subelements.end())
    {
      return result;
    }

  switch(index.column())
    {
    case 0:
      {
        result = std::move(authorData(it_b->subelements, role));
        break;
      }
    case 1:
      {
        result = std::move(bookData(it_b->subelements, role));
        break;
      }
    case 2:
      {
        result = std::move(seriesData(it_b->subelements, role));
        break;
      }
    case 3:
      {
        result = std::move(genreData(it_b->subelements, role));
        break;
      }
    case 4:
      {
        result = std::move(dateData(it_b->subelements, role));
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
BookMarksModel::headerData(int section, Qt::Orientation orientation,
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
                    QString str(tr("Author"));
                    result = QVariant(str);
                    break;
                  }
                case 1:
                  {
                    QString str(tr("Book"));
                    result = QVariant(str);
                    break;
                  }
                case 2:
                  {
                    QString str(tr("Series"));
                    result = QVariant(str);
                    break;
                  }
                case 3:
                  {
                    QString str(tr("Genre"));
                    result = QVariant(str);
                    break;
                  }
                case 4:
                  {
                    QString str(tr("Date"));
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
BookMarksModel::removeBookmark(const QModelIndex &index)
{
  if(!index.isValid())
    {
      return void();
    }
  if(static_cast<size_t>(index.row()) >= bookmarks->baseSize())
    {
      return void();
    }
  const UDBElement *bm
      = reinterpret_cast<const UDBElement *>(index.constInternalPointer());
  if(bm == nullptr)
    {
      return void();
    }
  beginRemoveRows(QModelIndex(), index.row(), index.row());
  bookmarks->removeBookmark(*bm);
  endRemoveRows();
}

QVariant
BookMarksModel::authorData(const std::vector<UDBElement> &book, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        QString str;
        for(auto it = book.begin(); it != book.end(); it++)
          {
            if(bid.getId(*it) != BaseID::Author)
              {
                continue;
              }
            QString l_str;
            if(it->content.empty())
              {
                auto it_auth = std::find_if(
                    it->subelements.begin(), it->subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::LastName;
                      });
                if(it_auth != it->subelements.end())
                  {
                    l_str = it_auth->content.c_str();
                  }

                it_auth = std::find_if(
                    it->subelements.begin(), it->subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::FirstName;
                      });
                if(it_auth != it->subelements.end())
                  {
                    if(!it_auth->content.empty())
                      {
                        if(!l_str.isEmpty())
                          {
                            l_str += " ";
                          }
                        l_str += it_auth->content.c_str();
                      }
                  }

                it_auth = std::find_if(
                    it->subelements.begin(), it->subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::MiddleName;
                      });
                if(it_auth != it->subelements.end())
                  {
                    if(!it_auth->content.empty())
                      {
                        if(!l_str.isEmpty())
                          {
                            l_str += " ";
                          }
                        l_str += it_auth->content.c_str();
                      }
                  }

                it_auth = std::find_if(
                    it->subelements.begin(), it->subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Nickname;
                      });
                if(it_auth != it->subelements.end())
                  {
                    if(!it_auth->content.empty())
                      {
                        if(!l_str.isEmpty())
                          {
                            l_str += " aka ";
                          }
                        l_str += it_auth->content.c_str();
                      }
                  }
              }
            else
              {
                l_str = it->content.c_str();
              }

            if(l_str.isEmpty())
              {
                continue;
              }

            if(!str.isEmpty())
              {
                str += ", ";
              }
            str += l_str;
          }
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        result = QVariant(Qt::AlignCenter | Qt::TextWordWrap);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
BookMarksModel::bookData(const std::vector<UDBElement> &book, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        QString str;
        for(auto it = book.begin(); it != book.end(); it++)
          {
            if(bid.getId(*it) != BaseID::BookTitle)
              {
                continue;
              }
            if(it->content.empty())
              {
                continue;
              }
            if(!str.isEmpty())
              {
                str += ", ";
              }
            str += it->content.c_str();
          }
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        result = QVariant(Qt::AlignCenter | Qt::TextWordWrap);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
BookMarksModel::seriesData(const std::vector<UDBElement> &book, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        QString str;
        for(auto it = book.begin(); it != book.end(); it++)
          {
            if(bid.getId(*it) != BaseID::Sequence)
              {
                continue;
              }
            QString l_str;
            if(it->content.empty())
              {
                auto it_seq = std::find_if(
                    it->subelements.begin(), it->subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::SequenceName;
                      });
                if(it_seq == it->subelements.end())
                  {
                    continue;
                  }
                else
                  {
                    if(it_seq->content.empty())
                      {
                        continue;
                      }
                    l_str = it_seq->content.c_str();
                  }

                it_seq = std::find_if(
                    it->subelements.begin(), it->subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::SequenceNumber;
                      });
                if(it_seq != it->subelements.end())
                  {
                    l_str += " ";
                    l_str += it_seq->content.c_str();
                  }
              }
            else
              {
                l_str = it->content.c_str();
              }
            if(l_str.isEmpty())
              {
                continue;
              }
            if(!str.isEmpty())
              {
                str += ", ";
              }
            str += l_str;
          }
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        result = QVariant(Qt::AlignCenter | Qt::TextWordWrap);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
BookMarksModel::genreData(const std::vector<UDBElement> &book, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        QString str;
        for(auto it = book.begin(); it != book.end(); it++)
          {
            if(bid.getId(*it) != BaseID::Genre)
              {
                continue;
              }
            QString l_str = genre_base->getTranslationByGenreCode(it->content);

            if(l_str.isEmpty())
              {
                continue;
              }
            if(!str.isEmpty())
              {
                str += ", ";
              }
            str += l_str;
          }
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        result = QVariant(Qt::AlignCenter | Qt::TextWordWrap);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
BookMarksModel::dateData(const std::vector<UDBElement> &book, int role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        QString str;
        for(auto it = book.begin(); it != book.end(); it++)
          {
            if(bid.getId(*it) != BaseID::Date)
              {
                continue;
              }
            if(it->content.empty())
              {
                continue;
              }
            if(!str.isEmpty())
              {
                str += ", ";
              }
            str += it->content.c_str();
          }
        result = QVariant(str);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        result = QVariant(Qt::AlignCenter | Qt::TextWordWrap);
        break;
      }
    default:
      break;
    }

  return result;
}
