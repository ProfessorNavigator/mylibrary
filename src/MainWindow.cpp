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
  Glib::ustring menubuf =
      "<interface>"
	  "<menu id='menubar'>"
	  "     <submenu>"
	  "         <attribute name='label'>"
	  + Glib::ustring(gettext("Collection")) + "</attribute>"
	      "              <section>"
	      "                 <item>"
	      "                     <attribute name='label'>"
	  + Glib::ustring(gettext("Create collection"))
	  + "</attribute>"
	      "                     <attribute name='action'>prefgr.collectioncr</attribute>"
	      "                 </item>"
	      "                 <item>"
	      "                      <attribute name='label'>"
	  + Glib::ustring(gettext("Refresh collection"))
	  + "			     </attribute>"
	      "			     <attribute name='action'>prefgr.collectionrefr</attribute>"
	      "			</item>"
	      "                 <item>"
	      "                      <attribute name='label'>"
	  + Glib::ustring(gettext("Import collection")) + "			     </attribute>"
	      "			     <attribute name='action'>prefgr.collimport</attribute>"
	      "			</item>"
	      "                 <item>"
	      "                      <attribute name='label'>"
	  + Glib::ustring(gettext("Export collection")) + "			     </attribute>"
	      "			     <attribute name='action'>prefgr.collexport</attribute>"
	      "			</item>"
	      "                 <item>"
	      "                      <attribute name='label'>"
	  + Glib::ustring(gettext("Add book to collection"))
	  + "			     </attribute>"
	      "			     <attribute name='action'>prefgr.book_add</attribute>"
	      "			</item>"
	      "    		<item>"
	      "		             <attribute name='label'>"
	  + Glib::ustring(gettext("Remove collection"))
	  + "</attribute>"
	      "                      <attribute name='action'>prefgr.collectionrem</attribute>"
	      "			</item>"
	      "              </section>"
	      "     </submenu>"
	      "	    <submenu>"
	      "         <attribute name='label'>"
	  + Glib::ustring(gettext("Book-marks")) + "</attribute>"
	      "              <section>"
	      "                 <item>"
	      "                     <attribute name='label'>"
	  + Glib::ustring(gettext("Show book-marks"))
	  + "</attribute>"
	      "                     <attribute name='action'>prefgr.bookm</attribute>"
	      "                 </item>"
	      "              </section>"
	      "     </submenu>"
	      "	    <submenu>"
	      "         <attribute name='label'>"
	  + Glib::ustring(gettext("About")) + "</attribute>"
	      "              <section>"
	      "                 <item>"
	      "                     <attribute name='label'>"
	  + Glib::ustring(gettext("About program"))
	  + "</attribute>"
	      "                     <attribute name='action'>prefgr.about</attribute>"
	      "                 </item>"
	      "              </section>"
	      "     </submenu>"
	      "</menu>"
	      "</interface>";
  builder->add_from_string(menubuf);

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
  SearchBook *sb = new SearchBook(collnm, "", "", "", "", "", "",
				  &prev_search_nm, &base_v, &search_result_v,
				  &search_cancel);
  std::thread *thr = new std::thread([this, sb]
  {
    this->searchmtx->lock();
    sb->searchBook();
    this->searchmtx->unlock();
    delete sb;
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
				 Gtk::Window *par_win)
{
  CollectionOpWindows copw(this);
  copw.collectionCreateFunc(coll_ent, path_ent, par_win);
}

void
MainWindow::creationPulseWin(Gtk::Window *window)
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
  cancel->signal_clicked().connect([this]
  {
    this->coll_cr_cancel = 1;
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
			     int variant)
{
  CollectionOpWindows copw(this);
  copw.collectionOpFunc(cmb, win, variant);
}

void
MainWindow::errorWin(int type, Gtk::Window *par_win, Glib::Dispatcher *disp)
{
  AuxWindows aw(this);
  aw.errorWin(type, par_win, disp);
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
  widg = right_grid->get_child_at(1, 1);
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
      widg = right_grid->get_child_at(0, 1);
      Gtk::ScrolledWindow *annot_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
      widg = annot_scrl->get_child();
      Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);
      Glib::ustring annotation(ac.annotationRet());
      cover_image = ac.coverRet();
      std::string::size_type n;
      n = cover_image.find("<epub>");
      if(n == std::string::npos)
	{
	  if(!cover_image.empty())
	    {
	      cover_image = Glib::Base64::decode(cover_image);
	      drar->set_opacity(1.0);
	      drar->queue_draw();
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
  widg = right_grid->get_child_at(1, 1);
  Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);
  if(!cover_image.empty())
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
						     nat.get_height(), true);
	  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	  cr->rectangle(0, 0, nat.get_width(), nat.get_height());
	  cr->fill();
	}
      std::filesystem::remove_all(filepath.parent_path());
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
  if(variant == 2)
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
	  if(variant == 2)
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
	      std::cout << "Command result code: " << check << std::endl;
#endif
#ifdef _WIN32
	      ShellExecuteA (0, af.utf8to ("open").c_str (), commstr.c_str (), 0, 0, 0);
#endif
	    }
	}
    }
}

void
MainWindow::copyTo()
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
	  std::filesystem::path filepath;
	  bool archive = false;
	  iter->get_value(0, id);
	  std::string filename = std::get<5>(search_result_v[id - 1]);
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
	  else
	    {
	      filepath = std::filesystem::u8path(filename);
	    }

	  if(std::filesystem::exists(filepath))
	    {
	      saveDialog(filepath, archive);
	    }
	}
    }
}

void
MainWindow::saveDialog(std::filesystem::path filepath, bool archive)
{
  Glib::RefPtr<Gtk::FileChooserNative> fch = Gtk::FileChooserNative::create(
      gettext("Save as..."), *this, Gtk::FileChooser::Action::SAVE,
      gettext("Save"), gettext("Cancel"));
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fch->set_current_folder(fl);
  fch->set_current_name(Glib::ustring(filepath.filename().u8string()));
  fch->signal_response().connect([filepath, fch, archive]
  (int resp)
    {
      if(resp == Gtk::ResponseType::ACCEPT)
	{
	  Glib::RefPtr<Gio::File>fl = fch->get_file();
	  std::string loc = fl->get_path();
	  std::filesystem::path outpath = std::filesystem::u8path(loc);
	  if(std::filesystem::exists(outpath))
	    {
	      std::filesystem::remove_all(outpath);
	    }
	  std::filesystem::copy(filepath, outpath);
	}
      if(archive)
	{
	  std::filesystem::remove_all(filepath.parent_path());
	}
    });
  fch->show();
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
MainWindow::collectionRefresh(Gtk::ComboBoxText *cmb, Gtk::Window *win1,
			      Gtk::Window *win2)
{
  CollectionOpWindows copw(this);
  copw.collectionRefresh(cmb, win1, win2);
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
MainWindow::bookAddWin(Gtk::ComboBoxText *cmb, Gtk::Window *win)
{
  std::string coll_name(cmb->get_active_text());
  win->unset_child();
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  win->set_title("Books adding...");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  win->set_child(*grid);
  win->set_deletable(false);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_margin(5);
  lab->set_text(gettext("Book adding in progress..."));
  grid->attach(*lab, 0, 0, 1, 1);

  int *cncl = new int(0);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  sigc::connection *con = new sigc::connection;
  *con = cancel->signal_clicked().connect([cncl]
  {
    *cncl = 1;
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  Gtk::Requisition min, nat;
  grid->get_preferred_size(min, nat);
  win->set_default_size(nat.get_width(), nat.get_height());

  RefreshCollection *rc = new RefreshCollection(coll_name, cncl);

  Glib::Dispatcher *disp_canceled = new Glib::Dispatcher;
  disp_canceled->connect([win, lab, cancel, con]
  {
    con->disconnect();
    lab->set_text(gettext("Book adding canceled"));
    cancel->set_label(gettext("Close"));
    cancel->signal_clicked().connect([win]
    {
      win->close();
    });
  });
  rc->refresh_canceled = [disp_canceled]
  {
    disp_canceled->emit();
  };

  Glib::Dispatcher *disp_finished = new Glib::Dispatcher;
  disp_finished->connect([win, lab, cancel, con]
  {
    con->disconnect();
    lab->set_text(gettext("Book adding finished"));
    cancel->set_label(gettext("Close"));
    cancel->signal_clicked().connect([win]
    {
      win->close();
    });
  });
  rc->refresh_finished = [disp_finished]
  {
    disp_finished->emit();
  };

  win->signal_close_request().connect([rc, disp_finished, disp_canceled, cncl]
  {
    delete disp_finished;
    delete disp_canceled;
    delete cncl;
    return true;
  },
				      false);

  while(mc->pending())
    {
      mc->iteration(true);
    }

  Glib::RefPtr<Gtk::FileChooserNative> fch = Gtk::FileChooserNative::create(
      gettext("Choose a book"), *win, Gtk::FileChooser::Action::OPEN,
      gettext("Open"), gettext("Cancel"));
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fch->set_current_folder(fl);
  fch->set_select_multiple(false);
  Glib::RefPtr<Gtk::FileFilter> fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".fb2, .epub, .zip");
  fl_filter->add_pattern("*.fb2");
  fl_filter->add_pattern("*.zip");
  fl_filter->add_pattern("*.epub");
  fch->add_filter(fl_filter);
  fch->signal_response().connect([fch, rc, win]
  (int resp)
    {
      if(resp == Gtk::ResponseType::ACCEPT)
	{
	  Glib::RefPtr<Gio::File> fl = fch->get_file();
	  if(fl)
	    {
	      std::thread *thr = new std::thread([rc, fl]
		    {
		      rc->addBook(fl->get_path());
		    });
	      thr->detach();
	      delete thr;
	    }
	  else
	    {
	      win->close();
	    }
	}
      else
	{
	  win->close();
	}
    });
  fch->show();
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
