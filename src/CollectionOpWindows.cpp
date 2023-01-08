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

#include <CollectionOpWindows.h>

CollectionOpWindows::CollectionOpWindows(MainWindow *mw)
{
  this->mw = mw;
}

CollectionOpWindows::~CollectionOpWindows()
{
  // TODO Auto-generated destructor stub
}

void
CollectionOpWindows::collectionOp(int variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  if(variant == 1)
    {
      window->set_title(gettext("Collection removing"));
    }
  else if(variant == 2)
    {
      window->set_title(gettext("Collection refreshing"));
    }
  else if(variant == 3)
    {
      window->set_title(gettext("Book adding"));
    }
  window->set_transient_for(*mw);
  window->set_modal(true);
  window->set_default_size(1, 1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::START);
  lab->set_margin(5);
  if(variant == 1)
    {
      lab->set_text(gettext("Collection for removing:"));
    }
  else if(variant == 2)
    {
      lab->set_text(gettext("Collection for refreshing:"));
    }
  else if(variant == 3)
    {
      lab->set_text(gettext("Collection which book should be added to:"));
    }
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::ComboBoxText *cmb = Gtk::make_managed<Gtk::ComboBoxText>();
  cmb->set_halign(Gtk::Align::CENTER);
  cmb->set_margin(5);
  CreateLeftGrid clgr(mw);
  clgr.formCollCombo(cmb);
  grid->attach(*cmb, 0, 1, 2, 1);

  Gtk::CheckButton *rem_empty_ch = nullptr;
  Gtk::CheckButton *ch_pack = nullptr;
  if(variant == 2)
    {
      Gtk::Label *thr_nm_lb = Gtk::make_managed<Gtk::Label>();
      thr_nm_lb->set_halign(Gtk::Align::END);
      thr_nm_lb->set_margin(5);
      unsigned int max_thr = std::thread::hardware_concurrency();
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << max_thr;
      thr_nm_lb->set_text(
	  Glib::ustring(
	      gettext(
		  "Number of threads to use (it is not recommended to exceed "))
	      + Glib::ustring(strm.str()) + ")");
      thr_nm_lb->set_width_chars(50);
      thr_nm_lb->set_max_width_chars(50);
      thr_nm_lb->set_wrap(true);
      thr_nm_lb->set_wrap_mode(Pango::WrapMode::WORD);
      thr_nm_lb->set_justify(Gtk::Justification::RIGHT);
      grid->attach(*thr_nm_lb, 0, 2, 1, 1);

      Gtk::Entry *thr_ent = Gtk::make_managed<Gtk::Entry>();
      thr_ent->set_halign(Gtk::Align::CENTER);
      thr_ent->set_valign(Gtk::Align::CENTER);
      thr_ent->set_margin(5);
      thr_ent->set_max_width_chars(2);
      thr_ent->set_alignment(Gtk::Align::CENTER);
      thr_ent->set_input_purpose(Gtk::InputPurpose::DIGITS);
      thr_ent->set_text("1");
      grid->attach(*thr_ent, 1, 2, 1, 1);

      Gtk::Label *rem_empty_lb = Gtk::make_managed<Gtk::Label>();
      rem_empty_lb->set_halign(Gtk::Align::END);
      rem_empty_lb->set_margin(5);
      rem_empty_lb->set_text(gettext("Remove empty directories in collection"));
      rem_empty_lb->set_wrap(true);
      rem_empty_lb->set_wrap_mode(Pango::WrapMode::WORD);
      rem_empty_lb->set_max_width_chars(50);
      rem_empty_lb->set_justify(Gtk::Justification::RIGHT);
      grid->attach(*rem_empty_lb, 0, 3, 1, 1);

      rem_empty_ch = Gtk::make_managed<Gtk::CheckButton>();
      rem_empty_ch->set_halign(Gtk::Align::CENTER);
      rem_empty_ch->set_valign(Gtk::Align::CENTER);
      rem_empty_ch->set_active(false);
      grid->attach(*rem_empty_ch, 1, 3, 1, 1);
    }
  else if(variant == 3)
    {
      Gtk::Label *book_path_lb = Gtk::make_managed<Gtk::Label>();
      book_path_lb->set_halign(Gtk::Align::START);
      book_path_lb->set_margin(5);
      book_path_lb->set_text(gettext("Path to source book file:"));
      grid->attach(*book_path_lb, 0, 2, 2, 1);

      Gtk::Entry *book_path_ent = Gtk::make_managed<Gtk::Entry>();
      book_path_ent->set_halign(Gtk::Align::START);
      book_path_ent->set_valign(Gtk::Align::CENTER);
      book_path_ent->set_margin(5);
      book_path_ent->set_width_chars(50);
      book_path_ent->set_editable(false);
      book_path_ent->set_can_focus(false);
      grid->attach(*book_path_ent, 0, 3, 1, 1);

      Gtk::Button *open = Gtk::make_managed<Gtk::Button>();
      open->set_halign(Gtk::Align::CENTER);
      open->set_margin(5);
      open->set_label(gettext("Open"));
      grid->attach(*open, 1, 3, 1, 1);

      Gtk::Label *book_nm_lb = Gtk::make_managed<Gtk::Label>();
      book_nm_lb->set_halign(Gtk::Align::START);
      book_nm_lb->set_margin(5);
      book_nm_lb->set_text(gettext("Book file name in collection:"));
      grid->attach(*book_nm_lb, 0, 4, 2, 1);

      Gtk::Entry *book_nm_ent = Gtk::make_managed<Gtk::Entry>();
      book_nm_ent->set_halign(Gtk::Align::START);
      book_nm_ent->set_valign(Gtk::Align::CENTER);
      book_nm_ent->set_margin(5);
      book_nm_ent->set_width_chars(50);
      grid->attach(*book_nm_ent, 0, 5, 1, 1);

      open->signal_clicked().connect(
	  sigc::bind(sigc::mem_fun(*mw, &MainWindow::bookAddWin), window,
		     book_path_ent, book_nm_ent));

      Gtk::Label *pack_lb = Gtk::make_managed<Gtk::Label>();
      pack_lb->set_halign(Gtk::Align::END);
      pack_lb->set_margin(5);
      pack_lb->set_text(
	  gettext("Pack book to zip archive, if it is not already packed"));
      pack_lb->set_justify(Gtk::Justification::RIGHT);
      pack_lb->set_wrap(true);
      pack_lb->set_wrap_mode(Pango::WrapMode::WORD);
      pack_lb->set_max_width_chars(50);
      grid->attach(*pack_lb, 0, 6, 1, 1);

      ch_pack = Gtk::make_managed<Gtk::CheckButton>();
      ch_pack->set_halign(Gtk::Align::CENTER);
      ch_pack->set_valign(Gtk::Align::CENTER);
      ch_pack->set_active(false);
      grid->attach(*ch_pack, 1, 6, 1, 1);
    }

  MainWindow *mwl = mw;
  Gtk::Button *remove = Gtk::make_managed<Gtk::Button>();
  remove->set_halign(Gtk::Align::START);
  remove->set_margin(5);
  if(variant == 1)
    {
      remove->set_label(gettext("Remove"));
    }
  else if(variant == 2)
    {
      remove->set_label(gettext("Refresh"));
    }
  else if(variant == 3)
    {
      remove->set_label(gettext("Add book"));
    }

  if(!Glib::ustring(cmb->get_active_text()).empty() && variant == 1)
    {
      remove->signal_clicked().connect([mwl, cmb, window, variant]
      {
	CollectionOpWindows copw(mwl);
	copw.collectionOpFunc(cmb, window, nullptr, variant);
      });

    }
  else if(!Glib::ustring(cmb->get_active_text()).empty() && variant == 2)
    {
      remove->signal_clicked().connect([mwl, cmb, window, rem_empty_ch, variant]
      {
	CollectionOpWindows copw(mwl);
	copw.collectionOpFunc(cmb, window, rem_empty_ch, variant);
      });
    }
  else if(!Glib::ustring(cmb->get_active_text()).empty() && variant == 3
      && ch_pack)
    {
      remove->signal_clicked().connect(
	  sigc::bind(sigc::mem_fun(*mw, &MainWindow::bookAddWinFunc), window,
		     ch_pack));
    }

  if(variant == 2)
    {
      grid->attach(*remove, 0, 4, 1, 1);
    }
  else
    {
      if(variant == 1)
	{
	  grid->attach(*remove, 0, 2, 1, 1);
	}
      else
	{
	  grid->attach(*remove, 0, 7, 1, 1);
	}
    }

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  if(variant == 2)
    {
      grid->attach(*cancel, 1, 4, 1, 1);
    }
  else
    {
      if(variant == 1)
	{
	  grid->attach(*cancel, 1, 2, 1, 1);
	}
      else
	{
	  grid->attach(*cancel, 1, 7, 1, 1);
	}

    }

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->present();
}

void
CollectionOpWindows::collectionOpFunc(Gtk::ComboBoxText *cmb, Gtk::Window *win,
				      Gtk::CheckButton *rem_empty_ch,
				      int variant)
{
  Gtk::MessageDialog *msg = new Gtk::MessageDialog(*win,
						   gettext("Are you sure?"),
						   false,
						   Gtk::MessageType::QUESTION,
						   Gtk::ButtonsType::YES_NO,
						   true);
  msg->set_application(mw->get_application());

  MainWindow *mwl = mw;
  msg->signal_response().connect(
      [cmb, win, msg, mwl, rem_empty_ch, variant]
      (int resp)
	{
	  if(resp == Gtk::ResponseType::NO)
	    {
	      msg->close();
	    }
	  else if(resp == Gtk::ResponseType::YES)
	    {
	      if(variant == 1)
		{
		  std::string filename;
		  AuxFunc af;
		  af.homePath(&filename);
		  filename = filename + "/.MyLibrary/Collections/" +
		  std::string(cmb->get_active_text());
		  std::filesystem::path filepath = std::filesystem::u8path(filename);
		  std::filesystem::remove_all(filepath);

		  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
		  Gtk::Paned *pn =
		  dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
		  Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
		  Gtk::ComboBoxText *collect_box =
		  dynamic_cast<Gtk::ComboBoxText*>(left_gr->get_child_at(0, 1));

		  collect_box->remove_all();
		  CreateLeftGrid clgr(mwl);
		  clgr.formCollCombo(collect_box);
		  msg->close();
		  win->close();
		}
	      else if(variant == 2)
		{
		  CollectionOpWindows copw(mwl);
		  copw.collectionRefresh(cmb, rem_empty_ch,win);
		  msg->close();
		}
	    }

	}
      );

  msg->signal_close_request().connect([msg]
  {
    msg->hide();
    delete msg;
    return true;
  },
				      false);
  msg->present();
}

void
CollectionOpWindows::collectionCreate()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Collection"));
  window->set_transient_for(*mw);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *coll_nm = Gtk::make_managed<Gtk::Label>();
  coll_nm->set_halign(Gtk::Align::START);
  coll_nm->set_margin(5);
  coll_nm->set_text(gettext("Collection name:"));
  grid->attach(*coll_nm, 0, 0, 2, 1);

  Gtk::Entry *coll_ent = Gtk::make_managed<Gtk::Entry>();
  coll_ent->set_halign(Gtk::Align::FILL);
  coll_ent->set_margin(5);
  grid->attach(*coll_ent, 0, 1, 2, 1);

  Gtk::Label *coll_path = Gtk::make_managed<Gtk::Label>();
  coll_path->set_halign(Gtk::Align::START);
  coll_path->set_margin(5);
  coll_path->set_text(gettext("Path to directory with books:"));
  grid->attach(*coll_path, 0, 2, 2, 1);

  Gtk::Entry *path_ent = Gtk::make_managed<Gtk::Entry>();
  path_ent->set_halign(Gtk::Align::CENTER);
  path_ent->set_margin(5);
  path_ent->set_width_chars(50);
  path_ent->set_can_focus(false);
  path_ent->set_editable(false);
  grid->attach(*path_ent, 0, 3, 1, 1);

  Gtk::Button *opbut = Gtk::make_managed<Gtk::Button>();
  opbut->set_halign(Gtk::Align::CENTER);
  opbut->set_margin(5);
  opbut->set_label(gettext("Open"));
  MainWindow *mwl = mw;
  opbut->signal_clicked().connect([mwl, window, path_ent]
  {
    CollectionOpWindows copw(mwl);
    copw.openDialogCC(window, path_ent, 1);
  });

  grid->attach(*opbut, 1, 3, 1, 1);

  Gtk::Label *thr_nm_lb = Gtk::make_managed<Gtk::Label>();
  thr_nm_lb->set_halign(Gtk::Align::START);
  thr_nm_lb->set_margin(5);
  thr_nm_lb->set_wrap(true);
  thr_nm_lb->set_wrap_mode(Pango::WrapMode::WORD);
  thr_nm_lb->set_max_width_chars(50);
  unsigned int max_thr = std::thread::hardware_concurrency();
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << max_thr;
  thr_nm_lb->set_text(
      Glib::ustring(
	  gettext("Number of threads to use (it is not recommended to exceed "))
	  + Glib::ustring(strm.str()) + ")");
  grid->attach(*thr_nm_lb, 0, 4, 1, 1);

  Gtk::Entry *thr_ent = Gtk::make_managed<Gtk::Entry>();
  thr_ent->set_halign(Gtk::Align::CENTER);
  thr_ent->set_valign(Gtk::Align::CENTER);
  thr_ent->set_margin(5);
  thr_ent->set_max_width_chars(2);
  thr_ent->set_alignment(Gtk::Align::CENTER);
  thr_ent->set_input_purpose(Gtk::InputPurpose::DIGITS);
  thr_ent->set_text("1");
  grid->attach(*thr_ent, 1, 4, 1, 1);

  Gtk::Button *create = Gtk::make_managed<Gtk::Button>();
  create->set_halign(Gtk::Align::START);
  create->set_margin(5);
  create->set_label(gettext("Create"));
  create->signal_clicked().connect([mwl, coll_ent, path_ent, thr_ent, window]
  {
    CollectionOpWindows copw(mwl);
    copw.collectionCreateFunc(coll_ent, path_ent, thr_ent, window);
  });

  grid->attach(*create, 0, 5, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 1, 5, 1, 1);

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);

  window->present();
}

void
CollectionOpWindows::collectionCreateFunc(Gtk::Entry *coll_ent,
					  Gtk::Entry *path_ent,
					  Gtk::Entry *thr_ent,
					  Gtk::Window *par_win)
{
  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(par_win->get_child());
  Gtk::Button *cr_but = dynamic_cast<Gtk::Button*>(gr->get_child_at(0, 5));
  Gtk::Button *cncl_but = dynamic_cast<Gtk::Button*>(gr->get_child_at(1, 5));
  cr_but->set_sensitive(false);
  cncl_but->set_sensitive(false);
  par_win->set_deletable(false);
  Glib::RefPtr<Gtk::EntryBuffer> buf_coll = coll_ent->get_buffer();
  std::string coll_nm(buf_coll->get_text());
  Glib::RefPtr<Gtk::EntryBuffer> buf_path = path_ent->get_buffer();
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << std::string(thr_ent->get_text());
  unsigned int thr_num;
  strm >> thr_num;
  if(thr_num == 0)
    {
      thr_num = 1;
    }
  std::string filename(buf_path->get_text());
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  AuxWindows aw(mw);
  if(coll_nm.empty())
    {
      aw.errorWin(0, par_win);
      cr_but->set_sensitive(true);
      cncl_but->set_sensitive(true);
      par_win->set_deletable(true);
      return void();
    }
  if(filename.empty())
    {
      aw.errorWin(1, par_win);
      cr_but->set_sensitive(true);
      cncl_but->set_sensitive(true);
      par_win->set_deletable(true);
      return void();
    }
  std::shared_ptr<int> cncl = std::make_shared<int>(0);
  std::shared_ptr<CreateCollection> crcol = std::make_shared<CreateCollection>(
      coll_nm, filepath, thr_num, nullptr, cncl);
  std::shared_ptr<Glib::Dispatcher> disp_finished = std::make_shared<
      Glib::Dispatcher>();
  std::shared_ptr<Glib::Dispatcher> disp_totfiles = std::make_shared<
      Glib::Dispatcher>();
  std::shared_ptr<Glib::Dispatcher> disp_progress = std::make_shared<
      Glib::Dispatcher>();
  std::shared_ptr<Glib::Dispatcher> disp_canceled = std::make_shared<
      Glib::Dispatcher>();
  std::shared_ptr<Glib::Dispatcher> disp_colexists = std::make_shared<
      Glib::Dispatcher>();
  std::shared_ptr<int> totfiles = std::make_shared<int>(0);
  std::shared_ptr<int> progr = std::make_shared<int>(0);
  MainWindow *mwl = mw;
  disp_colexists->connect([mwl, par_win, cr_but, cncl_but]
  {
    AuxWindows aw(mwl);
    aw.errorWin(3, par_win);
    cr_but->set_sensitive(true);
    cncl_but->set_sensitive(true);
    par_win->set_deletable(true);
  });
  disp_finished->connect(
      [crcol, disp_finished, mwl, par_win, totfiles, progr, disp_canceled,
       disp_totfiles, disp_progress, disp_colexists, coll_nm]
      {
	par_win->close();

	Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
	Gtk::Paned *pn =
	    dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
	Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
	Gtk::ComboBoxText *collect_box =
	    dynamic_cast<Gtk::ComboBoxText*>(left_gr->get_child_at(0, 1));

	collect_box->remove_all();
	CreateLeftGrid clgr(mwl);
	clgr.formCollCombo(collect_box);
	collect_box->set_active_text(Glib::ustring(coll_nm));
	AuxWindows aw(mwl);
	aw.errorWin(2, mwl);
      });

  disp_canceled->connect(
      [crcol, disp_finished, mwl, par_win, totfiles, progr, disp_canceled,
       disp_totfiles, disp_progress, disp_colexists]
      {
	par_win->close();
	AuxWindows aw(mwl);
	aw.errorWin(4, mwl);
      });

  disp_totfiles->connect(
      sigc::bind(sigc::mem_fun(*mwl, &MainWindow::creationPulseWin), par_win,
		 cncl));

  disp_progress->connect(
      [progr, totfiles, mwl]
      {
	mwl->coll_cr_prog->set_fraction(
	    static_cast<double>(*progr) / static_cast<double>(*totfiles));
      });

  crcol->creation_finished = [disp_finished]
  {
    disp_finished->emit();
  };

  crcol->op_canceled = [disp_canceled]
  {
    disp_canceled->emit();
  };

  crcol->collection_exist = [disp_colexists]
  {
    disp_colexists->emit();
  };

  crcol->total_files = [totfiles, disp_totfiles]
  (int files)
    {
      *totfiles = files;
      disp_totfiles->emit();
    };

  crcol->files_added = [progr, disp_progress]
  (int prg)
    {
      *progr = prg;
      disp_progress->emit();
    };

  std::thread *thr = new std::thread([crcol]
  {
    crcol->createCol();
  });
  thr->detach();
  delete thr;
}

void
CollectionOpWindows::openDialogCC(Gtk::Window *window, Gtk::Entry *path_ent,
				  int variant)
{
  MainWindow *mwl = mw;
  Glib::ustring dnm;
  if(variant == 1)
    {
      dnm = Glib::ustring(gettext("Book directory"));
    }
  else if(variant == 2)
    {
      dnm = Glib::ustring(gettext("Path to collection"));
    }
  else if(variant == 3)
    {
      dnm = Glib::ustring(gettext("Export as..."));
    }

  Gtk::Window *fch = new Gtk::Window;
  fch->set_application(mw->get_application());
  fch->set_title(dnm);
  fch->set_transient_for(*window);
  fch->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  fch->set_child(*grid);

  Gtk::FileChooserWidget *fchw = Gtk::make_managed<Gtk::FileChooserWidget>();
  fchw->set_margin(5);
  AuxFunc af;
  std::string filename;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fchw->set_current_folder(fl);

  if(variant == 1 || variant == 2)
    {
      fchw->set_action(Gtk::FileChooser::Action::SELECT_FOLDER);
      grid->attach(*fchw, 0, 0, 2, 1);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
      cancel->set_halign(Gtk::Align::START);
      cancel->set_margin(5);
      cancel->set_label(gettext("Cancel"));
      cancel->signal_clicked().connect(
	  sigc::mem_fun(*fch, &Gtk::Window::close));
      grid->attach(*cancel, 0, 1, 1, 1);

      Gtk::Button *open = Gtk::make_managed<Gtk::Button>();
      open->set_halign(Gtk::Align::END);
      open->set_margin(5);
      open->set_label(gettext("Open"));
      open->set_name("applyBut");
      open->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      open->signal_clicked().connect([fchw, path_ent, fch]
      {
	Glib::RefPtr<Gio::File> fl = fchw->get_file();
	if(fl)
	  {
	    std::string loc = fl->get_path();
	    path_ent->set_text(Glib::ustring(loc));
	    fch->close();
	  }
      });
      grid->attach(*open, 1, 1, 1, 1);
    }
  else if(variant == 3)
    {
      fchw->set_action(Gtk::FileChooser::Action::SAVE);
      fchw->set_create_folders(true);
      fchw->set_select_multiple(false);
      grid->attach(*fchw, 0, 0, 2, 1);

      Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
      cancel->set_halign(Gtk::Align::START);
      cancel->set_margin(5);
      cancel->set_label(gettext("Cancel"));
      cancel->signal_clicked().connect(
	  sigc::mem_fun(*fch, &Gtk::Window::close));
      grid->attach(*cancel, 0, 1, 1, 1);

      Gtk::Button *export_but = Gtk::make_managed<Gtk::Button>();
      export_but->set_halign(Gtk::Align::END);
      export_but->set_margin(5);
      export_but->set_label(gettext("Export"));
      export_but->set_name("applyBut");
      export_but->get_style_context()->add_provider(mw->css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_USER);
      export_but->signal_clicked().connect([fch, fchw, path_ent, mwl]
      {
	CollectionOpWindows copw(mwl);
	copw.openDialogExportFunc(fch, fchw, path_ent);
      });
      grid->attach(*export_but, 1, 1, 1, 1);
    }

  fch->signal_close_request().connect([fch]
  {
    fch->hide();
    delete fch;
    return true;
  },
				      false);
  fch->present();
}

void
CollectionOpWindows::openDialogExportFunc(Gtk::Window *fch,
					  Gtk::FileChooserWidget *fchw,
					  Gtk::Entry *path_ent)
{
  Glib::RefPtr<Gio::File> fl = fchw->get_file();
  if(fl)
    {
      std::string filename = fl->get_path();
      std::filesystem::path p = std::filesystem::u8path(filename);
      p.make_preferred();
      if(std::filesystem::exists(p))
	{
	  Glib::ustring msg = Glib::ustring(p.u8string())
	      + Glib::ustring(gettext(" already exists. Replace?"));
	  Gtk::MessageDialog *conf = new Gtk::MessageDialog(
	      *fch, msg, false, Gtk::MessageType::QUESTION,
	      Gtk::ButtonsType::YES_NO, true);
	  conf->set_application(fch->get_application());
	  conf->signal_response().connect([conf, path_ent, fch, p]
	  (int resp_id)
	    {
	      if(resp_id == Gtk::ResponseType::NO)
		{
		  conf->close();
		}
	      else if(resp_id == Gtk::ResponseType::YES)
		{
		  path_ent->set_text(p.u8string());
		  conf->close();
		  fch->close();
		}
	    });
	  conf->signal_close_request().connect([conf]
	  {
	    conf->hide();
	    delete conf;
	    return true;
	  },
					       false);
	  conf->present();
	}
      else
	{
	  path_ent->set_text(p.u8string());
	  fch->close();
	}
    }
  else
    {
      Gtk::MessageDialog *msg = new Gtk::MessageDialog(
	  *fch, gettext("File path is not valid!"), false,
	  Gtk::MessageType::INFO, Gtk::ButtonsType::CLOSE, true);
      msg->set_application(fch->get_application());
      msg->signal_response().connect([msg]
      (int resp)
	{
	  if(resp == Gtk::ResponseType::CLOSE)
	    {
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
    }
}

void
CollectionOpWindows::collectionRefresh(Gtk::ComboBoxText *cmb,
				       Gtk::CheckButton *rem_empty_ch,
				       Gtk::Window *win)
{
  std::string coll_nm(cmb->get_active_text());
  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(win->get_child());
  Gtk::Entry *thr_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(1, 2));
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << std::string(thr_ent->get_text());
  unsigned int thr_num;
  strm >> thr_num;
  if(thr_num == 0)
    {
      thr_num = 1;
    }
  bool col_empty_ch = rem_empty_ch->get_active();
  win->close();
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Collection refreshing"));
  window->set_transient_for(*mw);
  window->set_modal(true);
  window->set_deletable(false);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Files hashing..."));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::ProgressBar *prgb = Gtk::make_managed<Gtk::ProgressBar>();
  prgb->set_halign(Gtk::Align::CENTER);
  prgb->set_margin(5);
  prgb->set_show_text(true);
  grid->attach(*prgb, 0, 1, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  std::shared_ptr<sigc::connection> con = std::make_shared<sigc::connection>();
  MainWindow *mwl = mw;
  std::shared_ptr<int> cncl = std::make_shared<int>(0);
  *con = cancel->signal_clicked().connect([cncl]
  {
    *cncl = 1;
  });
  grid->attach(*cancel, 0, 2, 1, 1);

  std::shared_ptr<RefreshCollection> rc = std::make_shared<RefreshCollection>(
      coll_nm, thr_num, cncl);

  std::shared_ptr<Glib::Dispatcher> disp_cancel = std::make_shared<
      Glib::Dispatcher>();

  disp_cancel->connect(
      [lab, con, cancel, window, mwl, prgb]
      {
	mwl->prev_search_nm.clear();
	con->disconnect();
	lab->set_label(gettext("Collection refreshing canceled"));
	Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(window->get_child());
	gr->remove(*prgb);
	cancel->set_label(gettext("Close"));
	cancel->signal_clicked().connect(
	    sigc::mem_fun(*window, &Gtk::Window::close));
      });
  rc->refresh_canceled = [disp_cancel]
  {
    disp_cancel->emit();
  };

  std::shared_ptr<Glib::Dispatcher> disp_finished = std::make_shared<
      Glib::Dispatcher>();
  disp_finished->connect(
      [lab, con, cancel, window, mwl, prgb]
      {
	mwl->prev_search_nm.clear();
	con->disconnect();
	lab->set_label(gettext("Collection refreshing finished"));
	Gtk::Grid *grid = dynamic_cast<Gtk::Grid*>(window->get_child());
	window->set_default_size(1, 1);
	grid->remove(*prgb);
	Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
	while(mc->pending())
	  {
	    mc->iteration(true);
	  }
	cancel->set_label(gettext("Close"));
	cancel->signal_clicked().connect(
	    sigc::mem_fun(*window, &Gtk::Window::close));
      });
  rc->refresh_finished = [disp_finished]
  {
    disp_finished->emit();
  };

  std::shared_ptr<long unsigned int> tothsh =
      std::make_shared<long unsigned int>(0);
  std::shared_ptr<long unsigned int> hashed =
      std::make_shared<long unsigned int>(0);
  std::shared_ptr<Glib::Dispatcher> disp_tothash = std::make_shared<
      Glib::Dispatcher>();
  disp_tothash->connect([prgb]
  {
    prgb->set_fraction(0.0);
  });
  rc->total_hash = [tothsh]
  (long unsigned int tot)
    {
      *tothsh = tot;
    };

  std::shared_ptr<Glib::Dispatcher> disp_hashed = std::make_shared<
      Glib::Dispatcher>();
  disp_hashed->connect([tothsh, hashed, prgb]
  {
    mpf_set_default_prec(128);
    mpz_class tot(*tothsh);
    mpf_class tot_mpf;
    mpf_set_z(tot_mpf.get_mpf_t(), tot.get_mpz_t());

    mpz_class hsh(*hashed);
    mpf_class hsh_mpf;
    mpf_set_z(hsh_mpf.get_mpf_t(), hsh.get_mpz_t());

    prgb->set_fraction(mpf_class(hsh_mpf / tot_mpf).get_d());
  });

  rc->byte_hashed = [hashed, disp_hashed]
  (long unsigned int hsh)
    {
      *hashed = *hashed + hsh;
      disp_hashed->emit();
    };

  std::shared_ptr<double> totf = std::make_shared<double>(0.0);
  std::shared_ptr<double> added = std::make_shared<double>(0.0);
  std::shared_ptr<Glib::Dispatcher> disp_totalfiles = std::make_shared<
      Glib::Dispatcher>();
  disp_totalfiles->connect([prgb, lab]
  {
    lab->set_text(gettext("Collection refreshing..."));
    prgb->set_fraction(0.0);
  });
  rc->total_files = [totf, disp_totalfiles]
  (int total)
    {
      *totf = static_cast<double>(total);
      disp_totalfiles->emit();
    };

  std::shared_ptr<Glib::Dispatcher> disp_progr = std::make_shared<
      Glib::Dispatcher>();
  disp_progr->connect([prgb, added]
  {
    prgb->set_fraction(*added);
  });

  rc->files_added = [added, totf, disp_progr]
  (int add)
    {
      *added = static_cast<double>(add) / *totf;
      disp_progr->emit();
    };

  std::shared_ptr<Glib::Dispatcher> disp_coll_nf = std::make_shared<
      Glib::Dispatcher>();
  disp_coll_nf->connect(
      [lab, con, cancel, window, mwl, prgb]
      {
	mwl->prev_search_nm.clear();
	con->disconnect();
	lab->set_label(gettext("Collection book directory not found"));
	Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(window->get_child());
	gr->remove(*prgb);
	cancel->set_label(gettext("Close"));
	cancel->signal_clicked().connect(
	    sigc::mem_fun(*window, &Gtk::Window::close));
      });
  rc->collection_not_exists = [disp_coll_nf]
  {
    disp_coll_nf->emit();
  };

  window->signal_close_request().connect(
      [window, disp_cancel, disp_finished, disp_tothash, disp_totalfiles,
       disp_hashed, disp_progr, disp_coll_nf]
      {
	window->hide();
	delete window;
	return true;
      },
      false);
  Gtk::Requisition min, nat;
  grid->get_preferred_size(min, nat);
  window->set_default_size(nat.get_width(), nat.get_height());
  window->present();

  std::thread *thr = new std::thread([rc, col_empty_ch]
  {
    if(col_empty_ch)
      {
	rc->removeEmptyDirs();
      }
    rc->startRefreshing();
  });
  thr->detach();
  delete thr;
}

void
CollectionOpWindows::importCollection()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Collection import"));
  window->set_transient_for(*mw);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *coll_nm_lb = Gtk::make_managed<Gtk::Label>();
  coll_nm_lb->set_halign(Gtk::Align::START);
  coll_nm_lb->set_margin(5);
  coll_nm_lb->set_text(gettext("Collection name:"));
  grid->attach(*coll_nm_lb, 0, 0, 2, 1);

  Gtk::Entry *coll_nm_ent = Gtk::make_managed<Gtk::Entry>();
  coll_nm_ent->set_halign(Gtk::Align::FILL);
  coll_nm_ent->set_margin(5);
  grid->attach(*coll_nm_ent, 0, 1, 2, 1);

  Gtk::Label *coll_path_lb = Gtk::make_managed<Gtk::Label>();
  coll_path_lb->set_halign(Gtk::Align::START);
  coll_path_lb->set_margin(5);
  coll_path_lb->set_text(gettext("Path to collection files:"));
  grid->attach(*coll_path_lb, 0, 2, 2, 1);

  Gtk::Entry *coll_path_ent = Gtk::make_managed<Gtk::Entry>();
  coll_path_ent->set_halign(Gtk::Align::FILL);
  coll_path_ent->set_margin(5);
  coll_path_ent->set_editable(false);
  coll_path_ent->set_can_focus(false);
  coll_path_ent->set_width_chars(80);
  grid->attach(*coll_path_ent, 0, 3, 1, 1);

  Gtk::Button *open_coll = Gtk::make_managed<Gtk::Button>();
  open_coll->set_halign(Gtk::Align::CENTER);
  open_coll->set_margin(5);
  open_coll->set_label(gettext("Open"));
  MainWindow *mwl = mw;
  open_coll->signal_clicked().connect([mwl, window, coll_path_ent]
  {
    CollectionOpWindows copw(mwl);
    copw.openDialogCC(window, coll_path_ent, 2);
  });
  grid->attach(*open_coll, 1, 3, 1, 1);

  Gtk::Label *book_path_lb = Gtk::make_managed<Gtk::Label>();
  book_path_lb->set_halign(Gtk::Align::START);
  book_path_lb->set_margin(5);
  book_path_lb->set_text(gettext("Path to books:"));
  grid->attach(*book_path_lb, 0, 4, 2, 1);

  Gtk::Entry *book_path_ent = Gtk::make_managed<Gtk::Entry>();
  book_path_ent->set_halign(Gtk::Align::FILL);
  book_path_ent->set_margin(5);
  book_path_ent->set_editable(false);
  book_path_ent->set_can_focus(false);
  book_path_ent->set_width_chars(80);
  grid->attach(*book_path_ent, 0, 5, 1, 1);

  Gtk::Button *open_book = Gtk::make_managed<Gtk::Button>();
  open_book->set_halign(Gtk::Align::CENTER);
  open_book->set_margin(5);
  open_book->set_label(gettext("Open"));
  open_book->signal_clicked().connect([mwl, window, book_path_ent]
  {
    CollectionOpWindows copw(mwl);
    copw.openDialogCC(window, book_path_ent, 1);
  });
  grid->attach(*open_book, 1, 5, 1, 1);

  Gtk::Button *import_coll = Gtk::make_managed<Gtk::Button>();
  import_coll->set_halign(Gtk::Align::START);
  import_coll->set_margin(5);
  import_coll->set_label(gettext("Import collection"));
  import_coll->signal_clicked().connect(
      [window, coll_nm_ent, coll_path_ent, book_path_ent, mwl]
      {
	CollectionOpWindows copw(mwl);
	copw.importCollectionFunc(window, coll_nm_ent, coll_path_ent,
				  book_path_ent);
      });
  grid->attach(*import_coll, 0, 6, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::END);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 1, 6, 1, 1);

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->present();
}

void
CollectionOpWindows::importCollectionFunc(Gtk::Window *window,
					  Gtk::Entry *coll_nm_ent,
					  Gtk::Entry *coll_path_ent,
					  Gtk::Entry *book_path_ent)
{
  AuxWindows aw(mw);
  if(coll_nm_ent->get_text().empty())
    {
      aw.errorWin(0, window);
      return void();
    }
  else
    {
      std::string filename;
      AuxFunc af;
      af.homePath(&filename);
      filename = filename + "/.MyLibrary/Collections/"
	  + std::string(coll_nm_ent->get_text());
      std::filesystem::path filepath = std::filesystem::u8path(filename);
      if(std::filesystem::exists(filepath))
	{
	  aw.errorWin(3, window);
	  return void();
	}
    }
  if(coll_path_ent->get_text().empty())
    {
      aw.errorWin(5, window);
      return void();
    }
  if(book_path_ent->get_text().empty())
    {
      aw.errorWin(1, window);
      return void();
    }
  std::string book_path(book_path_ent->get_text());
  std::string coll_nm(coll_nm_ent->get_text());
  std::string coll_path_str(coll_path_ent->get_text());

  window->unset_child();
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + coll_nm;
  std::filesystem::path coll_path = std::filesystem::u8path(filename);
  std::filesystem::create_directories(coll_path);
  filename = coll_path_str;
  std::filesystem::path src_coll_path = std::filesystem::u8path(filename);
  if(std::filesystem::exists(src_coll_path))
    {
      for(auto &dirit : std::filesystem::directory_iterator(src_coll_path))
	{
	  std::filesystem::path p = dirit.path();
	  if(!std::filesystem::is_directory(p))
	    {
	      filename = coll_path.u8string();
	      filename = filename + "/" + p.filename().u8string();
	      std::filesystem::path outpath = std::filesystem::u8path(filename);
	      std::filesystem::copy(p, outpath);
	    }
	}

      if(std::filesystem::exists(coll_path))
	{
	  for(auto &dirit : std::filesystem::directory_iterator(coll_path))
	    {
	      std::filesystem::path p = dirit.path();
	      if(!std::filesystem::is_directory(p))
		{
		  filename = p.u8string();
		  std::string::size_type n;
		  n = filename.find("base");
		  if(n != std::string::npos)
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in | std::ios_base::binary);
		      if(f.is_open())
			{
			  std::string file_str;
			  file_str.resize(std::filesystem::file_size(p));
			  f.read(&file_str[0], file_str.size());
			  f.close();
			  std::string corr = book_path;
			  corr = "<bp>" + corr + "</bp>";
			  file_str.erase(
			      0,
			      file_str.find("</bp>")
				  + std::string("</bp>").size());
			  corr = corr + file_str;
			  f.open(p, std::ios_base::out | std::ios_base::binary);
			  if(f.is_open())
			    {
			      f.write(corr.c_str(), corr.size());
			      f.close();
			    }
			}
		    }
		  n = filename.find("hash");
		  if(n != std::string::npos)
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in);
		      if(f.is_open())
			{
			  std::vector<std::string> lv;
			  while(!f.eof())
			    {
			      std::string line;
			      getline(f, line);
			      if(!line.empty())
				{
				  lv.push_back(line);
				}
			    }
			  f.close();
			  if(lv.size() > 0)
			    {
			      lv[0] = book_path;
			      f.open(
				  p,
				  std::ios_base::out | std::ios_base::binary);
			      if(f.is_open())
				{
				  for(size_t i = 0; i < lv.size(); i++)
				    {
				      std::string line = lv[i] + "\n";
				      f.write(line.c_str(), line.size());
				    }
				  f.close();
				}
			    }
			}
		    }
		}
	    }
	}

      Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
      Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
      Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
      Gtk::ComboBoxText *collect_box =
	  dynamic_cast<Gtk::ComboBoxText*>(left_gr->get_child_at(0, 1));
      collect_box->remove_all();
      CreateLeftGrid clgr(mw);
      clgr.formCollCombo(collect_box);
    }

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Collection has been imported"));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*close, 0, 1, 1, 1);

  Gtk::Requisition min, nat;
  grid->get_preferred_size(min, nat);
  window->set_default_size(nat.get_width(), nat.get_height());
}

void
CollectionOpWindows::exportCollection()
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Export collection"));
  window->set_transient_for(*mw);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *coll_nm_lab = Gtk::make_managed<Gtk::Label>();
  coll_nm_lab->set_halign(Gtk::Align::START);
  coll_nm_lab->set_margin(5);
  coll_nm_lab->set_text(gettext("Collection for export: "));
  grid->attach(*coll_nm_lab, 0, 0, 2, 1);

  Gtk::ComboBoxText *cmb = Gtk::make_managed<Gtk::ComboBoxText>();
  cmb->set_halign(Gtk::Align::CENTER);
  cmb->set_margin(5);
  CreateLeftGrid clgr(mw);
  clgr.formCollCombo(cmb);
  grid->attach(*cmb, 0, 1, 2, 1);

  Gtk::Label *ep_path_lab = Gtk::make_managed<Gtk::Label>();
  ep_path_lab->set_halign(Gtk::Align::START);
  ep_path_lab->set_margin(5);
  ep_path_lab->set_text(gettext("Export as: "));
  grid->attach(*ep_path_lab, 0, 2, 2, 1);

  Gtk::Entry *exp_path_ent = Gtk::make_managed<Gtk::Entry>();
  exp_path_ent->set_halign(Gtk::Align::FILL);
  exp_path_ent->set_margin(5);
  exp_path_ent->set_width_chars(50);
  exp_path_ent->set_editable(false);
  exp_path_ent->set_can_focus(false);
  grid->attach(*exp_path_ent, 0, 3, 1, 1);

  Gtk::Button *open = Gtk::make_managed<Gtk::Button>();
  open->set_halign(Gtk::Align::CENTER);
  open->set_margin(5);
  open->set_label(gettext("Export as..."));
  MainWindow *mwl = mw;
  open->signal_clicked().connect([mwl, window, exp_path_ent]
  {
    CollectionOpWindows copw(mwl);
    copw.openDialogCC(window, exp_path_ent, 3);
  });
  grid->attach(*open, 1, 3, 1, 1);

  Gtk::Button *confirm = Gtk::make_managed<Gtk::Button>();
  confirm->set_halign(Gtk::Align::START);
  confirm->set_margin(5);
  confirm->set_label(gettext("Export"));
  confirm->signal_clicked().connect([mwl, cmb, exp_path_ent, window]
  {
    CollectionOpWindows copw(mwl);
    copw.exportCollectionFunc(cmb, exp_path_ent, window);
  });
  grid->attach(*confirm, 0, 4, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::END);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 1, 4, 1, 1);

  window->signal_close_request().connect([window]
  {
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->present();
}

void
CollectionOpWindows::exportCollectionFunc(Gtk::ComboBoxText *cmb,
					  Gtk::Entry *exp_path_ent,
					  Gtk::Window *win)
{
  std::string filename(exp_path_ent->get_text());
  if(filename.empty())
    {
      AuxWindows aw(mw);
      aw.errorWin(6, win);
      return void();
    }
  std::string coll_nm(cmb->get_active_text());
  win->unset_child();
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  std::filesystem::path outfolder = std::filesystem::u8path(filename);
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + coll_nm;
  std::filesystem::path src_path = std::filesystem::u8path(filename);
  if(std::filesystem::exists(outfolder))
    {
      std::filesystem::remove_all(outfolder);
    }
  std::filesystem::copy(src_path, outfolder);

  win->set_default_size(1, 1);
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  win->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Collection has been exported"));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(sigc::mem_fun(*win, &Gtk::Window::close));
  grid->attach(*close, 0, 1, 1, 1);
}
