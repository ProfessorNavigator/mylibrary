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
  window->set_default_size(1, 1);
  if(variant == 1)
    {
      window->set_transient_for(*mw);
    }
  else if(variant == 2)
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
  else if(variant == 2)
    {
      lab->set_text(
	  gettext("This action will remove book from bookmarks. Continue?"));
    }
  window->set_default_size(1, 1);
  lab->set_wrap(true);
  lab->set_wrap_mode(Pango::WrapMode::WORD);
  lab->set_justify(Gtk::Justification::FILL);
  lab->set_width_chars(30);
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
	    window->set_default_size(1, 1);
	    grid->remove(*yes);
	    grid->remove(*no);

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
	    Glib::Dispatcher *disp_file_nexists = new Glib::Dispatcher;
	    disp_file_nexists->connect([mwl, disp_file_nexists]
	    {
	      mwl->errorWin(7, mwl, disp_file_nexists);
	    });
	    rc->collection_not_exists = [disp_file_nexists]
	    {
	      disp_file_nexists->emit();
	    };
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
			    1, 2));
		    drar->set_opacity(0.0);
		    widg = right_grid->get_child_at(0, 2);
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
  else if(variant == 2)
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

void
BookOpWindows::fileInfo()
{
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl =
      dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  Gtk::Window *window = new Gtk::Window;
	  window->set_application(mw->get_application());
	  window->set_title(gettext("Book file info"));
	  window->set_transient_for(*mw);

	  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
	  grid->set_halign(Gtk::Align::FILL);
	  grid->set_valign(Gtk::Align::FILL);
	  window->set_child(*grid);

	  size_t id;
	  AuxFunc af;
	  std::filesystem::path filepath;
	  iter->get_value(0, id);
	  std::string filename;
	  filename = std::get<5>(mw->search_result_v[id - 1]);
	  std::string::size_type n;
	  n = filename.find("<zip>");
	  if(n != std::string::npos)
	    {
	      std::string archpath = filename;
	      archpath.erase(
		  0,
		  archpath.find("<archpath>")
		      + std::string("<archpath>").size());
	      archpath = archpath.substr(0, archpath.find("</archpath>"));
	      std::string ind_str = filename;
	      ind_str.erase(
		  0, ind_str.find("<index>") + std::string("<index>").size());
	      ind_str = ind_str.substr(0, ind_str.find("</index>"));
	      std::stringstream strm;
	      std::locale loc("C");
	      strm.imbue(loc);
	      strm << ind_str;
	      int index;
	      strm >> index;
	      filepath = std::filesystem::u8path(archpath);
	      filepath.make_preferred();

	      Gtk::Label *arch_lb = Gtk::make_managed<Gtk::Label>();
	      arch_lb->set_margin(5);
	      arch_lb->set_halign(Gtk::Align::START);
	      arch_lb->set_use_markup(true);
	      arch_lb->set_markup(
		  Glib::ustring("<b>") + Glib::ustring(gettext("Archive: "))
		      + Glib::ustring("</b>"));
	      grid->attach(*arch_lb, 0, 0, 1, 1);

	      Gtk::Label *arch_path_lb = Gtk::make_managed<Gtk::Label>();
	      arch_path_lb->set_margin(5);
	      arch_path_lb->set_halign(Gtk::Align::START);
	      arch_path_lb->set_use_markup(true);
	      arch_path_lb->set_markup(
		  Glib::ustring("<i>") + Glib::ustring(filepath.u8string())
		      + Glib::ustring("</i>"));
	      grid->attach(*arch_path_lb, 1, 0, 1, 1);

	      AuxFunc af;
	      std::vector<std::tuple<std::string, std::string>> infov;
	      infov = af.fileinfo(filepath.u8string(), index);

	      Gtk::Label *nm_lb = Gtk::make_managed<Gtk::Label>();
	      nm_lb->set_margin(5);
	      nm_lb->set_halign(Gtk::Align::START);
	      nm_lb->set_use_markup(true);
	      nm_lb->set_markup(
		  Glib::ustring("<b>")
		      + Glib::ustring(gettext("File path in archive: "))
		      + Glib::ustring("</b>"));
	      grid->attach(*nm_lb, 0, 1, 1, 1);

	      Gtk::Label *file_nm_lb = Gtk::make_managed<Gtk::Label>();
	      file_nm_lb->set_margin(5);
	      file_nm_lb->set_halign(Gtk::Align::START);
	      file_nm_lb->set_use_markup(true);
	      auto itinfv = std::find_if(infov.begin(), infov.end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "filename";
		});
	      if(itinfv != infov.end())
		{
		  file_nm_lb->set_markup(
		      Glib::ustring("<i>") + Glib::ustring(std::get<1>(*itinfv))
			  + Glib::ustring("</i>"));
		}
	      grid->attach(*file_nm_lb, 1, 1, 1, 1);

	      Gtk::Label *u_sz_lb = Gtk::make_managed<Gtk::Label>();
	      u_sz_lb->set_margin(5);
	      u_sz_lb->set_halign(Gtk::Align::START);
	      u_sz_lb->set_use_markup(true);
	      u_sz_lb->set_markup(
		  Glib::ustring("<b>")
		      + Glib::ustring(gettext("File size (uncompressed): "))
		      + Glib::ustring("</b>"));
	      grid->attach(*u_sz_lb, 0, 2, 1, 1);

	      Gtk::Label *sz_lb = Gtk::make_managed<Gtk::Label>();
	      sz_lb->set_margin(5);
	      sz_lb->set_halign(Gtk::Align::START);
	      sz_lb->set_use_markup(true);
	      itinfv = std::find_if(infov.begin(), infov.end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "filesizeunc";
		});
	      if(itinfv != infov.end())
		{
		  sz_lb->set_markup(
		      Glib::ustring("<i>") + Glib::ustring(std::get<1>(*itinfv))
			  + Glib::ustring("</i> ")
			  + Glib::ustring(gettext("bytes")));
		}
	      grid->attach(*sz_lb, 1, 2, 1, 1);

	      Gtk::Label *c_sz_lb = Gtk::make_managed<Gtk::Label>();
	      c_sz_lb->set_margin(5);
	      c_sz_lb->set_halign(Gtk::Align::START);
	      c_sz_lb->set_use_markup(true);
	      c_sz_lb->set_markup(
		  Glib::ustring("<b>")
		      + Glib::ustring(gettext("File size (compressed): "))
		      + Glib::ustring("</b>"));
	      grid->attach(*c_sz_lb, 0, 3, 1, 1);

	      Gtk::Label *sz_lb2 = Gtk::make_managed<Gtk::Label>();
	      sz_lb2->set_margin(5);
	      sz_lb2->set_halign(Gtk::Align::START);
	      sz_lb2->set_use_markup(true);
	      itinfv = std::find_if(infov.begin(), infov.end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "filesizec";
		});
	      if(itinfv != infov.end())
		{
		  sz_lb2->set_markup(
		      Glib::ustring("<i>") + Glib::ustring(std::get<1>(*itinfv))
			  + Glib::ustring("</i> ")
			  + Glib::ustring(gettext("bytes")));
		}
	      grid->attach(*sz_lb2, 1, 3, 1, 1);
	    }
	  else
	    {
	      filepath = std::filesystem::u8path(filename);
	      filepath.make_preferred();
	      Gtk::Label *fp_lb = Gtk::make_managed<Gtk::Label>();
	      fp_lb->set_margin(5);
	      fp_lb->set_halign(Gtk::Align::START);
	      fp_lb->set_use_markup(true);
	      fp_lb->set_markup(
		  Glib::ustring("<b>") + Glib::ustring(gettext("File: "))
		      + Glib::ustring("</b>"));
	      grid->attach(*fp_lb, 0, 0, 1, 1);

	      Gtk::Label *path_lb = Gtk::make_managed<Gtk::Label>();
	      path_lb->set_margin(5);
	      path_lb->set_halign(Gtk::Align::START);
	      path_lb->set_use_markup(true);
	      path_lb->set_markup(
		  Glib::ustring("<i>") + Glib::ustring(filepath.u8string())
		      + Glib::ustring("</i>"));
	      grid->attach(*path_lb, 1, 0, 1, 1);

	      Gtk::Label *fsz_lb = Gtk::make_managed<Gtk::Label>();
	      fsz_lb->set_margin(5);
	      fsz_lb->set_halign(Gtk::Align::START);
	      fsz_lb->set_use_markup(true);
	      fsz_lb->set_markup(
		  Glib::ustring("<b>") + Glib::ustring(gettext("File size: "))
		      + Glib::ustring("</b>"));
	      grid->attach(*fsz_lb, 0, 1, 1, 1);

	      Gtk::Label *sz_lb = Gtk::make_managed<Gtk::Label>();
	      sz_lb->set_margin(5);
	      sz_lb->set_halign(Gtk::Align::START);
	      sz_lb->set_use_markup(true);
	      std::stringstream strm;
	      std::locale loc("C");
	      strm.imbue(loc);
	      strm << std::filesystem::file_size(filepath);
	      sz_lb->set_markup(
		  Glib::ustring("<i>") + Glib::ustring(strm.str())
		      + Glib::ustring("</i> ")
		      + Glib::ustring(gettext("bytes")));
	      grid->attach(*sz_lb, 1, 1, 1, 1);
	    }
	  window->signal_close_request().connect([window]
	  {
	    window->hide();
	    delete window;
	    return true;
	  },
						 false);
	  window->show();
	}
    }
}

void
BookOpWindows::editBook()
{
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl =
      dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();

  std::vector<std::tuple<std::string, std::string>> *bookv = new std::vector<
      std::tuple<std::string, std::string>>;

  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  Glib::ustring val;
	  iter->get_value(1, val);
	  std::tuple<std::string, std::string> ttup;
	  std::get<0>(ttup) = "Author";
	  std::get<1>(ttup) = std::string(val);
	  bookv->push_back(ttup);

	  val.clear();
	  iter->get_value(2, val);
	  std::get<0>(ttup) = "Book";
	  std::get<1>(ttup) = std::string(val);
	  bookv->push_back(ttup);

	  val.clear();
	  iter->get_value(3, val);
	  std::get<0>(ttup) = "Series";
	  std::get<1>(ttup) = std::string(val);
	  bookv->push_back(ttup);

	  size_t id;
	  iter->get_value(0, id);
	  std::get<0>(ttup) = "Genre";
	  std::get<1>(ttup) = std::get<3>(mw->search_result_v[id - 1]);
	  bookv->push_back(ttup);

	  val.clear();
	  iter->get_value(5, val);
	  std::get<0>(ttup) = "Date";
	  std::get<1>(ttup) = std::string(val);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "filepath";
	  std::get<1>(ttup) = std::get<5>(mw->search_result_v[id - 1]);
	  bookv->push_back(ttup);
	}
    }

  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Book entry editor"));
  window->set_transient_for(*mw);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *auth_lb = Gtk::make_managed<Gtk::Label>();
  auth_lb->set_halign(Gtk::Align::START);
  auth_lb->set_margin(5);
  auth_lb->set_text(gettext("Author:"));
  grid->attach(*auth_lb, 0, 0, 1, 1);

  Gtk::Entry *auth_ent = Gtk::make_managed<Gtk::Entry>();
  auth_ent->set_halign(Gtk::Align::START);
  auth_ent->set_margin(5);
  auth_ent->set_width_chars(80);
  auto itbv = std::find_if(bookv->begin(), bookv->end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Author";
    });
  if(itbv != bookv->end())
    {
      auth_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
    }
  grid->attach(*auth_ent, 0, 1, 3, 1);

  Gtk::Label *book_lb = Gtk::make_managed<Gtk::Label>();
  book_lb->set_halign(Gtk::Align::START);
  book_lb->set_margin(5);
  book_lb->set_text(gettext("Book:"));
  grid->attach(*book_lb, 0, 2, 1, 1);

  Gtk::Entry *book_ent = Gtk::make_managed<Gtk::Entry>();
  book_ent->set_halign(Gtk::Align::START);
  book_ent->set_margin(5);
  book_ent->set_width_chars(80);
  itbv = std::find_if(bookv->begin(), bookv->end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Book";
    });
  if(itbv != bookv->end())
    {
      book_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
    }
  grid->attach(*book_ent, 0, 3, 3, 1);

  Gtk::Label *series_lb = Gtk::make_managed<Gtk::Label>();
  series_lb->set_halign(Gtk::Align::START);
  series_lb->set_margin(5);
  series_lb->set_text(gettext("Series:"));
  grid->attach(*series_lb, 0, 4, 1, 1);

  Gtk::Entry *series_ent = Gtk::make_managed<Gtk::Entry>();
  series_ent->set_halign(Gtk::Align::START);
  series_ent->set_margin(5);
  series_ent->set_width_chars(80);
  itbv = std::find_if(bookv->begin(), bookv->end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Series";
    });
  if(itbv != bookv->end())
    {
      series_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
    }
  grid->attach(*series_ent, 0, 5, 3, 1);

  Gtk::Label *genre_lb = Gtk::make_managed<Gtk::Label>();
  genre_lb->set_halign(Gtk::Align::START);
  genre_lb->set_margin(5);
  genre_lb->set_text(gettext("Genre:"));
  grid->attach(*genre_lb, 0, 6, 1, 1);

  Gtk::Entry *genre_ent = Gtk::make_managed<Gtk::Entry>();
  genre_ent->set_halign(Gtk::Align::START);
  genre_ent->set_margin(5);
  genre_ent->set_width_chars(80);
  itbv = std::find_if(bookv->begin(), bookv->end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Genre";
    });
  if(itbv != bookv->end())
    {
      genre_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
    }
  grid->attach(*genre_ent, 0, 7, 3, 1);

  Gtk::Label *addgenre_lb = Gtk::make_managed<Gtk::Label>();
  addgenre_lb->set_halign(Gtk::Align::START);
  addgenre_lb->set_margin(5);
  addgenre_lb->set_text(gettext("Add genre:"));
  grid->attach(*addgenre_lb, 0, 8, 1, 1);

  Gtk::MenuButton *genre_but = Gtk::make_managed<Gtk::MenuButton>();
  genre_but->set_halign(Gtk::Align::START);
  genre_but->set_margin(5);
  genre_but->set_label(gettext("<No>"));

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  Gtk::Grid *scrl_gr = Gtk::make_managed<Gtk::Grid>();
  scrl_gr->set_halign(Gtk::Align::START);

  Gtk::Popover *popover = Gtk::make_managed<Gtk::Popover>();
  popover->set_child(*scrl);
  genre_but->set_popover(*popover);

  int maxl = 0;
  Gtk::Expander *maxexp = nullptr;
  for(size_t i = 0; i < mw->genrev->size(); i++)
    {
      Glib::ustring g_group(std::get<0>(mw->genrev->at(i)));

      if(i == 0)
	{
	  Gtk::Label *txtl = Gtk::make_managed<Gtk::Label>();
	  txtl->set_margin(2);
	  txtl->set_halign(Gtk::Align::START);
	  txtl->set_text(g_group);
	  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
	  clck->set_button(1);
	  clck->signal_pressed().connect([txtl, genre_but]
	  (int but, double x, double y)
	    {
	      genre_but->set_label(txtl->get_text());
	      genre_but->popdown();
	    });
	  txtl->add_controller(clck);
	  scrl_gr->attach(*txtl, 0, i, 1, 1);
	}
      else
	{
	  Gtk::Expander *chexp = Gtk::make_managed<Gtk::Expander>();
	  std::vector<std::tuple<std::string, std::string>> tmpv = std::get<1>(
	      mw->genrev->at(i));
	  chexp->set_halign(Gtk::Align::START);
	  chexp->set_margin(2);
	  chexp->set_expanded(false);
	  chexp->set_label(g_group);
	  Gtk::Grid *chexp_gr = Gtk::make_managed<Gtk::Grid>();
	  chexp_gr->set_halign(Gtk::Align::CENTER);
	  chexp->set_child(*chexp_gr);

	  if(int(g_group.size()) > maxl)
	    {
	      maxl = g_group.size();
	      maxexp = chexp;
	    }

	  for(size_t j = 0; j < tmpv.size(); j++)
	    {
	      Gtk::Label *txtl = Gtk::make_managed<Gtk::Label>();
	      txtl->set_margin(2);
	      txtl->set_halign(Gtk::Align::END);
	      txtl->set_text(Glib::ustring(std::get<1>(tmpv[j])));
	      std::string code = std::get<0>(tmpv[j]);
	      Glib::RefPtr<Gtk::GestureClick> clck =
		  Gtk::GestureClick::create();
	      clck->set_button(1);
	      clck->signal_pressed().connect(
		  [chexp, txtl, genre_but, code, genre_ent]
		  (int but,
		   double x,
		   double y)
		     {
		       std::string ent(genre_ent->get_text());
		       ent.erase(std::remove_if(ent.begin(), ent.end(), [](auto &el)
				 {
				   return el == ' ';
				 }), ent.end());
		       if(!ent.empty() && code != "nill")
			 {
			   ent = std::string(genre_ent->get_text());
			   ent = ent + ", " + code;
			   genre_ent->set_text(Glib::ustring(ent));
			 }
		       else
			 {
			   ent = code;
			   genre_ent->set_text(Glib::ustring(ent));
			 }
		       genre_but->popdown();
		     });
	      txtl->add_controller(clck);
	      chexp_gr->attach(*txtl, 0, j, 1, 1);
	    }
	  scrl_gr->attach(*chexp, 0, i, 1, 1);
	}
    }
  int gvsz = mw->genrev->size();

  if(maxexp != nullptr)
    {
      Gtk::Requisition minreq, natreq;
      maxexp->get_preferred_size(minreq, natreq);
      Gdk::Rectangle req = mw->screenRes();
      if(natreq.get_width() < req.get_width())
	{
	  scrl->set_min_content_width(natreq.get_width());
	}
      else
	{
	  int width = natreq.get_width();
	  while(width > req.get_width())
	    {
	      width--;
	    }
	  scrl->set_min_content_width(width);
	}

      if(gvsz > 0)
	{
	  if(natreq.get_height() * gvsz < req.get_height())
	    {
	      scrl->set_min_content_height(natreq.get_height() * gvsz);
	    }
	  else
	    {
	      int height = natreq.get_height() * gvsz;
	      while(height > req.get_height())
		{
		  height = height - natreq.get_height();
		}
	      scrl->set_min_content_height(height);
	    }
	}
    }
  scrl->set_child(*scrl_gr);
  Gtk::Requisition minreq, natreq;
  maxexp->get_preferred_size(minreq, natreq);
  genre_but->set_size_request(natreq.get_width(), -1);
  grid->attach(*genre_but, 0, 9, 2, 1);

  Gtk::Label *date_lb = Gtk::make_managed<Gtk::Label>();
  date_lb->set_halign(Gtk::Align::START);
  date_lb->set_margin(5);
  date_lb->set_text(gettext("Date:"));
  grid->attach(*date_lb, 0, 10, 1, 1);

  Gtk::Entry *date_ent = Gtk::make_managed<Gtk::Entry>();
  date_ent->set_halign(Gtk::Align::START);
  date_ent->set_margin(5);
  date_ent->set_width_chars(30);
  itbv = std::find_if(bookv->begin(), bookv->end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Date";
    });
  if(itbv != bookv->end())
    {
      date_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
    }
  grid->attach(*date_ent, 0, 11, 3, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_halign(Gtk::Align::CENTER);
  save->set_margin(5);
  save->set_label(gettext("Save"));
  MainWindow *mwl = mw;
  save->signal_clicked().connect(
      [bookv, auth_ent, book_ent, series_ent, genre_ent, date_ent, mwl, window]
      {
	mwl->bookSaveRestore(window, bookv, 1);
      });
  grid->attach(*save, 0, 12, 1, 1);

  Gtk::Button *restore = Gtk::make_managed<Gtk::Button>();
  restore->set_halign(Gtk::Align::CENTER);
  restore->set_margin(5);
  restore->set_label(gettext("Restore"));
  restore->signal_clicked().connect(
      [bookv, auth_ent, book_ent, series_ent, genre_ent, date_ent, mwl, window]
      {
	mwl->bookSaveRestore(window, bookv, 2);
      });
  grid->attach(*restore, 1, 12, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 2, 12, 1, 1);

  window->signal_close_request().connect([window, bookv]
  {
    bookv->clear();
    delete bookv;
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->show();
}

void
BookOpWindows::bookSaveRestore(
    Gtk::Window *win, std::vector<std::tuple<std::string, std::string>> *bookv,
    int variant)
{
  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(win->get_child());
  Gtk::Entry *auth_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 1));
  Gtk::Entry *book_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 3));
  Gtk::Entry *series_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 5));
  Gtk::Entry *genre_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 7));
  Gtk::Entry *date_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 11));

  if(variant == 1)
    {
      std::vector<std::tuple<std::string, std::string>> *newbase =
	  new std::vector<std::tuple<std::string, std::string>>;
      std::tuple<std::string, std::string> ttup;
      std::get<0>(ttup) = "Author";
      std::get<1>(ttup) = std::string(auth_ent->get_text());
      newbase->push_back(ttup);

      std::get<0>(ttup) = "Book";
      std::get<1>(ttup) = std::string(book_ent->get_text());
      newbase->push_back(ttup);

      std::get<0>(ttup) = "Series";
      std::get<1>(ttup) = std::string(series_ent->get_text());
      newbase->push_back(ttup);

      std::get<0>(ttup) = "Genre";
      std::get<1>(ttup) = std::string(genre_ent->get_text());
      newbase->push_back(ttup);

      std::get<0>(ttup) = "Date";
      std::get<1>(ttup) = std::string(date_ent->get_text());
      newbase->push_back(ttup);

      Gtk::Window *window = new Gtk::Window;
      window->set_application(mw->get_application());
      window->set_title(gettext("Confirmation"));
      window->set_transient_for(*win);
      window->set_modal(true);
      window->set_default_size(1, 1);

      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
      grid->set_halign(Gtk::Align::FILL);
      grid->set_valign(Gtk::Align::FILL);
      window->set_child(*grid);

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_margin(5);
      lab->set_text(gettext("Are you shure?"));
      grid->attach(*lab, 0, 0, 2, 1);

      int *cncl = new int(0);
      RefreshCollection *rc = new RefreshCollection(mw->prev_search_nm, 1,
						    cncl);
      MainWindow *mwl = mw;
      Glib::Dispatcher *disp_corrected = new Glib::Dispatcher;
      disp_corrected->connect([window, win, mwl, bookv, newbase]
      {
	std::string searchstr;
	auto itbv = std::find_if(bookv->begin(), bookv->end(), []
	(auto &el)
	  {
	    return std::get<0>(el) == "filepath";
	  });
	if(itbv != bookv->end())
	  {
	    searchstr = std::get<1>(*itbv);
	  }
	auto itsv = std::find_if(mwl->search_result_v.begin(), mwl->search_result_v.end(),
      [searchstr]
      (auto &el)
	{
	  return std::get<5>(el) == searchstr;
	});
	if(itsv != mwl->search_result_v.end())
	  {
	    auto itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Author";
	      });
	    if(itnewb != newbase->end())
	      {
		std::get<0> (*itsv) = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Book";
	      });
	    if(itnewb != newbase->end())
	      {
		std::get<1> (*itsv) = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Series";
	      });
	    if(itnewb != newbase->end())
	      {
		std::get<2> (*itsv) = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Genre";
	      });
	    if(itnewb != newbase->end())
	      {
		std::get<3> (*itsv) = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Date";
	      });
	    if(itnewb != newbase->end())
	      {
		std::get<4> (*itsv) = std::get<1>(*itnewb);
	      }
	  }
	mwl->prev_search_nm.clear();
	Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
	Gtk::Paned *pn =
	    dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
	Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
	Gtk::ScrolledWindow *sres_scrl =
	    dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
	Glib::RefPtr<Gtk::Adjustment> adj = sres_scrl->get_vadjustment();
	double pos = adj->get_value();
	CreateRightGrid crgr(mwl);
	crgr.searchResultShow(1);
	Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
	while(mc->pending())
	  {
	    mc->iteration(true);
	  }
	adj->set_value(pos);

	window->unset_child();

	Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
	grid->set_halign(Gtk::Align::FILL);
	grid->set_valign(Gtk::Align::FILL);
	window->set_child(*grid);

	Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	lab->set_halign(Gtk::Align::CENTER);
	lab->set_margin(5);
	lab->set_text(gettext("Book entry has been corrected"));
	grid->attach(*lab, 0, 0, 1, 1);

	Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
	close->set_halign(Gtk::Align::CENTER);
	close->set_margin(5);
	close->set_label(gettext("Close"));
	close->signal_clicked().connect([window, win]
	{
	  window->close();
	  win->close();
	});
	grid->attach(*close, 0, 1, 1, 1);
      });

      rc->refresh_finished = [disp_corrected]
      {
	disp_corrected->emit();
      };

      Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
      yes->set_halign(Gtk::Align::CENTER);
      yes->set_margin(5);
      yes->set_label(gettext("Yes"));
      yes->signal_clicked().connect([window, mwl, bookv, newbase, rc]
      {
	window->unset_child();

	Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
	grid->set_halign(Gtk::Align::FILL);
	grid->set_valign(Gtk::Align::FILL);
	window->set_child(*grid);

	Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
	lab->set_halign(Gtk::Align::CENTER);
	lab->set_margin(5);
	lab->set_text(gettext("Correcting base..."));
	grid->attach(*lab, 0, 0, 1, 1);
	std::thread *thr = new std::thread([rc, bookv, newbase]
	{
	  rc->editBook(newbase, bookv);
	});
	thr->detach();
	delete thr;
      });
      grid->attach(*yes, 0, 1, 1, 1);

      Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
      no->set_halign(Gtk::Align::CENTER);
      no->set_margin(5);
      no->set_label(gettext("No"));
      no->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
      grid->attach(*no, 1, 1, 1, 1);

      window->signal_close_request().connect(
	  [window, newbase, disp_corrected, cncl]
	  {
	    newbase->clear();
	    delete newbase;
	    delete disp_corrected;
	    delete cncl;
	    window->hide();
	    delete window;
	    return true;
	  },
	  false);
      window->show();
    }

  else if(variant == 2)
    {
      auto itbv = std::find_if(bookv->begin(), bookv->end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Author";
	});
      if(itbv != bookv->end())
	{
	  auth_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
	}

      itbv = std::find_if(bookv->begin(), bookv->end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Book";
	});
      if(itbv != bookv->end())
	{
	  book_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
	}

      itbv = std::find_if(bookv->begin(), bookv->end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Series";
	});
      if(itbv != bookv->end())
	{
	  series_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
	}

      itbv = std::find_if(bookv->begin(), bookv->end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Genre";
	});
      if(itbv != bookv->end())
	{
	  genre_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
	}

      itbv = std::find_if(bookv->begin(), bookv->end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Date";
	});
      if(itbv != bookv->end())
	{
	  date_ent->set_text(Glib::ustring(std::get<1>(*itbv)));
	}
    }
}
