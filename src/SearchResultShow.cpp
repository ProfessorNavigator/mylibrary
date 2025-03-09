/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <BookParseEntry.h>
#include <Genre.h>
#include <SearchResultShow.h>
#include <filesystem>
#include <functional>
#include <glibmm/main.h>
#include <glibmm/signalproxy.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/columnviewcolumn.h>
#include <gtkmm/enums.h>
#include <gtkmm/expression.h>
#include <gtkmm/filterlistmodel.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <gtkmm/singleselection.h>
#include <gtkmm/sortlistmodel.h>
#include <gtkmm/stringfilter.h>
#include <gtkmm/stringsorter.h>
#include <libintl.h>
#include <pangomm/layout.h>
#include <sigc++/connection.h>
#include <thread>

SearchResultShow::SearchResultShow(const std::shared_ptr<AuxFunc> &af,
                                   Gtk::ColumnView *search_res)
{
  this->af = af;
  this->search_res = search_res;

  disp_adjust = new Glib::Dispatcher;
  disp_adjust->connect([this, search_res] {
    Glib::RefPtr<Gtk::Adjustment> adj = search_res->get_vadjustment();
    adj->set_value(0.0);
  });

  genre_list = af->get_genre_list();
}

SearchResultShow::~SearchResultShow()
{
  delete disp_adjust;
}

void
SearchResultShow::clearSearchResult()
{
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  selected_item.reset();
  selected_item_file.reset();
  selected_item_auth.reset();
  if(model)
    {
      model->remove_all();
      model.reset();
    }
  if(model_files)
    {
      model_files->remove_all();
      model_files.reset();
    }
  if(model_auth)
    {
      model_auth->remove_all();
      model_auth.reset();
    }
  str_filter.reset();
  filter_model.reset();
  author_filter.reset();
  book_filter.reset();
  series_filter.reset();
  genre_filter.reset();
  date_filter.reset();
  Glib::RefPtr<Gio::ListStoreBase> list
      = std::dynamic_pointer_cast<Gio::ListStoreBase>(
          search_res->get_columns());
  if(list)
    {
      std::vector<Glib::RefPtr<Gtk::ColumnViewColumn>> lv;
      for(guint i = 0; i < list->get_n_items(); i++)
        {
          Glib::RefPtr<Gtk::ColumnViewColumn> col
              = std::dynamic_pointer_cast<Gtk::ColumnViewColumn>(
                  list->get_object(i));
          if(col)
            {
              lv.emplace_back(col);
            }
        }
      for(auto it = lv.begin(); it != lv.end(); it++)
        {
          search_res->remove_column(*it);
        }
    }
  Glib::RefPtr<Gtk::SingleSelection> sel
      = std::dynamic_pointer_cast<Gtk::SingleSelection>(
          search_res->get_model());
  if(sel)
    {
      Glib::RefPtr<Gtk::SortListModel> sort_model
          = std::dynamic_pointer_cast<Gtk::SortListModel>(sel->get_model());
      if(sort_model)
        {
          list = std::dynamic_pointer_cast<Gio::ListStoreBase>(
              sort_model->get_model());
          if(list)
            {
              list->remove_all();
            }
        }
    }
}

void
SearchResultShow::searchResultShow(const std::vector<BookBaseEntry> &result)
{
  clearSearchResult();

  model = Gio::ListStore<SearchResultModelItem>::create();

  for(auto it = result.begin(); it != result.end(); it++)
    {
      Glib::RefPtr<SearchResultModelItem> item
          = SearchResultModelItem::create(*it);
      model->append(item);
    }

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
            Glib::ustring result;
            Glib::RefPtr<SearchResultModelItem> item
                = std::dynamic_pointer_cast<SearchResultModelItem>(list_item);
            if(item)
              {
                result = Glib::ustring(item->bbe.bpe.book_author);
              }
            return result;
          });
  author_filter = Gtk::StringFilter::create(expr);

  expr = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<SearchResultModelItem> item
            = std::dynamic_pointer_cast<SearchResultModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(item->bbe.bpe.book_name);
          }
        return result;
      });
  book_filter = Gtk::StringFilter::create(expr);

  expr = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<SearchResultModelItem> item
            = std::dynamic_pointer_cast<SearchResultModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(item->bbe.bpe.book_series);
          }
        return result;
      });
  series_filter = Gtk::StringFilter::create(expr);

  expr = Gtk::ClosureExpression<Glib::ustring>::create(
      [this](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<SearchResultModelItem> item
            = std::dynamic_pointer_cast<SearchResultModelItem>(list_item);
        if(item)
          {
            result = translate_genres(item->bbe.bpe.book_genre);
          }
        return result;
      });
  genre_filter = Gtk::StringFilter::create(expr);

  expr = Gtk::ClosureExpression<Glib::ustring>::create(
      [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
        Glib::ustring result;
        Glib::RefPtr<SearchResultModelItem> item
            = std::dynamic_pointer_cast<SearchResultModelItem>(list_item);
        if(item)
          {
            result = Glib::ustring(item->bbe.bpe.book_date);
          }
        return result;
      });
  date_filter = Gtk::StringFilter::create(expr);

  filter_model = Gtk::FilterListModel::create(model, author_filter);

  Glib::RefPtr<Gtk::SortListModel> sort_model
      = Gtk::SortListModel::create(filter_model, search_res->get_sorter());

  Glib::RefPtr<Gtk::SingleSelection> select
      = Gtk::SingleSelection::create(sort_model);

  search_res->set_model(select);

  formAuthorColumn();
  formBookColumn();
  formSeriesColumn();
  formGenreColumn();
  formDateColumn();

  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  std::thread thr([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    disp_adjust->emit();
  });
  thr.detach();
}

void
SearchResultShow::searchResultShow(const std::vector<FileParseEntry> &result)
{
  clearSearchResult();

  model_files = Gio::ListStore<SearchResultModelItemFL>::create();

  for(auto it = result.begin(); it != result.end(); it++)
    {
      Glib::RefPtr<SearchResultModelItemFL> item
          = SearchResultModelItemFL::create(*it);
      model_files->append(item);
    }

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
            Glib::ustring result;
            Glib::RefPtr<SearchResultModelItemFL> item
                = std::dynamic_pointer_cast<SearchResultModelItemFL>(
                    list_item);
            if(item)
              {
                result = Glib::ustring(item->entry.file_rel_path);
              }
            return result;
          });

  str_filter = Gtk::StringFilter::create(expr);
  Glib::RefPtr<Gtk::FilterListModel> filer_list_model
      = Gtk::FilterListModel::create(model_files, str_filter);

  Glib::RefPtr<Gtk::SortListModel> sort_model
      = Gtk::SortListModel::create(filer_list_model, search_res->get_sorter());

  Glib::RefPtr<Gtk::SingleSelection> select
      = Gtk::SingleSelection::create(sort_model);

  search_res->set_model(select);

  formFilesColumn();

  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  std::thread thr([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    disp_adjust->emit();
  });
  thr.detach();
}

void
SearchResultShow::searchResultShow(const std::vector<std::string> &result)
{
  clearSearchResult();

  model_auth = Gio::ListStore<SearchResultModelItemAuth>::create();

  for(auto it = result.begin(); it != result.end(); it++)
    {
      Glib::RefPtr<SearchResultModelItemAuth> item
          = SearchResultModelItemAuth::create(*it);
      model_auth->append(item);
    }

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
            Glib::ustring result;
            Glib::RefPtr<SearchResultModelItemAuth> item
                = std::dynamic_pointer_cast<SearchResultModelItemAuth>(
                    list_item);
            if(item)
              {
                result = Glib::ustring(item->auth);
              }
            return result;
          });

  str_filter = Gtk::StringFilter::create(expr);
  Glib::RefPtr<Gtk::FilterListModel> filer_list_model
      = Gtk::FilterListModel::create(model_auth, str_filter);

  Glib::RefPtr<Gtk::SortListModel> sort_model
      = Gtk::SortListModel::create(filer_list_model, search_res->get_sorter());

  Glib::RefPtr<Gtk::SingleSelection> select
      = Gtk::SingleSelection::create(sort_model);

  search_res->set_model(select);

  formAuthColumn();

  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  std::thread thr([this] {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    disp_adjust->emit();
  });
  thr.detach();
}

void
SearchResultShow::formAuthorColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory = createFactory(1);

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Author"), factory);

  column->set_fixed_width(0.25 * search_res->get_width());
  column->set_resizable(true);
  column->set_expand(true);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &SearchResultShow::expression_slot, this, std::placeholders::_1, 1));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  search_res->append_column(column);
  search_res->sort_by_column(column, Gtk::SortType::ASCENDING);
}

void
SearchResultShow::itemSetup(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::FILL);
  lab->set_valign(Gtk::Align::FILL);
  lab->set_expand(true);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_justify(Gtk::Justification::CENTER);
  list_item->set_child(*lab);
}

void
SearchResultShow::formBookColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory = createFactory(2);

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Book"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &SearchResultShow::expression_slot, this, std::placeholders::_1, 2));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_fixed_width(0.25 * search_res->get_width());
  column->set_resizable(true);
  column->set_expand(true);

  search_res->append_column(column);
}

void
SearchResultShow::formSeriesColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory = createFactory(3);

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Series"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &SearchResultShow::expression_slot, this, std::placeholders::_1, 3));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_fixed_width(0.2 * search_res->get_width());
  column->set_resizable(true);
  column->set_expand(true);

  search_res->append_column(column);
}

void
SearchResultShow::formGenreColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory = createFactory(4);

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Genre"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &SearchResultShow::expression_slot, this, std::placeholders::_1, 4));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_fixed_width(0.2 * search_res->get_width());
  column->set_resizable(true);
  column->set_expand(true);

  search_res->append_column(column);
}

Glib::RefPtr<Gtk::SignalListItemFactory>
SearchResultShow::createFactory(const int &variant)
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();

  factory->signal_setup().connect(
      std::bind(&SearchResultShow::itemSetup, this, std::placeholders::_1));

  factory->signal_bind().connect(std::bind(&SearchResultShow::itemBind, this,
                                           std::placeholders::_1, variant));
  return factory;
}

void
SearchResultShow::itemBind(const Glib::RefPtr<Gtk::ListItem> &list_item,
                           const int &variant)
{
  Glib::RefPtr<SearchResultModelItem> item
      = std::dynamic_pointer_cast<SearchResultModelItem>(
          list_item->get_item());
  if(item)
    {
      Gtk::Label *lab = dynamic_cast<Gtk::Label *>(list_item->get_child());
      if(lab)
        {
          if(item == selected_item)
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
                lab->set_text(item->bbe.bpe.book_author);
                break;
              }
            case 2:
              {
                lab->set_text(item->bbe.bpe.book_name);
                break;
              }
            case 3:
              {
                lab->set_text(item->bbe.bpe.book_series);
                break;
              }
            case 4:
              {
                lab->set_text(translate_genres(item->bbe.bpe.book_genre));
                break;
              }
            case 5:
              {
                lab->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
                lab->set_text(item->bbe.bpe.book_date);
                break;
              }
            default:
              break;
            }
        }
    }
}

Glib::ustring
SearchResultShow::translate_genres(const std::string &genres)
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
SearchResultShow::formDateColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory = createFactory(5);

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Date"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(std::bind(
          &SearchResultShow::expression_slot, this, std::placeholders::_1, 5));

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  search_res->append_column(column);
}

void
SearchResultShow::formFilesColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();

  factory->signal_setup().connect(
      [](const Glib::RefPtr<Gtk::ListItem> &l_item) {
        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_halign(Gtk::Align::START);
        lab->set_ellipsize(Pango::EllipsizeMode::START);
        l_item->set_child(*lab);
      });

  factory->signal_bind().connect(
      [this](const Glib::RefPtr<Gtk::ListItem> &l_item) {
        Glib::RefPtr<SearchResultModelItemFL> item
            = std::dynamic_pointer_cast<SearchResultModelItemFL>(
                l_item->get_item());
        if(item)
          {
            Gtk::Label *lab = dynamic_cast<Gtk::Label *>(l_item->get_child());
            if(lab)
              {
                lab->set_text(item->entry.file_rel_path);
                if(item == selected_item_file)
                  {
                    lab->set_name("selectedLab");
                  }
                else
                  {
                    lab->set_name("windowLabel");
                  }
              }
          }
      });

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("File path"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
            Glib::ustring result;
            Glib::RefPtr<SearchResultModelItemFL> item
                = std::dynamic_pointer_cast<SearchResultModelItemFL>(
                    list_item);
            if(item)
              {
                result = Glib::ustring(item->entry.file_rel_path);
              }
            return result;
          });

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  search_res->append_column(column);
}

void
SearchResultShow::formAuthColumn()
{
  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();

  factory->signal_setup().connect(
      [](const Glib::RefPtr<Gtk::ListItem> &l_item) {
        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_halign(Gtk::Align::START);
        lab->set_ellipsize(Pango::EllipsizeMode::START);
        l_item->set_child(*lab);
      });

  factory->signal_bind().connect(
      [this](const Glib::RefPtr<Gtk::ListItem> &l_item) {
        Glib::RefPtr<SearchResultModelItemAuth> item
            = std::dynamic_pointer_cast<SearchResultModelItemAuth>(
                l_item->get_item());
        if(item)
          {
            Gtk::Label *lab = dynamic_cast<Gtk::Label *>(l_item->get_child());
            if(lab)
              {
                lab->set_text(item->auth);
                if(item == selected_item_auth)
                  {
                    lab->set_name("selectedLab");
                  }
                else
                  {
                    lab->set_name("windowLabel");
                  }
              }
          }
      });

  Glib::RefPtr<Gtk::ColumnViewColumn> column
      = Gtk::ColumnViewColumn::create(gettext("Author"), factory);

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](const Glib::RefPtr<Glib::ObjectBase> &list_item) {
            Glib::ustring result;
            Glib::RefPtr<SearchResultModelItemAuth> item
                = std::dynamic_pointer_cast<SearchResultModelItemAuth>(
                    list_item);
            if(item)
              {
                result = Glib::ustring(item->auth);
              }
            return result;
          });

  Glib::RefPtr<Gtk::StringSorter> sorter = Gtk::StringSorter::create(expr);
  column->set_sorter(sorter);

  column->set_resizable(true);
  column->set_expand(true);

  search_res->append_column(column);
  search_res->sort_by_column(column, Gtk::SortType::ASCENDING);
}

void
SearchResultShow::append_genre(Glib::ustring &result, std::string &genre)
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
SearchResultShow::expression_slot(
    const Glib::RefPtr<Glib::ObjectBase> &list_item, const int &variant)
{
  Glib::RefPtr<SearchResultModelItem> item
      = std::dynamic_pointer_cast<SearchResultModelItem>(list_item);
  Glib::ustring result;
  if(item)
    {
      switch(variant)
        {
        case 1:
          {
            result = Glib::ustring(item->bbe.bpe.book_author);
            break;
          }
        case 2:
          {
            result = Glib::ustring(item->bbe.bpe.book_name);
            break;
          }
        case 3:
          {
            result = Glib::ustring(item->bbe.bpe.book_series);
            break;
          }
        case 4:
          {
            result = translate_genres(item->bbe.bpe.book_genre);
            break;
          }
        case 5:
          {
            result = Glib::ustring(item->bbe.bpe.book_date);
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

void
SearchResultShow::select_item(const Glib::RefPtr<SearchResultModelItem> &item)
{
  Glib::RefPtr<SearchResultModelItem> prev = selected_item;
  selected_item = item;
  for(guint i = 0; i < model->get_n_items(); i++)
    {
      Glib::RefPtr<SearchResultModelItem> si = model->get_item(i);
      if(si == prev || si == selected_item)
        {
          model->insert(i, si);
          model->remove(i);
        }
    }
}

void
SearchResultShow::select_item(
    const Glib::RefPtr<SearchResultModelItemFL> &item)
{
  Glib::RefPtr<SearchResultModelItemFL> prev = selected_item_file;
  selected_item_file = item;
  for(guint i = 0; i < model_files->get_n_items(); i++)
    {
      Glib::RefPtr<SearchResultModelItemFL> si = model_files->get_item(i);
      if(si == prev || si == selected_item_file)
        {
          model_files->insert(i, si);
          model_files->remove(i);
        }
    }
}

void
SearchResultShow::select_item(
    const Glib::RefPtr<SearchResultModelItemAuth> &item)
{
  Glib::RefPtr<SearchResultModelItemAuth> prev = selected_item_auth;
  selected_item_auth = item;
  for(guint i = 0; i < model_auth->get_n_items(); i++)
    {
      Glib::RefPtr<SearchResultModelItemAuth> si = model_auth->get_item(i);
      if(si == prev || si == selected_item_auth)
        {
          model_auth->insert(i, si);
          model_auth->remove(i);
        }
    }
}

Glib::RefPtr<SearchResultModelItem>
SearchResultShow::get_selected_item()
{
  return selected_item;
}

Glib::RefPtr<SearchResultModelItemFL>
SearchResultShow::get_selected_item_file()
{
  return selected_item_file;
}

Glib::RefPtr<SearchResultModelItemAuth>
SearchResultShow::get_selected_item_auth()
{
  return selected_item_auth;
}

void
SearchResultShow::removeItem(const Glib::RefPtr<SearchResultModelItem> &item)
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
SearchResultShow::removeBook(const Glib::RefPtr<SearchResultModelItem> &item)
{
  std::vector<Glib::RefPtr<SearchResultModelItem>> for_remove;
  for_remove.push_back(item);
  std::string ext = item->bbe.file_path.extension().u8string();
  ext = af->stringToLower(ext);
  if(ext == ".rar")
    {
      for(guint i = 0; i < model->get_n_items(); i++)
        {
          Glib::RefPtr<SearchResultModelItem> it = model->get_item(i);
          if(it != item)
            {
              if(it->bbe.file_path == item->bbe.file_path)
                {
                  for_remove.push_back(it);
                }
            }
        }
    }
  else
    {
      std::string ch_p;
      std::string ent = item->bbe.bpe.book_path;
      bool rar = false;
      std::string::size_type n;
      std::filesystem::path val;
      std::string sstr = "\n";
      for(;;)
        {
          n = ent.find(sstr);
          if(n != std::string::npos)
            {
              val = std::filesystem::u8path(ent.substr(0, n));
              ent.erase(0, n + sstr.size());
              if(ch_p.empty())
                {
                  ch_p = val.u8string();
                }
              else
                {
                  ch_p = ch_p + sstr + val.u8string();
                }
              ext = val.extension().u8string();
              ext = af->stringToLower(ext);
              if(ext == ".rar")
                {
                  rar = true;
                  break;
                }
            }
          else
            {
              if(!ent.empty())
                {
                  val = std::filesystem::u8path(ent);
                  if(ch_p.empty())
                    {
                      ch_p = val.u8string();
                    }
                  else
                    {
                      ch_p = ch_p + sstr + val.u8string();
                    }
                  ext = val.extension().u8string();
                  ext = af->stringToLower(ext);
                  if(ext == ".rar")
                    {
                      rar = true;
                    }
                }
              break;
            }
        }

      if(rar)
        {
          for(guint i = 0; i < model->get_n_items(); i++)
            {
              Glib::RefPtr<SearchResultModelItem> it = model->get_item(i);
              if(it != item)
                {
                  if(it->bbe.file_path == item->bbe.file_path)
                    {
                      std::string b_p = it->bbe.bpe.book_path;
                      n = b_p.find(ch_p);
                      if(n != std::string::npos)
                        {
                          for_remove.push_back(it);
                        }
                    }
                }
            }
        }
    }

  for(auto it = for_remove.begin(); it != for_remove.end(); it++)
    {
      removeItem(*it);
    }
}

Glib::RefPtr<Gio::ListStore<SearchResultModelItem>>
SearchResultShow::get_model()
{
  return model;
}

void
SearchResultShow::filterFiles(const Glib::ustring &filter_val)
{
  if(str_filter)
    {
      str_filter->set_search(filter_val);
    }
}

void
SearchResultShow::filterAuth(const Glib::ustring &filter_val)
{
  filterFiles(filter_val);
}

void
SearchResultShow::filterBooks(const Glib::ustring &filter_val,
                              const guint &variant)
{
  switch(variant)
    {
    case 0:
      {
        if(author_filter)
          {
            filter_model->set_filter(author_filter);
            author_filter->set_search(filter_val);
          }
        break;
      }
    case 1:
      {
        if(book_filter)
          {
            filter_model->set_filter(book_filter);
            book_filter->set_search(filter_val);
          }
        break;
      }
    case 2:
      {
        if(series_filter)
          {
            filter_model->set_filter(series_filter);
            series_filter->set_search(filter_val);
          }
        break;
      }
    case 3:
      {
        if(genre_filter)
          {
            filter_model->set_filter(genre_filter);
            genre_filter->set_search(filter_val);
          }
        break;
      }
    case 4:
      {
        if(date_filter)
          {
            filter_model->set_filter(date_filter);
            date_filter->set_search(filter_val);
          }
        break;
      }
    default:
      break;
    }
}
