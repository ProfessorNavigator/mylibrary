/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <BookMarksShow.h>
#include <gtkmm-4.0/gtkmm/expression.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>
#include <gtkmm-4.0/gtkmm/singleselection.h>
#include <gtkmm-4.0/gtkmm/sortlistmodel.h>
#include <gtkmm-4.0/gtkmm/stringsorter.h>
#include <libintl.h>

BookMarksShow::BookMarksShow(const std::shared_ptr<AuxFunc> &af,
                             Gtk::ColumnView *bm_view)
{
  this->af = af;
  this->bm_view = bm_view;
  model = Gio::ListStore<BookMarksModelItem>::create();
  genre_list = af->get_genre_list();
}

void
BookMarksShow::showBookMarks(
    const std::vector<std::tuple<std::string, BookBaseEntry>> &bm_v)
{
  {
    bool legacy = false;
    for(auto it = bm_v.begin(); it != bm_v.end(); it++)
      {
        std::tuple<std::string, BookBaseEntry> bm_tup = *it;
        if(std::get<0>(bm_tup).empty())
          {
            legacy = true;
          }
        Glib::RefPtr<BookMarksModelItem> item = BookMarksModelItem::create(
            std::get<0>(bm_tup), std::get<1>(bm_tup));
        model->append(item);
      }
    if(legacy && signal_legacy_bookmarks)
      {
        signal_legacy_bookmarks();
      }
  }
  bm_view->append_column(collectionColumn());
  bm_view->append_column(authorColumn());
  bm_view->append_column(bookColumn());
  bm_view->append_column(seriesColumn());
  bm_view->append_column(genreColumn());
  bm_view->append_column(dateColumn());

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> exp
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
            Glib::ustring result;
            Glib::RefPtr<BookMarksModelItem> item
                = std::dynamic_pointer_cast<BookMarksModelItem>(list_item);
            if(item)
              {
                result = Glib::ustring(std::get<0>(item->element));
              }
            return result;
          });
  coll_filter = Gtk::StringFilter::create(exp);

  exp = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<BookMarksModelItem> item
            = std::dynamic_pointer_cast<BookMarksModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(std::get<1>(item->element).bpe.book_author);
          }
        return result;
      });
  auth_filter = Gtk::StringFilter::create(exp);

  exp = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<BookMarksModelItem> item
            = std::dynamic_pointer_cast<BookMarksModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(std::get<1>(item->element).bpe.book_name);
          }
        return result;
      });
  book_filter = Gtk::StringFilter::create(exp);

  exp = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<BookMarksModelItem> item
            = std::dynamic_pointer_cast<BookMarksModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(std::get<1>(item->element).bpe.book_series);
          }
        return result;
      });
  series_filter = Gtk::StringFilter::create(exp);

  exp = Gtk::ClosureExpression<Glib::ustring>::create(
      [this](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<BookMarksModelItem> item
            = std::dynamic_pointer_cast<BookMarksModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(
                translate_genres(std::get<1>(item->element).bpe.book_genre));
          }
        return result;
      });
  genre_filter = Gtk::StringFilter::create(exp);

  exp = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<BookMarksModelItem> item
            = std::dynamic_pointer_cast<BookMarksModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(std::get<1>(item->element).bpe.book_date);
          }
        return result;
      });
  date_filter = Gtk::StringFilter::create(exp);

  filter_model = Gtk::FilterListModel::create(model, coll_filter);

  Glib::RefPtr<Gtk::SortListModel> sort_model
      = Gtk::SortListModel::create(filter_model, bm_view->get_sorter());

  Glib::RefPtr<Gtk::SingleSelection> s_selection
      = Gtk::SingleSelection::create(sort_model);

  bm_view->set_model(s_selection);
}

void
BookMarksShow::selectItem(const Glib::RefPtr<BookMarksModelItem> &item)
{
  Glib::RefPtr<BookMarksModelItem> prev = selected_item;
  selected_item = item;
  for(guint i = 0; i < model->get_n_items(); i++)
    {
      Glib::RefPtr<BookMarksModelItem> si = model->get_item(i);
      if(si == prev || si == selected_item)
        {
          model->insert(i, si);
          model->remove(i);
        }
    }
  selected_item = item;
}

Glib::RefPtr<BookMarksModelItem>
BookMarksShow::getSelectedItem()
{
  return selected_item;
}

void
BookMarksShow::removeItem(const Glib::RefPtr<BookMarksModelItem> &item)
{
  for(guint i = 0; i < model->get_n_items(); i++)
    {
      if(model->get_item(i) == item)
        {
          model->remove(i);
          if(selected_item == item)
            {
              selected_item.reset();
            }
          break;
        }
    }
}

void
BookMarksShow::filterBookmarks(const Glib::ustring &filter,
                               const guint &variant)
{
  switch(variant)
    {
    case 0:
      {
        coll_filter->set_search(filter);
        filter_model->set_filter(coll_filter);
        break;
      }
    case 1:
      {
        auth_filter->set_search(filter);
        filter_model->set_filter(auth_filter);
        break;
      }
    case 2:
      {
        book_filter->set_search(filter);
        filter_model->set_filter(book_filter);
        break;
      }
    case 3:
      {
        series_filter->set_search(filter);
        filter_model->set_filter(series_filter);
        break;
      }
    case 4:
      {
        genre_filter->set_search(filter);
        filter_model->set_filter(genre_filter);
        break;
      }
    case 5:
      {
        date_filter->set_search(filter);
        filter_model->set_filter(date_filter);
        break;
      }
    default:
      {
        break;
      }
    }
}

void
BookMarksShow::setWidth()
{
  Glib::RefPtr<Gio::ListStoreBase> list
      = std::dynamic_pointer_cast<Gio::ListStoreBase>(bm_view->get_columns());
  if(list)
    {
      int w = bm_view->get_width();
      for(guint i = 0; i < list->get_n_items(); i++)
        {
          Glib::RefPtr<Gtk::ColumnViewColumn> col
              = std::dynamic_pointer_cast<Gtk::ColumnViewColumn>(
                  list->get_object(i));
          if(col)
            {
              switch(i)
                {
                case 0:
                  {
                    col->set_fixed_width(w * 0.10);
                    break;
                  }
                case 1:
                  {
                    col->set_fixed_width(w * 0.16);
                    break;
                  }
                case 2:
                  {
                    col->set_fixed_width(w * 0.25);
                    break;
                  }
                case 3:
                  {
                    col->set_fixed_width(w * 0.16);
                    break;
                  }
                case 4:
                  {
                    col->set_fixed_width(w * 0.16);
                    break;
                  }
                case 5:
                  {
                    col->set_fixed_width(w * 0.10);
                    break;
                  }
                default:
                  break;
                }
            }
        }
    }
}

Glib::RefPtr<Gtk::ColumnViewColumn>
BookMarksShow::collectionColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&BookMarksShow::slotSetup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&BookMarksShow::slotBind, this, std::placeholders::_1, 1));

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Collection"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &BookMarksShow::expressionSlot, this, std::placeholders::_1, 1));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  return column;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
BookMarksShow::authorColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&BookMarksShow::slotSetup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&BookMarksShow::slotBind, this, std::placeholders::_1, 2));

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Author"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &BookMarksShow::expressionSlot, this, std::placeholders::_1, 2));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  return column;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
BookMarksShow::bookColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&BookMarksShow::slotSetup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&BookMarksShow::slotBind, this, std::placeholders::_1, 3));

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Book"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &BookMarksShow::expressionSlot, this, std::placeholders::_1, 3));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  return column;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
BookMarksShow::seriesColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&BookMarksShow::slotSetup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&BookMarksShow::slotBind, this, std::placeholders::_1, 4));

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Series"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &BookMarksShow::expressionSlot, this, std::placeholders::_1, 4));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  return column;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
BookMarksShow::genreColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&BookMarksShow::slotSetup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&BookMarksShow::slotBind, this, std::placeholders::_1, 5));

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Genre"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &BookMarksShow::expressionSlot, this, std::placeholders::_1, 5));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  return column;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
BookMarksShow::dateColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&BookMarksShow::slotSetup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&BookMarksShow::slotBind, this, std::placeholders::_1, 6));

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Date"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &BookMarksShow::expressionSlot, this, std::placeholders::_1, 6));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  return column;
}

void
BookMarksShow::slotSetup(const Glib::RefPtr<Gtk::ListItem> &item)
{
  if(item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_halign(Gtk::Align::FILL);
      lab->set_valign(Gtk::Align::FILL);
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_justify(Gtk::Justification::CENTER);
      item->set_child(*lab);
    }
}

void
BookMarksShow::slotBind(const Glib::RefPtr<Gtk::ListItem> &item,
                        const int &variant)
{
  Glib::RefPtr<BookMarksModelItem> m_item
      = std::dynamic_pointer_cast<BookMarksModelItem>(item->get_item());
  if(m_item)
    {
      Gtk::Label *lab = dynamic_cast<Gtk::Label *>(item->get_child());
      if(lab)
        {
          if(m_item == selected_item)
            {
              lab->set_name("selectedLab");
            }
          else
            {
              lab->set_name("windowLabel");
            }
          switch(variant)
            {
            case 1:
              {
                lab->set_text(Glib::ustring(std::get<0>(m_item->element)));
                break;
              }
            case 2:
              {
                lab->set_text(Glib::ustring(
                    std::get<1>(m_item->element).bpe.book_author));
                break;
              }
            case 3:
              {
                lab->set_text(
                    Glib::ustring(std::get<1>(m_item->element).bpe.book_name));
                break;
              }
            case 4:
              {
                lab->set_text(Glib::ustring(
                    std::get<1>(m_item->element).bpe.book_series));
                break;
              }
            case 5:
              {
                lab->set_text(Glib::ustring(translate_genres(
                    std::get<1>(m_item->element).bpe.book_genre)));
                break;
              }
            case 6:
              {
                lab->set_text(
                    Glib::ustring(std::get<1>(m_item->element).bpe.book_date));
                break;
              }
            default:
              break;
            }
        }
    }
}

Glib::ustring
BookMarksShow::translate_genres(const std::string &genres)
{
  Glib::ustring result;

  std::string loc_g = genres;
  std::string::size_type n;
  std::string sstr = ",";
  while(loc_g.size() > 0)
    {
      n = loc_g.find(sstr);
      if(n != std::string::npos)
        {
          std::string genre = loc_g.substr(0, n);
          loc_g.erase(0, n + sstr.size());
          append_genre(result, genre);
        }
      else
        {
          append_genre(result, loc_g);
          break;
        }
    }

  return result;
}

void
BookMarksShow::append_genre(Glib::ustring &result, std::string &genre)
{
  for(auto it = genre.begin(); it != genre.end();)
    {
      if(*it == ' ')
        {
          genre.erase(it);
        }
      else
        {
          break;
        }
    }

  if(!genre.empty())
    {
      Genre g;
      GenreGroup gg;
      bool stop = false;
      for(auto it = genre_list.begin(); it != genre_list.end(); it++)
        {
          for(auto itgg = it->genres.begin(); itgg != it->genres.end(); itgg++)
            {
              if(itgg->genre_code == genre)
                {
                  g = *itgg;
                  gg = *it;
                  stop = true;
                  break;
                }
            }
          if(stop)
            {
              break;
            }
        }
      if(!g.genre_name.empty())
        {
          if(result.empty())
            {
              if(gg.group_name != g.genre_name)
                {
                  result = Glib::ustring(gg.group_name + ", "
                                         + af->stringToLower(g.genre_name));
                }
              else
                {
                  result = Glib::ustring(gg.group_name);
                }
            }
          else
            {
              result = result + "\n";

              if(gg.group_name != g.genre_name)
                {
                  result = result
                           + Glib::ustring(gg.group_name + ", "
                                           + af->stringToLower(g.genre_name));
                }
              else
                {
                  result = result + Glib::ustring(gg.group_name);
                }
            }
        }
      else
        {
          if(result.empty())
            {
              result = Glib::ustring(genre);
            }
          else
            {
              result = result + "\n" + Glib::ustring(genre);
            }
        }
    }
}

Glib::ustring
BookMarksShow::expressionSlot(const Glib::RefPtr<Glib::ObjectBase> &item,
                              const int &variant)
{
  Glib::RefPtr<BookMarksModelItem> m_item
      = std::dynamic_pointer_cast<BookMarksModelItem>(item);
  Glib::ustring result;
  if(m_item)
    {
      switch(variant)
        {
        case 1:
          {
            result = Glib::ustring(std::get<0>(m_item->element));
            break;
          }
        case 2:
          {
            result
                = Glib::ustring(std::get<1>(m_item->element).bpe.book_author);
            break;
          }
        case 3:
          {
            result = Glib::ustring(std::get<1>(m_item->element).bpe.book_name);
            break;
          }
        case 4:
          {
            result
                = Glib::ustring(std::get<1>(m_item->element).bpe.book_series);
            break;
          }
        case 5:
          {
            result = Glib::ustring(
                translate_genres(std::get<1>(m_item->element).bpe.book_genre));
            break;
          }
        case 6:
          {
            result = Glib::ustring(std::get<1>(m_item->element).bpe.book_date);
            break;
          }
        default:
          break;
        }
      return result;
    }
  else
    {
      return result;
    }
}
