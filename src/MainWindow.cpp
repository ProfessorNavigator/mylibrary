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

#include "MainWindow.h"

MainWindow::MainWindow() :
    Gtk::ApplicationWindow()
{
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  AuxFunc af;
  std::filesystem::path p = std::filesystem::u8path(af.get_selfpath());
  css_provider->load_from_path(
      Glib::ustring(
	  std::string(p.parent_path().u8string())
	      + "/../share/MyLibrary/mainWindow.css"));
  Glib::RefPtr<Gdk::Display> disp = this->get_display();
#ifdef ML_GTK_OLD
  Gtk::StyleContext::add_provider_for_display(disp, css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
#ifndef ML_GTK_OLD
  Gtk::StyleProvider::add_provider_for_display(
      disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
  genrev = new std::vector<genres>;
  searchmtx = new std::mutex;
  mainWindow();
}

MainWindow::~MainWindow()
{
  searchmtx->lock();
  searchmtx->unlock();
  delete genrev;
  delete searchmtx;
#ifndef ML_GTK_OLD
  delete ms_sel_book;
#endif
}

void
MainWindow::mainWindow()
{
  this->set_title("MyLibrary");
  this->set_name("MLwindow");
  this->signal_realize().connect([this]
  {
    Gdk::Rectangle rec = this->screenRes();
    this->set_default_size(rec.get_width() * 0.75, rec.get_height() * 0.75);
  });

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);

  Glib::RefPtr<Gio::SimpleActionGroup> pref = Gio::SimpleActionGroup::create();
  pref->add_action("collectioncr", [this]
  {
    CollectionOpWindows copw(this);
    copw.collectionCreate();
  });
  pref->add_action("collectionrem", [this]
  {
    CollectionOpWindows copw(this);
    copw.collectionOp(1);
  });

  pref->add_action("collectionrefr", [this]
  {
    CollectionOpWindows copw(this);
    copw.collectionOp(2);
  });
  pref->add_action("bookm", [this]
  {
    AuxWindows aw(this);
    aw.bookmarkWindow();
  });
  pref->add_action("book_add", [this]
  {
    CollectionOpWindows copw(this);
    copw.collectionOp(3);
  });
  pref->add_action("about", [this]
  {
    AuxWindows aw(this);
    aw.aboutProg();
  });
  pref->add_action("collimport", [this]
  {
    CollectionOpWindows copw(this);
    copw.importCollection();
  });
  pref->add_action("collexport", [this]
  {
    CollectionOpWindows copw(this);
    copw.exportCollection();
  });
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
  pn->set_expand(true);

  CreateLeftGrid clgr(this);
  clgr.formGenreVect(genrev);
  Gtk::Grid *left_gr = clgr.formLeftGrid();
  pn->set_start_child(*left_gr);

  CreateRightGrid crgr(this);
  Gtk::Grid *right_gr = crgr.formRightGrid();

  pn->set_end_child(*right_gr);
  pn->set_resize_start_child(false);

  grid->attach(*pn, 0, 1, 1, 1);

  this->signal_close_request().connect([this]
  {
    return this->closeFunc();
  },
				       false);
  this->set_child(*grid);
}

#ifndef ML_GTK_OLD
void
MainWindow::readCollection(Gtk::DropDown *collect_box)
{
  Glib::RefPtr<ModelBoxes> mb = std::dynamic_pointer_cast<ModelBoxes>(
      collect_box->get_selected_item());
  if(mb)
    {
      std::string collnm(mb->menu_line);

      int *search_cancel = new int(0);
      SearchBook *sb = new SearchBook(collnm, "", "", "", "", "", "",
				      &prev_search_nm, &base_v,
				      &search_result_v, search_cancel);
      std::thread *thr = new std::thread([this, sb, search_cancel]
      {
	this->searchmtx->lock();
	sb->searchBook();
	this->searchmtx->unlock();
	delete search_cancel;
	delete sb;
      });
      thr->detach();
      delete thr;
    }
}
#endif

#ifdef ML_GTK_OLD
void
MainWindow::readCollection(Gtk::ComboBoxText *collect_box)
{
  std::string collnm(collect_box->get_active_text());

  int *search_cancel = new int(0);
  SearchBook *sb = new SearchBook(collnm, "", "", "", "", "", "",
				  &prev_search_nm, &base_v, &search_result_v,
				  search_cancel);

  std::thread *thr = new std::thread([this, sb, search_cancel]
  {
    this->searchmtx->lock();
    sb->searchBook();
    this->searchmtx->unlock();
    delete search_cancel;
    delete sb;
  });
  thr->detach();
  delete thr;
}
#endif

void
MainWindow::creationPulseWin(Gtk::Window *window, int *cncl)
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
  cancel->set_name("cancelBut");
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect([cncl]
  {
    *cncl = 1;
  });
  grid->attach(*cancel, 0, 2, 1, 1);
}

#ifdef ML_GTK_OLD
void
MainWindow::rowActivated(const Gtk::TreeModel::Path &path,
			 Gtk::TreeViewColumn *column)
{
  Gtk::Widget *widg = this->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(5, 7);
  Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);
  drar->set_opacity(0.0);
  cover_struct = cover_image();
  Gtk::TreeView *trv = column->get_tree_view();
  Glib::RefPtr<Gtk::TreeModel> model = trv->get_model();
  Gtk::TreeModel::iterator iter = model->get_iter(path);
  if(iter)
    {
      size_t id;
      iter->get_value(0, id);
      std::string filename = (search_result_v[id - 1]).path_to_book;
      AnnotationCover ac(filename);
      widg = right_grid->get_child_at(0, 7);
      Gtk::ScrolledWindow *annot_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
      widg = annot_scrl->get_child();
      Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);
      Glib::ustring annotation(ac.annotationRet());
      cover_struct = ac.coverRet();
      drar->set_opacity(1.0);
      drar->queue_draw();
      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
      tb->set_text("");
      tb->insert_markup(tb->begin(), annotation);
    }
}
#endif

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
  if(cover_struct.image.size() > 0)
    {
      if(cover_struct.format == "base64")
	{
	  std::string base64(cover_struct.image.begin(),
			     cover_struct.image.end());
	  base64 = Glib::Base64::decode(base64);
	  Glib::RefPtr<Glib::Bytes> bytes = Glib::Bytes::create(base64.c_str(),
								base64.size());

	  Glib::RefPtr<Gio::MemoryInputStream> strm =
	      Gio::MemoryInputStream::create();
	  strm->add_bytes(bytes);

	  auto image = Gdk::Pixbuf::create_from_stream_at_scale(strm, width,
								height, true);
	  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	  cr->rectangle(0, 0, width, height);
	  cr->fill();
	}
      else if(cover_struct.format == "rgba" || cover_struct.format == "rgb")
	{
	  guint8 *data = reinterpret_cast<guint8*>(cover_struct.image.data());
	  int ih = static_cast<int>(cover_struct.image.size())
	      / cover_struct.rowsize;
	  Glib::RefPtr<Gdk::Pixbuf> image;
	  if(cover_struct.format == "rgba")
	    {
	      int iw = cover_struct.rowsize / 4;

	      width = height * iw / ih;
	      image = Gdk::Pixbuf::create_from_data(data, Gdk::Colorspace::RGB,
						    true, 8, iw, ih,
						    cover_struct.rowsize);
	    }
	  else
	    {
	      int iw = cover_struct.rowsize / 3;

	      width = height * iw / ih;
	      image = Gdk::Pixbuf::create_from_data(data, Gdk::Colorspace::RGB,
						    false, 8, iw, ih,
						    cover_struct.rowsize);
	    }
	  image = image->scale_simple(width, height, Gdk::InterpType::BILINEAR);
	  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	  cr->rectangle(0, 0, width, height);
	  cr->fill();
	}
      else
	{
	  Glib::RefPtr<Glib::Bytes> bytes = Glib::Bytes::create(
	      cover_struct.image.data(), cover_struct.image.size());

	  Glib::RefPtr<Gio::MemoryInputStream> strm =
	      Gio::MemoryInputStream::create();
	  strm->add_bytes(bytes);

	  auto image = Gdk::Pixbuf::create_from_stream_at_scale(strm, width,
								height, true);
	  Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);
	  cr->rectangle(0, 0, width, height);
	  cr->fill();
	}
    }
}

bool
MainWindow::closeFunc()
{
  AuxFunc af;
  std::string outfolder = af.temp_path();
  outfolder = outfolder + "/MyLibraryForReading";
  std::filesystem::path ffr = std::filesystem::u8path(outfolder);
  if(std::filesystem::exists(ffr))
    {
      std::filesystem::remove_all(ffr);
    }
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
#ifdef ML_GTK_OLD
	  Gtk::ComboBoxText *collect_box =
	      dynamic_cast<Gtk::ComboBoxText*>(left_gr->get_child_at(0, 1));
	  outfolder = std::string(collect_box->get_active_text());
#endif
#ifndef ML_GTK_OLD
	  Gtk::DropDown *collect_box = dynamic_cast<Gtk::DropDown*>(left_gr
	      ->get_child_at(0, 1));
	  if(collect_box)
	    {
	      auto obj = collect_box->get_selected_item();
	      auto mod_box = std::dynamic_pointer_cast<ModelBoxes>(obj);
	      if(mod_box)
		{
		  outfolder = std::string(mod_box->menu_line);
		}
	    }
#endif
	  f.write(outfolder.c_str(), outfolder.size());
	  f.close();
	}
    }
  if(!std::filesystem::exists(resp.parent_path()))
    {
      std::filesystem::create_directories(resp.parent_path());
    }
#ifdef ML_GTK_OLD
  this->hide();
#endif
#ifndef ML_GTK_OLD
  this->set_visible(false);
#endif
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  return false;
}

#ifdef ML_GTK_OLD
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
	  filename = (search_result_v[id - 1]).path_to_book;
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
	  AuxWindows aw(this);
	  aw.errorWin(8, this);
	}
    }
}
#endif

void
MainWindow::bookAddWin(Gtk::Window *win, Gtk::Entry *book_path_ent,
		       Gtk::Entry *book_nm_ent)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fchd = Gtk::FileDialog::create();
  fchd->set_modal(true);
  fchd->set_title(gettext("Choose a book"));
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fchd->set_initial_folder(fl);

  auto fl_filter_model = Gio::ListStore<Gtk::FileFilter>::create();

  Glib::RefPtr<Gtk::FileFilter> fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(gettext("All supported"));
  fl_filter->add_pattern("*.fb2");
  fl_filter->add_pattern("*.epub");
  fl_filter->add_pattern("*.pdf");
  fl_filter->add_pattern("*.djvu");
  fl_filter->add_pattern("*.zip");
  fl_filter->add_pattern("*.rar");
  fl_filter->add_pattern("*.7z");
  fl_filter->add_pattern("*.jar");
  fl_filter->add_pattern("*.cpio");
  fl_filter->add_pattern("*.iso");
  fl_filter->add_pattern("*.a");
  fl_filter->add_pattern("*.ar");
  fl_filter->add_pattern("*.tar");
  fl_filter->add_pattern("*.tgz");
  fl_filter->add_pattern("*.tar.gz");
  fl_filter->add_pattern("*.tar.bz2");
  fl_filter->add_pattern("*.tar.xz");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".fb2");
  fl_filter->add_pattern("*.fb2");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".epub");
  fl_filter->add_pattern("*.epub");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".pdf");
  fl_filter->add_pattern("*.pdf");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".djvu");
  fl_filter->add_pattern("*.djvu");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".zip");
  fl_filter->add_pattern("*.zip");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".rar");
  fl_filter->add_pattern("*.rar");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".7z");
  fl_filter->add_pattern("*.7z");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".jar");
  fl_filter->add_pattern("*.jar");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".cpio");
  fl_filter->add_pattern("*.cpio");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".iso");
  fl_filter->add_pattern("*.iso");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".a");
  fl_filter->add_pattern("*.a");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".ar");
  fl_filter->add_pattern("*.ar");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar");
  fl_filter->add_pattern("*.tar");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tgz");
  fl_filter->add_pattern("*.tgz");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar.gz");
  fl_filter->add_pattern("*.tar.gz");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar.bz2");
  fl_filter->add_pattern("*.tar.bz2");
  fl_filter_model->append(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar.xz");
  fl_filter->add_pattern("*.tar.xz");
  fl_filter_model->append(fl_filter);

  fchd->set_filters(fl_filter_model);
  fchd->set_accept_label(gettext("Open"));

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fchd->open(*win, [book_path_ent, book_nm_ent]
  (Glib::RefPtr<Gio::AsyncResult> &result)
    {
      Glib::RefPtr<Gio::File> fl;
      auto obj = result->get_source_object_base();
      auto fchd = std::dynamic_pointer_cast<Gtk::FileDialog>(obj);
      if(!fchd)
	{
	  return void();
	}
      try
	{
	  fl = fchd->open_finish(result);
	}
      catch(Glib::Error &er)
	{
	  if(er.code() != Gtk::DialogError::DISMISSED)
	    {
	      std::cout << "bookAddWin: " << er.what() << std::endl;
	    }
	}
      if(fl)
	{
	  std::filesystem::path p;
	  p = std::filesystem::u8path(std::string(fl->get_path()));
	  book_path_ent->set_text(Glib::ustring(p.make_preferred().u8string()));
	  book_nm_ent->set_text(Glib::ustring(p.filename().u8string()));
	}
    },
	     cncl);
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fchd = new Gtk::FileChooserDialog(
      *win, gettext("Choose a book"), Gtk::FileChooser::Action::OPEN, false);
  fchd->set_application(this->get_application());
  fchd->set_modal(true);
  fchd->set_select_multiple(false);
  fchd->set_name("MLwindow");

  Gtk::Box *cont = fchd->get_content_area();
  cont->set_margin(5);

  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fchd->set_current_folder(fl);

  Glib::RefPtr<Gtk::FileFilter> fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(gettext("All supported"));
  fl_filter->add_pattern("*.fb2");
  fl_filter->add_pattern("*.epub");
  fl_filter->add_pattern("*.pdf");
  fl_filter->add_pattern("*.djvu");
  fl_filter->add_pattern("*.zip");
  fl_filter->add_pattern("*.rar");
  fl_filter->add_pattern("*.7z");
  fl_filter->add_pattern("*.jar");
  fl_filter->add_pattern("*.cpio");
  fl_filter->add_pattern("*.iso");
  fl_filter->add_pattern("*.a");
  fl_filter->add_pattern("*.ar");
  fl_filter->add_pattern("*.tar");
  fl_filter->add_pattern("*.tgz");
  fl_filter->add_pattern("*.tar.gz");
  fl_filter->add_pattern("*.tar.bz2");
  fl_filter->add_pattern("*.tar.xz");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".fb2");
  fl_filter->add_pattern("*.fb2");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".epub");
  fl_filter->add_pattern("*.epub");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".pdf");
  fl_filter->add_pattern("*.pdf");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".djvu");
  fl_filter->add_pattern("*.djvu");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".zip");
  fl_filter->add_pattern("*.zip");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".rar");
  fl_filter->add_pattern("*.rar");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".7z");
  fl_filter->add_pattern("*.7z");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".jar");
  fl_filter->add_pattern("*.jar");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".cpio");
  fl_filter->add_pattern("*.cpio");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".iso");
  fl_filter->add_pattern("*.iso");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".a");
  fl_filter->add_pattern("*.a");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".ar");
  fl_filter->add_pattern("*.ar");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar");
  fl_filter->add_pattern("*.tar");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tgz");
  fl_filter->add_pattern("*.tgz");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar.gz");
  fl_filter->add_pattern("*.tar.gz");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar.bz2");
  fl_filter->add_pattern("*.tar.bz2");
  fchd->add_filter(fl_filter);

  fl_filter = Gtk::FileFilter::create();
  fl_filter->set_name(".tar.xz");
  fl_filter->add_pattern("*.tar.xz");
  fchd->add_filter(fl_filter);

  Gtk::Button *cancel = fchd->add_button(gettext("Cancel"),
					 Gtk::ResponseType::CANCEL);
  cancel->set_name("cancelBut");
  cancel->set_margin(5);

  Gtk::Requisition min, nat;
  fchd->get_preferred_size(min, nat);

  Gtk::Button *open = fchd->add_button(gettext("Open"),
				       Gtk::ResponseType::ACCEPT);
  open->set_margin_bottom(5);
  open->set_margin_end(5);
  open->set_margin_top(5);
  open->set_margin_start(nat.get_width() - 15);
  open->set_name("applyBut");

  fchd->signal_response().connect(
      [fchd, book_path_ent, book_nm_ent]
      (int resp_id)
	{
	  if(resp_id == Gtk::ResponseType::CANCEL)
	    {
	      fchd->close();
	    }
	  else if (resp_id == Gtk::ResponseType::ACCEPT)
	    {
	      Glib::RefPtr<Gio::File> fl = fchd->get_file();
	      if(fl)
		{
		  std::filesystem::path p;
		  p = std::filesystem::u8path(std::string(fl->get_path()));
		  book_path_ent->set_text(Glib::ustring(p.make_preferred().u8string()));
		  book_nm_ent->set_text(Glib::ustring(p.filename().u8string()));
		  fchd->close();
		}
	    }
	});

  fchd->signal_close_request().connect([fchd]
  {
    fchd->hide();
    delete fchd;
    return true;
  },
				       false);
  fchd->present();
#endif
}

#ifndef ML_GTK_OLD
void
MainWindow::bookAddWinFunc(Gtk::Window *win, Gtk::DropDown *ext)
{
  auto obj = ext->get_selected_item();
  auto mod_box = std::dynamic_pointer_cast<ModelBoxes>(obj);
  std::string arch_ext;
  if(mod_box)
    {
      arch_ext = std::string(mod_box->menu_line);
    }
  else
    {
      return void();
    }
  if(arch_ext == std::string(gettext("<No>")))
    {
      arch_ext.clear();
    }
  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(win->get_child());
  Gtk::DropDown *cmb = dynamic_cast<Gtk::DropDown*>(gr->get_child_at(0, 1));
  std::string coll_name;
  obj = cmb->get_selected_item();
  mod_box = std::dynamic_pointer_cast<ModelBoxes>(obj);
  if(mod_box)
    {
      coll_name = std::string(mod_box->menu_line);
    }
  else
    {
      return void();
    }

  Gtk::Entry *book_path_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 3));
  std::string book_path(book_path_ent->get_text());

  Gtk::Entry *book_nm_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 5));
  std::string book_name(book_nm_ent->get_text());

  std::filesystem::path ch_p = std::filesystem::u8path(book_path);
  if(!book_path.empty() && !book_name.empty() && std::filesystem::exists(ch_p))
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(this->get_application());
      window->set_name("MLwindow");
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
      cancel->set_name("cancelBut");
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_margin(5);
      cancel->set_label(gettext("Cancel"));

      int *cncl = new int(0);
      RefreshCollection *rc = new RefreshCollection(coll_name, 1, false, cncl);

      sigc::connection *con = new sigc::connection();
      *con = cancel->signal_clicked().connect([cncl]
      {
	*cncl = 1;
      });
      grid->attach(*cancel, 0, 1, 1, 1);

      Glib::Dispatcher *disp_canceled = new Glib::Dispatcher();
      disp_canceled->connect([window, lab, cancel, con]
      {
	con->disconnect();
	lab->set_text(gettext("Book adding canceled"));
	cancel->set_name("applyBut");
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

      Glib::Dispatcher *disp_book_add_error = new Glib::Dispatcher();
      disp_book_add_error->connect([window, lab, cancel, con]
      {
	con->disconnect();
	lab->set_text(gettext("Book adding error!"));
	cancel->set_name("applyBut");
	cancel->set_label(gettext("Close"));
	cancel->signal_clicked().connect([window]
	{
	  window->close();
	});
      });
      rc->book_add_error = [disp_book_add_error]
      {
	disp_book_add_error->emit();
      };

      std::tuple<std::mutex*, int*> *ttup = new std::tuple<std::mutex*, int*>();
      Glib::Dispatcher *disp_book_exists = new Glib::Dispatcher();
      disp_book_exists->connect([window, this, ttup]
      {
	AuxWindows aw(this);
	aw.bookCopyConfirm(window, std::get<0>(*ttup), std::get<1>(*ttup));
      });
      rc->file_exists = [ttup, disp_book_exists]
      (std::mutex *inmtx, int *instopper)
	{
	  std::get<0>(*ttup) = inmtx;
	  std::get<1>(*ttup) = instopper;
	  disp_book_exists->emit();
	};

      Glib::Dispatcher *disp_finished = new Glib::Dispatcher();
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

      Glib::Dispatcher *disp_col_notfound = new Glib::Dispatcher();
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
	   disp_book_add_error, window, con, ttup]
	  {
	    window->set_visible(false);
	    delete disp_canceled;
	    delete disp_book_exists;
	    delete disp_finished;
	    delete disp_col_notfound;
	    delete disp_book_add_error;
	    delete con;
	    delete ttup;
	    delete window;
	    return true;
	  },
	  false);

      window->present();

      std::thread *thr = new std::thread(
	  [rc, book_path, book_name, arch_ext, cncl]
	  {
	    rc->addBook(book_path, book_name, arch_ext);
	    delete cncl;
	    delete rc;
	  });
      thr->detach();
      delete thr;
    }
  else
    {
      AuxWindows aw(this);
      if(book_path.empty())
	{
	  aw.errorWin(10, win);
	}
      else if(book_name.empty())
	{
	  aw.errorWin(11, win);
	}
      else if(!std::filesystem::exists(ch_p))
	{
	  aw.errorWin(12, win);
	}
    }
}
#endif

#ifdef ML_GTK_OLD
void
MainWindow::bookAddWinFunc(Gtk::Window *win, Gtk::ComboBoxText *ext)
{
  std::string arch_ext(ext->get_active_text());
  if(arch_ext == std::string(gettext("<No>")))
    {
      arch_ext.clear();
    }
  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(win->get_child());
  Gtk::ComboBoxText *cmb = dynamic_cast<Gtk::ComboBoxText*>(gr->get_child_at(0,
									     1));
  std::string coll_name(cmb->get_active_text());

  Gtk::Entry *book_path_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 3));
  std::string book_path(book_path_ent->get_text());

  Gtk::Entry *book_nm_ent = dynamic_cast<Gtk::Entry*>(gr->get_child_at(0, 5));
  std::string book_name(book_nm_ent->get_text());

  std::filesystem::path ch_p = std::filesystem::u8path(book_path);
  if(!book_path.empty() && !book_name.empty() && std::filesystem::exists(ch_p))
    {
      Gtk::Window *window = new Gtk::Window;
      window->set_application(this->get_application());
      window->set_name("MLwindow");
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
      cancel->set_name("cancelBut");
      cancel->set_halign(Gtk::Align::CENTER);
      cancel->set_margin(5);
      cancel->set_label(gettext("Cancel"));

      int *cncl = new int(0);
      RefreshCollection *rc = new RefreshCollection(coll_name, 1, false, cncl);

      sigc::connection *con = new sigc::connection();
      *con = cancel->signal_clicked().connect([cncl]
      {
	*cncl = 1;
      });
      grid->attach(*cancel, 0, 1, 1, 1);

      Glib::Dispatcher *disp_canceled = new Glib::Dispatcher();
      disp_canceled->connect([window, lab, cancel, con]
      {
	con->disconnect();
	lab->set_text(gettext("Book adding canceled"));
	cancel->set_name("applyBut");
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

      std::tuple<std::mutex*, int*> *ttup = new std::tuple<std::mutex*, int*>();
      Glib::Dispatcher *disp_book_exists = new Glib::Dispatcher();
      disp_book_exists->connect([window, this, ttup]
      {
	AuxWindows aw(this);
	aw.bookCopyConfirm(window, std::get<0>(*ttup), std::get<1>(*ttup));
      });
      rc->file_exists = [ttup, disp_book_exists]
      (std::mutex *inmtx, int *instopper)
	{
	  std::get<0>(*ttup) = inmtx;
	  std::get<1>(*ttup) = instopper;
	  disp_book_exists->emit();
	};

      Glib::Dispatcher *disp_finished = new Glib::Dispatcher();
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

      Glib::Dispatcher *disp_col_notfound = new Glib::Dispatcher();
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
	   window, con, ttup]
	  {
	    window->hide();
	    delete disp_canceled;
	    delete disp_book_exists;
	    delete disp_finished;
	    delete disp_col_notfound;
	    delete con;
	    delete ttup;
	    delete window;
	    return true;
	  },
	  false);

      window->present();

      std::thread *thr = new std::thread(
	  [rc, book_path, book_name, arch_ext, cncl]
	  {
	    rc->addBook(book_path, book_name, arch_ext);
	    delete cncl;
	    delete rc;
	  });
      thr->detach();
      delete thr;
    }
  else
    {
      AuxWindows aw(this);
      if(book_path.empty())
	{
	  aw.errorWin(10, win);
	}
      else if(book_name.empty())
	{
	  aw.errorWin(11, win);
	}
      else if(!std::filesystem::exists(ch_p))
	{
	  aw.errorWin(12, win);
	}
    }
}
#endif
