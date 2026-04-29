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
#include <SearchViewModelItem.h>

SearchViewModelItem::SearchViewModelItem()
{
}

SearchViewModelItem::SearchViewModelItem(
    const UDBElement &book_search_result,
    const std::shared_ptr<GenreBase> &genre_base)
{
  this->book_search_result = book_search_result;
  this->genre_base = genre_base;
  BaseID bid;
  auto it_book = std::find_if(this->book_search_result.subelements.begin(),
                              this->book_search_result.subelements.end(),
                              [bid](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book != this->book_search_result.subelements.end())
    {
      getAuthors(it_book->subelements, bid);
      getBookTitle(it_book->subelements, bid);
      getSeries(it_book->subelements, bid);
      getGenres(it_book->subelements, bid);
      getDate(it_book->subelements, bid);
    }
}

SearchViewModelItem::SearchViewModelItem(const SearchViewModelItem &other)
{
  authors = other.authors;
  book_title = other.book_title;
  series = other.series;
  genres = other.genres;
  date = other.date;
  book_search_result = other.book_search_result;
  genre_base = other.genre_base;
}

SearchViewModelItem::SearchViewModelItem(SearchViewModelItem &&other)
{
  authors = std::move(other.authors);
  book_title = std::move(other.book_title);
  series = std::move(other.series);
  genres = std::move(other.genres);
  date = std::move(other.date);
  book_search_result = std::move(other.book_search_result);
  genre_base = std::move(other.genre_base);
}

void
SearchViewModelItem::refresh()
{
  BaseID bid;
  authors.clear();
  book_title.clear();
  series.clear();
  genres.clear();
  date.clear();
  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [bid](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book != book_search_result.subelements.end())
    {
      getAuthors(it_book->subelements, bid);
      getBookTitle(it_book->subelements, bid);
      getSeries(it_book->subelements, bid);
      getGenres(it_book->subelements, bid);
      getDate(it_book->subelements, bid);
    }
}

SearchViewModelItem &
SearchViewModelItem::operator=(const SearchViewModelItem &other)
{
  if(this != &other)
    {
      authors = other.authors;
      book_title = other.book_title;
      series = other.series;
      genres = other.genres;
      date = other.date;
      book_search_result = other.book_search_result;
      genre_base = other.genre_base;
    }
  return *this;
}

SearchViewModelItem &
SearchViewModelItem::operator=(SearchViewModelItem &&other)
{
  if(this != &other)
    {
      authors = std::move(other.authors);
      book_title = std::move(other.book_title);
      series = std::move(other.series);
      genres = std::move(other.genres);
      date = std::move(other.date);
      book_search_result = std::move(other.book_search_result);
      genre_base = std::move(other.genre_base);
    }
  return *this;
}

void
SearchViewModelItem::getAuthors(const std::vector<UDBElement> &book,
                                BaseID &bid)
{
  for(auto it = book.begin(); it != book.end(); it++)
    {
      if(bid.getId(*it) != BaseID::Author)
        {
          continue;
        }
      if(it->content.empty())
        {
          QString l_str;
          auto it_auth
              = std::find_if(it->subelements.begin(), it->subelements.end(),
                             [bid](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::LastName;
                               });
          if(it_auth != it->subelements.end())
            {
              l_str += QString(it_auth->content.c_str());
            }
          it_auth
              = std::find_if(it->subelements.begin(), it->subelements.end(),
                             [bid](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::FirstName;
                               });
          if(it_auth != it->subelements.end())
            {
              if(!l_str.isEmpty())
                {
                  l_str += " ";
                }
              l_str += QString(it_auth->content.c_str());
            }
          it_auth
              = std::find_if(it->subelements.begin(), it->subelements.end(),
                             [bid](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::MiddleName;
                               });
          if(it_auth != it->subelements.end())
            {
              if(!l_str.isEmpty())
                {
                  l_str += " ";
                }
              l_str += QString(it_auth->content.c_str());
            }
          it_auth
              = std::find_if(it->subelements.begin(), it->subelements.end(),
                             [bid](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::Nickname;
                               });
          if(it_auth != it->subelements.end())
            {
              if(!l_str.isEmpty() && !it_auth->content.empty())
                {
                  l_str += " aka ";
                }
              l_str += QString(it_auth->content.c_str());
            }
          if(!authors.isEmpty() && !l_str.isEmpty())
            {
              authors += ", ";
            }
          authors += l_str;
        }
      else
        {
          if(!authors.isEmpty())
            {
              authors += ", ";
            }
          authors += QString(it->content.c_str());
        }
    }
}

void
SearchViewModelItem::getBookTitle(const std::vector<UDBElement> &book,
                                  BaseID &bid)
{
  for(auto it = book.begin(); it != book.end(); it++)
    {
      if(bid.getId(*it) != BaseID::BookTitle)
        {
          continue;
        }
      if(!book_title.isEmpty())
        {
          book_title += ", ";
        }
      book_title += QString(it->content.c_str());
    }
}

void
SearchViewModelItem::getSeries(const std::vector<UDBElement> &book,
                               BaseID &bid)
{
  for(auto it = book.begin(); it != book.end(); it++)
    {
      if(bid.getId(*it) != BaseID::Sequence)
        {
          continue;
        }
      if(it->content.empty())
        {
          auto it_s
              = std::find_if(it->subelements.begin(), it->subelements.end(),
                             [bid](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::SequenceName;
                               });
          if(it_s != it->subelements.end())
            {
              QString l_str(it_s->content.c_str());
              it_s = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [bid](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::SequenceNumber;
                    });
              if(it_s != it->subelements.end())
                {
                  if(!l_str.isEmpty())
                    {
                      l_str += " ";
                    }
                  l_str += QString(it_s->content.c_str());
                }
              if(!series.isEmpty())
                {
                  series += ", ";
                }
              series += l_str;
            }
        }
      else
        {
          if(!series.isEmpty())
            {
              series += ", ";
            }
          series += QString(it->content.c_str());
        }
    }
}

void
SearchViewModelItem::getGenres(const std::vector<UDBElement> &book,
                               BaseID &bid)
{
  for(auto it = book.begin(); it != book.end(); it++)
    {
      if(bid.getId(*it) != BaseID::Genre)
        {
          continue;
        }
      QString l_str = genre_base->getTranslationByGenreCode(it->content);
      if(!l_str.isEmpty())
        {
          if(!genres.isEmpty())
            {
              genres += ", ";
            }
          genres += l_str;
        }
      else
        {
          l_str = it->content.c_str();
          if(!genres.isEmpty())
            {
              genres += ", ";
            }
          genres += l_str;
        }
    }
}

void
SearchViewModelItem::getDate(const std::vector<UDBElement> &book, BaseID &bid)
{
  for(auto it = book.begin(); it != book.end(); it++)
    {
      if(bid.getId(*it) != BaseID::Date)
        {
          continue;
        }
      if(!date.isEmpty())
        {
          date += ", ";
        }
      date += it->content.c_str();
    }
}
