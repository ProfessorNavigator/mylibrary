/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <AuxWindows.h>

AuxWindows::AuxWindows(MainWindow *mw)
{
  this->mw = mw;
}

AuxWindows::~AuxWindows()
{

}

void
AuxWindows::errorWin(int type, Gtk::Window *par_win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::AlertDialog> info = Gtk::AlertDialog::create();
  info->set_modal(true);
  if(type == 0)
    {
      info->set_message(gettext("Collection name cannot be empty!"));
    }
  else if(type == 1)
    {
      info->set_message(gettext("Path to book directory is empty!"));
    }
  else if(type == 2)
    {
      info->set_message(gettext("Collection has been created"));
    }
  else if(type == 3)
    {
      info->set_message(gettext("Collection already existed!"));
    }
  else if(type == 4)
    {
      info->set_message(gettext("Collection creation canceled"));
    }
  else if(type == 5)
    {
      info->set_message(gettext("Collection to import path is empty!"));
    }
  else if(type == 6)
    {
      info->set_message(gettext("Export path is empty!"));
    }
  else if(type == 7)
    {
      info->set_message(
	  gettext(
	      "Book has been removed from collection database, "
	      "but book file was not found. Check if collection book directory "
	      "path exists and/or refresh collection."));
    }
  else if(type == 8)
    {
      info->set_message(gettext("Book-mark has been created!"));
    }
  else if(type == 9)
    {
      info->set_message(gettext("The book has been copied"));
    }
  else if(type == 10)
    {
      info->set_message(gettext("Error: book path is empty!"));
    }
  else if(type == 11)
    {
      info->set_message(gettext("Error: book name is empty!"));
    }
  else if(type == 12)
    {
      info->set_message(gettext("Error: source book file does not exists!"));
    }
  info->show(*par_win);
#endif
#ifdef ML_GTK_OLD
  Gtk::MessageDialog *info = new Gtk::MessageDialog(*par_win, "", false,
						    Gtk::MessageType::INFO,
						    Gtk::ButtonsType::CLOSE,
						    true);
  info->set_application(mw->get_application());

  if(type == 0)
    {
      info->set_message(gettext("Collection name cannot be empty!"), false);
    }
  else if(type == 1)
    {
      info->set_message(gettext("Path to book directory is empty!"), false);
    }
  else if(type == 2)
    {
      info->set_message(gettext("Collection has been created"), false);
    }
  else if(type == 3)
    {
      info->set_message(gettext("Collection already existed!"), false);
    }
  else if(type == 4)
    {
      info->set_message(gettext("Collection creation canceled"), false);
    }
  else if(type == 5)
    {
      info->set_message(gettext("Collection to import path is empty!"), false);
    }
  else if(type == 6)
    {
      info->set_message(gettext("Export path is empty!"), false);
    }
  else if(type == 7)
    {
      info->set_message(
	  gettext(
	      "Book has been removed from collection database, "
	      "but book file was not found. Check if collection book directory "
	      "path exists and/or refresh collection."),
	  false);
    }
  else if(type == 8)
    {
      info->set_message(gettext("Book-mark has been created!"), false);
    }
  else if(type == 9)
    {
      info->set_message(gettext("The book has been copied"), false);
    }
  else if(type == 10)
    {
      info->set_message(gettext("Error: book path is empty!"));
    }
  else if(type == 11)
    {
      info->set_message(gettext("Error: book name is empty!"));
    }
  else if(type == 12)
    {
      info->set_message(gettext("Error: source book file does not exists!"));
    }

  info->signal_response().connect([info]
  (int resp_id)
    {
      if(resp_id == Gtk::ResponseType::CLOSE)
	{
	  info->close();
	}
    });
  info->signal_close_request().connect([info]
  {
    info->hide();
    delete info;
    return true;
  },
				       false);
  info->present();
#endif
}

void
AuxWindows::bookmarkWindow()
{
#ifdef ML_GTK_OLD
  mw->bookmark_v.clear();
  mw->bookmark_v = formBmVector();
#endif
#ifndef ML_GTK_OLD
  std::vector<book_item> bookmark_v;
  bookmark_v = formBmVector();
#endif
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_name("MLwindow");
  window->set_title(gettext("Book-marks"));
  window->set_transient_for(*mw);
  window->set_modal(false);
  window->set_default_size(mw->get_width() * 0.75, mw->get_height());

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_halign(Gtk::Align::FILL);
  scrl->set_valign(Gtk::Align::FILL);
  scrl->set_expand(true);
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  grid->attach(*scrl, 0, 0, 1, 1);

#ifdef ML_GTK_OLD
  mw->bm_tv = Gtk::make_managed<Gtk::TreeView>();
  mw->bm_tv->set_name("searchRes");
  scrl->set_child(*mw->bm_tv);
  CreateRightGrid crgr(mw);
  crgr.searchResultShow(2, window);
#endif
  MainWindow *mwl = mw;
#ifndef ML_GTK_OLD
  Gtk::ColumnView *sres = Gtk::make_managed<Gtk::ColumnView>();
  mw->bm_col_view = sres;
  sres->set_halign(Gtk::Align::FILL);
  sres->set_valign(Gtk::Align::FILL);
  sres->set_expand(true);
  sres->set_single_click_activate(true);
  sres->set_show_column_separators(true);
  sres->set_show_row_separators(true);
  Glib::RefPtr<Gio::ListStore<ModelColumns>> list;
  std::shared_ptr<std::vector<style_item>> style_v = std::make_shared<
      std::vector<style_item>>();
  list = form_col_view(sres, bookmark_v, style_v, window);
  sres->signal_activate().connect([sres, mwl, style_v]
  (guint rownum)
    {
      if(mwl->bm_sel_book)
	{
	  *mwl->bm_sel_book = rownum;
	}
      else
	{
	  mwl->bm_sel_book = new guint(rownum);
	}
      auto sm = sres->get_model();
      auto ss = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
      if(ss)
	{
	  auto item = ss->get_selected_item();
	  for(size_t i = 0; i < style_v->size(); i++)
	    {
	      if(style_v->at(i).item == item)
		{
		  if(style_v->at(i).authors)
		    {
		      style_v->at(i).authors->set_name("selectedLab");
		    }
		  if(style_v->at(i).book)
		    {
		      style_v->at(i).book->set_name("selectedLab");
		    }
		  if(style_v->at(i).series)
		    {
		      style_v->at(i).series->set_name("selectedLab");
		    }
		  if(style_v->at(i).genre)
		    {
		      style_v->at(i).genre->set_name("selectedLab");
		    }
		  if(style_v->at(i).date)
		    {
		      style_v->at(i).date->set_name("selectedLab");
		    }
		}
	      else
		{
		  if(style_v->at(i).authors)
		    {
		      style_v->at(i).authors->set_name("unselectedLab");
		    }
		  if(style_v->at(i).book)
		    {
		      style_v->at(i).book->set_name("unselectedLab");
		    }
		  if(style_v->at(i).series)
		    {
		      style_v->at(i).series->set_name("unselectedLab");
		    }
		  if(style_v->at(i).genre)
		    {
		      style_v->at(i).genre->set_name("unselectedLab");
		    }
		  if(style_v->at(i).date)
		    {
		      style_v->at(i).date->set_name("unselectedLab");
		    }
		}
	    }
	}
    });
  scrl->set_child(*sres);
#endif

  Gtk::Popover *bm_pop = Gtk::make_managed<Gtk::Popover>();
#ifdef ML_GTK_OLD
  bm_pop->set_parent(*mw->bm_tv);
  mw->bm_tv->signal_unrealize().connect([bm_pop]
  {
    bm_pop->unparent();
  });
  Gtk::Grid *bm_pop_grid = formMenuGrid(window, bm_pop);
#endif
#ifndef ML_GTK_OLD
  bm_pop->set_parent(*sres);
  sres->signal_unrealize().connect([bm_pop]
  {
    bm_pop->unparent();
  });

  Gtk::Grid *bm_pop_grid = formMenuGrid(window, bm_pop, list, style_v);
#endif
  bm_pop->set_child(*bm_pop_grid);

#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect([bm_pop, sres, style_v, mwl]
  (int n_pressed, double x, double y)
    {
      auto sm = sres->get_model();
      auto ss = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
      if(ss)
	{
	  if(mwl->bm_sel_book)
	    {
	      *mwl->bm_sel_book = ss->get_selected();
	    }
	  else
	    {
	      mwl->bm_sel_book = new guint(ss->get_selected());
	    }
	  auto item = ss->get_selected_item();
	  for(size_t i = 0; i < style_v->size(); i++)
	    {
	      if(style_v->at(i).item == item)
		{
		  if(style_v->at(i).authors)
		    {
		      style_v->at(i).authors->set_name("selectedLab");
		    }
		  if(style_v->at(i).book)
		    {
		      style_v->at(i).book->set_name("selectedLab");
		    }
		  if(style_v->at(i).series)
		    {
		      style_v->at(i).series->set_name("selectedLab");
		    }
		  if(style_v->at(i).genre)
		    {
		      style_v->at(i).genre->set_name("selectedLab");
		    }
		  if(style_v->at(i).date)
		    {
		      style_v->at(i).date->set_name("selectedLab");
		    }
		}
	      else
		{
		  if(style_v->at(i).authors)
		    {
		      style_v->at(i).authors->set_name("unselectedLab");
		    }
		  if(style_v->at(i).book)
		    {
		      style_v->at(i).book->set_name("unselectedLab");
		    }
		  if(style_v->at(i).series)
		    {
		      style_v->at(i).series->set_name("unselectedLab");
		    }
		  if(style_v->at(i).genre)
		    {
		      style_v->at(i).genre->set_name("unselectedLab");
		    }
		  if(style_v->at(i).date)
		    {
		      style_v->at(i).date->set_name("unselectedLab");
		    }
		}
	    }
	}
      Gdk::Rectangle rect(x, y, 1, 1);
      bm_pop->set_pointing_to(rect);
      bm_pop->popup();
    });
  sres->add_controller(clck);
#endif
#ifdef ML_GTK_OLD
  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect([bm_pop]
  (int n_pressed, double x, double y)
    {
      Gdk::Rectangle rect(x, y, 1, 1);
      bm_pop->set_pointing_to(rect);
      bm_pop->popup();
    });
  mw->bm_tv->add_controller(clck);
#endif

  Gtk::MenuButton *book_op = Gtk::make_managed<Gtk::MenuButton>();
  book_op->set_margin(5);
  book_op->set_halign(Gtk::Align::CENTER);
  book_op->set_label(gettext("Book operations list"));
  book_op->set_name("menBut");
  grid->attach(*book_op, 0, 1, 1, 1);

  Gtk::Popover *men_but_pop = Gtk::make_managed<Gtk::Popover>();
  book_op->set_popover(*men_but_pop);
#ifdef ML_GTK_OLD
  Gtk::Grid *men_pop_grid = formMenuGrid(window, men_but_pop);
#endif
#ifndef ML_GTK_OLD
  Gtk::Grid *men_pop_grid = formMenuGrid(window, men_but_pop, list, style_v);
#endif
  men_but_pop->set_child(*men_pop_grid);
#ifdef ML_GTK_OLD
  window->signal_close_request().connect([mwl, window]
  {
    mwl->bookmark_v.clear();
    mwl->bm_tv = nullptr;
    window->hide();
    delete window;
    return true;
  },
					 false);
#endif
#ifndef ML_GTK_OLD
  window->signal_close_request().connect([window, mwl, style_v]
  {
    delete mwl->bm_sel_book;
    mwl->bm_sel_book = nullptr;
    mwl->bm_col_view = nullptr;
    window->set_visible(false);
    delete window;
    return true;
  },
					 false);
#endif
  window->present();
}

#ifdef ML_GTK_OLD
Gtk::Grid*
AuxWindows::formMenuGrid(Gtk::Window *window, Gtk::Popover *bm_pop)
#endif
#ifndef ML_GTK_OLD
Gtk::Grid*
AuxWindows::formMenuGrid(Gtk::Window *window, Gtk::Popover *bm_pop,
			 Glib::RefPtr<Gio::ListStore<ModelColumns>> list,
			 std::shared_ptr<std::vector<style_item>> style_v)
#endif
{
  MainWindow *mwl = mw;

  Gtk::Grid *bm_pop_grid = Gtk::make_managed<Gtk::Grid>();
  bm_pop_grid->set_halign(Gtk::Align::FILL);
  bm_pop_grid->set_valign(Gtk::Align::FILL);
  bm_pop_grid->set_expand(true);

  Gtk::Label *o_book_men = Gtk::make_managed<Gtk::Label>();
  o_book_men->set_name("menulab");
  o_book_men->set_margin(3);
  o_book_men->set_halign(Gtk::Align::START);
  o_book_men->set_valign(Gtk::Align::CENTER);
  o_book_men->set_text(gettext("Open selected book"));
  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(1);
#ifndef ML_GTK_OLD
  clck->signal_pressed().connect(
      [mwl, bm_pop]
      (int num_pressed,
       double x,
       double y)
	 {
	   auto sm = mwl->bm_col_view->get_model();
	   auto ss = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
	   if(ss)
	     {
	       if(mwl->bm_sel_book)
		 {
		   auto lm = ss->get_model();
		   if(lm)
		     {
		       if(lm->get_n_items() > *(mwl->bm_sel_book))
			 {
			   auto obj = lm->get_object(*(mwl->bm_sel_book));
			   if(obj)
			     {
			       auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
			       if(mod_col)
				 {
				   BookOpWindows bopw(mwl);
				   bopw.openBookFunc(mod_col->path);
				 }
			     }
			 }
		     }
		 }
	     }
	   bm_pop->popdown();
	 });
#endif
#ifdef ML_GTK_OLD
  clck->signal_pressed().connect([mwl, bm_pop]
  (int num_pressed, double x, double y)
    {
      BookOpWindows bopw(mwl);
      bopw.openBook(2);
      bm_pop->popdown();
    });
#endif
  o_book_men->add_controller(clck);
  bm_pop_grid->attach(*o_book_men, 0, 0, 1, 1);

  Gtk::Label *file_info = Gtk::make_managed<Gtk::Label>();
  file_info->set_name("menulab");
  file_info->set_margin(3);
  file_info->set_halign(Gtk::Align::START);
  file_info->set_valign(Gtk::Align::CENTER);
  file_info->set_text(gettext("File info"));
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, bm_pop]
  (int n_press, double x, double y)
    {
      BookOpWindows bopw(mwl);
      bopw.fileInfo(2);
      bm_pop->popdown();
    });
  file_info->add_controller(clck);
  bm_pop_grid->attach(*file_info, 0, 1, 1, 1);

  Gtk::Label *copy_book_men = Gtk::make_managed<Gtk::Label>();
  copy_book_men->set_name("menulab");
  copy_book_men->set_margin(3);
  copy_book_men->set_halign(Gtk::Align::START);
  copy_book_men->set_valign(Gtk::Align::CENTER);
  copy_book_men->set_text(gettext("Save book as..."));
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
#ifdef ML_GTK_OLD
  clck->signal_pressed().connect([mwl, window, bm_pop]
  (int num_pressed, double x, double y)
    {
      bm_pop->popdown();
      BookOpWindows bopw(mwl);
      bopw.copyTo(mwl->bm_tv, 2, window);
    });
#endif
#ifndef ML_GTK_OLD
  clck->signal_pressed().connect(
      [mwl, window, bm_pop]
      (int num_pressed,
       double x,
       double y)
	 {
	   bm_pop->popdown();
	   auto sm = mwl->bm_col_view->get_model();
	   auto ss = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
	   if(ss)
	     {
	       if(mwl->bm_sel_book)
		 {
		   auto lm = ss->get_model();
		   if(lm)
		     {
		       if(lm->get_n_items() > *(mwl->bm_sel_book))
			 {
			   auto obj = lm->get_object(*(mwl->bm_sel_book));
			   if(obj)
			     {
			       auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
			       if(mod_col)
				 {
				   BookOpWindows bopw(mwl);
				   bopw.copyToFunc(mod_col->path, mod_col->author, mod_col->book, window);
				 }
			     }
			 }
		     }
		 }
	     }
	 });
#endif
  copy_book_men->add_controller(clck);
  bm_pop_grid->attach(*copy_book_men, 0, 2, 1, 1);

  Gtk::Label *del_book_men = Gtk::make_managed<Gtk::Label>();
  del_book_men->set_name("menulab");
  del_book_men->set_margin(3);
  del_book_men->set_halign(Gtk::Align::START);
  del_book_men->set_valign(Gtk::Align::CENTER);
  del_book_men->set_text(gettext("Remove selected book from book-marks"));
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
#ifdef ML_GTK_OLD
  clck->signal_pressed().connect([mwl, window, bm_pop]
  (int num_pressed, double x, double y)
    {
      bm_pop->popdown();
      BookOpWindows bopw(mwl);
      bopw.bookRemoveWin(2, window, mwl->bm_tv);
    });
#endif
#ifndef ML_GTK_OLD
  clck->signal_pressed().connect(
      [mwl, window, bm_pop, list, style_v]
      (int num_pressed,
       double x,
       double y)
	 {
	   bm_pop->popdown();
	   auto sm = mwl->bm_col_view->get_model();
	   if(sm)
	     {
	       auto ss = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
	       if(ss)
		 {
		   if(mwl->bm_sel_book)
		     {
		       auto lm = ss->get_model();
		       if(lm)
			 {
			   if(lm->get_n_items() > *(mwl->bm_sel_book))
			     {
			       auto obj = lm->get_object(*(mwl->bm_sel_book));
			       if(obj)
				 {
				   auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
				   if(mod_col)
				     {
				       AuxWindows aw(mwl);
				       aw.removeBMDialog(window, list, mod_col, style_v);
				     }
				 }
			     }
			 }
		     }
		 }
	     }
	 });
#endif
  del_book_men->add_controller(clck);
  bm_pop_grid->attach(*del_book_men, 0, 3, 1, 1);

  return bm_pop_grid;
}

#ifndef ML_GTK_OLD
Glib::RefPtr<Gio::ListStore<ModelColumns>>
AuxWindows::form_col_view(Gtk::ColumnView *sres,
			  std::vector<book_item> &bookmark_v,
			  std::shared_ptr<std::vector<style_item>> style_v,
			  Gtk::Window *win)
{
  Glib::RefPtr<Gio::ListStore<ModelColumns>> result;
  if(sres)
    {
      result = Gio::ListStore<ModelColumns>::create();
      for(size_t i = 0; i < bookmark_v.size(); i++)
	{
	  Glib::RefPtr<ModelColumns> mod_col = ModelColumns::create(
	      (bookmark_v[i]).authors, (bookmark_v[i]).book,
	      (bookmark_v[i]).series, (bookmark_v[i]).genre,
	      (bookmark_v[i]).date, (bookmark_v[i]).path_to_book);
	  style_item item;
	  item.item = mod_col;
	  style_v->push_back(item);
	  result->append(mod_col);
	}
      Glib::RefPtr<Gtk::SingleSelection> ss_mod = Gtk::SingleSelection::create(
	  result);
      sres->set_model(ss_mod);
      int win_width = 0;
      int win_height = 0;
      win->get_default_size(win_width, win_height);

      auto author_expr = Gtk::ClosureExpression<Glib::ustring>::create([]
      (const Glib::RefPtr<Glib::ObjectBase> &item)
	{
	  const auto col = std::dynamic_pointer_cast<ModelColumns>(item);
	  if(col)
	    {
	      return col->author;
	    }
	  else
	    {
	      return Glib::ustring("");
	    }
	});
      auto author_sort = Gtk::StringSorter::create(author_expr);
      auto sort_model = Gtk::SortListModel::create(result, author_sort);

      Glib::RefPtr<Gtk::SignalListItemFactory> factory =
	  Gtk::SignalListItemFactory::create();
      factory->signal_setup().connect([]
      (const Glib::RefPtr<Gtk::ListItem> &item)
	{
	  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	  lab->set_halign(Gtk::Align::FILL);
	  lab->set_name("unselectedLab");
	  lab->set_wrap(true);
	  lab->set_wrap_mode(Pango::WrapMode::WORD);
	  lab->set_justify(Gtk::Justification::CENTER);
	  item->set_child(*lab);
	});
      factory->signal_bind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->author);
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).authors = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).authors = nullptr;
		    }
		}
	    }
	  );
      Glib::RefPtr<Gtk::ColumnViewColumn> col_author =
	  Gtk::ColumnViewColumn::create(gettext("Author"), factory);
      col_author->set_fixed_width(win_width * 0.25);
      col_author->set_resizable(true);
      col_author->set_expand(true);

      factory = Gtk::SignalListItemFactory::create();
      factory->signal_setup().connect([]
      (const Glib::RefPtr<Gtk::ListItem> &item)
	{
	  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	  lab->set_halign(Gtk::Align::FILL);
	  lab->set_name("unselectedLab");
	  lab->set_wrap(true);
	  lab->set_wrap_mode(Pango::WrapMode::WORD);
	  lab->set_justify(Gtk::Justification::CENTER);
	  item->set_child(*lab);
	});
      factory->signal_bind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->book);
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).book = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).book = nullptr;
		    }
		}
	    }
	  );
      Glib::RefPtr<Gtk::ColumnViewColumn> col_book =
	  Gtk::ColumnViewColumn::create(gettext("Book"), factory);
      col_book->set_fixed_width(win_width * 0.25);
      col_book->set_resizable(true);
      col_book->set_expand(true);

      auto book_expr = Gtk::ClosureExpression<Glib::ustring>::create([]
      (const Glib::RefPtr<Glib::ObjectBase> &item)
	{
	  const auto col = std::dynamic_pointer_cast<ModelColumns>(item);
	  if(col)
	    {
	      return col->book;
	    }
	  else
	    {
	      return Glib::ustring("");
	    }
	});
      auto book_sort = Gtk::StringSorter::create(book_expr);

      factory = Gtk::SignalListItemFactory::create();
      factory->signal_setup().connect([]
      (const Glib::RefPtr<Gtk::ListItem> &item)
	{
	  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	  lab->set_halign(Gtk::Align::FILL);
	  lab->set_name("unselectedLab");
	  lab->set_wrap(true);
	  lab->set_wrap_mode(Pango::WrapMode::WORD);
	  lab->set_justify(Gtk::Justification::CENTER);
	  item->set_child(*lab);
	});
      factory->signal_bind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->series);
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).series = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).series = nullptr;
		    }
		}
	    }
	  );
      Glib::RefPtr<Gtk::ColumnViewColumn> col_series =
	  Gtk::ColumnViewColumn::create(gettext("Series"), factory);
      col_series->set_fixed_width(win_width * 0.2);
      col_series->set_resizable(true);
      col_series->set_expand(true);

      auto series_expr = Gtk::ClosureExpression<Glib::ustring>::create([]
      (const Glib::RefPtr<Glib::ObjectBase> &item)
	{
	  const auto col = std::dynamic_pointer_cast<ModelColumns>(item);
	  if(col)
	    {
	      return col->series;
	    }
	  else
	    {
	      return Glib::ustring("");
	    }
	});
      auto series_sort = Gtk::StringSorter::create(series_expr);

      factory = Gtk::SignalListItemFactory::create();
      factory->signal_setup().connect([]
      (const Glib::RefPtr<Gtk::ListItem> &item)
	{
	  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	  lab->set_halign(Gtk::Align::FILL);
	  lab->set_name("unselectedLab");
	  lab->set_wrap(true);
	  lab->set_wrap_mode(Pango::WrapMode::WORD);
	  lab->set_justify(Gtk::Justification::CENTER);
	  item->set_child(*lab);
	});
      MainWindow *mwl = mw;
      factory->signal_bind().connect(
	  [mwl, style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->genre);
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).genre = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).genre = nullptr;
		    }
		}
	    }
	  );
      Glib::RefPtr<Gtk::ColumnViewColumn> col_genre =
	  Gtk::ColumnViewColumn::create(gettext("Genre"), factory);
      col_genre->set_fixed_width(win_width * 0.2);
      col_genre->set_resizable(true);
      col_genre->set_expand(true);

      auto genre_expr = Gtk::ClosureExpression<Glib::ustring>::create([]
      (const Glib::RefPtr<Glib::ObjectBase> &item)
	{
	  const auto col = std::dynamic_pointer_cast<ModelColumns>(item);
	  if(col)
	    {
	      return col->genre;
	    }
	  else
	    {
	      return Glib::ustring("");
	    }
	});
      auto genre_sort = Gtk::StringSorter::create(genre_expr);

      factory = Gtk::SignalListItemFactory::create();
      factory->signal_setup().connect([]
      (const Glib::RefPtr<Gtk::ListItem> &item)
	{
	  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	  lab->set_halign(Gtk::Align::FILL);
	  lab->set_name("unselectedLab");
	  lab->set_justify(Gtk::Justification::CENTER);
	  item->set_child(*lab);
	});
      factory->signal_bind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->date);
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).date = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [style_v]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(style_v->begin(), style_v->end(), [col](auto &el)
			{
			  return el.item == col;
			});
		  if(it != style_v->end())
		    {
		      (*it).date = nullptr;
		    }
		}
	    }
	  );
      Glib::RefPtr<Gtk::ColumnViewColumn> col_date =
	  Gtk::ColumnViewColumn::create(gettext("Date"), factory);
      col_date->set_fixed_width(
	  win_width - col_author->get_fixed_width()
	      - col_book->get_fixed_width() - col_series->get_fixed_width()
	      - col_genre->get_fixed_width());
      col_date->set_resizable(true);
      col_date->set_expand(true);

      auto date_expr = Gtk::ClosureExpression<Glib::ustring>::create([]
      (const Glib::RefPtr<Glib::ObjectBase> &item)
	{
	  const auto col = std::dynamic_pointer_cast<ModelColumns>(item);
	  if(col)
	    {
	      return col->date;
	    }
	  else
	    {
	      return Glib::ustring("");
	    }
	});
      auto date_sort = Gtk::StringSorter::create(date_expr);

      sres->append_column(col_author);
      sres->append_column(col_book);
      sres->append_column(col_series);
      sres->append_column(col_genre);
      sres->append_column(col_date);

      sort_model->set_sorter(sres->get_sorter());
      ss_mod->set_model(sort_model);
      col_author->set_sorter(author_sort);
      col_book->set_sorter(book_sort);
      col_series->set_sorter(series_sort);
      col_genre->set_sorter(genre_sort);
      col_date->set_sorter(date_sort);
    }
  return result;
}

void
AuxWindows::removeBMDialog(Gtk::Window *par_win,
			   Glib::RefPtr<Gio::ListStore<ModelColumns>> list,
			   Glib::RefPtr<Glib::ObjectBase> rem_item,
			   std::shared_ptr<std::vector<style_item>> style_v)
{
  Glib::RefPtr<Gtk::AlertDialog> msg = Gtk::AlertDialog::create();
  msg->set_modal(true);
  msg->set_message(
      gettext("This action will remove book from bookmarks. Continue?"));
  std::vector<Glib::ustring> buttons;
  buttons.push_back(gettext("No"));
  buttons.push_back(gettext("Yes"));
  msg->set_buttons(buttons);
  msg->set_cancel_button(0);
  msg->set_default_button(0);
  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  MainWindow *mwl = mw;
  msg->choose(
      *par_win,
      [list, rem_item, mwl, style_v]
      (Glib::RefPtr<Gio::AsyncResult> &result)
	{
	  int resp;
	  auto obj = result->get_source_object_base();
	  auto msg = std::dynamic_pointer_cast<Gtk::AlertDialog>(obj);
	  if(msg)
	    {
	      resp = msg->choose_finish(result);
	    }
	  else
	    {
	      return void();
	    }
	  if(resp == 1)
	    {
	      style_v->erase(std::remove_if(style_v->begin(), style_v->end(), [rem_item]
		      (auto &el)
			{
			  return el.item == rem_item;
			}),
		  style_v->end());
	      AuxWindows aw(mwl);
	      aw.removeBMFunc(list, rem_item);
	    }
	},
      cncl);
}

void
AuxWindows::removeBMFunc(Glib::RefPtr<Gio::ListStore<ModelColumns>> list,
			 Glib::RefPtr<Glib::ObjectBase> rem_item)
{
  Glib::RefPtr<ModelColumns> mod_col = std::dynamic_pointer_cast<ModelColumns>(
      rem_item);
  if(mod_col)
    {
      std::string sstr(mod_col->path);
      std::vector<book_item> bookmark_v;
      bookmark_v = formBmVector();
      bookmark_v.erase(
	  std::remove_if(bookmark_v.begin(), bookmark_v.end(), [sstr]
	  (auto &el)
	    {
	      return el.path_to_book == sstr;
	    }),
	  bookmark_v.end());
      AuxFunc af;
      std::string filename;
      af.homePath(&filename);
      filename = filename + "/.MyLibrary/Bookmarks";
      std::filesystem::path filepath = std::filesystem::u8path(filename);
      std::fstream f;
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
	{
	  for(size_t i = 0; i < bookmark_v.size(); i++)
	    {
	      std::string item = bookmark_v[i].authors;
	      item = item + "<?>";
	      item = item + bookmark_v[i].book;
	      item = item + "<?>";
	      item = item + bookmark_v[i].series;
	      item = item + "<?>";
	      item = item + bookmark_v[i].genre;
	      item = item + "<?>";
	      item = item + bookmark_v[i].date;
	      item = item + "<?>";
	      item = item + bookmark_v[i].path_to_book;
	      item = item + "<?L>";
	      f.write(item.c_str(), item.size());
	    }
	  f.close();
	}
      auto sp = list->find(mod_col);
      if(std::get<0>(sp))
	{
	  list->remove(std::get<1>(sp));
	  delete mw->bm_sel_book;
	  mw->bm_sel_book = nullptr;
	}
    }
}
#endif

std::vector<book_item>
AuxWindows::formBmVector()
{
  std::vector<book_item> bookmark_v;
  AuxFunc af;
  std::string filename;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Bookmarks";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      if(std::filesystem::file_size(filepath) > 0)
	{
	  file_str.resize(std::filesystem::file_size(filepath));
	  f.read(&file_str[0], file_str.size());
	}
      f.close();
      if(file_str.size() > 0)
	{
	  while(!file_str.empty())
	    {
	      std::string line = file_str.substr(
		  0, file_str.find("<?L>") + std::string("<?L>").size());
	      file_str.erase(0, line.size());
	      line = line.substr(0, line.find("<?L>"));
	      book_item bi;
	      std::string val = line.substr(
		  0, line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      bi.authors = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      bi.book = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      bi.series = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      bi.genre = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      bi.date = val;

	      val = line;
	      bi.path_to_book = val;
	      bookmark_v.push_back(bi);
	    }
	}
    }
  return bookmark_v;
}

void
AuxWindows::aboutProg()
{
  Gtk::AboutDialog *aboutd = new Gtk::AboutDialog;
  aboutd->set_application(mw->get_application());
  aboutd->set_transient_for(*mw);
  aboutd->set_name("MLwindow");

  aboutd->set_program_name("MyLibrary");
  aboutd->set_version("2.3.1");
  aboutd->set_copyright(
      "Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>");
  AuxFunc af;
  std::filesystem::path p = std::filesystem::u8path(af.get_selfpath());
  std::string filename = p.parent_path().u8string()
      + "/../share/MyLibrary/COPYING";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  std::fstream f;
  Glib::ustring abbuf;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t sz = static_cast<size_t>(std::filesystem::file_size(filepath));
      std::vector<char> ab;
      ab.resize(sz);
      f.read(&ab[0], ab.size());
      f.close();
      abbuf = Glib::ustring(ab.begin(), ab.end());
    }
  else
    {
      std::cerr << "License file not found" << std::endl;
    }

  if(abbuf.size() == 0)
    {
      aboutd->set_license_type(Gtk::License::GPL_3_0_ONLY);
    }
  else
    {
      aboutd->set_license(abbuf);
    }

  filename = p.parent_path().u8string() + "/../share/MyLibrary/mylibrary.png";
  Glib::RefPtr<Gio::File> logofile = Gio::File::create_for_path(filename);
  aboutd->set_logo(Gdk::Texture::create_from_file(logofile));
  abbuf = Glib::ustring(gettext("MyLibrary is a home librarian"))
      + Glib::ustring("\n") + Glib::ustring(gettext("Author Yury Bobylev"))
      + Glib::ustring(" <bobilev_yury@mail.ru>.\n")
      + Glib::ustring(gettext("Program uses next libraries:"))
      + Glib::ustring(
	  "\n"
	  "GTK https://www.gtk.org/\n"
	  "libzip https://libzip.org/\n"
	  "libarchive https://libarchive.org\n"
	  "libgcrypt https://www.gnupg.org/software/libgcrypt/\n"
	  "libgpg-error https://www.gnupg.org/software/libgpg-error/\n"
	  "ICU https://icu.unicode.org/\n"
	  "Poppler https://poppler.freedesktop.org/\n"
	  "DjVuLibre https://djvu.sourceforge.net/");
  aboutd->set_comments(abbuf);

  aboutd->signal_close_request().connect([aboutd]
  {
#ifdef ML_GTK_OLD
					   aboutd->hide();
#endif
#ifndef ML_GTK_OLD
					   aboutd->set_visible(false);
#endif
					   delete aboutd;
					   return true;
					 },
					 false);
  aboutd->present();
}

void
AuxWindows::bookCopyConfirm(Gtk::Window *win, std::mutex *addbmtx, int *stopper)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::AlertDialog> msg = Gtk::AlertDialog::create();
  msg->set_modal(true);
  msg->set_message(gettext("Book file exits. Replace?"));
  std::vector<Glib::ustring> labels;
  labels.push_back(gettext("No"));
  labels.push_back(gettext("Yes"));
  msg->set_buttons(labels);
  msg->set_cancel_button(0);
  msg->set_default_button(0);
  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  msg->choose(*win, [stopper, addbmtx]
  (Glib::RefPtr<Gio::AsyncResult> &result)
    {
      int resp;
      auto obj = result->get_source_object_base();
      auto msg = std::dynamic_pointer_cast<Gtk::AlertDialog>(obj);
      if(msg)
	{
	  resp = msg->choose_finish(result);
	}
      else
	{
	  return void();
	}
      if(resp == 1)
	{
	  *stopper = 0;
	  addbmtx->unlock();
	}
      else if(resp == 0)
	{
	  *stopper = 1;
	  addbmtx->unlock();
	}
    },
	      cncl);
#endif
#ifdef ML_GTK_OLD
  Gtk::MessageDialog *msg = new Gtk::MessageDialog(
      *win, gettext("Book file exits. Replace?"), false,
      Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO, true);
  msg->set_application(mw->get_application());

  msg->signal_response().connect([msg, addbmtx, stopper]
  (int resp)
    {
      if(resp == Gtk::ResponseType::YES)
	{
	  *stopper = 0;
	  addbmtx->unlock();
	  msg->close();
	}
      else if(resp == Gtk::ResponseType::NO)
	{
	  *stopper = 1;
	  addbmtx->unlock();
	  msg->close();
	}
    });

  msg->signal_close_request().connect([msg]
  {
    msg->hide();
    delete msg;
    return true;
  },
				      false);
  msg->present();
#endif
}

void
AuxWindows::bookcoverWindow()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Book cover"));
  window->set_transient_for(*mw);

  Glib::RefPtr<Gdk::Pixbuf> pix_buf;
  if(mw->cover_struct.image.size() > 0)
    {
      if(mw->cover_struct.format == "base64")
	{
	  std::string base64(mw->cover_struct.image.begin(),
			     mw->cover_struct.image.end());
	  base64 = Glib::Base64::decode(base64);
	  Glib::RefPtr<Glib::Bytes> bytes = Glib::Bytes::create(base64.c_str(),
								base64.size());

	  Glib::RefPtr<Gio::MemoryInputStream> strm =
	      Gio::MemoryInputStream::create();
	  strm->add_bytes(bytes);
	  pix_buf = Gdk::Pixbuf::create_from_stream(strm);
	}
      else if(mw->cover_struct.format == "rgba"
	  || mw->cover_struct.format == "rgb")
	{
	  guint8 *data =
	      reinterpret_cast<guint8*>(mw->cover_struct.image.data());
	  int ih = static_cast<int>(mw->cover_struct.image.size())
	      / mw->cover_struct.rowsize;
	  if(mw->cover_struct.format == "rgba")
	    {
	      int iw = mw->cover_struct.rowsize / 4;
	      pix_buf = Gdk::Pixbuf::create_from_data(data,
						      Gdk::Colorspace::RGB,
						      true, 8, iw, ih,
						      mw->cover_struct.rowsize);
	    }
	  else
	    {
	      int iw = mw->cover_struct.rowsize / 3;
	      pix_buf = Gdk::Pixbuf::create_from_data(data,
						      Gdk::Colorspace::RGB,
						      false, 8, iw, ih,
						      mw->cover_struct.rowsize);
	    }
	}
      else
	{
	  Glib::RefPtr<Glib::Bytes> bytes = Glib::Bytes::create(
	      mw->cover_struct.image.data(), mw->cover_struct.image.size());

	  Glib::RefPtr<Gio::MemoryInputStream> strm =
	      Gio::MemoryInputStream::create();
	  strm->add_bytes(bytes);

	  pix_buf = Gdk::Pixbuf::create_from_stream(strm);
	}
    }
  if(pix_buf)
    {
      Gdk::Rectangle rec = mw->screenRes();
      int w, h;
      w = pix_buf->get_width();
      h = pix_buf->get_height();
      if(h > rec.get_height())
	{
	  h = rec.get_height();
	  w = h * w / pix_buf->get_height();
	  pix_buf = pix_buf->scale_simple(w, h, Gdk::InterpType::BILINEAR);
	}
      if(w > rec.get_width())
	{
	  w = rec.get_width();
	  h = h * w / pix_buf->get_width();
	  pix_buf = pix_buf->scale_simple(w, h, Gdk::InterpType::BILINEAR);
	}
      window->set_default_size(w, h);
    }
  else
    {
      delete window;
      return void();
    }

  Gtk::DrawingArea *cover = Gtk::make_managed<Gtk::DrawingArea>();
  cover->set_halign(Gtk::Align::FILL);
  cover->set_valign(Gtk::Align::FILL);
  cover->set_expand(true);
  cover->set_draw_func(
      [pix_buf]
      (const Cairo::RefPtr<Cairo::Context> &cr,
       int width,
       int height)
	 {
	   auto image = pix_buf->scale_simple(width, height, Gdk::InterpType::BILINEAR);
	   Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	   cr->rectangle(0, 0, width, height);
	   cr->fill();
	 });
  window->set_child(*cover);

  window->signal_close_request().connect([window]
  {
#ifdef ML_GTK_OLD
					   window->hide();
#endif
#ifndef ML_GTK_OLD
					   window->set_visible(false);
#endif
					   delete window;
					   return true;
					 },
					 false);

  window->present();
}
