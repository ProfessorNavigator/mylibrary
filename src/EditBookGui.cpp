/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <EditBookGui.h>
#include <gdkmm/rectangle.h>
#include <giomm/menuitem.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>
#include <glibmm/signalproxy.h>
#include <glibmm/ustring.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/columnview.h>
#include <gtkmm/enums.h>
#include <gtkmm/expander.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/label.h>
#include <gtkmm/object.h>
#include <gtkmm/requisition.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/signallistitemfactory.h>
#include <libintl.h>
#include <MLException.h>
#include <RefreshCollection.h>
#include <sigc++/connection.h>
#include <stddef.h>
#include <atomic>
#include <iostream>
#include <iterator>

EditBookGui::EditBookGui(const std::shared_ptr<AuxFunc> &af,
			 Gtk::Window *parent_window,
			 const std::shared_ptr<BookMarks> &bookmarks,
			 const std::string &collection_name,
			 const BookBaseEntry &bbe)
{
  this->af = af;
  this->parent_window = parent_window;
  this->bookmarks = bookmarks;
  this->collection_name = collection_name;
  this->bbe = bbe;
  genre_list = af->get_genre_list();
}

EditBookGui::~EditBookGui()
{
  delete restore_disp;
}

void
EditBookGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Book entry editor"));
  window->set_transient_for(*parent_window);
  window->set_modal(true);
  window->set_name("MLwindow");
  create_genre_action_group(window);

  restore_disp = new Glib::Dispatcher;
  restore_disp->connect([this, window]
  {
    window->unset_child();
    this->form_window_grid(window);
  });

  form_window_grid(window);

  window->signal_close_request().connect([window, this]
  {
    std::shared_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    delete this;
    return true;
  },
					 false);

  window->present();
}

Glib::RefPtr<Gio::ListStore<EditBookGenreModelItem>>
EditBookGui::create_genre_model()
{
  Glib::RefPtr<Gio::ListStore<EditBookGenreModelItem>> result = Gio::ListStore<
      EditBookGenreModelItem>::create();

  std::string genres = bbe.bpe.book_genre;

  std::string::size_type n;
  std::string genre_code;
  Glib::RefPtr<EditBookGenreModelItem> item;
  for(;;)
    {
      genre_code.clear();
      n = genres.find(",");
      if(n != std::string::npos)
	{
	  genre_code = genres.substr(0, n);
	  genres.erase(0, n + std::string(",").size());
	  item = create_item(genre_code);
	  if(item)
	    {
	      result->append(item);
	    }
	}
      else
	{
	  if(!genres.empty())
	    {
	      item = create_item(genres);
	      if(item)
		{
		  result->append(item);
		}
	    }
	  break;
	}
    }

  return result;
}

Glib::RefPtr<EditBookGenreModelItem>
EditBookGui::create_item(std::string &genre_code)
{
  std::string genre_name;
  for(auto it = genre_code.begin(); it != genre_code.end();)
    {
      if(*it == ' ')
	{
	  genre_code.erase(it);
	}
      else
	{
	  break;
	}
    }

  if(genre_code.empty())
    {
      Glib::RefPtr<EditBookGenreModelItem> item;
      return item;
    }

  std::vector<Genre> g_v;
  for(auto it = genre_list.begin(); it != genre_list.end(); it++)
    {
      g_v = it->genres;
      bool found = false;
      for(auto itgv = g_v.begin(); itgv != g_v.end(); itgv++)
	{
	  if(itgv->genre_code == genre_code)
	    {
	      found = true;
	      if(itgv->genre_name != it->group_name)
		{
		  genre_name = it->group_name + ", "
		      + af->stringToLower(itgv->genre_name);
		}
	      else
		{
		  genre_name = it->group_name;
		}
	    }
	}
      if(found)
	{
	  break;
	}
    }
  if(genre_name.empty())
    {
      genre_name = genre_code;
    }

  return EditBookGenreModelItem::create(genre_code, genre_name);
}

Glib::RefPtr<Gtk::ColumnViewColumn>
EditBookGui::genre_col_name()
{
  Glib::RefPtr<Gtk::ColumnViewColumn> result = Gtk::ColumnViewColumn::create(
      gettext("Genre name"));

  Glib::RefPtr<Gtk::SignalListItemFactory> factory =
      Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&EditBookGui::slot_genre_setup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&EditBookGui::slot_genre_bind, this, std::placeholders::_1, 1));
  result->set_factory(factory);

  result->set_expand(true);
  result->set_resizable(true);

  return result;
}

Glib::RefPtr<Gtk::ColumnViewColumn>
EditBookGui::genre_col_code()
{
  Glib::RefPtr<Gtk::ColumnViewColumn> result = Gtk::ColumnViewColumn::create(
      gettext("Genre code"));

  Glib::RefPtr<Gtk::SignalListItemFactory> factory =
      Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect(
      std::bind(&EditBookGui::slot_genre_setup, this, std::placeholders::_1));
  factory->signal_bind().connect(
      std::bind(&EditBookGui::slot_genre_bind, this, std::placeholders::_1, 2));
  result->set_factory(factory);

  result->set_expand(true);
  result->set_resizable(true);

  return result;
}

void
EditBookGui::slot_genre_setup(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_justify(Gtk::Justification::CENTER);
  list_item->set_child(*lab);
}

void
EditBookGui::slot_genre_bind(const Glib::RefPtr<Gtk::ListItem> &list_item,
			     const int &variant)
{
  Glib::RefPtr<EditBookGenreModelItem> item = std::dynamic_pointer_cast<
      EditBookGenreModelItem>(list_item->get_item());

  if(item)
    {
      Gtk::Label *lab = dynamic_cast<Gtk::Label*>(list_item->get_child());
      if(item == selected_genre)
	{
	  lab->set_name("selectedLab");
	}
      else
	{
	  lab->set_name("unselectedLab");
	}
      switch(variant)
	{
	case 1:
	  {
	    lab->set_text(Glib::ustring(item->genre_name));
	    break;
	  }
	case 2:
	  {
	    lab->set_text(Glib::ustring(item->genre_code));
	    break;
	  }
	default:
	  {
	    break;
	  }
	}
    }
}

void
EditBookGui::select_genre(guint pos)
{

  auto item = selected_genre;
  selected_genre = genre_model->get_item(pos);
  for(guint i = 0; i < genre_model->get_n_items(); i++)
    {
      auto it = genre_model->get_item(i);
      if(it == item || it == selected_genre)
	{
	  genre_model->insert(i, it);
	  genre_model->remove(i);
	}
    }
}

void
EditBookGui::setGenrePopover(Gtk::MenuButton *genre_button)
{
  genre_popover = Gtk::make_managed<Gtk::Popover>();
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

  Gtk::Expander *exp = nullptr;
  Gtk::Grid *exp_grid = nullptr;
  for(size_t i = 0; i < genre_list.size(); i++)
    {
      exp = Gtk::make_managed<Gtk::Expander>();
      exp->set_margin_end(5);
      exp->set_halign(Gtk::Align::START);
      exp->set_expand(true);
      exp->set_expanded(false);
      exp->set_label(Glib::ustring(genre_list[i].group_name));
      exp_grid = formGenreExpanderGrid(genre_list[i].group_name,
				       genre_list[i].genres);
      exp->set_child(*exp_grid);
      grid->attach(*exp, 0, static_cast<int>(i), 1, 1);
    }

  Gtk::Requisition min, nat;
  grid->get_preferred_size(min, nat);
  genre_scrl->set_min_content_width(nat.get_width());
  genre_scrl->set_min_content_height(nat.get_height());
}

Gtk::Grid*
EditBookGui::formGenreExpanderGrid(const std::string &group_name,
				   const std::vector<Genre> &genre)
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
      clck->signal_pressed().connect(
	  std::bind(&EditBookGui::slot_genre_select, this,
		    std::placeholders::_1, std::placeholders::_2,
		    std::placeholders::_3, g, group_name));
      lab->add_controller(clck);
      grid->attach(*lab, 0, static_cast<int>(i), 1, 1);
    }

  return grid;
}

void
EditBookGui::slot_genre_select(int numclc, double x, double y, const Genre &g,
			       const std::string &group_name)
{
  std::string nm;
  if(g.genre_name == group_name)
    {
      nm = group_name;
    }
  else
    {
      nm = group_name + ", " + this->af->stringToLower(g.genre_name);
    }
  Glib::RefPtr<EditBookGenreModelItem> item = EditBookGenreModelItem::create(
      g.genre_code, nm);
  genre_model->append(item);

  genre_popover->popdown();
}

void
EditBookGui::remove_genre()
{
  for(guint i = 0; i < genre_model->get_n_items(); i++)
    {
      if(genre_model->get_item(i) == selected_genre)
	{
	  genre_model->remove(i);
	  selected_genre.reset();
	  break;
	}
    }
}

void
EditBookGui::create_genre_action_group(Gtk::Window *win)
{
  Glib::RefPtr<Gio::SimpleActionGroup> group = Gio::SimpleActionGroup::create();

  group->add_action("remove_genre",
		    std::bind(&EditBookGui::remove_genre, this));

  win->insert_action_group("genre_ac", group);
}

Glib::RefPtr<Gio::Menu>
EditBookGui::create_genre_menu()
{
  Glib::RefPtr<Gio::Menu> result = Gio::Menu::create();

  Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create(
      gettext("Remove genre"), "genre_ac.remove_genre");
  result->append_item(item);

  return result;
}

void
EditBookGui::form_window_grid(Gtk::Window *window)
{
  selected_genre.reset();

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  int row_num = 0;

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Book name:"));
  grid->attach(*lab, 0, row_num, 3, 1);
  row_num++;

  book_ent = Gtk::make_managed<Gtk::Entry>();
  book_ent->set_margin(5);
  book_ent->set_halign(Gtk::Align::FILL);
  book_ent->set_hexpand(true);
  book_ent->set_width_chars(70);
  book_ent->set_text(bbe.bpe.book_name);
  grid->attach(*book_ent, 0, row_num, 3, 1);
  row_num++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Author:"));
  grid->attach(*lab, 0, row_num, 3, 1);
  row_num++;

  author_ent = Gtk::make_managed<Gtk::Entry>();
  author_ent->set_margin(5);
  author_ent->set_halign(Gtk::Align::FILL);
  author_ent->set_hexpand(true);
  author_ent->set_width_chars(70);
  author_ent->set_text(bbe.bpe.book_author);
  grid->attach(*author_ent, 0, row_num, 3, 1);
  row_num++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Series:"));
  grid->attach(*lab, 0, row_num, 2, 1);
  row_num++;

  series_ent = Gtk::make_managed<Gtk::Entry>();
  series_ent->set_margin(5);
  series_ent->set_halign(Gtk::Align::FILL);
  series_ent->set_hexpand(true);
  series_ent->set_width_chars(70);
  series_ent->set_text(bbe.bpe.book_series);
  grid->attach(*series_ent, 0, row_num, 3, 1);
  row_num++;

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Genre:"));
  grid->attach(*lab, 0, row_num, 3, 1);
  row_num++;

  Gtk::ScrolledWindow *genre_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  genre_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
			 Gtk::PolicyType::AUTOMATIC);
  genre_scrl->set_halign(Gtk::Align::FILL);
  genre_scrl->set_valign(Gtk::Align::FILL);
  genre_scrl->set_expand(true);
  genre_scrl->set_margin(5);
  grid->attach(*genre_scrl, 0, row_num, 3, 1);
  row_num++;

  genre_model = create_genre_model();

  Glib::RefPtr<Gtk::SingleSelection> selection = Gtk::SingleSelection::create(
      genre_model);

  Glib::RefPtr<Gtk::ColumnViewColumn> col_name = genre_col_name();
  Glib::RefPtr<Gtk::ColumnViewColumn> col_code = genre_col_code();

  Gtk::ColumnView *genre_view = Gtk::make_managed<Gtk::ColumnView>();
  genre_view->set_halign(Gtk::Align::FILL);
  genre_view->set_valign(Gtk::Align::FILL);
  genre_view->set_expand(true);
  genre_view->set_model(selection);
  genre_view->append_column(col_name);
  genre_view->append_column(col_code);
  genre_view->set_single_click_activate(true);
  genre_view->set_reorderable(false);
  genre_view->signal_activate().connect(
      std::bind(&EditBookGui::select_genre, this, std::placeholders::_1));
  genre_scrl->set_child(*genre_view);

  Glib::RefPtr<Gio::Menu> menu = create_genre_menu();

  Gtk::PopoverMenu *genre_menu = Gtk::make_managed<Gtk::PopoverMenu>();
  genre_menu->set_menu_model(menu);
  genre_menu->set_parent(*genre_view);
  genre_view->signal_unrealize().connect(
      std::bind(&Gtk::PopoverMenu::unparent, genre_menu));

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect(
      std::bind(&EditBookGui::show_genre_menu, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3, genre_menu,
		selection));
  genre_view->add_controller(clck);

  Gtk::Grid *genre_action_grid = Gtk::make_managed<Gtk::Grid>();
  genre_action_grid->set_halign(Gtk::Align::FILL);
  genre_action_grid->set_valign(Gtk::Align::FILL);
  genre_action_grid->set_hexpand(true);
  grid->attach(*genre_action_grid, 0, row_num, 3, 1);
  row_num++;

  Gtk::MenuButton *add_genre = Gtk::make_managed<Gtk::MenuButton>();
  add_genre->set_margin(5);
  add_genre->set_halign(Gtk::Align::CENTER);
  add_genre->set_hexpand(true);
  add_genre->set_name("menBut");
  add_genre->set_label(gettext("Add genre"));
  setGenrePopover(add_genre);
  genre_action_grid->attach(*add_genre, 0, 0, 1, 1);

  Gtk::Button *remove_genre = Gtk::make_managed<Gtk::Button>();
  remove_genre->set_margin(5);
  remove_genre->set_halign(Gtk::Align::CENTER);
  remove_genre->set_hexpand(true);
  remove_genre->set_name("removeBut");
  remove_genre->set_label(gettext("Remove genre"));
  remove_genre->signal_clicked().connect(
      std::bind(&EditBookGui::remove_genre, this));
  genre_action_grid->attach(*remove_genre, 1, 0, 1, 1);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_text(gettext("Date:"));
  grid->attach(*lab, 0, row_num, 3, 1);
  row_num++;

  date_ent = Gtk::make_managed<Gtk::Entry>();
  date_ent->set_margin(5);
  date_ent->set_halign(Gtk::Align::FILL);
  date_ent->set_hexpand(true);
  date_ent->set_width_chars(70);
  date_ent->set_text(bbe.bpe.book_date);
  grid->attach(*date_ent, 0, row_num, 3, 1);
  row_num++;

  Gtk::Button *apply = Gtk::make_managed<Gtk::Button>();
  apply->set_margin(5);
  apply->set_halign(Gtk::Align::CENTER);
  apply->set_name("applyBut");
  apply->set_label(gettext("Apply"));
  apply->signal_clicked().connect(
      std::bind(&EditBookGui::confirmationDialog, this, window));
  grid->attach(*apply, 0, row_num, 1, 1);

  Gtk::Button *restore = Gtk::make_managed<Gtk::Button>();
  restore->set_margin(5);
  restore->set_halign(Gtk::Align::CENTER);
  restore->set_name("operationBut");
  restore->set_label(gettext("Restore"));
  restore->signal_clicked().connect([this]
  {
    this->refresh_thr = std::make_shared<std::thread>([this]
    {
      this->restore_disp->emit();
    });
    this->refresh_thr->detach();
  });
  grid->attach(*restore, 1, row_num, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*cancel, 2, row_num, 1, 1);
  row_num++;

  auto signal = grid->signal_realize().connect([grid, genre_scrl, row_num]
  {
    Gtk::Requisition min, nat;
    grid->get_preferred_size(min, nat);

    int height = nat.get_height() / row_num * 5;
    genre_scrl->set_min_content_height(height);
  });
}

void
EditBookGui::show_genre_menu(
    int numclck, double x, double y, Gtk::PopoverMenu *menu,
    const Glib::RefPtr<Gtk::SingleSelection> &selection)
{
  Glib::RefPtr<EditBookGenreModelItem> item = std::dynamic_pointer_cast<
      EditBookGenreModelItem>(selection->get_selected_item());
  if(item)
    {
      Glib::RefPtr<EditBookGenreModelItem> prev = selected_genre;
      selected_genre = item;
      for(guint i = 0; i < genre_model->get_n_items(); i++)
	{
	  auto it = genre_model->get_item(i);
	  if(it == prev || it == selected_genre)
	    {
	      genre_model->insert(i, it);
	      genre_model->remove(i);
	    }
	}
      Gdk::Rectangle rec(static_cast<int>(x), static_cast<int>(y), 1, 1);
      menu->set_pointing_to(rec);
      menu->popup();
    }
}

void
EditBookGui::confirmationDialog(Gtk::Window *win)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(win->get_application());
  window->set_title(gettext("Confirmation"));
  window->set_transient_for(*win);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  lab->set_text(gettext("Are you sure?"));
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_margin(5);
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_name("applyBut");
  yes->set_label(gettext("Yes"));
  yes->signal_clicked().connect([window, this]
  {
    this->wait_window(window);
    this->edit_book(window);
  });
  grid->attach(*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_margin(5);
  no->set_halign(Gtk::Align::CENTER);
  no->set_name("cancelBut");
  no->set_label(gettext("No"));
  no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*no, 1, 1, 1, 1);

  window->signal_close_request().connect([window]
  {
    std::shared_ptr<Gtk::Window> win(window);
    win->set_visible(false);
    return true;
  },
					 false);

  window->present();
}

void
EditBookGui::wait_window(Gtk::Window *win)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(false);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(20);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_valign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_text(gettext("Wait..."));
  win->set_child(*lab);
}

void
EditBookGui::edit_book(Gtk::Window *win)
{
  BookBaseEntry bbe_new;

  bbe_new.file_path = bbe.file_path;

  bbe_new.bpe.book_author = std::string(author_ent->get_text());
  bbe_new.bpe.book_date = std::string(date_ent->get_text());
  bbe_new.bpe.book_name = std::string(book_ent->get_text());
  bbe_new.bpe.book_path = bbe.bpe.book_path;
  bbe_new.bpe.book_series = std::string(series_ent->get_text());

  for(guint i = 0; i < genre_model->get_n_items(); i++)
    {
      Glib::RefPtr<EditBookGenreModelItem> item = genre_model->get_item(i);
      if(bbe_new.bpe.book_genre.empty())
	{
	  if(!item->genre_code.empty())
	    {
	      bbe_new.bpe.book_genre = item->genre_code;
	    }
	}
      else
	{
	  if(!item->genre_code.empty())
	    {
	      bbe_new.bpe.book_genre = bbe_new.bpe.book_genre + ", "
		  + item->genre_code;
	    }
	}
    }
  std::atomic<bool> cancel;
  cancel.store(false);
  std::shared_ptr<RefreshCollection> rfr = std::make_shared<RefreshCollection>(
      af, collection_name, std::thread::hardware_concurrency(), &cancel, false,
      true, true, bookmarks);
  try
    {
      if(rfr->editBook(bbe, bbe_new))
	{
	  finish_dialog(win, 1);
	  if(successfully_edited_signal)
	    {
	      successfully_edited_signal(bbe, bbe_new, collection_name);
	    }
	}
      else
	{
	  finish_dialog(win, 2);
	}
    }
  catch(MLException &er)
    {
      std::cout << er.what() << std::endl;
      finish_dialog(win, 3);
    }
}

void
EditBookGui::finish_dialog(Gtk::Window *win, const int &variant)
{
  win->unset_child();
  win->set_default_size(1, 1);
  win->set_deletable(false);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  win->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  switch(variant)
    {
    case 1:
      {
	lab->set_text(gettext("Book entry was successfully edited!"));
	break;
      }
    case 2:
      {
	lab->set_text(
	    gettext("Error! Book entry was not found in collection."));
	break;
      }
    case 3:
      {
	lab->set_text(gettext("Critical error! See system log for details."));
	break;
      }
    default:
      break;
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  switch(variant)
    {
    case 1:
      {
	close->signal_clicked().connect([win]
	{
	  Gtk::Window *p = win->get_transient_for();
	  win->unset_transient_for();
	  p->close();
	  win->close();
	});
	break;
      }
    default:
      {
	close->signal_clicked().connect(std::bind(&Gtk::Window::close, win));
	break;
      }
    }
  grid->attach(*close, 0, 1, 1, 1);
}
