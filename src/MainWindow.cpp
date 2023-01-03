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

#include "MainWindow.h"

MainWindow::MainWindow() :
    Gtk::ApplicationWindow()
{
  css_provider = Gtk::CssProvider::create();
  AuxFunc af;
  std::filesystem::path p = std::filesystem::u8path(af.get_selfpath());
  css_provider->load_from_path(
      Glib::ustring(
	  std::string(p.parent_path().u8string())
	      + "/../share/MyLibrary/mainWindow.css"));
  genrev =
      new std::vector<
	  std::tuple<std::string,
	      std::vector<std::tuple<std::string, std::string>>>>;
  searchmtx = new std::mutex;
  mainWindow();
}

MainWindow::~MainWindow()
{
  searchmtx->lock();
  searchmtx->unlock();
  delete genrev;
  delete searchmtx;
}

void
MainWindow::mainWindow()
{
  this->set_title("MyLibrary");
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_vexpand(true);

  Glib::RefPtr<Gio::SimpleActionGroup> pref = Gio::SimpleActionGroup::create();
  pref->add_action("collectioncr",
		   sigc::mem_fun(*this, &MainWindow::collectionCreate));
  pref->add_action(
      "collectionrem",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::collectionOp), 1));
  pref->add_action(
      "collectionrefr",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::collectionOp), 2));
  pref->add_action("bookm", sigc::mem_fun(*this, &MainWindow::bookmarkWindow));
  pref->add_action(
      "book_add",
      sigc::bind(sigc::mem_fun(*this, &MainWindow::collectionOp), 3));
  pref->add_action("about", sigc::mem_fun(*this, &MainWindow::aboutProg));
  pref->add_action("collimport",
		   sigc::mem_fun(*this, &MainWindow::importCollection));
  pref->add_action("collexport",
		   sigc::mem_fun(*this, &MainWindow::exportCollection));
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();
  this->insert_action_group("prefgr", pref);
  builder->add_from_resource("/mainmenu/mainmenu.xml");

  Gtk::Box *box = Gtk::make_managed<Gtk::Box>();
  auto object = builder->get_object("menubar");
  auto gmenu = std::dynamic_pointer_cast<Gio::Menu>(object);
  if(!gmenu)
    {
      std::cerr << "GMenu not found" << std::endl;
    }
  else
    {
      Gtk::PopoverMenuBar *menbar = Gtk::make_managed<Gtk::PopoverMenuBar>(
	  gmenu);
      menbar->set_halign(Gtk::Align::START);
      box->append(*menbar);
    }
  box->set_halign(Gtk::Align::START);
  grid->attach(*box, 0, 0, 1, 1);

  Gtk::Paned *pn = Gtk::make_managed<Gtk::Paned>();
  pn->set_halign(Gtk::Align::FILL);
  pn->set_valign(Gtk::Align::FILL);

  CreateLeftGrid clgr(this);
  clgr.formGenreVect(genrev);
  Gtk::Grid *left_gr = clgr.formLeftGrid();
  pn->set_start_child(*left_gr);

  CreateRightGrid crgr(this);
  Gtk::Grid *right_gr = crgr.formRightGrid();

  pn->set_end_child(*right_gr);
  pn->set_resize_start_child(false);
  pn->set_resize_end_child(true);
  pn->set_vexpand(true);

  grid->attach(*pn, 0, 1, 1, 1);

  this->signal_close_request().connect(
      sigc::mem_fun(*this, &MainWindow::closeFunc), false);
  this->set_child(*grid);
}

void
MainWindow::readCollection(Gtk::ComboBoxText *collect_box)
{
  std::string collnm(collect_box->get_active_text());

  std::shared_ptr<SearchBook> sb = std::make_shared<SearchBook>(
      collnm, "", "", "", "", "", "", &prev_search_nm, &base_v,
      &search_result_v, &search_cancel);
  std::thread *thr = new std::thread([this, sb]
  {
    this->searchmtx->lock();
    sb->searchBook();
    this->searchmtx->unlock();
  });
  thr->detach();
  delete thr;
}

void
MainWindow::collectionCreate()
{
  CollectionOpWindows copw(this);
  copw.collectionCreate();
}

void
MainWindow::collectionCreateFunc(Gtk::Entry *coll_ent, Gtk::Entry *path_ent,
				 Gtk::Entry *thr_ent, Gtk::Window *par_win)
{
  CollectionOpWindows copw(this);
  copw.collectionCreateFunc(coll_ent, path_ent, thr_ent, par_win);
}

void
MainWindow::creationPulseWin(Gtk::Window *window, std::shared_ptr<int> cncl)
{
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  window->unset_default_widget();
  window->unset_child();
  window->set_title(gettext("Collection creation progress"));
  window->set_default_size(-1, -1);
  window->set_deletable(false);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Creation progress"));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::ProgressBar *prg = Gtk::make_managed<Gtk::ProgressBar>();
  prg->set_halign(Gtk::Align::CENTER);
  prg->set_margin(5);
  prg->set_show_text(true);
  grid->attach(*prg, 0, 1, 1, 1);
  coll_cr_prog = prg;

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect([cncl]
  {
    *cncl = 1;
  });
  grid->attach(*cancel, 0, 2, 1, 1);
}

void
MainWindow::collectionOp(int variant)
{
  CollectionOpWindows copw(this);
  copw.collectionOp(variant);
}

void
MainWindow::collectionOpFunc(Gtk::ComboBoxText *cmb, Gtk::Window *win,
			     Gtk::CheckButton *rem_empty_ch, int variant)
{
  CollectionOpWindows copw(this);
  copw.collectionOpFunc(cmb, win, rem_empty_ch, variant);
}

void
MainWindow::errorWin(int type, Gtk::Window *par_win)
{
  AuxWindows aw(this);
  aw.errorWin(type, par_win);
}

void
MainWindow::openDialogCC(Gtk::Window *window, Gtk::Entry *path_ent, int variant)
{
  CollectionOpWindows copw(this);
  copw.openDialogCC(window, path_ent, variant);
}

void
MainWindow::searchBook(Gtk::ComboBoxText *coll_nm, Gtk::Entry *surname_ent,
		       Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
		       Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent)
{
  BookOpWindows bopw(this);
  bopw.searchBook(coll_nm, surname_ent, name_ent, secname_ent, booknm_ent,
		  ser_ent);
}

void
MainWindow::rowActivated(const Gtk::TreeModel::Path &path,
			 Gtk::TreeViewColumn *column)
{
  cover_image.clear();
  Gtk::Widget *widg = this->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(1, 2);
  Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);
  drar->set_opacity(0.0);
  Gtk::TreeView *trv = column->get_tree_view();
  Glib::RefPtr<Gtk::TreeModel> model = trv->get_model();
  Gtk::TreeModel::iterator iter = model->get_iter(path);
  if(iter)
    {
      size_t id;
      iter->get_value(0, id);
      std::string filename = std::get<5>(search_result_v[id - 1]);
      AnnotationCover ac(filename);
      widg = right_grid->get_child_at(0, 2);
      Gtk::ScrolledWindow *annot_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
      widg = annot_scrl->get_child();
      Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);
      Glib::ustring annotation(ac.annotationRet());
      cover_image = ac.coverRet();
      std::string::size_type n;
      n = cover_image.find("<epub>");
      if(n == std::string::npos)
	{
	  n = cover_image.find("<pdf>");
	  if(!cover_image.empty() && n == std::string::npos)
	    {
	      n = cover_image.find("<djvu>");
	      if(!cover_image.empty() && n == std::string::npos)
		{
		  cover_image = Glib::Base64::decode(cover_image);
		  drar->set_opacity(1.0);
		  drar->queue_draw();
		}
	      else
		{
		  if(!cover_image.empty())
		    {
		      drar->set_opacity(1.0);
		      drar->queue_draw();
		    }
		}
	    }
	  else
	    {
	      cover_image.erase(0, std::string("<pdf>").size());
	      cover_image_path = cover_image;
	      cover_image.clear();
	      if(!cover_image_path.empty())
		{
		  drar->set_opacity(1.0);
		  drar->queue_draw();
		}
	    }
	}
      else
	{
	  cover_image.erase(0, std::string("<epub>").size());
	  cover_image_path = cover_image;
	  cover_image.clear();
	  if(!cover_image_path.empty())
	    {
	      drar->set_opacity(1.0);
	      drar->queue_draw();
	    }
	}
      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
      tb->set_text("");
      tb->insert_markup(tb->begin(), annotation);
    }
}

Gdk::Rectangle
MainWindow::screenRes()
{
  Glib::RefPtr<Gdk::Surface> surf = this->get_surface();
  Glib::RefPtr<Gdk::Display> disp = this->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);
  return req;
}

void
MainWindow::drawCover(const Cairo::RefPtr<Cairo::Context> &cr, int width,
		      int height)
{
  Gtk::Widget *widg = this->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(1, 2);
  Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);
  if(!cover_image.empty())
    {
      std::string::size_type n_djvu;
      n_djvu = cover_image.find("<djvu>");
      if(n_djvu == std::string::npos)
	{
	  AuxFunc af;
	  std::string filename;
#ifdef __linux
	  filename = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
	  filename =
	      std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	  filename = filename + "/" + af.randomFileName() + "/cover";
	  std::filesystem::path filepath = std::filesystem::u8path(filename);
	  if(std::filesystem::exists(filepath.parent_path()))
	    {
	      std::filesystem::remove_all(filepath.parent_path());
	    }
	  std::filesystem::create_directories(filepath.parent_path());
	  std::fstream f;
	  f.open(filepath, std::ios_base::out | std::ios_base::binary);
	  if(f.is_open())
	    {
	      Gtk::Requisition min, nat;
	      drar->get_preferred_size(min, nat);
	      f.write(cover_image.c_str(), cover_image.size());
	      f.close();
	      auto image = Gdk::Pixbuf::create_from_file(filepath.u8string(),
							 nat.get_width(),
							 nat.get_height(),
							 true);
	      Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	      cr->rectangle(0, 0, nat.get_width(), nat.get_height());
	      cr->fill();
	    }
	  std::filesystem::remove_all(filepath.parent_path());
	}
      else
	{
	  cover_image.erase(0, n_djvu + std::string("<djvu>").size());
	  std::string param = cover_image.substr(
	      0, cover_image.find("</djvu>") + std::string("</djvu>").size());
	  cover_image.erase(0, param.size());
	  param = param.substr(0, param.find("</djvu>"));
	  int rowsize;
	  std::memcpy(&rowsize, &param[0], sizeof(rowsize));
	  param.erase(0, sizeof(rowsize));

	  int iw;
	  std::memcpy(&iw, &param[0], sizeof(iw));
	  param.erase(0, sizeof(iw));

	  int ih;
	  std::memcpy(&ih, &param[0], sizeof(ih));
	  param.erase(0, sizeof(ih));

	  std::vector<unsigned char> vect;
	  auto vp = &vect;
	  std::for_each(cover_image.begin(), cover_image.end(), [vp]
	  (auto &el)
	    {
	      vp->push_back(static_cast<unsigned char>(el));
	    });

	  auto image = Gdk::Pixbuf::create_from_data(&vect[0],
						     Gdk::Colorspace::RGB,
						     false, 8, iw, ih, rowsize);
	  image = image->scale_simple(width, height, Gdk::InterpType::BILINEAR);
	  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	  cr->rectangle(0, 0, width, height);
	  cr->fill();
	}
    }
  else
    {
      if(!cover_image_path.empty())
	{
	  Gtk::Requisition min, nat;
	  drar->get_preferred_size(min, nat);
	  auto image = Gdk::Pixbuf::create_from_file(cover_image_path,
						     nat.get_width(),
						     nat.get_height(), true);
	  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	  cr->rectangle(0, 0, nat.get_width(), nat.get_height());
	  cr->fill();
	  std::filesystem::path filepath = std::filesystem::u8path(
	      cover_image_path);
	  if(std::filesystem::exists(filepath.parent_path()))
	    {
	      std::filesystem::remove_all(filepath.parent_path());
	    }
	}
    }
  cover_image_path.clear();
  cover_image.clear();
}

void
MainWindow::openBook(int variant)
{
  Glib::RefPtr<Gtk::TreeSelection> selection;
  if(variant == 1)
    {
      Gtk::Widget *widg = this->get_child();
      Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
      widg = main_grid->get_child_at(0, 1);
      Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
      widg = pn->get_end_child();
      Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
      widg = right_grid->get_child_at(0, 0);
      Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
      widg = sres_scrl->get_child();
      Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(widg);
      selection = sres->get_selection();
    }
  else if(variant == 2)
    {
      selection = bm_tv->get_selection();
    }
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  size_t id;
	  AuxFunc af;
	  std::filesystem::path filepath;
	  iter->get_value(0, id);
	  std::string filename;
	  if(variant == 1)
	    {
	      filename = std::get<5>(search_result_v[id - 1]);
	    }
	  else if(variant == 2)
	    {
	      filename = std::get<5>(bookmark_v[id - 1]);
	    }
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
	      std::string outfolder;
#ifdef __linux
	      outfolder = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
	      outfolder =
	      		  std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	      outfolder = outfolder + "/MyLibraryForReading";
	      std::filesystem::path ffr = std::filesystem::u8path(outfolder);
	      if(std::filesystem::exists(ffr))
		{
		  std::filesystem::remove_all(ffr);
		}
	      af.unpackByIndex(archpath, outfolder, index);
	      if(std::filesystem::exists(ffr))
		{
		  for(auto &dirit : std::filesystem::directory_iterator(ffr))
		    {
		      std::filesystem::path p = dirit.path();
		      if(!std::filesystem::is_directory(p))
			{
			  filepath = p;
			  break;
			}
		    }
		}
	    }
	  else
	    {
	      filepath = std::filesystem::u8path(filename);
	    }
	  if(std::filesystem::exists(filepath))
	    {
	      std::string commstr;
#ifdef __linux
	      commstr = "xdg-open \'" + filepath.u8string() + "\'";
#endif
#ifdef _WIN32
	      commstr = filepath.u8string();
#endif
	      commstr = af.utf8to(commstr);
#ifdef __linux
	      int check = std::system(commstr.c_str());
	      std::cout << "Book open command result code: " << check
		  << std::endl;
#endif
#ifdef _WIN32
	      ShellExecuteA (0, af.utf8to ("open").c_str (), commstr.c_str (), 0, 0, 0);
#endif
	    }
	}
    }
}

void
MainWindow::copyTo(Gtk::TreeView *sres, int variant, Gtk::Window *win)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  size_t id;
	  AuxFunc af;
	  std::filesystem::path filepath;
	  bool archive = false;
	  iter->get_value(0, id);
	  std::string filename;
	  if(variant == 1)
	    {
	      filename = std::get<5>(search_result_v[id - 1]);
	    }
	  else if(variant == 2)
	    {
	      filename = std::get<5>(bookmark_v[id - 1]);
	    }
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
	      std::string outfolder;
#ifdef __linux
	      outfolder = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
	      outfolder =
		  std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	      outfolder = outfolder + "/" + af.randomFileName();
	      std::filesystem::path ffr = std::filesystem::u8path(outfolder);
	      if(std::filesystem::exists(ffr))
		{
		  std::filesystem::remove_all(ffr);
		}
	      af.unpackByIndex(archpath, outfolder, index);
	      if(std::filesystem::exists(ffr))
		{
		  for(auto &dirit : std::filesystem::directory_iterator(ffr))
		    {
		      std::filesystem::path p = dirit.path();
		      if(!std::filesystem::is_directory(p))
			{
			  filepath = p;
			  break;
			}
		    }
		}
	      archive = true;
	    }
	  else if(!filename.empty())
	    {
	      filepath = std::filesystem::u8path(filename);
	    }

	  if(std::filesystem::exists(filepath))
	    {
	      saveDialog(filepath, archive, win);
	    }
	}
    }
}

void
MainWindow::saveDialog(std::filesystem::path filepath, bool archive,
		       Gtk::Window *win)
{
  std::shared_ptr<Gtk::Window> window = std::make_shared<Gtk::Window>();
  window->set_application(this->get_application());
  window->set_title(gettext("Save as..."));
  window->set_transient_for(*win);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Box *nm_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL,
						 0);
  nm_box->set_halign(Gtk::Align::FILL);
  nm_box->set_valign(Gtk::Align::FILL);
  nm_box->set_hexpand(true);
  grid->attach(*nm_box, 0, 0, 3, 1);

  Gtk::Label *nm_lb = Gtk::make_managed<Gtk::Label>();
  nm_lb->set_margin(5);
  nm_lb->set_halign(Gtk::Align::START);
  nm_lb->set_valign(Gtk::Align::FILL);
  nm_lb->set_text(gettext("Name:"));
  nm_lb->set_hexpand(false);
  nm_box->append(*nm_lb);

  Gtk::Entry *nm_ent = Gtk::make_managed<Gtk::Entry>();
  nm_ent->set_margin(5);
  nm_ent->set_halign(Gtk::Align::FILL);
  nm_ent->set_valign(Gtk::Align::FILL);
  nm_ent->set_hexpand(true);
  nm_ent->set_text(Glib::ustring(filepath.filename().u8string()));
  nm_box->append(*nm_ent);

  Gtk::FileChooserWidget *fchw = Gtk::make_managed<Gtk::FileChooserWidget>();
  fchw->set_action(Gtk::FileChooser::Action::SAVE);
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fchw->set_current_folder(fl);
  fchw->set_current_name(Glib::ustring(filepath.filename().u8string()));
  fchw->set_select_multiple(false);
  fchw->set_margin(5);
  grid->attach(*fchw, 0, 0, 2, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::START);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*cancel, 0, 1, 1, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_halign(Gtk::Align::END);
  save->set_margin(5);
  save->set_label(gettext("Save"));
  save->set_name("applyBut");
  save->get_style_context()->add_provider(this->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  save->signal_clicked().connect(
      [window, fchw, filepath, this, win]
      {
	Glib::RefPtr<Gio::File> fl = fchw->get_file();
	std::string loc;
	if(fl)
	  {
	    loc = fl->get_path();
	    std::filesystem::path outpath = std::filesystem::u8path(loc);
	    if(!std::filesystem::exists(outpath))
	      {
		std::filesystem::copy(filepath, outpath);
		window->close();
		this->errorWin(9, win);
	      }
	    else
	      {
		Glib::ustring msgtxt = Glib::ustring(outpath.u8string())
		    + Glib::ustring(gettext(" already exists. Repalce?"));
		std::shared_ptr<Gtk::MessageDialog> msg = std::make_shared<
		    Gtk::MessageDialog>(*window, msgtxt, false,
					Gtk::MessageType::QUESTION,
					Gtk::ButtonsType::YES_NO, true);

		msg->signal_response().connect(
		    [msg, window, this, outpath, filepath, win]
		    (int resp)
		      {
			if(resp == Gtk::ResponseType::NO)
			  {
			    msg->close();
			  }
			else if(resp == Gtk::ResponseType::YES)
			  {
			    std::filesystem::remove_all(outpath);
			    std::filesystem::copy(filepath, outpath);
			    msg->close();
			    window->close();
			    this->errorWin(9, win);
			  }
		      });

		msg->signal_close_request().connect([msg]
		{
		  msg->hide();
		  return true;
		},
						    false);
		msg->present();
	      }
	  }
	else
	  {
	    std::shared_ptr<Gtk::MessageDialog> msg = std::make_shared<
		Gtk::MessageDialog>(*window, gettext("File path is not valid!"),
				    false, Gtk::MessageType::INFO,
				    Gtk::ButtonsType::CLOSE, true);

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
	      return true;
	    },
						false);
	    msg->present();
	  }
      });
  grid->attach(*save, 1, 1, 1, 1);

  window->signal_close_request().connect([window, archive, filepath]
  {
    if(archive)
      {
	if(std::filesystem::exists(filepath.parent_path()))
	  {
	    std::filesystem::remove_all(filepath.parent_path());
	  }
      }
    window->hide();
    return true;
  },
					 false);

  window->present();
}

bool
MainWindow::closeFunc()
{
  std::string outfolder;
#ifdef __linux
  outfolder = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
  outfolder =
      std::filesystem::temp_directory_path().parent_path().u8string();
#endif
  outfolder = outfolder + "/MyLibraryForReading";
  std::filesystem::path ffr = std::filesystem::u8path(outfolder);
  if(std::filesystem::exists(ffr))
    {
      std::filesystem::remove_all(ffr);
    }
  AuxFunc af;
  af.homePath(&outfolder);
  outfolder = outfolder + "/.MyLibrary/CurrentCollection";
  ffr = std::filesystem::u8path(outfolder);
  outfolder = ffr.parent_path().u8string();
  outfolder = outfolder + "/MWSize";
  std::filesystem::path resp = std::filesystem::u8path(outfolder);
  if(std::filesystem::exists(ffr.parent_path()))
    {
      std::fstream f;
      f.open(ffr, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
	{
	  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(this->get_child());
	  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0,
									     1));
	  Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
	  Gtk::ComboBoxText *collect_box =
	      dynamic_cast<Gtk::ComboBoxText*>(left_gr->get_child_at(0, 1));
	  outfolder = std::string(collect_box->get_active_text());
	  f.write(outfolder.c_str(), outfolder.size());
	  f.close();
	}
    }
  if(!std::filesystem::exists(resp.parent_path()))
    {
      std::filesystem::create_directories(resp.parent_path());
    }
  this->hide();
  return true;
}

void
MainWindow::bookRemoveWin(int variant, Gtk::Window *win)
{
  BookOpWindows bopw(this);
  bopw.bookRemoveWin(variant, win);
}

void
MainWindow::fileInfo()
{
  BookOpWindows bopw(this);
  bopw.fileInfo();
}

void
MainWindow::editBook()
{
  BookOpWindows bopw(this);
  bopw.editBook();
}

void
MainWindow::bookSaveRestore(
    Gtk::Window *win, std::vector<std::tuple<std::string, std::string>> *bookv,
    int variant)
{
  BookOpWindows bopw(this);
  bopw.bookSaveRestore(win, bookv, variant);
}

void
MainWindow::createBookmark()
{
  Gtk::Widget *widg = this->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(0, 0);
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
  widg = sres_scrl->get_child();
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(widg);
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  size_t id;
	  AuxFunc af;
	  std::string filename;
	  af.homePath(&filename);
	  filename = filename + "/.MyLibrary/Bookmarks";
	  std::filesystem::path filepath = std::filesystem::u8path(filename);
	  iter->get_value(0, id);
	  filename = std::get<5>(search_result_v[id - 1]);
	  std::string bookline;
	  Glib::ustring val;
	  iter->get_value(1, val);
	  bookline = std::string(val);
	  iter->get_value(2, val);
	  bookline = bookline + "<?>" + std::string(val);
	  iter->get_value(3, val);
	  bookline = bookline + "<?>" + std::string(val);
	  iter->get_value(4, val);
	  bookline = bookline + "<?>" + std::string(val);
	  iter->get_value(5, val);
	  bookline = bookline + "<?>" + std::string(val);
	  bookline = bookline + "<?>" + filename;
	  bookline = bookline + "<?L>";
	  std::fstream f;
	  f.open(
	      filepath,
	      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
	  f.write(bookline.c_str(), bookline.size());
	  f.close();
	  errorWin(8, this);
	}
    }
}

void
MainWindow::bookmarkWindow()
{
  AuxWindows aw(this);
  aw.bookmarkWindow();
}

void
MainWindow::bookAddWin(Gtk::Window *win, Gtk::Entry *book_path_ent,
		       Gtk::Entry *book_nm_ent)
{
  std::shared_ptr<Gtk::Dialog> fch = std::make_shared<Gtk::Dialog>(
      gettext("Choose a book"), *win, true, false);
  fch->set_application(this->get_application());

  Gtk::FileChooserWidget *fchw = Gtk::make_managed<Gtk::FileChooserWidget>();
  fchw->set_margin(5);
  fchw->set_action(Gtk::FileChooser::Action::OPEN);

  Gtk::Box *box = fch->get_content_area();
  box->append(*fchw);

  Glib::RefPtr<Gtk::FileFilter> fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(gettext("All supported"));
  fl_filter->add_pattern("*.fb2");
  fl_filter->add_pattern("*.zip");
  fl_filter->add_pattern("*.epub");
  fl_filter->add_pattern("*.pdf");
  fl_filter->add_pattern("*.djvu");
  fchw->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".fb2");
  fl_filter->add_pattern("*.fb2");
  fchw->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".zip");
  fl_filter->add_pattern("*.zip");
  fchw->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".epub");
  fl_filter->add_pattern("*.epub");
  fchw->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".pdf");
  fl_filter->add_pattern("*.pdf");
  fchw->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".djvu");
  fl_filter->add_pattern("*.djvu");
  fchw->add_filter(fl_filter);

  fchw->set_select_multiple(false);

  Gtk::Button *cancel = fch->add_button(gettext("Cancel"),
					Gtk::ResponseType::CANCEL);
  cancel->set_margin(5);

  Gtk::Button *open = fch->add_button(gettext("Open"),
				      Gtk::ResponseType::ACCEPT);
  Gtk::Requisition min, nat;
  fch->get_preferred_size(min, nat);
  open->set_margin_bottom(5);
  open->set_margin_end(5);
  open->set_margin_top(5);
  open->set_margin_start(nat.get_width() - 15);
  open->set_name("applyBut");
  open->get_style_context()->add_provider(this->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);

  fch->signal_response().connect(
      [fch, book_path_ent, book_nm_ent, fchw]
      (int resp)
	{
	  if(resp == Gtk::ResponseType::ACCEPT)
	    {
	      Glib::RefPtr<Gio::File>fl = fchw->get_file();
	      if(fl)
		{
		  book_path_ent->set_text(Glib::ustring(fl->get_path()));
		  std::string ch(book_nm_ent->get_text());
		  ch.erase(std::remove_if(ch.begin(), ch.end(), [](auto &el)
			    {
			      return el == ' ';
			    }), ch.end());
		  if(ch.empty())
		    {
		      std::filesystem::path p = std::filesystem::u8path(std::string(fl->get_path()));
		      book_nm_ent->set_text(Glib::ustring(p.filename().u8string()));
		    }
		}
	      fch->close();
	    }
	  else if (resp == Gtk::ResponseType::CANCEL)
	    {
	      fch->close();
	    }
	});

  fch->signal_close_request().connect([fch]
  {
    fch->hide();
    return true;
  },
				      false);
  fch->present();
}

void
MainWindow::bookAddWinFunc(Gtk::Window *win, Gtk::CheckButton *ch_pack)
{
  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(win->get_child());
  Gtk::ComboBoxText *cmb = dynamic_cast<Gtk::ComboBoxText*>(gr->get_child_at(0,
									     1));
  std::string coll_name(cmb->get_active_text());

  Gtk::Entry *book_path_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 3));
  std::string book_path(book_path_ent->get_text());

  Gtk::Entry *book_nm_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 5));
  std::string book_name(book_nm_ent->get_text());

  bool pack = ch_pack->get_active();

  std::shared_ptr<Gtk::Window> window = std::make_shared<Gtk::Window>();
  window->set_application(this->get_application());
  window->set_title(gettext("Books adding..."));
  window->set_transient_for(*win);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);
  window->set_deletable(false);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Book adding in progress..."));
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));

  std::shared_ptr<int> cncl = std::make_shared<int>();
  std::shared_ptr<RefreshCollection> rc = std::make_shared<RefreshCollection>(
      coll_name, 1, cncl);

  std::shared_ptr<sigc::connection> con = std::make_shared<sigc::connection>();
  *con = cancel->signal_clicked().connect([cncl]
  {
    *cncl = 1;
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  std::shared_ptr<Glib::Dispatcher> disp_canceled = std::make_shared<
      Glib::Dispatcher>();
  disp_canceled->connect([window, lab, cancel, con]
  {
    con->disconnect();
    lab->set_text(gettext("Book adding canceled"));
    cancel->set_label(gettext("Close"));
    cancel->signal_clicked().connect([window]
    {
      window->close();
    });
  });
  rc->refresh_canceled = [disp_canceled]
  {
    disp_canceled->emit();
  };

  std::shared_ptr<std::tuple<std::mutex*, int*>> ttup = std::make_shared<
      std::tuple<std::mutex*, int*>>();
  std::shared_ptr<Glib::Dispatcher> disp_book_exists = std::make_shared<
      Glib::Dispatcher>();
  disp_book_exists->connect([window, this, ttup]
  {
    this->bookCopyConfirm(window.get(), std::get<0>(*ttup), std::get<1>(*ttup));
  });
  rc->file_exists = [ttup, disp_book_exists]
  (std::mutex *inmtx, int *instopper)
    {
      std::get<0>(*ttup) = inmtx;
      std::get<1>(*ttup) = instopper;
      disp_book_exists->emit();
    };

  std::shared_ptr<Glib::Dispatcher> disp_finished = std::make_shared<
      Glib::Dispatcher>();
  disp_finished->connect([window, lab, cancel, con, this, win]
  {
    con->disconnect();
    lab->set_text(gettext("Book adding finished"));
    cancel->set_label(gettext("Close"));
    cancel->signal_clicked().connect([window, win]
    {
      window->close();
      win->close();
    });
    this->prev_search_nm.clear();
  });
  rc->refresh_finished = [disp_finished]
  {
    disp_finished->emit();
  };

  std::shared_ptr<Glib::Dispatcher> disp_col_notfound = std::make_shared<
      Glib::Dispatcher>();
  disp_col_notfound->connect([con, lab, cancel, window]
  {
    con->disconnect();
    lab->set_text(gettext("Collection book path does not exists!"));
    cancel->set_label(gettext("Close"));
    cancel->signal_clicked().connect([window]
    {
      window->close();
    });
  });

  rc->collection_not_exists = [disp_col_notfound]
  {
    disp_col_notfound->emit();
  };

  window->signal_close_request().connect(
      [disp_finished, disp_canceled, disp_col_notfound, disp_book_exists,
       window]
      {
	window->hide();
	return true;
      },
      false);

  window->present();

  std::thread *thr = new std::thread([rc, book_path, book_name, pack]
  {
    rc->addBook(book_path, book_name, pack);
  });
  thr->detach();
  delete thr;

}

void
MainWindow::bookCopyConfirm(Gtk::Window *win, std::mutex *addbmtx, int *stopper)
{
  AuxWindows aw(this);
  aw.bookCopyConfirm(win, addbmtx, stopper);
}

void
MainWindow::importCollection()
{
  CollectionOpWindows copw(this);
  copw.importCollection();
}

void
MainWindow::importCollectionFunc(Gtk::Window *window, Gtk::Entry *coll_nm_ent,
				 Gtk::Entry *coll_path_ent,
				 Gtk::Entry *book_path_ent)
{
  CollectionOpWindows copw(this);
  copw.importCollectionFunc(window, coll_nm_ent, coll_path_ent, book_path_ent);
}

void
MainWindow::exportCollection()
{
  CollectionOpWindows copw(this);
  copw.exportCollection();
}

void
MainWindow::exportCollectionFunc(Gtk::ComboBoxText *cmb,
				 Gtk::Entry *exp_path_ent, Gtk::Window *win)
{
  CollectionOpWindows copw(this);
  copw.exportCollectionFunc(cmb, exp_path_ent, win);
}

void
MainWindow::aboutProg()
{
  AuxWindows aw(this);
  aw.aboutProg();
}
