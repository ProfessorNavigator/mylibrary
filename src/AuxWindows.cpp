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
AuxWindows::errorWin(int type, Gtk::Window *par_win, Glib::Dispatcher *disp)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_title(gettext("Message"));
  window->set_transient_for(*par_win);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  window->set_child(*grid);

  Gtk::Label *warn_lab = Gtk::make_managed<Gtk::Label>();
  warn_lab->set_halign(Gtk::Align::CENTER);
  warn_lab->set_margin(5);
  if(type == 0)
    {
      warn_lab->set_text(gettext("Collection name cannot be empty!"));
    }
  if(type == 1)
    {
      warn_lab->set_text(gettext("Path to book directory is empty!"));
    }
  if(type == 2)
    {
      warn_lab->set_text(gettext("Collection has been created"));
    }
  if(type == 3)
    {
      warn_lab->set_text(gettext("Collection already existed!"));
    }
  if(type == 4)
    {
      warn_lab->set_text(gettext("Collection creation canceled"));
    }
  if(type == 5)
    {
      warn_lab->set_text(gettext("Collection to import path is empty!"));
    }
  if(type == 6)
    {
      warn_lab->set_text(gettext("Export path is empty!"));
    }
  grid->attach(*warn_lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_halign(Gtk::Align::CENTER);
  close->set_margin(5);
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(sigc::mem_fun(*window, &Gtk::Window::close));
  grid->attach(*close, 0, 1, 1, 1);

  window->signal_close_request().connect([window, disp]
  {
    delete disp;
    window->hide();
    delete window;
    return true;
  },
					 false);
  window->show();
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
  grid->attach(*scrl, 0, 0, 2, 1);

  mw->bm_tv = Gtk::make_managed<Gtk::TreeView>();
  mw->bm_tv->set_name("searchRes");
  mw->bm_tv->get_style_context()->add_provider(mw->css_provider,
  GTK_STYLE_PROVIDER_PRIORITY_USER);
  scrl->set_child(*mw->bm_tv);
  CreateRightGrid crgr(mw);
  crgr.searchResultShow(2);
  Gtk::TreeViewColumn *columni = mw->bm_tv->get_column(1);
  int x, y, h, w;
  columni->cell_get_size(x, y, w, h);
  scrl->set_min_content_height(10 * h);

  Glib::RefPtr<Gio::SimpleActionGroup> acgroup =
      Gio::SimpleActionGroup::create();
  acgroup->add_action("openbook",
		      sigc::bind(sigc::mem_fun(*mw, &MainWindow::openBook), 2));
  acgroup->add_action(
      "removebook",
      sigc::bind(sigc::mem_fun(*mw, &MainWindow::bookRemoveWin), 2, window));
  mw->bm_tv->insert_action_group("popup", acgroup);
  Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
  menu->append(gettext("Open book"), "popup.openbook");
  menu->append(gettext("Remove book from book-marks"), "popup.removebook");
  Gtk::PopoverMenu *Menu = new Gtk::PopoverMenu;
  Menu->set_parent(*mw->bm_tv);
  Menu->set_menu_model(menu);
  Menu->set_has_arrow(false);
  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect([Menu]
  (int n_pressed, double x, double y)
    {
      Gdk::Rectangle rect(x, y, 1, 1);
      Menu->set_pointing_to(rect);
      Menu->popup();
    });
  mw->bm_tv->add_controller(clck);

  Gtk::Button *o_book = Gtk::make_managed<Gtk::Button>();
  o_book->set_margin(5);
  o_book->set_halign(Gtk::Align::CENTER);
  o_book->set_label(gettext("Open selected book"));
  o_book->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*mw, &MainWindow::openBook), 2));
  grid->attach(*o_book, 0, 1, 1, 1);

  Gtk::Button *del_book = Gtk::make_managed<Gtk::Button>();
  del_book->set_margin(5);
  del_book->set_halign(Gtk::Align::CENTER);
  del_book->set_label(gettext("Remove selected book from book-marks"));
  del_book->signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*mw, &MainWindow::bookRemoveWin), 2, window));
  grid->attach(*del_book, 1, 1, 1, 1);
  MainWindow *mwl = mw;
  window->signal_close_request().connect([mwl, window, Menu]
  {
    mwl->bookmark_v.clear();
    mwl->bm_tv = nullptr;
    window->hide();
    delete window;
    delete Menu;
    return true;
  },
					 false);
  window->show();
}

void
AuxWindows::aboutProg()
{
  Gtk::AboutDialog *aboutd = new Gtk::AboutDialog;
  aboutd->set_transient_for(*mw);
  aboutd->set_application(mw->get_application());

  aboutd->set_program_name("MyLibrary");
  aboutd->set_version("1.1");
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
  abbuf = gettext("MyLibrary is a home librarian\n"
		  "Author Yury Bobylev <bobilev_yury@mail.ru>.\n"
		  "Program uses next libraries:\n"
		  "GTK https://www.gtk.org\n"
		  "libzip https://libzip.org\n"
		  "libgcrypt https://www.gnupg.org/software/libgcrypt/\n"
		  "ICU https://icu.unicode.org");
  aboutd->set_comments(abbuf);

  aboutd->signal_close_request().connect([aboutd]
  {
    aboutd->hide();
    delete aboutd;
    return true;
  },
					 false);
  aboutd->show();
}
