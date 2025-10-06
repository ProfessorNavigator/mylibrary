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

#include <ByteOrder.h>
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
#include <thread>

LeftGrid::LeftGrid(const std::shared_ptr<AuxFunc> &af,
                   Gtk::Window *main_window,
                   const std::shared_ptr<NotesKeeper> &notes)
{
  this->af = af;
  this->main_window = main_window;
  this->notes = notes;
  base_keeper = new BaseKeeper(af);
  total_books_num_disp = new Glib::Dispatcher;
  total_books_num_load_disp = new Glib::Dispatcher;
  total_books_num.store(0);
  readSearchSettings();
}

LeftGrid::~LeftGrid()
{
  load_collection_thr.reset();
  delete base_keeper;
  delete total_books_num_disp;
  delete total_books_num_load_disp;
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

  Gtk::Grid *col_grid = Gtk::make_managed<Gtk::Grid>();
  col_grid->set_halign(Gtk::Align::FILL);
  col_grid->set_hexpand(true);
  grid->attach(*col_grid, 0, row, 1, 1);
  row++;

  collection_select = Gtk::make_managed<Gtk::DropDown>();
  collection_select->set_halign(Gtk::Align::CENTER);
  collection_select->set_hexpand(true);
  collection_select->set_margin(5);
  collection_select->set_name("comboBox");
  col_grid->attach(*collection_select, 0, 0, 2, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::END);
  lab->set_margin(5);
  lab->set_text(gettext("Books quantity:"));
  lab->set_name("windowLabel");
  col_grid->attach(*lab, 0, 1, 1, 1);

  Gtk::Label *total_books_num_lab = Gtk::make_managed<Gtk::Label>();
  total_books_num_lab->set_halign(Gtk::Align::START);
  total_books_num_lab->set_margin(5);
  total_books_num_lab->set_use_markup(true);
  total_books_num_lab->set_text("0");
  total_books_num_lab->set_name("windowLabel");
  col_grid->attach(*total_books_num_lab, 1, 1, 1, 1);

  total_books_num_disp->connect([this, total_books_num_lab] {
    std::stringstream strm;
    strm.imbue(std::locale("C"));
    strm << total_books_num.load();
    total_books_num_lab->set_text(Glib::ustring(strm.str()));
  });

  total_books_num_load_disp->connect([total_books_num_lab] {
    total_books_num_lab->set_markup(Glib::ustring("<i>") + gettext("loading")
                                    + "</i>");
  });

  Gtk::Separator *sep
      = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::HORIZONTAL);
  sep->set_margin(5);
  sep->set_halign(Gtk::Align::FILL);
  sep->set_hexpand(true);
  grid->attach(*sep, 0, row, 1, 1);
  row++;

  Glib::RefPtr<Gtk::StringList> list = createCollectionsList();
  collection_select->set_model(list);

  setActiveCollection();

  loadCollection(std::dynamic_pointer_cast<Gtk::StringObject>(
      collection_select->get_selected_item()));

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
  Glib::RefPtr<Gtk::StringObject> obj
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection_select->get_selected_item());
  if(obj)
    {
      loadCollection(obj);
      std::string col(obj->get_string());
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

void
LeftGrid::loadCollection(const Glib::RefPtr<Gtk::StringObject> &selected)
{
  if(selected)
    {
      std::string col(selected->get_string());
      if(load_collection_thr)
        {
          if(load_collection_thr->joinable())
            {
              load_collection_thr->join();
            }
        }
      total_books_num.store(0);
      total_books_num_load_disp->emit();

      load_collection_thr = std::shared_ptr<std::thread>(
          new std::thread([this, col] {
            try
              {
                base_keeper->loadCollection(col);
              }
            catch(MLException &e)
              {
                std::cout << e.what() << std::endl;
              }
            total_books_num.store(base_keeper->getBooksQuantity());
            total_books_num_disp->emit();
          }),
          [](std::thread *thr) {
            if(thr->joinable())
              {
                thr->join();
              }
            delete thr;
          });
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
  spg->createWindow(bbe, coef_coincedence);
}

void
LeftGrid::showCollectionFiles()
{
  Glib::RefPtr<Gtk::StringObject> obj
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection_select->get_selected_item());
  if(obj)
    {
      SearchProcessGui *spg = new SearchProcessGui(base_keeper, main_window);
      spg->search_result_file = search_result_show_files;
      std::string coll_name(obj->get_string());
      spg->createWindow(coll_name, af, 1);
    }
}

void
LeftGrid::showCollectionAuthors()
{
  Glib::RefPtr<Gtk::StringObject> obj
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection_select->get_selected_item());
  if(obj)
    {
      SearchProcessGui *spg = new SearchProcessGui(base_keeper, main_window);
      spg->search_result_authors = search_result_authors;
      std::string coll_name(obj->get_string());
      spg->createWindow(coll_name, af, 2);
    }
}

void
LeftGrid::showBooksWithNotes()
{
  Glib::RefPtr<Gtk::StringObject> obj
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection_select->get_selected_item());
  if(obj)
    {
      SearchProcessGui *spg = new SearchProcessGui(base_keeper, main_window);
      spg->search_result_show = search_result_show;
      std::string coll_name = std::string(obj->get_string());
      std::vector<NotesBaseEntry> nt = notes->getNotesForCollection(coll_name);
      spg->createWindow(nt);
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

  Glib::RefPtr<Gtk::StringObject> obj
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection_select->get_selected_item());
  if(obj)
    {
      if(std::string(obj->get_string()) == col_name)
        {
          base_keeper->clearBase();
          if(load_collection_thr)
            {
              if(load_collection_thr->joinable())
                {
                  load_collection_thr->join();
                }
            }

          total_books_num.store(0);
          total_books_num_load_disp->emit();

          load_collection_thr = std::shared_ptr<std::thread>(
              new std::thread([this, col_name] {
                try
                  {
                    base_keeper->loadCollection(col_name);
                  }
                catch(MLException &e)
                  {
                    std::cout << "LeftGrid::reloadCollection: " << e.what()
                              << std::endl;
                  }
                total_books_num.store(base_keeper->getBooksQuantity());
                total_books_num_disp->emit();
              }),
              [](std::thread *thr) {
                if(thr->joinable())
                  {
                    thr->join();
                  }
                delete thr;
              });
          result = true;
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
  spg->createWindow(bbe, 0.99);
}

void
LeftGrid::reloadCollectionList()
{
  Glib::RefPtr<Gtk::StringObject> obj
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          collection_select->get_selected_item());

  Glib::RefPtr<Gtk::StringList> list = createCollectionsList();
  collection_select->set_model(list);
  if(obj)
    {
      for(guint i = 0; i < list->get_n_items(); i++)
        {
          if(obj->get_string() == list->get_string(i))
            {
              collection_select->set_selected(i);
              break;
            }
        }
    }
}

void
LeftGrid::setCoefOfCoincedence(const double &coef)
{
  coef_coincedence = coef;
}

double
LeftGrid::getCoefOfCoincedence()
{
  return coef_coincedence;
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

void
LeftGrid::readSearchSettings()
{
  std::filesystem::path p = af->homePath();
  p /= std::filesystem::u8path(".config");
  p /= std::filesystem::u8path("MyLibrary");
  p /= std::filesystem::u8path("SearchSettings");
  std::fstream f;
  f.open(p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      double val;
      size_t sz = sizeof(val);
      f.seekg(0, std::ios_base::end);
      size_t fsz = f.tellg();
      if(fsz >= sz)
        {
          f.seekg(0, std::ios_base::beg);
          f.read(reinterpret_cast<char *>(&val), sz);
          ByteOrder bo;
          bo.set_little(val);
          coef_coincedence = bo;

#ifdef USE_GPUOFFLOADING
          if(fsz >= sz * 3)
            {
              f.read(reinterpret_cast<char *>(&val), sz);
              bo.set_little(val);
              double cpu_gpu_balance_auth = bo;

              f.read(reinterpret_cast<char *>(&val), sz);
              bo.set_little(val);
              val = bo;

              af->setCpuGpuBalance(cpu_gpu_balance_auth, val);
            }
#endif
        }
      f.close();
    }
}
