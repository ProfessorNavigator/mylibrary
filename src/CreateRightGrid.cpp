/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of MyLibrary.
 MyLibrary is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 MyLibrary is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with MyLibrary. If not,
 see <https://www.gnu.org/licenses/>.
 */

#include <CreateRightGrid.h>

CreateRightGrid::CreateRightGrid(MainWindow *mw)
{
  this->mw = mw;
}

CreateRightGrid::~CreateRightGrid()
{
  // TODO Auto-generated destructor stub
}

Gtk::Grid*
CreateRightGrid::formRightGrid()
{
  Gtk::Grid *right_grid = Gtk::make_managed<Gtk::Grid>();
  right_grid->set_halign(Gtk::Align::FILL);
  right_grid->set_valign(Gtk::Align::FILL);
  right_grid->set_hexpand(true);
  right_grid->set_vexpand(true);

  Gtk::ScrolledWindow *search_res = Gtk::make_managed<Gtk::ScrolledWindow>();
  search_res->set_halign(Gtk::Align::FILL);
  search_res->set_valign(Gtk::Align::FILL);
  search_res->set_margin(5);
  search_res->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::ALWAYS);
  search_res->set_hexpand(true);
  search_res->set_vexpand(true);
  right_grid->attach(*search_res, 0, 0, 2, 1);

  Gtk::TreeView *sres = Gtk::make_managed<Gtk::TreeView>();
  sres->set_halign(Gtk::Align::FILL);
  sres->set_hexpand(true);
  sres->set_headers_clickable(true);
  sres->set_activate_on_single_click(true);
  sres->set_name("searchRes");
  sres->signal_row_activated().connect( // @suppress("Invalid arguments")
      sigc::mem_fun(*mw, &MainWindow::rowActivated)); // @suppress("Invalid arguments")
  sres->set_hexpand(true);

  Gtk::Popover *book_men = Gtk::make_managed<Gtk::Popover>();
  book_men->set_parent(*sres);
  book_men->set_name("popoverSt");
  Gtk::Grid *book_men_grid = formPopoverGrid(sres, book_men);
  book_men->set_child(*book_men_grid);

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect([book_men]
  (int n_pressed, double x, double y)
    {
      Gdk::Rectangle rect(x, y, 1, 1);
      book_men->set_pointing_to(rect);
      book_men->popup();
    });
  sres->add_controller(clck);
  search_res->set_child(*sres);

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
  right_grid->attach(*book_op_but, 0, 1, 1, 1);

  Gtk::ScrolledWindow *annot_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  annot_scrl->set_margin(5);
  annot_scrl->set_halign(Gtk::Align::FILL);
  annot_scrl->set_valign(Gtk::Align::FILL);
  annot_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
			 Gtk::PolicyType::AUTOMATIC);
  annot_scrl->set_hexpand(true);
  annot_scrl->set_vexpand(false);

  Gtk::TextView *annot = Gtk::make_managed<Gtk::TextView>();
  annot->set_editable(false);
  annot->set_wrap_mode(Gtk::WrapMode::WORD);
  annot_scrl->set_child(*annot);
  right_grid->attach(*annot_scrl, 0, 2, 1, 1);

  Gtk::DrawingArea *drar = Gtk::make_managed<Gtk::DrawingArea>();
  drar->set_halign(Gtk::Align::FILL);
  drar->set_valign(Gtk::Align::FILL);
  drar->set_margin(5);
  drar->set_draw_func(sigc::mem_fun(*mw, &MainWindow::drawCover)); // @suppress("Invalid arguments")
  drar->set_hexpand(false);
  drar->set_vexpand(false);
  right_grid->attach(*drar, 1, 2, 1, 1);

  MainWindow *mwl = mw;
  mw->signal_show().connect([mwl]
  {
    Gdk::Rectangle scr_res = mwl->screenRes();
    mwl->set_default_size(scr_res.get_width() * 0.9, scr_res.get_height() * 0.7);

    int main_width, main_height;
    mwl->get_default_size(main_width, main_height);
    Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
    Gtk::Box *box = dynamic_cast<Gtk::Box*>(main_grid->get_child_at(0, 0));
    Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
    Gtk::Grid *left_grid = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
    Gtk::Requisition min_left_grid, nat_left_grid;
    left_grid->get_preferred_size(min_left_grid, nat_left_grid);
    Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
    Gtk::Requisition min_box, nat_box;
    box->get_preferred_size(min_box, nat_box);
    Gtk::ScrolledWindow *sres_scrl =
	dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
    sres_scrl->set_min_content_height(
	(main_height - nat_box.get_height()) * 0.6);

    Gtk::Requisition min_sres_scrl, nat_sres_scrl;
    sres_scrl->get_preferred_size(min_sres_scrl, nat_sres_scrl);

    Gtk::MenuButton *book_op_but =
	dynamic_cast<Gtk::MenuButton*>(right_grid->get_child_at(0, 1));
    Gtk::Requisition book_op_but_min, book_op_but_nat;
    book_op_but->get_preferred_size(book_op_but_min, book_op_but_nat);
    Gtk::ScrolledWindow *annot_scrl =
	dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 2));
    annot_scrl->set_min_content_height(
	main_height - nat_box.get_height() - nat_sres_scrl.get_height()
	    - book_op_but_nat.get_height());
    annot_scrl->set_min_content_width(
	(main_width - nat_left_grid.get_width()) * 0.8);

    Gtk::DrawingArea *drar =
	dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(1, 2));
    drar->set_content_width(
	main_width - nat_left_grid.get_width()
	    - annot_scrl->get_min_content_width() - 20);
    drar->set_content_height(annot_scrl->get_min_content_height());
  });

  return right_grid;
}

Gtk::Grid*
CreateRightGrid::formPopoverGrid(Gtk::TreeView *sres,
				 Gtk::Popover *book_popover)
{
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  MainWindow *mwl = mw;

  Gtk::Label *openbook = Gtk::make_managed<Gtk::Label>();
  openbook->set_name("menulab");
  openbook->set_margin(3);
  openbook->set_halign(Gtk::Align::CENTER);
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
      bopw.openBook(1);
    });
  openbook->add_controller(clck);
  grid->attach(*openbook, 0, 0, 1, 1);

  Gtk::Label *fileinfo = Gtk::make_managed<Gtk::Label>();
  fileinfo->set_name("menulab");
  fileinfo->set_margin(3);
  fileinfo->set_halign(Gtk::Align::CENTER);
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
      bopw.fileInfo();
    });
  fileinfo->add_controller(clck);
  grid->attach(*fileinfo, 0, 1, 1, 1);

  Gtk::Label *bookmark = Gtk::make_managed<Gtk::Label>();
  bookmark->set_name("menulab");
  bookmark->set_margin(3);
  bookmark->set_halign(Gtk::Align::CENTER);
  bookmark->set_valign(Gtk::Align::CENTER);
  bookmark->set_text(gettext("Create book-mark"));
  bookmark->set_selectable(false);
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, book_popover]
  (int num_pressed, double x, double y)
    {
      book_popover->popdown();
      mwl->createBookmark();
    });
  bookmark->add_controller(clck);
  grid->attach(*bookmark, 0, 2, 1, 1);

  Gtk::Label *editbook = Gtk::make_managed<Gtk::Label>();
  editbook->set_name("menulab");
  editbook->set_margin(3);
  editbook->set_halign(Gtk::Align::CENTER);
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
  copyto->set_halign(Gtk::Align::CENTER);
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
  removebook->set_halign(Gtk::Align::CENTER);
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
      bopw.bookRemoveWin(1, nullptr, sres);
    });
  removebook->add_controller(clck);
  grid->attach(*removebook, 0, 5, 1, 1);

  return grid;
}

void
CreateRightGrid::searchResultShow(int variant)
{
  Glib::RefPtr<Gtk::TreeModel> del_model;
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl =
      dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
  Gtk::ScrolledWindow *annot_scrl =
      dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 2));
  Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(annot_scrl->get_child());
  if(variant == 1)
    {
      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
      tb->set_text("");
      Gtk::DrawingArea *drar =
	  dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(1, 2));
      drar->set_opacity(0.0);
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
      if(variant == 1)
	{
	  row[author] = std::get<0>(mw->search_result_v[i]);
	  row[book] = std::get<1>(mw->search_result_v[i]);
	  row[series] = std::get<2>(mw->search_result_v[i]);
	}
      else if(variant == 2)
	{
	  row[author] = std::get<0>(mw->bookmark_v[i]);
	  row[book] = std::get<1>(mw->bookmark_v[i]);
	  row[series] = std::get<2>(mw->bookmark_v[i]);
	}

      std::string *genre_str = new std::string();
      std::vector<std::string> genre_v;
      std::string::size_type genre_n = 0;
      std::string tmp;
      if(variant == 1)
	{
	  tmp = std::get<3>(mw->search_result_v[i]);
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
		      std::vector<std::tuple<std::string, std::string>>tempv;
		      tempv = std::get<1>(el);
		      auto itgv = std::find_if(tempv.begin(), tempv.end(), [tmp](auto &el)
			    {
			      return std::get<0>(el) == tmp;
			    });
		      if(itgv != tempv.end())
			{
			  if(genre_str->empty())
			    {
			      *genre_str = std::get<1>(*itgv);
			    }
			  else
			    {
			      *genre_str = *genre_str + ", " + std::get<1>(*itgv);
			    }
			}
		    });
	    }
	}
      else if(variant == 2)
	{
	  *genre_str = std::get<3>(mw->bookmark_v[i]);
	}

      row[genre] = *genre_str;
      if(variant == 1)
	{
	  row[date] = std::get<4>(mw->search_result_v[i]);
	}
      else if(variant == 2)
	{
	  row[date] = std::get<4>(mw->bookmark_v[i]);
	}
      delete genre_str;
    }

  if(variant == 1)
    {
      sres->set_model(model);
      sres->append_column(gettext("Author"), author);
      sres->append_column(gettext("Book"), book);
      sres->append_column(gettext("Series"), series);
      sres->append_column(gettext("Genre"), genre);
      sres->append_column(gettext("Date"), date);
    }
  else if(variant == 2)
    {
      mw->bm_tv->set_model(model);
      mw->bm_tv->append_column(gettext("Author"), author);
      mw->bm_tv->append_column(gettext("Book"), book);
      mw->bm_tv->append_column(gettext("Series"), series);
      mw->bm_tv->append_column(gettext("Genre"), genre);
      mw->bm_tv->append_column(gettext("Date"), date);
    }
  std::vector<Gtk::TreeViewColumn*> trvc_v;
  if(variant == 1)
    {
      trvc_v = sres->get_columns();
    }
  else if(variant == 2)
    {
      trvc_v = mw->bm_tv->get_columns();
    }
  for(size_t i = 0; i < trvc_v.size(); i++)
    {
      Gtk::CellRenderer *rnd = nullptr;
      if(variant == 1)
	{
	  rnd = sres->get_column_cell_renderer(i);
	}
      else if(variant == 2)
	{
	  rnd = mw->bm_tv->get_column_cell_renderer(i);
	}
      Gtk::CellRendererText *renderer =
	  dynamic_cast<Gtk::CellRendererText*>(rnd);
      if(i >= 0)
	{
	  if(i < 2)
	    {
	      Glib::PropertyProxy<int> max_width =
		  renderer->property_max_width_chars();
	      max_width.set_value(50);
	      Glib::PropertyProxy<int> width_chars =
		  renderer->property_width_chars();
	      width_chars.set_value(50);
	      Glib::PropertyProxy<Pango::WrapMode> wrap =
		  renderer->property_wrap_mode();
	      wrap.set_value(Pango::WrapMode::WORD);
	      Glib::PropertyProxy<int> wrap_width =
		  renderer->property_wrap_width();
	      wrap_width.set_value(5);
	    }
	  else if(i == 2)
	    {
	      Glib::PropertyProxy<int> max_width =
		  renderer->property_max_width_chars();
	      max_width.set_value(40);
	      Glib::PropertyProxy<Pango::WrapMode> wrap =
		  renderer->property_wrap_mode();
	      wrap.set_value(Pango::WrapMode::WORD);
	      Glib::PropertyProxy<int> wrap_width =
		  renderer->property_wrap_width();
	      wrap_width.set_value(5);
	      Glib::PropertyProxy<int> width_chars =
		  renderer->property_width_chars();
	      width_chars.set_value(40);
	    }
	  else if(i == 3)
	    {
	      Glib::PropertyProxy<int> max_width =
		  renderer->property_max_width_chars();
	      max_width.set_value(20);
	      Glib::PropertyProxy<Pango::WrapMode> wrap =
		  renderer->property_wrap_mode();
	      wrap.set_value(Pango::WrapMode::WORD);
	      Glib::PropertyProxy<int> wrap_width =
		  renderer->property_wrap_width();
	      wrap_width.set_value(5);
	      Glib::PropertyProxy<int> width_chars =
		  renderer->property_width_chars();
	      width_chars.set_value(20);
	    }
	  else if(i == 4)
	    {
	      Glib::PropertyProxy<int> max_width =
		  renderer->property_max_width_chars();
	      max_width.set_value(10);
	      Glib::PropertyProxy<Pango::WrapMode> wrap =
		  renderer->property_wrap_mode();
	      wrap.set_value(Pango::WrapMode::WORD);
	      Glib::PropertyProxy<int> wrap_width =
		  renderer->property_wrap_width();
	      wrap_width.set_value(5);
	    }
	}
    }
  if(variant == 1)
    {
      sres->set_headers_clickable(true);
    }
  else if(variant == 2)
    {
      mw->bm_tv->set_headers_clickable(true);
    }

  for(size_t i = 0; i < trvc_v.size(); i++)
    {
      Gtk::TreeViewColumn *col = trvc_v[i];
      col->signal_clicked().connect([col, i]
      {
	col->set_sort_column(i + 1);
      });
    }
}
