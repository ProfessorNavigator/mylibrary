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

#include <CreateRightGrid.h>

CreateRightGrid::CreateRightGrid(MainWindow *mw)
{
  this->mw = mw;
}

CreateRightGrid::~CreateRightGrid()
{

}

Gtk::Grid*
CreateRightGrid::formRightGrid()
{
  Gtk::Grid *right_grid = Gtk::make_managed<Gtk::Grid>();
  right_grid->set_halign(Gtk::Align::FILL);
  right_grid->set_valign(Gtk::Align::FILL);
  right_grid->set_expand(true);

  Gtk::ScrolledWindow *search_res = Gtk::make_managed<Gtk::ScrolledWindow>();
  search_res->set_halign(Gtk::Align::FILL);
  search_res->set_valign(Gtk::Align::FILL);
  search_res->set_margin(5);
  search_res->set_policy(Gtk::PolicyType::AUTOMATIC,
			 Gtk::PolicyType::AUTOMATIC);
  search_res->set_expand(true);
  search_res->set_overlay_scrolling(true);
  right_grid->attach(*search_res, 0, 0, 6, 5);

#ifdef ML_GTK_OLD
  Gtk::TreeView *sres = Gtk::make_managed<Gtk::TreeView>();
  sres->set_halign(Gtk::Align::FILL);
  sres->set_valign(Gtk::Align::FILL);
  sres->set_expand(true);
  sres->set_headers_clickable(true);
  sres->set_activate_on_single_click(true);
  sres->set_name("searchRes");
  sres->signal_row_activated().connect(
      std::bind(&MainWindow::rowActivated, mw, std::placeholders::_1,
		std::placeholders::_2));
#endif
  MainWindow *mwl = mw;
#ifndef ML_GTK_OLD
  Gtk::ColumnView *sres = Gtk::make_managed<Gtk::ColumnView>();
  sres->set_halign(Gtk::Align::FILL);
  sres->set_valign(Gtk::Align::FILL);
  sres->set_expand(true);
  sres->set_single_click_activate(true);
  sres->set_show_column_separators(true);
  sres->set_show_row_separators(true);
  sres->signal_activate().connect([mwl, sres]
  (guint rownum)
    {
      CreateRightGrid crg(mwl);
      crg.rowActivated(rownum, sres);
    });
#endif
  search_res->set_child(*sres);

  Gtk::Popover *book_men = Gtk::make_managed<Gtk::Popover>();
  book_men->set_parent(*sres);
  book_men->set_name("popoverSt");

  Gtk::Grid *book_men_grid = formPopoverGrid(sres, book_men);
  book_men->set_child(*book_men_grid);

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect([book_men, mwl, sres]
  (int n_pressed, double x, double y)
    {
      Gdk::Rectangle rect(x, y, 1, 1);
      book_men->set_pointing_to(rect);
      book_men->popup();
#ifndef ML_GTK_OLD
   delete mwl->ms_sel_book;
   mwl->ms_sel_book = nullptr;
   auto ss = sres->get_model();
   if(ss)
     {
       auto selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(ss);
       if(selection)
	 {
	   CreateRightGrid crg(mwl);
	   crg.rowActivated(selection->get_selected(), sres);
	 }
     }
#endif
 });
  sres->add_controller(clck);

  sres->signal_unrealize().connect([book_men]
  {
    book_men->unparent();
  });

  Gtk::Popover *book_op_pop = Gtk::make_managed<Gtk::Popover>();
  book_op_pop->set_name("popoverSt");

  Gtk::Grid *book_op_pop_gr = formPopoverGrid(sres, book_op_pop);
  book_op_pop_gr->set_halign(Gtk::Align::CENTER);
  book_op_pop_gr->set_valign(Gtk::Align::CENTER);
  book_op_pop->set_child(*book_op_pop_gr);

  Gtk::MenuButton *book_op_but = Gtk::make_managed<Gtk::MenuButton>();
  book_op_but->set_name("menBut");
  book_op_but->set_popover(*book_op_pop);
  book_op_but->set_margin(5);
  book_op_but->set_halign(Gtk::Align::START);
  book_op_but->set_valign(Gtk::Align::CENTER);
  book_op_but->set_label(gettext("Book operations list"));
  right_grid->attach(*book_op_but, 0, 6, 1, 1);

  Gtk::ScrolledWindow *annot_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  annot_scrl->set_margin(5);
  annot_scrl->set_halign(Gtk::Align::FILL);
  annot_scrl->set_valign(Gtk::Align::FILL);
  annot_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
			 Gtk::PolicyType::AUTOMATIC);
  annot_scrl->set_expand(true);
  right_grid->attach(*annot_scrl, 0, 7, 5, 1);

  Gtk::TextView *annot = Gtk::make_managed<Gtk::TextView>();
  annot->set_editable(false);
  annot->set_wrap_mode(Gtk::WrapMode::WORD);
  annot_scrl->set_child(*annot);

  Glib::RefPtr<Gtk::GestureClick> clck_cover = Gtk::GestureClick::create();
  clck_cover->set_button(0);
  clck_cover->signal_pressed().connect([mwl]
  (int n_press, double x, double y)
    {
      AuxWindows aux(mwl);
      aux.bookcoverWindow();
    });

  Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea>();
  drar->set_halign(Gtk::Align::FILL);
  drar->set_valign(Gtk::Align::FILL);
  drar->set_margin(5);
  drar->set_draw_func(
      std::bind(&MainWindow::drawCover, mw, std::placeholders::_1,
		std::placeholders::_2, std::placeholders::_3));
  drar->set_expand(true);
  drar->add_controller(clck_cover);
  right_grid->attach(*drar, 5, 7, 1, 1);

  return right_grid;
}

#ifdef ML_GTK_OLD
Gtk::Grid*
CreateRightGrid::formPopoverGrid(Gtk::TreeView *sres,
				 Gtk::Popover *book_popover)
#endif
#ifndef ML_GTK_OLD
Gtk::Grid*
CreateRightGrid::formPopoverGrid(Gtk::ColumnView *sres,
				 Gtk::Popover *book_popover)
#endif
{
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);

  MainWindow *mwl = mw;

  Gtk::Label *openbook = Gtk::make_managed<Gtk::Label>();
  openbook->set_name("menulab");
  openbook->set_margin(3);
  openbook->set_halign(Gtk::Align::START);
  openbook->set_valign(Gtk::Align::CENTER);
  openbook->set_text(gettext("Open selected book"));
  openbook->set_selectable(false);
  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
      BookOpWindows bopw(mwl);
#ifndef ML_GTK_OLD
   bopw.openBook();
#endif
#ifdef ML_GTK_OLD
   bopw.openBook(1);
#endif
 });
  openbook->add_controller(clck);
  grid->attach(*openbook, 0, 0, 1, 1);

  Gtk::Label *fileinfo = Gtk::make_managed<Gtk::Label>();
  fileinfo->set_name("menulab");
  fileinfo->set_margin(3);
  fileinfo->set_halign(Gtk::Align::START);
  fileinfo->set_valign(Gtk::Align::CENTER);
  fileinfo->set_text(gettext("File info"));
  fileinfo->set_selectable(false);
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
      BookOpWindows bopw(mwl);
      bopw.fileInfo(1);
    });
  fileinfo->add_controller(clck);
  grid->attach(*fileinfo, 0, 1, 1, 1);

  Gtk::Label *bookmark = Gtk::make_managed<Gtk::Label>();
  bookmark->set_name("menulab");
  bookmark->set_margin(3);
  bookmark->set_halign(Gtk::Align::START);
  bookmark->set_valign(Gtk::Align::CENTER);
  bookmark->set_text(gettext("Create book-mark"));
  bookmark->set_selectable(false);
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
#ifndef ML_GTK_OLD
   BookOpWindows bopw(mwl);
   bopw.createBookmark();
#endif
#ifdef ML_GTK_OLD
   mwl->createBookmark();
#endif
 });
  bookmark->add_controller(clck);
  grid->attach(*bookmark, 0, 2, 1, 1);

  Gtk::Label *editbook = Gtk::make_managed<Gtk::Label>();
  editbook->set_name("menulab");
  editbook->set_margin(3);
  editbook->set_halign(Gtk::Align::START);
  editbook->set_valign(Gtk::Align::CENTER);
  editbook->set_text(gettext("Edit book entry"));
  editbook->set_selectable(false);
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
      BookOpWindows bopw(mwl);
      bopw.editBook();
    });
  editbook->add_controller(clck);
  grid->attach(*editbook, 0, 3, 1, 1);

  Gtk::Label *copyto = Gtk::make_managed<Gtk::Label>();
  copyto->set_name("menulab");
  copyto->set_margin(3);
  copyto->set_halign(Gtk::Align::START);
  copyto->set_valign(Gtk::Align::CENTER);
  copyto->set_text(gettext("Save book as..."));
  copyto->set_selectable(false);
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover, sres]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
      BookOpWindows bopw(mwl);
      bopw.copyTo(sres, 1, mwl);
    });
  copyto->add_controller(clck);
  grid->attach(*copyto, 0, 4, 1, 1);

  Gtk::Label *removebook = Gtk::make_managed<Gtk::Label>();
  removebook->set_name("menulab");
  removebook->set_margin(3);
  removebook->set_halign(Gtk::Align::START);
  removebook->set_valign(Gtk::Align::CENTER);
  removebook->set_text(gettext("Remove book"));
  removebook->set_selectable(false);
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover, sres]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
      BookOpWindows bopw(mwl);
#ifdef ML_GTK_OLD
   bopw.bookRemoveWin(1, nullptr, sres);
#endif
#ifndef ML_GTK_OLD
   bopw.bookRemoveWin(mwl);
#endif
 });
  removebook->add_controller(clck);
  grid->attach(*removebook, 0, 5, 1, 1);

  return grid;
}

#ifndef ML_GTK_OLD
void
CreateRightGrid::searchResultShow()
{
  delete mw->ms_sel_book;
  mw->ms_sel_book = nullptr;
  mw->ms_style_v.clear();
  MainWindow *mwl = mw;
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(right_grid
      ->get_child_at(0, 0));
  Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(sres_scrl->get_child());
  Gtk::ScrolledWindow *annot_scrl =
      dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 7));
  Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(annot_scrl->get_child());
  Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
  tb->set_text("");
  Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(right_grid
      ->get_child_at(5, 7));
  drar->set_opacity(0.0);
  mw->cover_struct = cover_image();
  Glib::RefPtr<Gtk::Adjustment> v_adg = sres_scrl->get_vadjustment();
  v_adg->set_value(v_adg->get_lower());
  if(sres)
    {
      for(size_t i = 0; i < mw->search_res_col.size(); i++)
	{
	  sres->remove_column(mw->search_res_col[i]);
	}
      mw->search_res_col.clear();
      mw->list_sr = Gio::ListStore<ModelColumns>::create();
      for(size_t i = 0; i < mw->search_result_v.size(); i++)
	{
	  Glib::RefPtr<ModelColumns> mod_col = ModelColumns::create(
	      (mw->search_result_v[i]).authors, (mw->search_result_v[i]).book,
	      (mw->search_result_v[i]).series, (mw->search_result_v[i]).genre,
	      (mw->search_result_v[i]).date,
	      (mw->search_result_v[i]).path_to_book);
	  style_item item;
	  item.item = mod_col;
	  mw->ms_style_v.push_back(item);
	  mw->list_sr->append(mod_col);
	}
      mw->search_result_v.clear();
      Glib::RefPtr<Gtk::SingleSelection> ss_mod = Gtk::SingleSelection::create(
	  mw->list_sr);
      sres->set_model(ss_mod);

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
      auto sort_model = Gtk::SortListModel::create(mw->list_sr, author_sort);

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
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->author);
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).authors = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).authors = nullptr;
		    }
		}
	    });
      Glib::RefPtr<Gtk::ColumnViewColumn> col_author =
	  Gtk::ColumnViewColumn::create(gettext("Author"), factory);
      col_author->set_fixed_width(sres->get_width() * 0.25);
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
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->book);
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).book = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).book = nullptr;
		    }
		}
	    });
      Glib::RefPtr<Gtk::ColumnViewColumn> col_book =
	  Gtk::ColumnViewColumn::create(gettext("Book"), factory);
      col_book->set_fixed_width(sres->get_width() * 0.25);
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
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->series);
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).series = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).series = nullptr;
		    }
		}
	    });
      Glib::RefPtr<Gtk::ColumnViewColumn> col_series =
	  Gtk::ColumnViewColumn::create(gettext("Series"), factory);
      col_series->set_fixed_width(sres->get_width() * 0.2);
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
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  std::string src(col->genre);
		  CreateRightGrid crg(mwl);
		  lab->set_text(Glib::ustring(crg.genre_str(src)));
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).genre = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).genre = nullptr;
		    }
		}
	    });
      Glib::RefPtr<Gtk::ColumnViewColumn> col_genre =
	  Gtk::ColumnViewColumn::create(gettext("Genre"), factory);
      col_genre->set_fixed_width(sres->get_width() * 0.2);
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
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
	      if(col && lab)
		{
		  lab->set_text(col->date);
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).date = lab;
		    }
		}
	    });
      factory->signal_unbind().connect(
	  [mwl]
	  (const Glib::RefPtr<Gtk::ListItem> &item)
	    {
	      Glib::RefPtr<ModelColumns> col =
	      std::dynamic_pointer_cast<ModelColumns>(item->get_item());
	      if(col)
		{
		  auto it = std::find_if(mwl->ms_style_v.begin(),mwl->ms_style_v.end(), [col](auto &el)
			{
			  return col == el.item;
			});
		  if(it != mwl->ms_style_v.end())
		    {
		      (*it).date = nullptr;
		    }
		}
	    });
      Glib::RefPtr<Gtk::ColumnViewColumn> col_date =
	  Gtk::ColumnViewColumn::create(gettext("Date"), factory);
      col_date->set_fixed_width(
	  sres->get_width() - col_author->get_fixed_width()
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

      mw->search_res_col.reserve(5);
      sres->append_column(col_author);
      mw->search_res_col.push_back(col_author);
      sres->append_column(col_book);
      mw->search_res_col.push_back(col_book);
      sres->append_column(col_series);
      mw->search_res_col.push_back(col_series);
      sres->append_column(col_genre);
      mw->search_res_col.push_back(col_genre);
      sres->append_column(col_date);
      mw->search_res_col.push_back(col_date);

      sort_model->set_sorter(sres->get_sorter());
      ss_mod->set_model(sort_model);
      col_author->set_sorter(author_sort);
      col_book->set_sorter(book_sort);
      col_series->set_sorter(series_sort);
      col_genre->set_sorter(genre_sort);
      col_date->set_sorter(date_sort);
    }
  mw->ms_style_v.shrink_to_fit();
}

std::string
CreateRightGrid::genre_str(std::string src)
{
  std::string *genre_str = new std::string();
  std::vector<std::string> genre_v;
  std::string::size_type genre_n = 0;
  std::string tmp;
  tmp = src;
  while(genre_n != std::string::npos)
    {
      std::string s_s = tmp;
      genre_n = s_s.find(", ");
      if(genre_n != std::string::npos)
	{
	  s_s = s_s.substr(0, genre_n);
	  genre_v.push_back(s_s);
	  s_s = s_s + ", ";
	  tmp.erase(0, s_s.size());
	}
      else
	{
	  if(!tmp.empty())
	    {
	      tmp.erase(std::remove_if(tmp.begin(), tmp.end(), []
	      (char &el)
		{
		  return el == ' ';
		}),
			tmp.end());
	      genre_v.push_back(tmp);
	    }
	}
    }

  for(size_t j = 0; j < genre_v.size(); j++)
    {
      tmp = genre_v[j];
      std::for_each(mw->genrev->begin(), mw->genrev->end(), [genre_str, tmp]
      (auto &el)
	{
	  std::vector<genre_item>tempv;
	  tempv = el.items;
	  auto itgv = std::find_if(tempv.begin(), tempv.end(), [tmp](auto &el)
		{
		  return el.code == tmp;
		});
	  if(itgv != tempv.end())
	    {
	      if(genre_str->empty())
		{
		  *genre_str = (*itgv).name;
		}
	      else
		{
		  *genre_str = *genre_str + ", " + (*itgv).name;
		}
	    }
	});
    }
  std::string result = *genre_str;
  delete genre_str;
  return result;
}

void
CreateRightGrid::rowActivated(guint rownum, Gtk::ColumnView *sres)
{
  delete mw->ms_sel_book;
  mw->ms_sel_book = new guint(rownum);
  mw->cover_struct = cover_image();
  Gtk::Widget *widg = mw->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(5, 7);
  Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);
  drar->set_opacity(0.0);

  std::string filename;

  auto sel_mod = sres->get_model();
  if(sel_mod)
    {
      auto ss = std::dynamic_pointer_cast<Gtk::SingleSelection>(sel_mod);
      if(ss)
	{
	  auto model = ss->get_model();
	  if(model)
	    {
	      auto obj = model->get_object(rownum);
	      if(obj)
		{
		  auto lv = mw->ms_style_v;
		  MainWindow *mwl = mw;
		  auto itlv = std::find_if(lv.begin(), lv.end(), [mwl]
		  (auto &el)
		    {
		      return mwl->ms_sel_item == el.item;
		    });
		  if(itlv != lv.end())
		    {
		      if((*itlv).authors)
			{
			  (*itlv).authors->set_name("unselectedLab");
			}
		      if((*itlv).book)
			{
			  (*itlv).book->set_name("unselectedLab");
			}
		      if((*itlv).series)
			{
			  (*itlv).series->set_name("unselectedLab");
			}
		      if((*itlv).genre)
			{
			  (*itlv).genre->set_name("unselectedLab");
			}
		      if((*itlv).date)
			{
			  (*itlv).date->set_name("unselectedLab");
			}
		    }
		  auto mod_list = std::dynamic_pointer_cast<ModelColumns>(obj);
		  if(mod_list)
		    {
		      filename = std::string(mod_list->path);
		    }
		  mw->ms_sel_item = mod_list;
		  itlv = std::find_if(lv.begin(), lv.end(), [mwl]
		  (auto &el)
		    {
		      return mwl->ms_sel_item == el.item;
		    });
		  if(itlv != lv.end())
		    {
		      if((*itlv).authors)
			{
			  (*itlv).authors->set_name("selectedLab");
			}
		      if((*itlv).book)
			{
			  (*itlv).book->set_name("selectedLab");
			}
		      if((*itlv).series)
			{
			  (*itlv).series->set_name("selectedLab");
			}
		      if((*itlv).genre)
			{
			  (*itlv).genre->set_name("selectedLab");
			}
		      if((*itlv).date)
			{
			  (*itlv).date->set_name("selectedLab");
			}
		    }
		}
	    }
	}
    }
  if(!filename.empty())
    {
      AnnotationCover ac(filename);
      widg = right_grid->get_child_at(0, 7);
      Gtk::ScrolledWindow *annot_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
      widg = annot_scrl->get_child();
      Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);
      Glib::ustring annotation(ac.annotationRet());
      mw->cover_struct = ac.coverRet();
      drar->set_opacity(1.0);
      drar->queue_draw();
      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
      tb->set_text("");
      tb->insert_markup(tb->begin(), annotation);
    }
}
#endif

#ifdef ML_GTK_OLD
void
CreateRightGrid::searchResultShow(int variant, Gtk::Window *win)
{
  Glib::RefPtr<Gtk::TreeModel> del_model;
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(right_grid
      ->get_child_at(0, 0));
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
  Gtk::ScrolledWindow *annot_scrl =
      dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 7));
  Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(annot_scrl->get_child());
  if(variant == 1)
    {
      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
      tb->set_text("");
      Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(right_grid
	  ->get_child_at(5, 7));
      drar->set_opacity(0.0);
      mw->cover_struct = cover_image();
      Glib::RefPtr<Gtk::Adjustment> v_adg = sres_scrl->get_vadjustment();
      v_adg->set_value(v_adg->get_lower());
      del_model = sres->get_model();
    }
  else if(variant == 2)
    {
      del_model = mw->bm_tv->get_model();
    }

  if(del_model)
    {
      if(variant == 1)
	{
	  sres->remove_all_columns();
	  sres->unset_model();
	}
      else if(variant == 2)
	{
	  mw->bm_tv->remove_all_columns();
	  mw->bm_tv->unset_model();
	}
    }
  Gtk::TreeModel::ColumnRecord record;
  Gtk::TreeModelColumn<size_t> id;
  Gtk::TreeModelColumn<Glib::ustring> author;
  Gtk::TreeModelColumn<Glib::ustring> book;
  Gtk::TreeModelColumn<Glib::ustring> series;
  Gtk::TreeModelColumn<Glib::ustring> genre;
  Gtk::TreeModelColumn<Glib::ustring> date;
  record.add(id);
  record.add(author);
  record.add(book);
  record.add(series);
  record.add(genre);
  record.add(date);

  Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(record);
  size_t sz_v = 0;
  if(variant == 1)
    {
      sz_v = mw->search_result_v.size();
    }
  else if(variant == 2)
    {
      sz_v = mw->bookmark_v.size();
    }
  for(size_t i = 0; i < sz_v; i++)
    {
      auto row = *(model->append());
      row[id] = i + 1;
      switch(variant)
	{
	case 1:
	  {
	    row[author] = (mw->search_result_v[i]).authors;
	    row[book] = (mw->search_result_v[i]).book;
	    row[series] = (mw->search_result_v[i]).series;
	    break;
	  }
	case 2:
	  {
	    row[author] = (mw->bookmark_v[i]).authors;
	    row[book] = (mw->bookmark_v[i]).book;
	    row[series] = (mw->bookmark_v[i]).series;
	    break;
	  }
	default:
	  break;
	}

      std::string *genre_str = new std::string();
      std::vector<std::string> genre_v;
      std::string::size_type genre_n = 0;
      std::string tmp;
      switch(variant)
	{
	case 1:
	  {
	    tmp = (mw->search_result_v[i]).genre;
	    while(genre_n != std::string::npos)
	      {
		std::string s_s = tmp;
		genre_n = s_s.find(", ");
		if(genre_n != std::string::npos)
		  {
		    s_s = s_s.substr(0, genre_n);
		    genre_v.push_back(s_s);
		    s_s = s_s + ", ";
		    tmp.erase(0, s_s.size());
		  }
		else
		  {
		    if(!tmp.empty())
		      {
			tmp.erase(std::remove_if(tmp.begin(), tmp.end(), []
			(char &el)
			  {
			    return el == ' ';
			  }),
				  tmp.end());
			genre_v.push_back(tmp);
		      }
		  }
	      }

	    for(size_t j = 0; j < genre_v.size(); j++)
	      {
		tmp = genre_v[j];
		std::for_each(
		    mw->genrev->begin(),
		    mw->genrev->end(),
		    [genre_str, tmp]
		    (auto &el)
		      {
			std::vector<genre_item>tempv;
			tempv = el.items;
			auto itgv = std::find_if(tempv.begin(), tempv.end(), [tmp](auto &el)
			      {
				return el.code == tmp;
			      });
			if(itgv != tempv.end())
			  {
			    if(genre_str->empty())
			      {
				*genre_str = (*itgv).name;
			      }
			    else
			      {
				*genre_str = *genre_str + ", " + (*itgv).name;
			      }
			  }
		      });
	      }
	    break;
	  }
	case 2:
	  {
	    *genre_str = (mw->bookmark_v[i]).genre;
	    break;
	  }
	default:
	  break;
	}

      row[genre] = *genre_str;
      switch(variant)
	{
	case 1:
	  {
	    row[date] = (mw->search_result_v[i]).date;
	    break;
	  }
	case 2:
	  {
	    row[date] = (mw->bookmark_v[i]).date;
	    break;
	  }
	default:
	  break;
	}
      delete genre_str;
    }

  switch(variant)
    {
    case 1:
      {
	sres->set_model(model);
	sres->append_column(gettext("Author"), author);
	sres->append_column(gettext("Book"), book);
	sres->append_column(gettext("Series"), series);
	sres->append_column(gettext("Genre"), genre);
	sres->append_column(gettext("Date"), date);
	break;
      }
    case 2:
      {
	mw->bm_tv->set_model(model);
	mw->bm_tv->append_column(gettext("Author"), author);
	mw->bm_tv->append_column(gettext("Book"), book);
	mw->bm_tv->append_column(gettext("Series"), series);
	mw->bm_tv->append_column(gettext("Genre"), genre);
	mw->bm_tv->append_column(gettext("Date"), date);
	break;
      }
    default:
      break;
    }

  std::vector<Gtk::TreeViewColumn*> trvc_v;
  switch(variant)
    {
    case 1:
      {
	trvc_v = sres->get_columns();
	break;
      }
    case 2:
      {
	trvc_v = mw->bm_tv->get_columns();
	break;
      }
    default:
      break;
    }
  for(size_t i = 0; i < trvc_v.size(); i++)
    {
      Gtk::CellRenderer *rnd = nullptr;
      switch(variant)
	{
	case 1:
	  {
	    rnd = sres->get_column_cell_renderer(i);
	    break;
	  }
	case 2:
	  {
	    rnd = mw->bm_tv->get_column_cell_renderer(i);
	    break;
	  }
	default:
	  break;
	}
      Gtk::CellRendererText *renderer =
	  dynamic_cast<Gtk::CellRendererText*>(rnd);
      Glib::PropertyProxy<Pango::WrapMode> wrap =
	  renderer->property_wrap_mode();
      wrap.set_value(Pango::WrapMode::WORD);
      Glib::PropertyProxy<int> wrap_width = renderer->property_wrap_width();
      wrap_width.set_value(1);
      Glib::PropertyProxy<float> xalign = renderer->property_xalign();
      xalign.set_value(0.5);
      Glib::PropertyProxy<Pango::Alignment> align =
	  renderer->property_alignment();
      align.set_value(Pango::Alignment::CENTER);
    }
  switch(variant)
    {
    case 1:
      {
	sres->set_headers_clickable(true);
	break;
      }
    case 2:
      {
	mw->bm_tv->set_headers_clickable(true);
	break;
      }
    default:
      break;
    }
  int summ_width = 0;
  int tv_width = 0;
  switch(variant)
    {
    case 1:
      {
	tv_width = sres->get_width();
	break;
      }
    case 2:
      {
	int h;
	win->get_default_size(tv_width, h);
	tv_width = tv_width * 0.75;
	break;
      }
    default:
      break;
    }
  for(size_t i = 0; i < trvc_v.size(); i++)
    {
      Gtk::TreeViewColumn *col = trvc_v[i];
      col->set_expand(true);
      col->set_resizable(true);
      switch(i)
	{
	case 0:
	  {
	    col->set_fixed_width(tv_width * 0.25);
	    summ_width += col->get_fixed_width();
	    break;
	  }
	case 1:
	  {
	    col->set_fixed_width(tv_width * 0.25);
	    summ_width += col->get_fixed_width();
	    break;
	  }
	case 2:
	  {
	    col->set_fixed_width(tv_width * 0.2);
	    summ_width += col->get_fixed_width();
	    break;
	  }
	case 3:
	  {
	    col->set_fixed_width(tv_width * 0.2);
	    summ_width += col->get_fixed_width();
	    break;
	  }
	case 4:
	  {
	    col->set_fixed_width(tv_width - summ_width);
	    break;
	  }
	default:
	  break;
	}
      col->signal_clicked().connect([col, i]
      {
	col->set_sort_column(i + 1);
      });
    }
}
#endif
