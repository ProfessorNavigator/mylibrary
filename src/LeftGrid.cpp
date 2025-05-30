/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <Genre.h>
#include <GenreGroup.h>
#include <LeftGrid.h>
#include <MLException.h>
#include <SearchProcessGui.h>
#include <filesystem>
#include <fstream>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/expander.h>
#include <gtkmm-4.0/gtkmm/gestureclick.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/popover.h>
#include <gtkmm-4.0/gtkmm/requisition.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/separator.h>
#include <iostream>
#include <libintl.h>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#endif

LeftGrid::LeftGrid(const std::shared_ptr<AuxFunc> &af,
                   Gtk::Window *main_window,
                   const std::shared_ptr<NotesKeeper> &notes)
{
  this->af = af;
  this->main_window = main_window;
  this->notes = notes;
  base_keeper = new BaseKeeper(af);
}

LeftGrid::~LeftGrid()
{
  delete base_keeper;
}

Gtk::Grid *
LeftGrid::createGrid()
{
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_hexpand(true);

  int row = 0;

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Collection"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  collection_select = Gtk::make_managed<Gtk::DropDown>();
  collection_select->set_halign(Gtk::Align::CENTER);
  collection_select->set_margin(5);
  collection_select->set_name("comboBox");
  grid->attach(*collection_select, 0, row, 1, 1);
  row++;

  Glib::RefPtr<Gtk::StringList> list = createCollectionsList();
  collection_select->set_model(list);

  setActiveCollection();

  loadCollection(collection_select->get_selected());

  Glib::PropertyProxy<guint> selcol_prop
      = collection_select->property_selected();
  selcol_prop.signal_changed().connect(
      std::bind(&LeftGrid::saveActiveCollection, this));

  collection_select->set_enable_search(true);

  formAuthorSection(grid, row);

  formBookSection(grid, row);

  formSearchSection(grid, row);

  return grid;
}

Glib::RefPtr<Gtk::StringList>
LeftGrid::createCollectionsList()
{
  Glib::RefPtr<Gtk::StringList> list
      = Gtk::StringList::create(std::vector<Glib::ustring>());

  std::filesystem::path collections = af->homePath();
  collections /= std::filesystem::u8path(".local")
                 / std::filesystem::u8path("share")
                 / std::filesystem::u8path("MyLibrary")
                 / std::filesystem::u8path("Collections");
  if(std::filesystem::exists(collections))
    {
      for(auto &dirit : std::filesystem::directory_iterator(collections))
        {
          std::filesystem::path p = dirit.path();
          if(std::filesystem::is_directory(p))
            {
              list->append(Glib::ustring(p.filename().u8string()));
            }
        }
    }

  return list;
}

void
LeftGrid::formAuthorSection(Gtk::Grid *grid, int &row)
{
  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_text(gettext("Author"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Surname:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  surname = Gtk::make_managed<Gtk::Entry>();
  surname->set_halign(Gtk::Align::FILL);
  surname->set_margin(5);
  surname->set_width_chars(30);
  surname->set_activates_default(true);
  surname->set_name("windowEntry");
  grid->attach(*surname, 0, row, 1, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Name:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  name = Gtk::make_managed<Gtk::Entry>();
  name->set_halign(Gtk::Align::FILL);
  name->set_margin(5);
  name->set_width_chars(30);
  name->set_activates_default(true);
  name->set_name("windowEntry");
  grid->attach(*name, 0, row, 1, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Second name:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  sec_name = Gtk::make_managed<Gtk::Entry>();
  sec_name->set_halign(Gtk::Align::FILL);
  sec_name->set_margin(5);
  sec_name->set_width_chars(30);
  sec_name->set_activates_default(true);
  sec_name->set_name("windowEntry");
  grid->attach(*sec_name, 0, row, 1, 1);
  row++;
}

void
LeftGrid::formBookSection(Gtk::Grid *grid, int &row)
{
  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_text(gettext("Book"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Book name:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  book_name = Gtk::make_managed<Gtk::Entry>();
  book_name->set_halign(Gtk::Align::FILL);
  book_name->set_margin(5);
  book_name->set_width_chars(30);
  book_name->set_activates_default(true);
  book_name->set_name("windowEntry");
  grid->attach(*book_name, 0, row, 1, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Series:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  series = Gtk::make_managed<Gtk::Entry>();
  series->set_halign(Gtk::Align::FILL);
  series->set_margin(5);
  series->set_width_chars(30);
  series->set_activates_default(true);
  series->set_name("windowEntry");
  grid->attach(*series, 0, row, 1, 1);
  row++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Genre:"));
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, row, 1, 1);
  row++;

  Gtk::Grid *genre_grid = Gtk::make_managed<Gtk::Grid>();
  genre_grid->set_halign(Gtk::Align::FILL);
  genre_grid->set_valign(Gtk::Align::FILL);
  genre_grid->set_hexpand(true);
  genre_grid->set_vexpand(false);
  grid->attach(*genre_grid, 0, row, 1, 1);
  row++;

  genre_button = Gtk::make_managed<Gtk::MenuButton>();
  genre_button->set_margin(5);
  genre_button->set_halign(Gtk::Align::FILL);
  genre_button->set_hexpand(true);
  genre_button->set_vexpand(false);
  genre_button->set_name("menBut");
  genre_button->set_label(gettext("<No>"));
  genre_grid->attach(*genre_button, 0, 0, 1, 1);

  setGenrePopover(genre_button);

  Gtk::Button *clear_genre = Gtk::make_managed<Gtk::Button>();
  clear_genre->set_halign(Gtk::Align::CENTER);
  clear_genre->set_valign(Gtk::Align::CENTER);
  clear_genre->set_margin(5);
  clear_genre->set_label(gettext("Clear genre"));
  clear_genre->set_name("cancelBut");
  clear_genre->signal_clicked().connect([this] {
    genre_button->set_label(gettext("<No>"));
    selected_genre = Genre();
  });
  genre_grid->attach(*clear_genre, 1, 0, 1, 1);
}

void
LeftGrid::setGenrePopover(Gtk::MenuButton *genre_button)
{
  Gtk::Popover *genre_popover = Gtk::make_managed<Gtk::Popover>();
  genre_button->set_popover(*genre_popover);

  Gtk::ScrolledWindow *genre_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  genre_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                         Gtk::PolicyType::AUTOMATIC);
  genre_scrl->set_halign(Gtk::Align::FILL);
  genre_scrl->set_valign(Gtk::Align::FILL);
  genre_scrl->set_expand(true);
  genre_popover->set_child(*genre_scrl);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  genre_scrl->set_child(*grid);

  std::vector<GenreGroup> gg = af->get_genre_list();

  Gtk::Expander *exp = nullptr;
  Gtk::Grid *exp_grid = nullptr;
  for(size_t i = 0; i < gg.size(); i++)
    {
      exp = Gtk::make_managed<Gtk::Expander>();
      exp->set_margin_end(5);
      exp->set_halign(Gtk::Align::START);
      exp->set_expand(true);
      exp->set_expanded(false);
      exp->set_label(Glib::ustring(gg[i].group_name));
      exp_grid = formGenreExpanderGrid(gg[i].genres, genre_popover);
      exp->set_child(*exp_grid);
      grid->attach(*exp, 0, static_cast<int>(i), 1, 1);
    }

  Gtk::Requisition min, nat;
  grid->get_preferred_size(min, nat);
  genre_scrl->set_min_content_width(nat.get_width());
  genre_scrl->set_min_content_height(nat.get_height());
}

void
LeftGrid::formSearchSection(Gtk::Grid *grid, int &row)
{
  Gtk::Grid *search_grid = Gtk::make_managed<Gtk::Grid>();
  search_grid->set_halign(Gtk::Align::FILL);
  search_grid->set_valign(Gtk::Align::START);
  search_grid->set_hexpand(true);
  search_grid->set_vexpand(false);
  search_grid->set_column_homogeneous(true);
  grid->attach(*search_grid, 0, row, 1, 1);
  row++;

  Gtk::Button *search = Gtk::make_managed<Gtk::Button>();
  search->set_halign(Gtk::Align::CENTER);
  search->set_margin(5);
  search->set_label(gettext("Search"));
  search->set_name("applyBut");
  search->signal_clicked().connect(std::bind(&LeftGrid::searchBook, this));
  search_grid->attach(*search, 0, 0, 1, 1);

  main_window->set_default_widget(*search);

  Gtk::Button *clear = Gtk::make_managed<Gtk::Button>();
  clear->set_halign(Gtk::Align::CENTER);
  clear->set_margin(5);
  clear->set_label(gettext("Clear"));
  clear->set_name("cancelBut");
  clear->signal_clicked().connect(
      std::bind(&LeftGrid::clear_all_search_fields, this));
  search_grid->attach(*clear, 1, 0, 1, 1);

  Gtk::Separator *sep = Gtk::make_managed<Gtk::Separator>();
  sep->set_margin(5);
  sep->set_halign(Gtk::Align::FILL);
  search_grid->attach(*sep, 0, 1, 2, 1);

  Gtk::Button *show_files = Gtk::make_managed<Gtk::Button>();
  show_files->set_halign(Gtk::Align::CENTER);
  show_files->set_margin(5);
  show_files->set_label(gettext("Show collection files"));
  show_files->set_name("applyBut");
  show_files->signal_clicked().connect(
      std::bind(&LeftGrid::showCollectionFiles, this));
  search_grid->attach(*show_files, 0, 2, 1, 1);

  Gtk::Button *show_authors = Gtk::make_managed<Gtk::Button>();
  show_authors->set_halign(Gtk::Align::CENTER);
  show_authors->set_margin(5);
  show_authors->set_label(gettext("Show collection authors"));
  show_authors->set_name("applyBut");
  show_authors->signal_clicked().connect(
      std::bind(&LeftGrid::showCollectionAuthors, this));
  search_grid->attach(*show_authors, 1, 2, 1, 1);

  Gtk::Button *show_books_with_notes = Gtk::make_managed<Gtk::Button>();
  show_books_with_notes->set_halign(Gtk::Align::CENTER);
  show_books_with_notes->set_margin(5);
  show_books_with_notes->set_label(gettext("Show books with notes"));
  show_books_with_notes->set_name("applyBut");
  show_books_with_notes->signal_clicked().connect(
      std::bind(&LeftGrid::showBooksWithNotes, this));
  search_grid->attach(*show_books_with_notes, 0, 3, 2, 1);
}

void
LeftGrid::add_new_collection(const std::string &col_name)
{
  Glib::RefPtr<Gtk::StringList> model
      = std::dynamic_pointer_cast<Gtk::StringList>(
          collection_select->get_model());
  if(model)
    {
      model->append(Glib::ustring(col_name));
    }
}

Gtk::DropDown *
LeftGrid::get_collection_select()
{
  return collection_select;
}

void
LeftGrid::clear_all_search_fields()
{
  surname->set_text("");
  name->set_text("");
  sec_name->set_text("");
  book_name->set_text("");
  series->set_text("");
  genre_button->set_label(gettext("<No>"));
  selected_genre = Genre();
  if(clear_search_result)
    {
      clear_search_result();
    }
}

void
LeftGrid::setActiveCollection()
{
  std::filesystem::path selcol = af->homePath();
  selcol /= std::filesystem::u8path(".cache")
            / std::filesystem::u8path("MyLibrary")
            / std::filesystem::u8path("ActiveCollection");
  std::fstream f;
  f.open(selcol, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string col;
      f.seekg(0, std::ios_base::end);
      col.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(col.data(), col.size());
      f.close();

      Glib::RefPtr<Gtk::StringList> list
          = std::dynamic_pointer_cast<Gtk::StringList>(
              collection_select->get_model());
      if(list)
        {
          Glib::ustring search(col);
          for(guint i = 0; i < list->get_n_items(); i++)
            {
              if(search == list->get_string(i))
                {
                  collection_select->set_selected(i);
                  break;
                }
            }
        }
    }
}

void
LeftGrid::saveActiveCollection()
{
  Glib::RefPtr<Gtk::StringList> list
      = std::dynamic_pointer_cast<Gtk::StringList>(
          collection_select->get_model());
  if(list)
    {
      guint sel = collection_select->get_selected();
      if(sel != GTK_INVALID_LIST_POSITION)
        {
          loadCollection(sel);
          std::string col(list->get_string(sel));
          std::filesystem::path p = af->homePath();
          p /= std::filesystem::u8path(".cache")
               / std::filesystem::u8path("MyLibrary")
               / std::filesystem::u8path("ActiveCollection");
          std::filesystem::remove_all(p);
          std::filesystem::create_directories(p.parent_path());
          std::fstream f;
          f.open(p, std::ios_base::out | std::ios_base::binary);
          if(f.is_open())
            {
              f.write(col.c_str(), col.size());
              f.close();
            }
        }
    }
}

void
LeftGrid::loadCollection(const guint &sel)
{
  Glib::RefPtr<Gtk::StringList> list
      = std::dynamic_pointer_cast<Gtk::StringList>(
          collection_select->get_model());
  if(list && sel != GTK_INVALID_LIST_POSITION)
    {
      std::string col(list->get_string(sel));
#ifndef USE_OPENMP
      std::thread thr([this, col] {
        try
          {
            base_keeper->loadCollection(col);
          }
        catch(MLException &e)
          {
            std::cout << e.what() << std::endl;
          }
      });
      thr.detach();
#else
#pragma omp masked
      {
        omp_event_handle_t event;
#pragma omp task detach(event)
        {
          try
            {
              base_keeper->loadCollection(col);
            }
          catch(MLException &e)
            {
              std::cout << e.what() << std::endl;
            }
          omp_fulfill_event(event);
        }
      }
#endif
    }
}

void
LeftGrid::searchBook()
{
  BookBaseEntry bbe;
  bbe.bpe.book_author = std::string(surname->get_text()) + "\n"
                        + std::string(name->get_text()) + "\n"
                        + std::string(sec_name->get_text());
  bbe.bpe.book_name = std::string(book_name->get_text());
  bbe.bpe.book_series = std::string(series->get_text());
  bbe.bpe.book_genre = selected_genre.genre_code;
  SearchProcessGui *spg = new SearchProcessGui(base_keeper, main_window);
  spg->search_result_show = search_result_show;
  spg->createWindow(bbe);
}

void
LeftGrid::showCollectionFiles()
{
  std::string coll_name;
  guint selected = collection_select->get_selected();
  if(selected != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> list
          = std::dynamic_pointer_cast<Gtk::StringList>(
              collection_select->get_model());
      if(list)
        {
          SearchProcessGui *spg
              = new SearchProcessGui(base_keeper, main_window);
          spg->search_result_file = search_result_show_files;
          coll_name = std::string(list->get_string(selected));
          spg->createWindow(coll_name, af, 1);
        }
    }
}

void
LeftGrid::showCollectionAuthors()
{
  std::string coll_name;
  guint selected = collection_select->get_selected();
  if(selected != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> list
          = std::dynamic_pointer_cast<Gtk::StringList>(
              collection_select->get_model());
      if(list)
        {
          SearchProcessGui *spg
              = new SearchProcessGui(base_keeper, main_window);
          spg->search_result_authors = search_result_authors;
          coll_name = std::string(list->get_string(selected));
          spg->createWindow(coll_name, af, 2);
        }
    }
}

void
LeftGrid::showBooksWithNotes()
{
  std::string coll_name;
  guint selected = collection_select->get_selected();
  if(selected != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> list
          = std::dynamic_pointer_cast<Gtk::StringList>(
              collection_select->get_model());
      if(list)
        {
          SearchProcessGui *spg
              = new SearchProcessGui(base_keeper, main_window);
          spg->search_result_show = search_result_show;
          coll_name = std::string(list->get_string(selected));
          std::vector<NotesBaseEntry> nt
              = notes->getNotesForCollection(coll_name);
          spg->createWindow(nt);
        }
    }
}

void
LeftGrid::clearCollectionBase()
{
  base_keeper->clearBase();
}

bool
LeftGrid::reloadCollection(const std::string &col_name)
{
  bool result = false;
  guint selected = collection_select->get_selected();
  if(selected != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> list
          = std::dynamic_pointer_cast<Gtk::StringList>(
              collection_select->get_model());
      if(list)
        {
          if(std::string(list->get_string(selected)) == col_name)
            {
              base_keeper->clearBase();
#ifndef USE_OPENMP
              std::thread thr([this, col_name] {
                try
                  {
                    base_keeper->loadCollection(col_name);
                  }
                catch(MLException &e)
                  {
                    std::cout << "LeftGrid::reloadCollection: " << e.what()
                              << std::endl;
                  }
              });
              thr.detach();
#else
#pragma omp masked
              {
                omp_event_handle_t event;
#pragma omp task detach(event)
                {
                  try
                    {
                      base_keeper->loadCollection(col_name);
                    }
                  catch(MLException &e)
                    {
                      std::cout << "LeftGrid::reloadCollection: " << e.what()
                                << std::endl;
                    }
                  omp_fulfill_event(event);
                }
              }
#endif
              result = true;
            }
        }
    }

  return result;
}

void
LeftGrid::searchAuth(const std::string &auth)
{
  BookBaseEntry bbe;
  bbe.bpe.book_author = auth + "\n\n";
  SearchProcessGui *spg = new SearchProcessGui(base_keeper, main_window);
  spg->search_result_show = search_result_show;
  spg->createWindow(bbe);
}

void
LeftGrid::reloadCollectionList()
{
  guint selected = collection_select->get_selected();
  Glib::ustring sel;
  if(selected != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::StringList> list
          = std::dynamic_pointer_cast<Gtk::StringList>(
              collection_select->get_model());
      if(list)
        {
          sel = list->get_string(selected);
        }
    }

  Glib::RefPtr<Gtk::StringList> list = createCollectionsList();
  collection_select->set_model(list);
  for(guint i = 0; i < list->get_n_items(); i++)
    {
      if(sel == list->get_string(i))
        {
          collection_select->set_selected(i);
          break;
        }
    }
}

Gtk::Grid *
LeftGrid::formGenreExpanderGrid(const std::vector<Genre> &genre,
                                Gtk::Popover *pop)
{
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);

  Gtk::Label *lab = nullptr;
  Glib::RefPtr<Gtk::GestureClick> clck;
  for(size_t i = 0; i < genre.size(); i++)
    {
      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_halign(Gtk::Align::START);
      lab->set_expand(true);
      lab->set_margin_start(20);
      Genre g = genre[i];
      lab->set_text(Glib::ustring(g.genre_name));
      clck = Gtk::GestureClick::create();
      clck->set_button(1);
      clck->signal_pressed().connect([this, pop, g](int, double, double) {
        selected_genre = g;
        genre_button->set_label(g.genre_name);
        pop->popdown();
      });
      lab->add_controller(clck);
      grid->attach(*lab, 0, static_cast<int>(i), 1, 1);
    }

  return grid;
}
