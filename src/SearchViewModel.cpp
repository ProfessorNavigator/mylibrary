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
#include <SearchViewModel.h>
#include <algorithm>

SearchViewModel::SearchViewModel(QObject *parent, const UDBase &base,
                                 const std::shared_ptr<GenreBase> &genre_base,
                                 const std::shared_ptr<MLBookProc> &mlbp)
    : QAbstractItemModel(parent)
{
  std::vector<UDBElement> *raw_base = base.getRawBase();
#pragma omp parallel for
  for(auto it = raw_base->begin(); it != raw_base->end(); it++)
    {
      switch(bid.getId(*it))
        {
        case BaseID::BookSearchResult:
          {
            SearchViewModelItem item(*it, genre_base);
#pragma omp critical
            {
              this->base.emplace_back(item);
            }
            break;
          }
        case BaseID::CollectionInfo:
          {
            collection_info = *it;
            break;
          }
        default:
          break;
        }
    }
  for(auto it = collection_info.subelements.begin();
      it != collection_info.subelements.end(); it++)
    {
      if(bid.getId(*it) == BaseID::CollectionType)
        {
          if(it->content == "native")
            {
              editable = Qt::ItemIsEditable;
            }
          break;
        }
    }

  this->genre_base = genre_base;
  this->mlbp = mlbp;
  current_sort = std::make_tuple(-1, Qt::AscendingOrder);
}

QModelIndex
SearchViewModel::index(int row, int column, const QModelIndex &parent) const
{
  QModelIndex result;

  if(parent.isValid())
    {
      return result;
    }

  std::vector<SearchViewModelItem> *vect;
  if(filter_enabled)
    {
      vect = const_cast<std::vector<SearchViewModelItem> *>(&filtered);
    }
  else
    {
      vect = const_cast<std::vector<SearchViewModelItem> *>(&base);
    }

  if(static_cast<size_t>(row) >= vect->size())
    {
      return result;
    }

  SearchViewModelItem *el = vect->data() + row;
  result = createIndex(row, column, el);

  return result;
}

QModelIndex
SearchViewModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int
SearchViewModel::rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    {
      return 0;
    }
  if(filter_enabled)
    {
      return static_cast<int>(filtered.size());
    }
  else
    {
      return static_cast<int>(base.size());
    }
}

int
SearchViewModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 5;
}

QVariant
SearchViewModel::data(const QModelIndex &index, int role) const
{
  QVariant result;

  if(!index.isValid())
    {
      return result;
    }
  const SearchViewModelItem *el
      = reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer());
  if(el == nullptr)
    {
      return result;
    }

  switch(index.column())
    {
    case 0:
      {
        result = std::move(
            authorData(const_cast<SearchViewModelItem *>(el), role));
        break;
      }
    case 1:
      {
        result
            = std::move(bookData(const_cast<SearchViewModelItem *>(el), role));
        break;
      }
    case 2:
      {
        result = std::move(
            seriesData(const_cast<SearchViewModelItem *>(el), role));
        break;
      }
    case 3:
      {
        result = std::move(
            genreData(const_cast<SearchViewModelItem *>(el), role));
        break;
      }
    case 4:
      {
        result
            = std::move(dateData(const_cast<SearchViewModelItem *>(el), role));
        break;
      }
    default:
      break;
    }

  return result;
}

Qt::ItemFlags
SearchViewModel::flags(const QModelIndex &index) const
{
  if(!index.isValid())
    {
      return Qt::NoItemFlags;
    }

  Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled
                         | Qt::ItemNeverHasChildren | editable;

  return result;
}

QVariant
SearchViewModel::headerData(int section, Qt::Orientation orientation,
                            int role) const
{
  QVariant result;

  switch(orientation)
    {
    case Qt::Horizontal:
      {
        switch(section)
          {
          case 0:
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
          case 1:
            {
              switch(role)
                {
                case Qt::DisplayRole:
                  {
                    QString str(tr("Book"));
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
                    QString str(tr("Series"));
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
          case 3:
            {
              switch(role)
                {
                case Qt::DisplayRole:
                  {
                    QString str(tr("Genre"));
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
          case 4:
            {
              switch(role)
                {
                case Qt::DisplayRole:
                  {
                    QString str(tr("Date"));
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
SearchViewModel::sort(int column, Qt::SortOrder order)
{
  Algorithm alg;
  std::vector<SearchViewModelItem> *src;
  if(filter_enabled)
    {
      src = const_cast<std::vector<SearchViewModelItem> *>(&filtered);
    }
  else
    {
      src = const_cast<std::vector<SearchViewModelItem> *>(&base);
    }
  beginResetModel();
  if(column != std::get<0>(current_sort))
    {
      switch(column)
        {
        case 0:
          {
            alg.parallelSort(src->begin(), src->end(),
                             [order](const SearchViewModelItem &el1,
                                     const SearchViewModelItem &el2)
                               {
                                 int res = QString::localeAwareCompare(
                                     el1.authors.toLower(),
                                     el2.authors.toLower());
                                 if(order == Qt::DescendingOrder)
                                   {
                                     return res <= 0;
                                   }
                                 else
                                   {
                                     return res >= 0;
                                   }
                               });
            break;
          }
        case 1:
          {
            alg.parallelSort(src->begin(), src->end(),
                             [order](const SearchViewModelItem &el1,
                                     const SearchViewModelItem &el2)
                               {
                                 int res = QString::localeAwareCompare(
                                     el1.book_title.toLower(),
                                     el2.book_title.toLower());
                                 if(order == Qt::DescendingOrder)
                                   {
                                     return res <= 0;
                                   }
                                 else
                                   {
                                     return res >= 0;
                                   }
                               });
            break;
          }
        case 2:
          {
            alg.parallelSort(src->begin(), src->end(),
                             [order](const SearchViewModelItem &el1,
                                     const SearchViewModelItem &el2)
                               {
                                 int res = QString::localeAwareCompare(
                                     el1.series.toLower(),
                                     el2.series.toLower());
                                 if(order == Qt::DescendingOrder)
                                   {
                                     return res <= 0;
                                   }
                                 else
                                   {
                                     return res >= 0;
                                   }
                               });
            break;
          }
        case 3:
          {
            alg.parallelSort(src->begin(), src->end(),
                             [order](const SearchViewModelItem &el1,
                                     const SearchViewModelItem &el2)
                               {
                                 int res = QString::localeAwareCompare(
                                     el1.genres.toLower(),
                                     el2.genres.toLower());
                                 if(order == Qt::DescendingOrder)
                                   {
                                     return res <= 0;
                                   }
                                 else
                                   {
                                     return res >= 0;
                                   }
                               });
            break;
          }
        case 4:
          {
            alg.parallelSort(src->begin(), src->end(),
                             [order](const SearchViewModelItem &el1,
                                     const SearchViewModelItem &el2)
                               {
                                 int res = QString::localeAwareCompare(
                                     el1.date.toLower(), el2.date.toLower());
                                 if(order == Qt::DescendingOrder)
                                   {
                                     return res <= 0;
                                   }
                                 else
                                   {
                                     return res >= 0;
                                   }
                               });
            break;
          }
        default:
          break;
        }
    }
  else if(order != std::get<1>(current_sort))
    {
      SearchViewModelItem *start = src->data();
      SearchViewModelItem *end = src->data() + src->size() - 1;
      while(start < end)
        {
          std::swap(*start, *end);
          start++;
          end--;
        }
    }
  endResetModel();
  current_sort = std::make_tuple(column, order);
}

Qt::ItemFlags
SearchViewModel::getCollectionEditable()
{
  return editable;
}

void
SearchViewModel::setFilter(const QString &filter, const int &column)
{
  if(filter.isEmpty())
    {
      return void();
    }
  current_sort = std::make_tuple(-1, Qt::AscendingOrder);

  beginResetModel();
  filter_enabled = true;
  filtered.clear();
  switch(column)
    {
    case 0:
      {
        authorFilter(filter);
        break;
      }
    case 1:
      {
        bookFilter(filter);
        break;
      }
    case 2:
      {
        seriesFilter(filter);
        break;
      }
    case 3:
      {
        genreFilter(filter);
        break;
      }
    case 4:
      {
        dateFilter(filter);
        break;
      }
    default:
      break;
    }
  endResetModel();
}

void
SearchViewModel::removeFilter()
{
  beginResetModel();
  filter_enabled = false;
  filtered.clear();
  filtered.shrink_to_fit();
  endResetModel();
}

void
SearchViewModel::removeBook(const UDBElement &book_search_result)
{
  beginResetModel();

  auto it_fl = std::find_if(book_search_result.subelements.begin(),
                            book_search_result.subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::File;
                              });
  if(it_fl == book_search_result.subelements.end())
    {
      endResetModel();
      return void();
    }
  std::string rar_path;

  std::string ext = mlbp->getExtension(it_fl->content);
  ext = mlbp->stringToLower(ext);
  if(ext == ".rar")
    {
      rar_path = it_fl->content;
    }

  Algorithm alg;
  if(!rar_path.empty())
    {
      std::filesystem::path rmp
          = std::u8string(rar_path.begin(), rar_path.end());
      base.erase(alg.parallelRemoveIf(
                     base.begin(), base.end(),
                     [rmp, this](const SearchViewModelItem &el)
                       {
                         auto it_fl = std::find_if(
                             el.book_search_result.subelements.begin(),
                             el.book_search_result.subelements.end(),
                             [this](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::File;
                               });
                         if(it_fl == el.book_search_result.subelements.end())
                           {
                             return false;
                           }
                         if(rmp
                            != std::filesystem::path(std::u8string(
                                it_fl->content.begin(), it_fl->content.end())))
                           {
                             return false;
                           }
                         return true;
                       }),
                 base.end());

      if(filter_enabled)
        {
          filtered.erase(
              alg.parallelRemoveIf(
                  filtered.begin(), filtered.end(),
                  [rmp, this](const SearchViewModelItem &el)
                    {
                      auto it_fl = std::find_if(
                          el.book_search_result.subelements.begin(),
                          el.book_search_result.subelements.end(),
                          [this](const UDBElement &el)
                            {
                              return bid.getId(el) == BaseID::File;
                            });
                      if(it_fl == el.book_search_result.subelements.end())
                        {
                          return false;
                        }
                      if(rmp
                         != std::filesystem::path(std::u8string(
                             it_fl->content.begin(), it_fl->content.end())))
                        {
                          return false;
                        }
                      return true;
                    }),
              filtered.end());
        }
    }
  else
    {
      auto it_book = std::find_if(book_search_result.subelements.begin(),
                                  book_search_result.subelements.end(),
                                  [this](const UDBElement &el)
                                    {
                                      return bid.getId(el) == BaseID::Book;
                                    });
      if(it_book == book_search_result.subelements.end())
        {
          endResetModel();
          return void();
        }

      auto it_path = std::find_if(
          it_book->subelements.begin(), it_book->subelements.end(),
          [this](const UDBElement &el)
            {
              return bid.getId(el) == BaseID::PathInFile;
            });
      if(it_path == it_book->subelements.end())
        {
          base.erase(alg.parallelRemoveIf(
                         base.begin(), base.end(),
                         [book_search_result](const SearchViewModelItem &el)
                           {
                             return el.book_search_result
                                    == book_search_result;
                           }),
                     base.end());

          if(filter_enabled)
            {
              filtered.erase(
                  alg.parallelRemoveIf(
                      filtered.begin(), filtered.end(),
                      [book_search_result](const SearchViewModelItem &el)
                        {
                          return el.book_search_result == book_search_result;
                        }),
                  filtered.end());
            }
        }
      else
        {
          rar_path = getRarPath(*it_path);
          if(rar_path.empty())
            {
              base.erase(
                  alg.parallelRemoveIf(
                      base.begin(), base.end(),
                      [book_search_result](const SearchViewModelItem &el)
                        {
                          return el.book_search_result == book_search_result;
                        }),
                  base.end());

              if(filter_enabled)
                {
                  filtered.erase(
                      alg.parallelRemoveIf(
                          filtered.begin(), filtered.end(),
                          [book_search_result](const SearchViewModelItem &el)
                            {
                              return el.book_search_result
                                     == book_search_result;
                            }),
                      filtered.end());
                }
            }
          else
            {
              base.erase(alg.parallelRemoveIf(
                             base.begin(), base.end(),
                             [this, rar_path](const SearchViewModelItem &el)
                               {
                                 auto it_book = std::find_if(
                                     el.book_search_result.subelements.begin(),
                                     el.book_search_result.subelements.end(),
                                     [this](const UDBElement &el)
                                       {
                                         return bid.getId(el) == BaseID::Book;
                                       });
                                 if(it_book
                                    == el.book_search_result.subelements.end())
                                   {
                                     return false;
                                   }
                                 auto it_path = std::find_if(
                                     it_book->subelements.begin(),
                                     it_book->subelements.end(),
                                     [this](const UDBElement &el)
                                       {
                                         return bid.getId(el)
                                                == BaseID::PathInFile;
                                       });
                                 if(it_path == it_book->subelements.end())
                                   {
                                     return false;
                                   }

                                 return rarRemove(*it_path, rar_path);
                               }),
                         base.end());

              if(filter_enabled)
                {
                  filtered.erase(
                      alg.parallelRemoveIf(
                          filtered.begin(), filtered.end(),
                          [this, rar_path](const SearchViewModelItem &el)
                            {
                              auto it_book = std::find_if(
                                  el.book_search_result.subelements.begin(),
                                  el.book_search_result.subelements.end(),
                                  [this](const UDBElement &el)
                                    {
                                      return bid.getId(el) == BaseID::Book;
                                    });
                              if(it_book
                                 == el.book_search_result.subelements.end())
                                {
                                  return false;
                                }
                              auto it_path = std::find_if(
                                  it_book->subelements.begin(),
                                  it_book->subelements.end(),
                                  [this](const UDBElement &el)
                                    {
                                      return bid.getId(el)
                                             == BaseID::PathInFile;
                                    });
                              if(it_path == it_book->subelements.end())
                                {
                                  return false;
                                }

                              return rarRemove(*it_path, rar_path);
                            }),
                      filtered.end());
                }
            }
        }
    }

  endResetModel();
}

void
SearchViewModel::setAuthors(const QModelIndex &index,
                            const std::vector<UDBElement> &authors)
{
  std::vector<UDBElement> l_authors = authors;
  l_authors.erase(
      std::remove_if(l_authors.begin(), l_authors.end(),
                     [this](const UDBElement &el)
                       {
                         if(bid.getId(el) != BaseID::Author)
                           {
                             return true;
                           }
                         UDBElement *ptr = const_cast<UDBElement *>(&el);
                         ptr->subelements.erase(
                             std::remove_if(ptr->subelements.begin(),
                                            ptr->subelements.end(),
                                            [this](const UDBElement &el)
                                              {
                                                switch(bid.getId(el))
                                                  {
                                                  case BaseID::FirstName:
                                                  case BaseID::MiddleName:
                                                  case BaseID::LastName:
                                                    {
                                                      if(!el.content.empty())
                                                        {
                                                          return false;
                                                        }
                                                      break;
                                                    }
                                                  default:
                                                    break;
                                                  }
                                                return true;
                                              }),
                             ptr->subelements.end());
                         if(el.subelements.size() == 0)
                           {
                             return true;
                           }
                         return false;
                       }),
      l_authors.end());

  SearchViewModelItem *el = const_cast<SearchViewModelItem *>(
      reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer()));
  if(el == nullptr)
    {
      return void();
    }
  auto it_book = std::find_if(el->book_search_result.subelements.begin(),
                              el->book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == el->book_search_result.subelements.end())
    {
      return void();
    }

  it_book->subelements.erase(
      std::remove_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Author;
                       }),
      it_book->subelements.end());

  std::copy(l_authors.begin(), l_authors.end(),
            std::back_inserter(it_book->subelements));

  emit signalEditBook(el->book_search_result);

  el->refresh();

  if(std::get<0>(current_sort) == 0)
    {
      std::get<0>(current_sort) = -1;
      sort(0, std::get<1>(current_sort));
    }
  else
    {
      emit dataChanged(index, index);
    }
}

void
SearchViewModel::setBookTitle(const QModelIndex &index, const QString &title)
{
  if(title.isEmpty())
    {
      return void();
    }

  SearchViewModelItem *el = const_cast<SearchViewModelItem *>(
      reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer()));
  if(el == nullptr)
    {
      return void();
    }
  auto it_book = std::find_if(el->book_search_result.subelements.begin(),
                              el->book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == el->book_search_result.subelements.end())
    {
      return void();
    }

  it_book->subelements.erase(
      std::remove_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::BookTitle;
                       }),
      it_book->subelements.end());

  UDBElement bt;
  bid.setId(bt, BaseID::BookTitle);
  bt.content = title.toStdString();
  it_book->subelements.emplace_back(bt);

  emit signalEditBook(el->book_search_result);

  el->refresh();

  if(std::get<0>(current_sort) == 1)
    {
      std::get<0>(current_sort) = -1;
      sort(1, std::get<1>(current_sort));
    }
  else
    {
      emit dataChanged(index, index);
    }
}

void
SearchViewModel::setSeries(const QModelIndex &index,
                           const std::vector<UDBElement> &series)
{
  std::vector<UDBElement> l_series = series;
  l_series.erase(
      std::remove_if(l_series.begin(), l_series.end(),
                     [this](const UDBElement &el)
                       {
                         if(bid.getId(el) != BaseID::Sequence)
                           {
                             return true;
                           }
                         UDBElement *ptr = const_cast<UDBElement *>(&el);
                         ptr->subelements.erase(
                             std::remove_if(
                                 ptr->subelements.begin(),
                                 ptr->subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     BaseID::ID id = bid.getId(el);
                                     if(id == BaseID::SequenceName
                                        || id == BaseID::SequenceNumber)
                                       {
                                         if(!el.content.empty())
                                           {
                                             return false;
                                           }
                                       }
                                     return true;
                                   }),
                             ptr->subelements.end());
                         auto it = std::find_if(
                             el.subelements.begin(), el.subelements.end(),
                             [this](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::SequenceName;
                               });
                         if(it == el.subelements.end())
                           {
                             return true;
                           }
                         return false;
                       }),
      l_series.end());

  SearchViewModelItem *el = const_cast<SearchViewModelItem *>(
      reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer()));
  if(el == nullptr)
    {
      return void();
    }
  auto it_book = std::find_if(el->book_search_result.subelements.begin(),
                              el->book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == el->book_search_result.subelements.end())
    {
      return void();
    }

  it_book->subelements.erase(
      std::remove_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Sequence;
                       }),
      it_book->subelements.end());

  std::copy(l_series.begin(), l_series.end(),
            std::back_inserter(it_book->subelements));

  emit signalEditBook(el->book_search_result);

  el->refresh();

  if(std::get<0>(current_sort) == 2)
    {
      std::get<0>(current_sort) = -1;
      sort(2, std::get<1>(current_sort));
    }
  else
    {
      emit dataChanged(index, index);
    }
}

void
SearchViewModel::setGenres(const QModelIndex &index,
                           const std::vector<UDBElement> &genres)
{
  std::vector<UDBElement> l_genres = genres;
  l_genres.erase(std::remove_if(l_genres.begin(), l_genres.end(),
                                [this](const UDBElement &el)
                                  {
                                    if(bid.getId(el) != BaseID::Genre)
                                      {
                                        return true;
                                      }
                                    if(el.content.empty())
                                      {
                                        return true;
                                      }
                                    return false;
                                  }),
                 l_genres.end());

  SearchViewModelItem *el = const_cast<SearchViewModelItem *>(
      reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer()));
  if(el == nullptr)
    {
      return void();
    }
  auto it_book = std::find_if(el->book_search_result.subelements.begin(),
                              el->book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == el->book_search_result.subelements.end())
    {
      return void();
    }

  it_book->subelements.erase(
      std::remove_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Genre;
                       }),
      it_book->subelements.end());

  std::copy(l_genres.begin(), l_genres.end(),
            std::back_inserter(it_book->subelements));

  emit signalEditBook(el->book_search_result);

  el->refresh();

  if(std::get<0>(current_sort) == 3)
    {
      std::get<0>(current_sort) = -1;
      sort(3, std::get<1>(current_sort));
    }
  else
    {
      emit dataChanged(index, index);
    }
}

void
SearchViewModel::setDate(const QModelIndex &index, const QString &date)
{
  SearchViewModelItem *el = const_cast<SearchViewModelItem *>(
      reinterpret_cast<const SearchViewModelItem *>(
          index.constInternalPointer()));
  if(el == nullptr)
    {
      return void();
    }
  auto it_book = std::find_if(el->book_search_result.subelements.begin(),
                              el->book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == el->book_search_result.subelements.end())
    {
      return void();
    }

  it_book->subelements.erase(
      std::remove_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::Date;
                       }),
      it_book->subelements.end());

  UDBElement bd;
  bid.setId(bd, BaseID::Date);
  bd.content = date.toStdString();
  it_book->subelements.emplace_back(bd);

  emit signalEditBook(el->book_search_result);

  el->refresh();

  if(std::get<0>(current_sort) == 4)
    {
      std::get<0>(current_sort) = -1;
      sort(4, std::get<1>(current_sort));
    }
  else
    {
      emit dataChanged(index, index);
    }
}

QVariant
SearchViewModel::authorData(SearchViewModelItem *item, const int &role) const
{
  QVariant result;
  switch(role)
    {
    case Qt::DisplayRole:
      {
        result = QVariant(item->authors);
        break;
      }
    case Qt::TextAlignmentRole:
      {
        result = QVariant(Qt::AlignCenter | Qt::TextWordWrap);
        break;
      }
    case Qt::EditRole:
      {
        auto it = std::find_if(item->book_search_result.subelements.begin(),
                               item->book_search_result.subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::Book;
                                 });
        if(it == item->book_search_result.subelements.end())
          {
            break;
          }
        std::vector<UDBElement> res;
        for(auto it_b = it->subelements.begin(); it_b != it->subelements.end();
            it_b++)
          {
            if(bid.getId(*it_b) != BaseID::Author)
              {
                continue;
              }
            res.push_back(*it_b);
          }
        result = QVariant::fromValue(res);
        break;
      }
    default:
      break;
    }

  return result;
}

QVariant
SearchViewModel::bookData(SearchViewModelItem *item, const int &role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      {
        result = QVariant(item->book_title);
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
SearchViewModel::seriesData(SearchViewModelItem *item, const int &role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        result = QVariant(item->series);
        break;
      }
    case Qt::EditRole:
      {
        auto it = std::find_if(item->book_search_result.subelements.begin(),
                               item->book_search_result.subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::Book;
                                 });
        if(it == item->book_search_result.subelements.end())
          {
            break;
          }
        std::vector<UDBElement> res;
        for(auto it_b = it->subelements.begin(); it_b != it->subelements.end();
            it_b++)
          {
            if(bid.getId(*it_b) != BaseID::Sequence)
              {
                continue;
              }
            res.push_back(*it_b);
          }
        result = QVariant::fromValue(res);
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
SearchViewModel::genreData(SearchViewModelItem *item, const int &role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
      {
        result = QVariant(item->genres);
        break;
      }
    case Qt::EditRole:
      {
        auto it = std::find_if(item->book_search_result.subelements.begin(),
                               item->book_search_result.subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::Book;
                                 });
        if(it == item->book_search_result.subelements.end())
          {
            break;
          }
        std::vector<UDBElement> res;
        for(auto it_b = it->subelements.begin(); it_b != it->subelements.end();
            it_b++)
          {
            if(bid.getId(*it_b) != BaseID::Genre)
              {
                continue;
              }
            res.push_back(*it_b);
          }
        result = QVariant::fromValue(res);
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
SearchViewModel::dateData(SearchViewModelItem *item, const int &role) const
{
  QVariant result;

  switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
      {
        result = QVariant(item->date);
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

void
SearchViewModel::authorFilter(const QString &filter)
{
  filtered.clear();
#pragma omp parallel for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->authors.contains(filter, Qt::CaseInsensitive))
        {
#pragma omp critical
          {
            filtered.push_back(*it);
          }
        }
    }
}

void
SearchViewModel::bookFilter(const QString &filter)
{
  filtered.clear();
#pragma omp parallel for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->book_title.contains(filter, Qt::CaseInsensitive))
        {
#pragma omp critical
          {
            filtered.push_back(*it);
          }
        }
    }
}

void
SearchViewModel::seriesFilter(const QString &filter)
{
  filtered.clear();
#pragma omp parallel for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->series.contains(filter, Qt::CaseInsensitive))
        {
#pragma omp critical
          {
            filtered.push_back(*it);
          }
        }
    }
}

void
SearchViewModel::genreFilter(const QString &filter)
{
  filtered.clear();
#pragma omp parallel for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->genres.contains(filter, Qt::CaseInsensitive))
        {
#pragma omp critical
          {
            filtered.push_back(*it);
          }
        }
    }
}

void
SearchViewModel::dateFilter(const QString &filter)
{
  filtered.clear();
#pragma omp parallel for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      if(it->date.contains(filter, Qt::CaseInsensitive))
        {
#pragma omp critical
          {
            filtered.push_back(*it);
          }
        }
    }
}

std::string
SearchViewModel::getRarPath(const UDBElement &path)
{
  std::string result;

  std::string ext = mlbp->getExtension(path.content);
  ext = mlbp->stringToLower(ext);
  if(ext == ".rar")
    {
      result = path.content;
    }
  else
    {
      auto it = std::find_if(path.subelements.begin(), path.subelements.end(),
                             [this](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::PathInFile;
                               });
      if(it != path.subelements.end())
        {
          result = getRarPath(*it);
        }
    }

  return result;
}

bool
SearchViewModel::rarRemove(const UDBElement &path, const std::string &rar_path)
{
  bool result = false;

  if(path.content == rar_path)
    {
      result = true;
    }
  else
    {
      auto it = std::find_if(path.subelements.begin(), path.subelements.end(),
                             [this](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::PathInFile;
                               });
      if(it != path.subelements.end())
        {
          result = rarRemove(*it, rar_path);
        }
    }

  return result;
}
