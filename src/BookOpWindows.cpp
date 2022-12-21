/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

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

#include <BookOpWindows.h>

BookOpWindows::BookOpWindows(MainWindow *mw)
{
  this->mw = mw;
}

BookOpWindows::~BookOpWindows()
{
  // TODO Auto-generated destructor stub
}

void
BookOpWindows::searchBook(Gtk::ComboBoxText *coll_nm, Gtk::Entry *surname_ent,
			  Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
			  Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent)
{
  mw->search_result_v.clear();
  std::string collnm(coll_nm->get_active_text());
  std::string surnm(surname_ent->get_text());
  std::string name(name_ent->get_text());
  std::string secname(secname_ent->get_text());
  std::string book(booknm_ent->get_text());
  std::string series(ser_ent->get_text());
  std::string genre(mw->active_genre);

  SearchBook *sb = new SearchBook(collnm, surnm, name, secname, book, series,
				  genre, &mw->prev_search_nm, &mw->base_v,
				  &mw->search_result_v, &mw->search_cancel);

  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Search"));
  window->set_deletable(false);
  window->set_transient_for(*mw);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Search in progress..."));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  MainWindow *mwl = mw;
  cancel->signal_clicked().connect([mwl]
  {
    mwl->search_cancel = 1;
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  delete mwl->search_compl_disp;
  mwl->search_compl_disp = new Glib::Dispatcher;
  mwl->search_compl_disp->connect([window, mwl]
  {
    CreateRightGrid crgr(mwl);
    crgr.searchResultShow(1);
    window->close();
  });
  sb->search_completed = [mwl]
  {
    mwl->search_compl_disp->emit();
  };

  window->signal_close_request().connect([mwl, window, sb]
  {
    mwl->search_cancel = 0;
    delete sb;
    window->hide();
    delete window;
    return true;
  },
					 false);

  window->show();

  std::thread *thr = new std::thread([sb, mwl]
  {
    mwl->searchmtx->lock();
    sb->searchBook();
    sb->cleanSearchV();
    mwl->searchmtx->unlock();
  });
  thr->detach();
  delete thr;
}

void
BookOpWindows::bookRemoveWin(int variant, Gtk::Window *win)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Confirmation"));
  if(variant == 1)
    {
      window->set_transient_for(*mw);
    }
  if(variant == 2)
    {
      window->set_transient_for(*win);
    }
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  if(variant == 1)
    {
      lab->set_text(
	  gettext(
	      "This action will remove book from collection and file system. Continue?"));
    }
  if(variant == 2)
    {
      lab->set_text(
	  gettext("This action will remove book from bookmarks. Continue?"));
    }
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_justify(Gtk::Justification::FILL);
  lab->set_max_width_chars(30);
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_margin(5);
  yes->set_label(gettext("Yes"));
  grid->attach(*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_halign(Gtk::Align::CENTER);
  no->set_margin(5);
  no->set_label(gettext("No"));
  no->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*no, 1, 1, 1, 1);
  MainWindow *mwl = mw;
  if(variant == 1)
    {
      int *cncl = new int(0);
      Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
      Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
      Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
      Gtk::ComboBoxText *collect_box =
	  dynamic_cast<Gtk::ComboBoxText*>(left_gr->get_child_at(0, 1));
      RefreshCollection *rc = new RefreshCollection(
	  std::string(collect_box->get_active_text()), 1, cncl);
      Glib::Dispatcher *disp_finished = new Glib::Dispatcher;

      yes->signal_clicked().connect(
	  [mwl, lab, yes, no, grid, window, rc, disp_finished]
	  {
	    window->set_title(gettext("Removing"));
	    window->set_deletable(false);
	    lab->set_text(gettext("Removing... It will take some time"));
	    grid->remove(*yes);
	    grid->remove(*no);
	    Glib::RefPtr<Glib::MainContext> mc =
		Glib::MainContext::get_default();
	    while(mc->pending())
	      {
		mc->iteration(true);
	      }
	    Gtk::Requisition min, nat;
	    grid->get_preferred_size(min, nat);
	    window->set_default_size(nat.get_width(), nat.get_height());

	    disp_finished->connect([window, mwl]
	    {
	      mwl->prev_search_nm.clear();
	      Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
	      Gtk::Paned *pn =
		  dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
	      Gtk::Grid *right_grid =
		  dynamic_cast<Gtk::Grid*>(pn->get_end_child());
	      Gtk::ScrolledWindow *sres_scrl =
		  dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0,0 ));
	      Glib::RefPtr<Gtk::Adjustment> adj = sres_scrl->get_vadjustment();
	      double pos = adj->get_value();
	      CreateRightGrid crgr(mwl);
	      crgr.searchResultShow(1);
	      Glib::RefPtr<Glib::MainContext> mc =
		  Glib::MainContext::get_default();
	      while(mc->pending())
		{
		  mc->iteration(true);
		}
	      adj->set_value(pos);
	      window->close();
	    });
	    Gtk::Widget *widg = mwl->get_child();
	    Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
	    widg = main_grid->get_child_at(0, 1);
	    Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
	    widg = pn->get_end_child();
	    Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
	    widg = right_grid->get_child_at(0, 0);
	    Gtk::ScrolledWindow *sres_scrl =
		dynamic_cast<Gtk::ScrolledWindow*>(widg);
	    widg = sres_scrl->get_child();
	    Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(widg);
	    Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();
	    if(selection)
	      {
		Gtk::TreeModel::iterator iter = selection->get_selected();
		if(iter)
		  {
		    size_t id;
		    iter->get_value(0, id);
		    std::string filename = std::get<5>(
			mwl->search_result_v[id - 1]);
		    Glib::RefPtr<Gtk::ListStore> liststore =
			std::dynamic_pointer_cast<Gtk::ListStore>(
			    sres->get_model());
		    liststore->erase(iter);
		    mwl->search_result_v.erase(
			mwl->search_result_v.begin() + id - 1);
		    Gtk::DrawingArea *drar =
			dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(
			    1, 1));
		    drar->set_opacity(0.0);
		    widg = right_grid->get_child_at(0, 1);
		    Gtk::ScrolledWindow *annot_scrl =
			dynamic_cast<Gtk::ScrolledWindow*>(widg);
		    widg = annot_scrl->get_child();
		    Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);
		    Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
		    tb->set_text("");
		    std::thread *thr = new std::thread(
			[rc, disp_finished, filename]
			{
			  rc->removeBook(filename);
			  disp_finished->emit();
			});
		    thr->detach();
		    delete thr;
		  }
	      }
	    else
	      {
		window->close();
	      }
	  });

      window->signal_close_request().connect([window, rc, cncl, disp_finished]
      {
	window->hide();
	delete rc;
	delete cncl;
	delete window;
	delete disp_finished;
	return true;
      },
					     false);
    }
  if(variant == 2)
    {
      yes->signal_clicked().connect([mwl, window]
      {
	Glib::RefPtr<Gtk::TreeSelection> selection =
	    mwl->bm_tv->get_selection();
	if(selection)
	  {
	    Gtk::TreeModel::iterator iter = selection->get_selected();
	    if(iter)
	      {
		size_t id;
		iter->get_value(0, id);
		mwl->bookmark_v.erase(mwl->bookmark_v.begin() + id - 1);
		Glib::RefPtr<Gtk::ListStore> liststore =
		    std::dynamic_pointer_cast<Gtk::ListStore>(mwl->bm_tv->get_model());
		liststore->erase(iter);
		std::string filename;
		AuxFunc af;
		af.homePath(&filename);
		filename = filename + "/.MyLibrary/Bookmarks";
		std::filesystem::path filepath = std::filesystem::u8path(
		    filename);
		if(std::filesystem::exists(filepath))
		  {
		    std::filesystem::remove(filepath);
		  }
		std::fstream f;
		f.open(filepath, std::ios_base::out | std::ios_base::binary);
		if(f.is_open())
		  {
		    for(size_t i = 0; i < mwl->bookmark_v.size(); i++)
		      {
			std::string line = std::get<0>(mwl->bookmark_v[i])
			    + "<?>";
			line = line + std::get<1>(mwl->bookmark_v[i]) + "<?>";
			line = line + std::get<2>(mwl->bookmark_v[i]) + "<?>";
			line = line + std::get<3>(mwl->bookmark_v[i]) + "<?>";
			line = line + std::get<4>(mwl->bookmark_v[i]) + "<?>";
			line = line + std::get<5>(mwl->bookmark_v[i]) + "<?L>";
			f.write(line.c_str(), line.size());
		      }
		    f.close();
		  }
	      }
	  }
	window->close();
      });
      window->signal_close_request().connect([window]
	{
	  window->hide();
	  delete window;
	  return true;
	},
      false);
    }
  window->show();
}
