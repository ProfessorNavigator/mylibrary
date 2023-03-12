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

#include <CreateLeftGrid.h>

CreateLeftGrid::CreateLeftGrid(MainWindow *mw)
{
  this->mw = mw;
}

CreateLeftGrid::~CreateLeftGrid()
{
  // TODO Auto-generated destructor stub
}

Gtk::Grid*
CreateLeftGrid::formLeftGrid()
{
  Gtk::Grid *left_gr = Gtk::make_managed<Gtk::Grid>();
  left_gr->set_halign(Gtk::Align::FILL);
  left_gr->set_valign(Gtk::Align::FILL);

  Gtk::Label *collectlab = Gtk::make_managed<Gtk::Label>();
  collectlab->set_halign(Gtk::Align::CENTER);
  collectlab->set_margin(5);
  collectlab->set_text(gettext("Collection"));
  left_gr->attach(*collectlab, 0, 0, 2, 1);

  Gtk::ComboBoxText *collect_box = Gtk::make_managed<Gtk::ComboBoxText>();
  collect_box->set_name("comboBox");
  collect_box->set_halign(Gtk::Align::START);
  collect_box->set_margin(5);
  formCollCombo(collect_box);
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/CurrentCollection";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      filename.clear();
      filename.resize(std::filesystem::file_size(filepath));
      if(filename.size() > 0)
	{
	  f.read(&filename[0], filename.size());
	  collect_box->set_active_text(Glib::ustring(filename));
	}
      f.close();
    }

  mw->readCollection(collect_box);
  collect_box->signal_changed().connect(
      sigc::bind(sigc::mem_fun(*mw, &MainWindow::readCollection), collect_box));
  left_gr->attach(*collect_box, 0, 1, 2, 1);

  Gtk::Label *authlab = Gtk::make_managed<Gtk::Label>();
  authlab->set_halign(Gtk::Align::CENTER);
  authlab->set_margin(5);
  authlab->set_text(gettext("Author"));
  left_gr->attach(*authlab, 0, 2, 2, 1);

  Gtk::Label *surnamelab = Gtk::make_managed<Gtk::Label>();
  surnamelab->set_halign(Gtk::Align::START);
  surnamelab->set_margin(5);
  surnamelab->set_text(gettext("Surname:"));
  left_gr->attach(*surnamelab, 0, 3, 2, 1);

  Gtk::Entry *surname_ent = Gtk::make_managed<Gtk::Entry>();
  surname_ent->set_halign(Gtk::Align::FILL);
  surname_ent->set_margin(5);
  surname_ent->set_max_length(50);
  surname_ent->set_activates_default(true);
  left_gr->attach(*surname_ent, 0, 4, 2, 1);

  Gtk::Label *namelab = Gtk::make_managed<Gtk::Label>();
  namelab->set_halign(Gtk::Align::START);
  namelab->set_margin(5);
  namelab->set_text(gettext("Name:"));
  left_gr->attach(*namelab, 0, 5, 2, 1);

  Gtk::Entry *name_ent = Gtk::make_managed<Gtk::Entry>();
  name_ent->set_halign(Gtk::Align::FILL);
  name_ent->set_margin(5);
  name_ent->set_activates_default(true);
  left_gr->attach(*name_ent, 0, 6, 2, 1);

  Gtk::Label *secnamelab = Gtk::make_managed<Gtk::Label>();
  secnamelab->set_halign(Gtk::Align::START);
  secnamelab->set_margin(5);
  secnamelab->set_text(gettext("Second name:"));
  left_gr->attach(*secnamelab, 0, 7, 2, 1);

  Gtk::Entry *secname_ent = Gtk::make_managed<Gtk::Entry>();
  secname_ent->set_halign(Gtk::Align::FILL);
  secname_ent->set_margin(5);
  secname_ent->set_activates_default(true);
  left_gr->attach(*secname_ent, 0, 8, 2, 1);

  Gtk::Label *booklab = Gtk::make_managed<Gtk::Label>();
  booklab->set_halign(Gtk::Align::CENTER);
  booklab->set_margin(5);
  booklab->set_text(gettext("Book"));
  left_gr->attach(*booklab, 0, 9, 2, 1);

  Gtk::Label *booknmlab = Gtk::make_managed<Gtk::Label>();
  booknmlab->set_halign(Gtk::Align::START);
  booknmlab->set_margin(5);
  booknmlab->set_text(gettext("Book name:"));
  left_gr->attach(*booknmlab, 0, 10, 2, 1);

  Gtk::Entry *booknm_ent = Gtk::make_managed<Gtk::Entry>();
  booknm_ent->set_halign(Gtk::Align::FILL);
  booknm_ent->set_margin(5);
  booknm_ent->set_activates_default(true);
  left_gr->attach(*booknm_ent, 0, 11, 2, 1);

  Gtk::Label *serlab = Gtk::make_managed<Gtk::Label>();
  serlab->set_halign(Gtk::Align::START);
  serlab->set_margin(5);
  serlab->set_text(gettext("Series:"));
  left_gr->attach(*serlab, 0, 12, 2, 1);

  Gtk::Entry *ser_ent = Gtk::make_managed<Gtk::Entry>();
  ser_ent->set_halign(Gtk::Align::FILL);
  ser_ent->set_margin(5);
  ser_ent->set_activates_default(true);
  left_gr->attach(*ser_ent, 0, 13, 2, 1);

  Gtk::Label *genrelab = Gtk::make_managed<Gtk::Label>();
  genrelab->set_halign(Gtk::Align::START);
  genrelab->set_margin(5);
  genrelab->set_text(gettext("Genre:"));
  left_gr->attach(*genrelab, 0, 14, 2, 1);

  Gtk::MenuButton *genre_but = Gtk::make_managed<Gtk::MenuButton>();
  genre_but->set_name("menBut");
  genre_but->set_halign(Gtk::Align::START);
  genre_but->set_margin(5);
  genre_but->set_label(gettext("<No>"));

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  Gtk::Grid *scrl_gr = Gtk::make_managed<Gtk::Grid>();
  scrl_gr->set_halign(Gtk::Align::START);

  Gtk::Popover *popover = Gtk::make_managed<Gtk::Popover>();
  popover->set_name("popoverSt");
  popover->set_child(*scrl);
  genre_but->set_popover(*popover);

  int maxl = 0;
  Gtk::Expander *maxexp = nullptr;
  MainWindow *mwl = mw;
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
	  clck->signal_pressed().connect([txtl, genre_but, mwl]
	  (int but, double x, double y)
	    {
	      genre_but->set_label(txtl->get_text());
	      genre_but->popdown();
	      mwl->active_genre = "nill";
	    });
	  txtl->add_controller(clck);
	  scrl_gr->attach(*txtl, 0, i, 1, 1);
	}
      else
	{
	  Gtk::Expander *chexp = Gtk::make_managed<Gtk::Expander>();
	  mw->expv.push_back(chexp);
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
	      clck->signal_pressed().connect([chexp, txtl, genre_but, code, mwl]
	      (int but, double x, double y)
		{
		  genre_but->set_label(txtl->get_text());
		  genre_but->popdown();
		  mwl->active_genre = code;
		});
	      txtl->add_controller(clck);
	      chexp_gr->attach(*txtl, 0, j, 1, 1);
	    }
	  scrl_gr->attach(*chexp, 0, i, 1, 1);
	}
    }
  scrl->set_child(*scrl_gr);
  Gtk::Requisition minreq, natreq;
  maxexp->get_preferred_size(minreq, natreq);
  genre_but->set_size_request(natreq.get_width(), -1);
  left_gr->attach(*genre_but, 0, 15, 2, 1);

  Gtk::Button *searchbut = Gtk::make_managed<Gtk::Button>();
  searchbut->set_name("applyBut");
  searchbut->set_halign(Gtk::Align::CENTER);
  searchbut->set_margin(5);
  searchbut->set_label(gettext("Search"));
  searchbut->signal_clicked().connect(
      [mwl, collect_box, surname_ent, name_ent, secname_ent, booknm_ent,
       ser_ent]
      {
	BookOpWindows bopw(mwl);
	bopw.searchBook(collect_box, surname_ent, name_ent, secname_ent,
			booknm_ent, ser_ent);
      });
  left_gr->attach(*searchbut, 0, 16, 1, 1);
  mw->set_default_widget(*searchbut);

  Gtk::Button *clearbut = Gtk::make_managed<Gtk::Button>();
  clearbut->set_name("cancelBut");
  clearbut->set_halign(Gtk::Align::CENTER);
  clearbut->set_margin(5);
  clearbut->set_label(gettext("Clear"));
  clearbut->signal_clicked().connect(
      [mwl, surname_ent, name_ent, secname_ent, booknm_ent, ser_ent, genre_but]
      {
	Gtk::Widget *widg = mwl->get_child();
	Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
	widg = main_grid->get_child_at(0, 1);
	Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
	widg = pn->get_end_child();
	Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
	widg = right_grid->get_child_at(0, 0);
	Gtk::ScrolledWindow *scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
	widg = scrl->get_child();
	Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(widg);

	genre_but->set_label(gettext("<No>"));
	genre_but->popdown();
	mwl->active_genre = "nill";

	Glib::RefPtr<Gtk::TreeModel> model = sres->get_model();
	if(model)
	  {
	    sres->remove_all_columns();
	    sres->unset_model();
	  }
	surname_ent->set_text("");
	name_ent->set_text("");
	secname_ent->set_text("");
	booknm_ent->set_text("");
	ser_ent->set_text("");

	widg = right_grid->get_child_at(0, 2);
	Gtk::ScrolledWindow *annot_scrl =
	    dynamic_cast<Gtk::ScrolledWindow*>(widg);
	widg = annot_scrl->get_child();
	Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);

	Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
	tb->set_text("");

	widg = right_grid->get_child_at(1, 2);
	Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);

	drar->set_opacity(0.0);

	mwl->search_result_v.clear();
      });
  left_gr->attach(*clearbut, 1, 16, 1, 1);

  int gvsz = mw->genrev->size();

  mw->signal_show().connect([mwl, maxexp, scrl, gvsz]
  {
    if(maxexp != nullptr)
      {
	Gtk::Requisition minreq, natreq;
	maxexp->get_preferred_size(minreq, natreq);
	Gdk::Rectangle req = mwl->screenRes();
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
  });

  return left_gr;
}

void
CreateLeftGrid::formCollCombo(Gtk::ComboBoxText *combo)
{
  std::vector<std::filesystem::path> coll_vect;
  AuxFunc af;
  std::string filename;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(std::filesystem::exists(filepath))
    {
      for(auto &dirit : std::filesystem::directory_iterator(filepath))
	{
	  std::filesystem::path p = dirit.path();
	  coll_vect.push_back(p);
	}
    }
  std::sort(coll_vect.begin(), coll_vect.end(), []
  (auto &el1, auto &el2)
    {
      std::string one = el1.filename().u8string();
      std::string two = el2.filename().u8string();
      return one < two;
    });
  for(size_t i = 0; i < coll_vect.size(); i++)
    {
      std::string item = coll_vect[i].filename().u8string();
      combo->append(Glib::ustring(item));
    }
  if(coll_vect.size() > 0)
    {
      combo->set_active(0);
    }
}

void
CreateLeftGrid::formGenreVect(
    std::vector<
	std::tuple<std::string,
	    std::vector<std::tuple<std::string, std::string>>>> *genrev)
{
  std::tuple<std::string, std::vector<std::tuple<std::string, std::string>>> genrevtup;
  std::string genstr(gettext("Science Fiction & Fantasy"));
  std::vector<std::tuple<std::string, std::string>> genre;
  std::tuple<std::string, std::string> ttup;
  std::get<0>(genrevtup) = std::string(gettext("<No>"));
  std::get<1>(genrevtup) = genre;
  genrev->push_back(genrevtup);
  std::get<0>(ttup) = "sf_history";
  std::get<1>(ttup) = std::string(gettext("Alternative history"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_action";
  std::get<1>(ttup) = std::string(gettext("Action"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_epic";
  std::get<1>(ttup) = std::string(gettext("Epic"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_heroic";
  std::get<1>(ttup) = std::string(gettext("Heroic"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_detective";
  std::get<1>(ttup) = std::string(gettext("Detective"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_cyberpunk";
  std::get<1>(ttup) = std::string(gettext("Cyberpunk"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_space";
  std::get<1>(ttup) = std::string(gettext("Space"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_social";
  std::get<1>(ttup) = std::string(gettext("Social-philosophical"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_horror";
  std::get<1>(ttup) = std::string(gettext("Horror & mystic"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_humor";
  std::get<1>(ttup) = std::string(gettext("Humor"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf_fantasy";
  std::get<1>(ttup) = std::string(gettext("Fantasy"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sf";
  std::get<1>(ttup) = std::string(gettext("Science Fiction"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Detectives & Thrillers"));
  std::get<0>(ttup) = "det_classic";
  std::get<1>(ttup) = std::string(gettext("Classical detectives"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_police";
  std::get<1>(ttup) = std::string(gettext("Police Stories"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_action";
  std::get<1>(ttup) = std::string(gettext("Action"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_irony";
  std::get<1>(ttup) = std::string(gettext("Ironical detectives"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_history";
  std::get<1>(ttup) = std::string(gettext("Historical detectives"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_espionage";
  std::get<1>(ttup) = std::string(gettext("Espionage detectives"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_crime";
  std::get<1>(ttup) = std::string(gettext("Crime detectives"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_political";
  std::get<1>(ttup) = std::string(gettext("Political detectives"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_maniac";
  std::get<1>(ttup) = std::string(gettext("Maniacs"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "det_hard";
  std::get<1>(ttup) = std::string(gettext("Hard-boiled"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "thriller";
  std::get<1>(ttup) = std::string(gettext("Thrillers"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "detective";
  std::get<1>(ttup) = std::string(gettext("Detectives"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Prose"));
  std::get<0>(ttup) = "prose_classic";
  std::get<1>(ttup) = std::string(gettext("Classics prose"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "prose_history";
  std::get<1>(ttup) = std::string(gettext("Historical prose"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "prose_contemporary";
  std::get<1>(ttup) = std::string(gettext("Contemporary prose"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "prose_counter";
  std::get<1>(ttup) = std::string(gettext("Counterculture"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "prose_rus_classic";
  std::get<1>(ttup) = std::string(gettext("Russial classics prose"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "prose_su_classics";
  std::get<1>(ttup) = std::string(gettext("Soviet classics prose"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Romance"));
  std::get<0>(ttup) = "love_contemporary";
  std::get<1>(ttup) = std::string(gettext("Contemporary Romance"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "love_history";
  std::get<1>(ttup) = std::string(gettext("Historical Romance"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "love_detective";
  std::get<1>(ttup) = std::string(gettext("Detective Romance"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "love_short";
  std::get<1>(ttup) = std::string(gettext("Short Romance"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "love_erotica";
  std::get<1>(ttup) = std::string(gettext("Erotica"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Adventure"));
  std::get<0>(ttup) = "adv_western";
  std::get<1>(ttup) = std::string(gettext("Western"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "adv_history";
  std::get<1>(ttup) = std::string(gettext("History"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "adv_indian";
  std::get<1>(ttup) = std::string(gettext("Indians"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "adv_maritime";
  std::get<1>(ttup) = std::string(gettext("Maritime Fiction"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "adv_geo";
  std::get<1>(ttup) = std::string(gettext("Travel & geography"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "adv_animal";
  std::get<1>(ttup) = std::string(gettext("Nature & animals"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "adventure";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Children's"));
  std::get<0>(ttup) = "child_tale";
  std::get<1>(ttup) = std::string(gettext("Fairy Tales"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "child_verse";
  std::get<1>(ttup) = std::string(gettext("Verses"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "child_prose";
  std::get<1>(ttup) = std::string(gettext("Prose"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "child_sf";
  std::get<1>(ttup) = std::string(gettext("Science Fiction"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "child_det";
  std::get<1>(ttup) = std::string(gettext("Detectives & Thrillers"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "child_adv";
  std::get<1>(ttup) = std::string(gettext("Adventures"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "child_education";
  std::get<1>(ttup) = std::string(gettext("Educational"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "children";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Poetry & Dramaturgy"));
  std::get<0>(ttup) = "poetry";
  std::get<1>(ttup) = std::string(gettext("Poetry"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "dramaturgy";
  std::get<1>(ttup) = std::string(gettext("Dramaturgy"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Antique literature"));
  std::get<0>(ttup) = "antique_ant";
  std::get<1>(ttup) = std::string(gettext("Antique"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "antique_european";
  std::get<1>(ttup) = std::string(gettext("European"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "antique_russian";
  std::get<1>(ttup) = std::string(gettext("Old russian"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "antique_east";
  std::get<1>(ttup) = std::string(gettext("Old east"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "antique_myths";
  std::get<1>(ttup) = std::string(gettext("Myths. Legends. Epos."));
  genre.push_back(ttup);
  std::get<0>(ttup) = "antique";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Scientific-educational"));
  std::get<0>(ttup) = "sci_history";
  std::get<1>(ttup) = std::string(gettext("History"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_psychology";
  std::get<1>(ttup) = std::string(gettext("Psychology"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_culture";
  std::get<1>(ttup) = std::string(gettext("Cultural science"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_religion";
  std::get<1>(ttup) = std::string(gettext("Religious studies"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_philosophy";
  std::get<1>(ttup) = std::string(gettext("Philosophy"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_politics";
  std::get<1>(ttup) = std::string(gettext("Politics"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_business";
  std::get<1>(ttup) = std::string(gettext("Business literature"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_juris";
  std::get<1>(ttup) = std::string(gettext("Jurisprudence"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_linguistic";
  std::get<1>(ttup) = std::string(gettext("Linguistics"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_medicine";
  std::get<1>(ttup) = std::string(gettext("Medicine"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_phys";
  std::get<1>(ttup) = std::string(gettext("Physics"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_math";
  std::get<1>(ttup) = std::string(gettext("Mathematics"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_chem";
  std::get<1>(ttup) = std::string(gettext("Chemistry"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_biology";
  std::get<1>(ttup) = std::string(gettext("Biology"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "sci_tech";
  std::get<1>(ttup) = std::string(gettext("Technical"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "science";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Computers & Internet"));
  std::get<0>(ttup) = "comp_www";
  std::get<1>(ttup) = std::string(gettext("Internet"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "comp_programming";
  std::get<1>(ttup) = std::string(gettext("Programming"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "comp_hard";
  std::get<1>(ttup) = std::string(gettext("Hardware"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "comp_soft";
  std::get<1>(ttup) = std::string(gettext("Software"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "comp_db";
  std::get<1>(ttup) = std::string(gettext("Databases"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "comp_osnet";
  std::get<1>(ttup) = std::string(gettext("OS & Networking"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "computers";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Reference"));
  std::get<0>(ttup) = "ref_encyc";
  std::get<1>(ttup) = std::string(gettext("Encyclopedias"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "ref_dict";
  std::get<1>(ttup) = std::string(gettext("Dictionaries"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "ref_ref";
  std::get<1>(ttup) = std::string(gettext("Reference"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "ref_guide";
  std::get<1>(ttup) = std::string(gettext("Guidebooks"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "reference";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Nonfiction"));
  std::get<0>(ttup) = "nonf_biography";
  std::get<1>(ttup) = std::string(gettext("Biography & Memoirs"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "nonf_publicism";
  std::get<1>(ttup) = std::string(gettext("Publicism"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "nonf_criticism";
  std::get<1>(ttup) = std::string(gettext("Criticism"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "design";
  std::get<1>(ttup) = std::string(gettext("Art & design"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "nonfiction";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Religion & Inspirationn"));
  std::get<0>(ttup) = "religion_rel";
  std::get<1>(ttup) = std::string(gettext("Religion"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "religion_esoterics";
  std::get<1>(ttup) = std::string(gettext("Esoterics"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "religion_self";
  std::get<1>(ttup) = std::string(gettext("Self-improvement"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "religion";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Humor"));
  std::get<0>(ttup) = "humor_anecdote";
  std::get<1>(ttup) = std::string(gettext("Anecdote (funny stories)"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "humor_prose";
  std::get<1>(ttup) = std::string(gettext("Prose"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "humor_verse";
  std::get<1>(ttup) = std::string(gettext("Verses"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "humor";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);

  genstr = std::string(gettext("Home & Family"));
  std::get<0>(ttup) = "home_cooking";
  std::get<1>(ttup) = std::string(gettext("Cooking"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_pets";
  std::get<1>(ttup) = std::string(gettext("Pets"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_crafts";
  std::get<1>(ttup) = std::string(gettext("Hobbies & Crafts"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_entertain";
  std::get<1>(ttup) = std::string(gettext("Entertaining"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_health";
  std::get<1>(ttup) = std::string(gettext("Health"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_garden";
  std::get<1>(ttup) = std::string(gettext("Garden"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_diy";
  std::get<1>(ttup) = std::string(gettext("Do it yourself"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_sport";
  std::get<1>(ttup) = std::string(gettext("Sports"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home_sex";
  std::get<1>(ttup) = std::string(gettext("Erotica & sex"));
  genre.push_back(ttup);
  std::get<0>(ttup) = "home";
  std::get<1>(ttup) = std::string(gettext("Other"));
  genre.push_back(ttup);
  std::get<0>(genrevtup) = genstr;
  std::get<1>(genrevtup) = genre;
  genre.clear();
  genrev->push_back(genrevtup);
}
