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
#include <GenreBase.h>
#include <QByteArray>
#include <QFile>
#include <QLocale>
#include <iostream>

GenreBase::GenreBase() : UDBase()
{
  createBase();
}

GenreBase::~GenreBase()
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
}

QString
GenreBase::getTranslationByGenreCode(const std::string &genre_code)
{
  QString result(genre_code.c_str());

  std::shared_lock shlock(base_mtx);

#pragma omp parallel
#pragma omp for
  for(auto it = base.begin(); it != base.end(); it++)
    {
      for(auto it_g = it->subelements.begin(); it_g != it->subelements.end();
          it_g++)
        {
          if(it_g->id != "genre")
            {
              continue;
            }
          if(it_g->content == genre_code)
            {
              auto it_tr = std::find_if(it_g->subelements.begin(),
                                        it_g->subelements.end(),
                                        [](const UDBElement &el)
                                          {
                                            return el.id == "translation";
                                          });
              if(it_tr != it_g->subelements.end())
                {
#pragma omp critical
                  {
                    result = QString(it_tr->content.c_str());
                  }
#pragma omp cancel for
                }
            }
        }
    }

  return result;
}

std::string
GenreBase::loadFile(const QString &path)
{
  std::string result;
  QFile fl(path);
  if(fl.open(QIODeviceBase::ReadOnly))
    {
      QByteArray arr = fl.readAll();
      fl.close();

      result.reserve(arr.size());
      for(auto it = arr.begin(); it != arr.end(); it++)
        {
          result.push_back(*it);
        }
    }
  return result;
}

void
GenreBase::createBase()
{
  std::string g_groups = loadFile(":/genres/genre_groups.csv");
  int g_groups_column = getCsvColumnNumber(g_groups);
  if(g_groups_column < 0)
    {
      g_groups_column = 0;
    }

  std::string endl("\n");
  std::string::size_type n = g_groups.find(endl);
  if(n != std::string::npos)
    {
      g_groups.erase(0, n + endl.size());
    }
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
  loadGroups(g_groups, g_groups_column);

  std::string genres = loadFile(":/genres/genres.csv");
  int genre_column = getCsvColumnNumber(genres);
  if(genre_column < 0)
    {
      genre_column = 0;
    }
  n = genres.find(endl);
  if(n != std::string::npos)
    {
      genres.erase(0, n + endl.size());
    }
  loadGenres(genres, genre_column);
}

int
GenreBase::getCsvColumnNumber(const std::string &source)
{
  int result = -1;

  std::string locale;
  QLocale loc = QLocale::system();
  locale += QLocale::languageToCode(loc.language()).toStdString();
  locale += "_";
  locale += QLocale::territoryToCode(loc.territory()).toStdString();
  std::string endl("\n");
  std::string::size_type n = source.find(endl);
  if(n == std::string::npos)
    {
      std::cout << "GenreBase::getCsvColumnNumber: incorrect file"
                << std::endl;
      return result;
    }

  std::string lang_line(source.begin(), source.begin() + n);

  std::string separator(";");
  n = 0;
  int count = 0;
  std::string::size_type n2;
  while(n != std::string::npos)
    {
      n2 = lang_line.find(separator, n);
      if(n2 != std::string::npos)
        {
          std::string lang_code(lang_line.begin() + n, lang_line.begin() + n2);
          n = n2 + separator.size();
          n2 = lang_code.find(locale);
          if(n2 != std::string::npos)
            {
              result = count;
              return result;
            }
        }
      else
        {
          std::string lang_code(lang_line.begin() + n, lang_line.end());
          n = n2;
          n2 = lang_code.find(locale);
          if(n2 != std::string::npos)
            {
              result = count;
              return result;
            }
        }
      count++;
    }

  return result;
}

void
GenreBase::loadGroups(const std::string &g_groups, const int &g_groups_column)
{
  std::string::size_type n = 0;
  std::string separator(";");
  std::string endl("\n");
  std::string::size_type n2;
#pragma omp parallel
#pragma omp masked
  {
    while(n != std::string::npos)
      {
        n2 = g_groups.find(endl, n);
        std::string group_line;
        if(n2 != std::string::npos)
          {
            group_line
                = std::string(g_groups.begin() + n, g_groups.begin() + n2);
            n = n2 + endl.size();
          }
        else
          {
            group_line = std::string(g_groups.begin() + n, g_groups.end());
            n = n2;
          }

        while(group_line.size() > 0)
          {
            char val = *group_line.rbegin();
            if(val >= 0 && val <= ' ')
              {
                group_line.pop_back();
              }
            else
              {
                break;
              }
          }
        if(group_line.empty())
          {
            continue;
          }

#pragma omp task
        {
          std::string::size_type n_sep1 = 0;
          std::string::size_type n_sep2;
          int count = 0;
          UDBElement el;
          while(n_sep1 != std::string::npos)
            {
              n_sep2 = group_line.find(separator, n_sep1);
              if(n_sep2 != std::string::npos)
                {
                  if(count == 0)
                    {
                      el.id = "group";
                      el.content = std::string(group_line.begin() + n_sep1,
                                               group_line.begin() + n_sep2);
                    }
                  else if(count == g_groups_column)
                    {
                      UDBElement tr_el;
                      tr_el.id = "translation";
                      tr_el.content = std::string(group_line.begin() + n_sep1,
                                                  group_line.begin() + n_sep2);
                      el.subelements.emplace_back(tr_el);
                    }
                  n_sep1 = n_sep2 + separator.size();
                }
              else
                {
                  if(count == 0)
                    {
                      el.id = "group";
                      el.content = std::string(group_line.begin() + n_sep1,
                                               group_line.end());
                    }
                  else if(count == g_groups_column)
                    {
                      UDBElement tr_el;
                      tr_el.id = "translation";
                      tr_el.content = std::string(group_line.begin() + n_sep1,
                                                  group_line.end());
                      el.subelements.emplace_back(tr_el);
                    }
                  n_sep1 = n_sep2;
                }
              count++;
            }
          if(!el.content.empty())
            {
              el.subelements.shrink_to_fit();
#pragma omp critical
              {
                base.emplace_back(el);
              }
            }
        }
      }
  }
}

void
GenreBase::loadGenres(const std::string &genres, const int &genres_column)
{
  std::string::size_type n = 0;
  std::string separator(";");
  std::string endl("\n");
  std::string::size_type n2;
#pragma omp parallel
#pragma omp masked
  {
    while(n != std::string::npos)
      {
        n2 = genres.find(endl, n);
        std::string genre_line;
        if(n2 != std::string::npos)
          {
            genre_line = std::string(genres.begin() + n, genres.begin() + n2);
            n = n2 + endl.size();
          }
        else
          {
            genre_line = std::string(genres.begin() + n, genres.end());
            n = n2;
          }

        while(genre_line.size() > 0)
          {
            char val = *genre_line.rbegin();
            if(val >= 0 && val <= ' ')
              {
                genre_line.pop_back();
              }
            else
              {
                break;
              }
          }

        if(genre_line.empty())
          {
            continue;
          }
#pragma omp task
        {
          std::string::size_type n_sep1 = 0;
          std::string::size_type n_sep2;
          int count = 0;
          UDBElement genre;
          std::string group;
          while(n_sep1 != std::string::npos)
            {
              n_sep2 = genre_line.find(separator, n_sep1);
              if(n_sep2 != std::string::npos)
                {
                  switch(count)
                    {
                    case 0:
                      {
                        genre.id = "genre";
                        genre.content
                            = std::string(genre_line.begin() + n_sep1,
                                          genre_line.begin() + n_sep2);
                        break;
                      }
                    case 1:
                      {
                        group = std::string(genre_line.begin() + n_sep1,
                                            genre_line.begin() + n_sep2);
                        break;
                      }
                    default:
                      {
                        if(count == genres_column)
                          {
                            UDBElement el;
                            el.id = "translation";
                            el.content
                                = std::string(genre_line.begin() + n_sep1,
                                              genre_line.begin() + n_sep2);
                            genre.subelements.emplace_back(el);
                          }
                        break;
                      }
                    }

                  n_sep1 = n_sep2 + separator.size();
                }
              else
                {
                  switch(count)
                    {
                    case 0:
                      {
                        genre.id = "genre";
                        genre.content = std::string(
                            genre_line.begin() + n_sep1, genre_line.end());
                        break;
                      }
                    case 1:
                      {
                        group = std::string(genre_line.begin() + n_sep1,
                                            genre_line.end());
                        break;
                      }
                    default:
                      {
                        if(count == genres_column)
                          {
                            UDBElement el;
                            el.id = "translation";
                            el.content = std::string(
                                genre_line.begin() + n_sep1, genre_line.end());
                            genre.subelements.emplace_back(el);
                          }
                        break;
                      }
                    }
                  n_sep1 = n_sep2;
                }
              count++;
            }
          if(!genre.content.empty() && !group.empty())
            {
#pragma omp critical
              {
                auto it_base = std::find_if(base.begin(), base.end(),
                                            [group](const UDBElement &el)
                                              {
                                                return el.content == group;
                                              });
                if(it_base != base.end())
                  {
                    it_base->subelements.emplace_back(genre);
                  }
              }
            }
        }
      }
  }
}
