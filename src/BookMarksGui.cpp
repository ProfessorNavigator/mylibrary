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

#include <BookBaseEntry.h>
#include <BookInfoGui.h>
#include <BookMarksGui.h>
#include <CopyBookGui.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gdkmm/rectangle.h>
#include <gdkmm/surface.h>
#include <giomm/menuitem.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>
#include <glibmm/propertyproxy.h>
#include <glibmm/signalproxy.h>
#include <gtk/gtktypes.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/object.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/singleselection.h>
#include <gtkmm/sortlistmodel.h>
#include <libintl.h>
#include <MLException.h>
#include <sigc++/connection.h>
#include <stddef.h>
#include <SearchResultModelItem.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

BookMarksGui::BookMarksGui(const std::shared_ptr<AuxFunc> &af,
			   const std::shared_ptr<BookMarks> &bookmarks,
			   Gtk::Window *main_window)
{
  this->af = af;
  this->bookmarks = bookmarks;
  this->main_window = main_window;
  open_book = new OpenBook(af);
  loadWindowSizes();
}

BookMarksGui::~BookMarksGui()
{
  delete srs;
  delete open_book;
}

void
BookMarksGui::createWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Bookmarks"));
  window->set_transient_for(*main_window);
  window->set_modal(true);
  window->set_default_size(window_width, window_height);
  window->set_name("MLwindow");

  creat_bookmarks_action_group(window);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::ScrolledWindow *book_marks_scrl =
      Gtk::make_managed<Gtk::ScrolledWindow>();
  book_marks_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
			      Gtk::PolicyType::AUTOMATIC);
  book_marks_scrl->set_halign(Gtk::Align::FILL);
  book_marks_scrl->set_valign(Gtk::Align::FILL);
  book_marks_scrl->set_margin(5);
  book_marks_scrl->set_expand(true);
  grid->attach(*book_marks_scrl, 0, 0, 1, 1);

  book_marks = Gtk::make_managed<Gtk::ColumnView>();
  book_marks->set_halign(Gtk::Align::FILL);
  book_marks->set_valign(Gtk::Align::FILL);
  book_marks->set_reorderable(true);
  book_marks->set_single_click_activate(true);
  Glib::PropertyProxy<bool> row_sep =
      book_marks->property_show_row_separators();
  row_sep.set_value(true);
  Glib::PropertyProxy<bool> column_sep =
      book_marks->property_show_column_separators();
  column_sep.set_value(true);
  book_marks->signal_activate().connect(
      std::bind(&BookMarksGui::slot_row_activated, this,
		std::placeholders::_1));
  book_marks_scrl->set_child(*book_marks);

  srs = new SearchResultShow(af, book_marks);
  std::vector<BookBaseEntry> book_marks_v = bookmarks->getBookMarks();
  srs->searchResultShow(book_marks_v);

  Glib::RefPtr<Gio::Menu> menu = bookmark_menu();

  Gtk::PopoverMenu *pop_menu = Gtk::make_managed<Gtk::PopoverMenu>();
  pop_menu->set_menu_model(menu);
  pop_menu->set_parent(*book_marks);
  book_marks->signal_unrealize().connect([pop_menu]
  {
    pop_menu->unparent();
  });

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect(
      std::bind(&BookMarksGui::show_popup_menu, this, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3, pop_menu));
  book_marks->add_controller(clck);

  Gtk::MenuButton *book_ops = Gtk::make_managed<Gtk::MenuButton>();
  book_ops->set_margin(5);
  book_ops->set_halign(Gtk::Align::CENTER);
  book_ops->set_label(gettext("Bookmark operations"));
  book_ops->set_menu_model(menu);
  book_ops->set_name("menBut");
  grid->attach(*book_ops, 0, 1, 1, 1);

  window->signal_close_request().connect([window, this]
  {
    std::shared_ptr<Gtk::Window> win(window);
    saveWindowSizes(win.get());
    win->set_visible(false);
    delete this;
    return true;
  },
					 false);

  window->present();
}

void
BookMarksGui::loadWindowSizes()
{
  std::filesystem::path bmp = af->homePath();
  bmp /= std::filesystem::u8path(".cache/MyLibrary/bmwsizes");
  std::fstream f;
  f.open(bmp, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      bool remove = false;
      size_t fsz;
      f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);
      int32_t val;
      if(fsz >= 2 * sizeof(val))
	{
	  f.read(reinterpret_cast<char*>(&val), sizeof(val));
	  window_width = static_cast<int>(val);
	  f.read(reinterpret_cast<char*>(&val), sizeof(val));
	  window_height = static_cast<int>(val);
	}
      else
	{
	  remove = true;
	  setWindowSizesByMonitor();
	}
      f.close();

      if(remove)
	{
	  std::filesystem::remove_all(bmp);
	}
    }
  else
    {
      setWindowSizesByMonitor();
    }
}

void
BookMarksGui::setWindowSizesByMonitor()
{
  Glib::RefPtr<Gdk::Surface> surf = main_window->get_surface();
  Glib::RefPtr<Gdk::Display> disp = main_window->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);

  req.set_width(req.get_width() * mon->get_scale_factor());
  req.set_height(req.get_height() * mon->get_scale_factor());

  window_height = req.get_height() * 0.5;
  window_width = req.get_width() * 0.5;
}

void
BookMarksGui::saveWindowSizes(Gtk::Window *win)
{
  window_width = win->get_width();
  window_height = win->get_height();
  std::filesystem::path bmp = af->homePath();
  bmp /= std::filesystem::u8path(".cache/MyLibrary/bmwsizes");
  std::filesystem::create_directories(bmp.parent_path());
  std::fstream f;
  f.open(bmp, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      int32_t val = static_cast<int32_t>(window_width);
      f.write(reinterpret_cast<char*>(&val), sizeof(val));
      val = static_cast<int32_t>(window_height);
      f.write(reinterpret_cast<char*>(&val), sizeof(val));
      f.close();
    }
}

void
BookMarksGui::slot_row_activated(guint pos)
{
  Glib::RefPtr<Gtk::SingleSelection> model = std::dynamic_pointer_cast<
      Gtk::SingleSelection>(book_marks->get_model());
  if(model && pos != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::SortListModel> sort_model = std::dynamic_pointer_cast<
	  Gtk::SortListModel>(model->get_model());
      if(sort_model)
	{
	  Glib::RefPtr<SearchResultModelItem> item = std::dynamic_pointer_cast<
	      SearchResultModelItem>(sort_model->get_object(pos));
	  if(item)
	    {
	      srs->select_item(item);
	    }
	}
    }
}

void
BookMarksGui::creat_bookmarks_action_group(Gtk::Window *win)
{
  Glib::RefPtr<Gio::SimpleActionGroup> bookmark_actions =
      Gio::SimpleActionGroup::create();

  bookmark_actions->add_action(
      "open_book",
      [this]
      {
	auto item = this->srs->get_selected_item();
	if(item)
	  {
	    try
	      {
		std::filesystem::path tmp = af->temp_path();
		tmp /= std::filesystem::u8path("MyLibraryReading");
		this->open_book->open_book(
		    item->bbe,
		    false,
		    tmp, false,
		    std::bind(&AuxFunc::open_book_callback, af.get(),
			      std::placeholders::_1));
	      }
	    catch(MLException &er)
	      {
		std::cout << er.what() << std::endl;
	      }
	  }
      });

  bookmark_actions->add_action("book_info", [this, win]
  {
    auto item = this->srs->get_selected_item();
    if(item)
      {
	BookInfoGui *big = new BookInfoGui(this->af, win);
	big->creatWindow(item->bbe);
      }
  });

  bookmark_actions->add_action("copy_book", [this, win]
  {
    auto item = this->srs->get_selected_item();
    if(item)
      {
	CopyBookGui *cbg = new CopyBookGui(this->af, win, item->bbe);
	cbg->createWindow();
      }
  });

  bookmark_actions->add_action("remove_book", [this, win]
  {
    this->confirmationDialog(win);
  });

  win->insert_action_group("book_mark_ag", bookmark_actions);
}

void
BookMarksGui::confirmationDialog(Gtk::Window *win)
{
  if(srs->get_selected_item())
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
      lab->set_expand(true);
      lab->set_text(gettext("Are you sure?"));
      grid->attach(*lab, 0, 0, 2, 1);

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
      yes->set_margin(5);
      yes->set_halign(Gtk::Align::CENTER);
      yes->set_label(gettext("Yes"));
      yes->set_name("removeBut");
      yes->signal_clicked().connect([this, window]
      {
	auto item = this->srs->get_selected_item();
	if(item)
	  {
	    this->srs->removeItem(item);
	    this->bookmarks->removeBookMark(item->bbe);
	  }
	window->close();
      });
      grid->attach(*yes, 0, 1, 1, 1);

      Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
      no->set_margin(5);
      no->set_halign(Gtk::Align::CENTER);
      no->set_label(gettext("No"));
      no->set_name("cancelBut");
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
}

Glib::RefPtr<Gio::Menu>
BookMarksGui::bookmark_menu()
{
  Glib::RefPtr<Gio::Menu> result = Gio::Menu::create();

  Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create(
      gettext("Open book"), "book_mark_ag.open_book");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Book info"), "book_mark_ag.book_info");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Save book as..."),
			       "book_mark_ag.copy_book");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Remove bookmark"),
			       "book_mark_ag.remove_book");
  result->append_item(item);

  return result;
}

void
BookMarksGui::show_popup_menu(int num, double x, double y,
			      Gtk::PopoverMenu *pop_menu)
{
  Gdk::Rectangle rec(static_cast<int>(x), static_cast<int>(y), 1, 1);
  pop_menu->set_pointing_to(rec);
  pop_menu->popup();
  Glib::RefPtr<Gtk::SingleSelection> sing_sel = std::dynamic_pointer_cast<
      Gtk::SingleSelection>(book_marks->get_model());
  if(sing_sel)
    {
      Glib::RefPtr<SearchResultModelItem> item = std::dynamic_pointer_cast<
	  SearchResultModelItem>(sing_sel->get_selected_item());
      if(item)
	{
	  srs->select_item(item);
	}
    }
}
