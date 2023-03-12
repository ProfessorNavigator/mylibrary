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

#include <AuxWindows.h>

AuxWindows::AuxWindows(MainWindow *mw)
{
  this->mw = mw;
}

AuxWindows::~AuxWindows()
{
  // TODO Auto-generated destructor stub
}

void
AuxWindows::errorWin(int type, Gtk::Window *par_win)
{
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
}

void
AuxWindows::bookmarkWindow()
{
  mw->bookmark_v.clear();
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
	      std::tuple<std::string, std::string, std::string, std::string,
		  std::string, std::string> ttup;
	      std::string val = line.substr(
		  0, line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      std::get<0>(ttup) = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      std::get<1>(ttup) = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      std::get<2>(ttup) = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      std::get<3>(ttup) = val;

	      val = line.substr(0,
				line.find("<?>") + std::string("<?>").size());
	      line.erase(0, val.size());
	      val = val.substr(0, val.find("<?>"));
	      std::get<4>(ttup) = val;

	      val = line;
	      std::get<5>(ttup) = val;
	      mw->bookmark_v.push_back(ttup);
	    }
	}
    }

  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_name("MLwindow");
  window->set_title(gettext("Book-marks"));
  window->set_transient_for(*mw);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_margin(5);
  scrl->set_halign(Gtk::Align::CENTER);
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::ALWAYS);
  Gtk::Requisition min, nat;
  mw->get_preferred_size(min, nat);
  scrl->set_min_content_width(nat.get_width() * 0.75);
  grid->attach(*scrl, 0, 0, 3, 1);

  mw->bm_tv = Gtk::make_managed<Gtk::TreeView>();
  mw->bm_tv->set_name("searchRes");
  scrl->set_child(*mw->bm_tv);
  CreateRightGrid crgr(mw);
  crgr.searchResultShow(2);
  Gtk::TreeViewColumn *columni = mw->bm_tv->get_column(1);
  int x, y, h, w;
  columni->cell_get_size(x, y, w, h);
  scrl->set_min_content_height(10 * h);

  MainWindow *mwl = mw;
  Gtk::Grid *bm_pop_grid = Gtk::make_managed<Gtk::Grid>();
  bm_pop_grid->set_halign(Gtk::Align::CENTER);
  bm_pop_grid->set_valign(Gtk::Align::CENTER);

  Gtk::Popover *bm_pop = Gtk::make_managed<Gtk::Popover>();
  bm_pop->set_name("popoverSt");
  bm_pop->set_parent(*mw->bm_tv);
  bm_pop->set_child(*bm_pop_grid);

  Gtk::Label *o_book_men = Gtk::make_managed<Gtk::Label>();
  o_book_men->set_name("menulab");
  o_book_men->set_margin(3);
  o_book_men->set_halign(Gtk::Align::CENTER);
  o_book_men->set_valign(Gtk::Align::CENTER);
  o_book_men->set_text(gettext("Open selected book"));
  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, bm_pop]
  (int num_pressed, double x, double y)
    {
      bm_pop->popdown();
      BookOpWindows bopw(mwl);
      bopw.openBook(2);
    });
  o_book_men->add_controller(clck);
  bm_pop_grid->attach(*o_book_men, 0, 0, 1, 1);

  Gtk::Label *copy_book_men = Gtk::make_managed<Gtk::Label>();
  copy_book_men->set_name("menulab");
  copy_book_men->set_margin(3);
  copy_book_men->set_halign(Gtk::Align::CENTER);
  copy_book_men->set_valign(Gtk::Align::CENTER);
  copy_book_men->set_text(gettext("Save book as..."));
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, window, bm_pop]
  (int num_pressed, double x, double y)
    {
      bm_pop->popdown();
      BookOpWindows bopw(mwl);
      bopw.copyTo(mwl->bm_tv, 2, window);
    });
  copy_book_men->add_controller(clck);
  bm_pop_grid->attach(*copy_book_men, 0, 1, 1, 1);

  Gtk::Label *del_book_men = Gtk::make_managed<Gtk::Label>();
  del_book_men->set_name("menulab");
  del_book_men->set_margin(3);
  del_book_men->set_halign(Gtk::Align::CENTER);
  del_book_men->set_valign(Gtk::Align::CENTER);
  del_book_men->set_text(gettext("Remove selected book from book-marks"));
  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect([mwl, window, bm_pop]
  (int num_pressed, double x, double y)
    {
      bm_pop->popdown();
      BookOpWindows bopw(mwl);
      bopw.bookRemoveWin(2, window, mwl->bm_tv);
    });
  del_book_men->add_controller(clck);
  bm_pop_grid->attach(*del_book_men, 0, 2, 1, 1);

  clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect([bm_pop]
  (int n_pressed, double x, double y)
    {
      Gdk::Rectangle rect(x, y, 1, 1);
      bm_pop->set_pointing_to(rect);
      bm_pop->popup();
    });
  mw->bm_tv->add_controller(clck);

  Gtk::Button *o_book = Gtk::make_managed<Gtk::Button>();
  o_book->set_name("applyBut");
  o_book->set_margin(5);
  o_book->set_halign(Gtk::Align::CENTER);
  o_book->set_label(gettext("Open selected book"));
  o_book->signal_clicked().connect([mwl]
  {
    BookOpWindows bopw(mwl);
    bopw.openBook(2);
  });
  grid->attach(*o_book, 0, 1, 1, 1);

  Gtk::Button *copy_book = Gtk::make_managed<Gtk::Button>();
  copy_book->set_name("operationBut");
  copy_book->set_margin(5);
  copy_book->set_halign(Gtk::Align::CENTER);
  copy_book->set_label(gettext("Save book as..."));
  copy_book->signal_clicked().connect([mwl, window]
  {
    BookOpWindows bopw(mwl);
    bopw.copyTo(mwl->bm_tv, 2, window);
  });
  grid->attach(*copy_book, 1, 1, 1, 1);

  Gtk::Button *del_book = Gtk::make_managed<Gtk::Button>();
  del_book->set_name("cancelBut");
  del_book->set_margin(5);
  del_book->set_halign(Gtk::Align::CENTER);
  del_book->set_label(gettext("Remove selected book from book-marks"));
  del_book->signal_clicked().connect([mwl, window]
  {
    BookOpWindows bopw(mwl);
    bopw.bookRemoveWin(2, window, mwl->bm_tv);
  });
  grid->attach(*del_book, 2, 1, 1, 1);

  window->signal_close_request().connect([mwl, window]
  {
    mwl->bookmark_v.clear();
    mwl->bm_tv = nullptr;
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->present();
}

void
AuxWindows::aboutProg()
{
  Gtk::AboutDialog *aboutd = new Gtk::AboutDialog;
  aboutd->set_application(mw->get_application());
  aboutd->set_transient_for(*mw);
  aboutd->set_name("MLwindow");

  aboutd->set_program_name("MyLibrary");
  aboutd->set_version("2.1");
  aboutd->set_copyright("Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>");
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
      size_t sz = std::filesystem::file_size(filepath);
      std::vector<char> ab;
      ab.resize(sz);
      f.read(&ab[0], ab.size());
      f.close();
      abbuf = Glib::ustring(ab.begin(), ab.end());
    }
  else
    {
      std::cerr << "Licence file not found" << std::endl;
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
      + Glib::ustring("\n"
		      "GTK https://www.gtk.org/\n"
		      "libzip https://libzip.org/\n"
		      "libarchive https://libarchive.org\n"
		      "libgcrypt https://www.gnupg.org/software/libgcrypt/\n"
		      "ICU https://icu.unicode.org/\n"
		      "GMP https://gmplib.org/\n"
		      "Poppler https://poppler.freedesktop.org/\n"
		      "DjVuLibre https://djvu.sourceforge.net/");
  aboutd->set_comments(abbuf);

  aboutd->signal_close_request().connect([aboutd]
  {
    aboutd->hide();
    delete aboutd;
    return true;
  },
					 false);
  aboutd->present();
}

void
AuxWindows::bookCopyConfirm(Gtk::Window *win, std::mutex *addbmtx, int *stopper)
{
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
}
