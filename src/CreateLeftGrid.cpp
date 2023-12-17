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

#include <CreateLeftGrid.h>

CreateLeftGrid::CreateLeftGrid(MainWindow *mw)
{
  this->mw = mw;
}

CreateLeftGrid::~CreateLeftGrid()
{

}

Gtk::Grid*
CreateLeftGrid::formLeftGrid()
{
  MainWindow *mwl = mw;

  Gtk::Grid *left_gr = Gtk::make_managed<Gtk::Grid>();
  left_gr->set_halign(Gtk::Align::FILL);
  left_gr->set_valign(Gtk::Align::FILL);
  left_gr->set_expand(true);

  Gtk::Label *collectlab = Gtk::make_managed<Gtk::Label>();
  collectlab->set_halign(Gtk::Align::CENTER);
  collectlab->set_margin(5);
  collectlab->set_text(gettext("Collection"));
  left_gr->attach(*collectlab, 0, 0, 2, 1);

#ifdef ML_GTK_OLD
  Gtk::ComboBoxText *collect_box = Gtk::make_managed<Gtk::ComboBoxText>();
  collect_box->set_name("comboBox");
  collect_box->set_halign(Gtk::Align::CENTER);
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
      std::bind(&MainWindow::readCollection, mw, collect_box));
#endif
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gio::ListStore<ModelBoxes>> men_mod;
  men_mod = formCollCombo();
  auto men_expr = Gtk::ClosureExpression<Glib::ustring>::create([]
  (const Glib::RefPtr<Glib::ObjectBase> &item)
    {
      const auto mod_box = std::dynamic_pointer_cast<ModelBoxes>(item);
      if(mod_box)
	{
	  return mod_box->menu_line;
	}
      else
	{
	  return Glib::ustring("");
	}
    });
  Gtk::DropDown *collect_box = Gtk::make_managed<Gtk::DropDown>(men_mod,
								men_expr);
  collect_box->set_name("comboBox");
  collect_box->set_halign(Gtk::Align::CENTER);
  collect_box->set_margin(5);
  collect_box->set_enable_search(true);

  Glib::RefPtr<Gtk::SignalListItemFactory> factory =
      Gtk::SignalListItemFactory::create();
  factory->signal_setup().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_valign(Gtk::Align::CENTER);
      item->set_child(*lab);
    });
  factory->signal_bind().connect([]
  (const Glib::RefPtr<Gtk::ListItem> &item)
    {
      Glib::RefPtr<ModelBoxes> mod_box =
      std::dynamic_pointer_cast<ModelBoxes>(item->get_item());
      auto lab = dynamic_cast<Gtk::Label*>(item->get_child());
      if(lab && mod_box)
	{
	  lab->set_text(mod_box->menu_line);
	}
    });
  collect_box->set_factory(factory);
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
	}
      f.close();
    }
  auto s_mod_box = ModelBoxes::create(filename);
  auto sp = men_mod->find(
      s_mod_box,
      []
      (const Glib::RefPtr<const Glib::ObjectBase> &item1,
       const Glib::RefPtr<const Glib::ObjectBase> &item2)
	 {
	   auto el1 = std::dynamic_pointer_cast<const ModelBoxes>(item1);
	   auto el2 = std::dynamic_pointer_cast<const ModelBoxes>(item2);
	   if(el1 && el2)
	     {
	       return el1->menu_line == el2->menu_line;
	     }
	   else
	     {
	       return false;
	     }
	 });
  if(std::get<0>(sp))
    {
      collect_box->set_selected(std::get<1>(sp));
      mw->readCollection(collect_box);
    }
  Glib::PropertyProxy<guint> prop_sel = collect_box->property_selected();
  prop_sel.signal_changed().connect([mwl, collect_box]
  {
    mwl->readCollection(collect_box);
  });
#endif
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
  surname_ent->set_width_chars(30);
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
  genre_but->set_halign(Gtk::Align::FILL);
  genre_but->set_margin(5);
  genre_but->set_label(gettext("<No>"));

  Gtk::Popover *popover = Gtk::make_managed<Gtk::Popover>();
  popover->set_name("popoverSt");
  genre_but->set_popover(*popover);

  Gtk::ScrolledWindow *scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  popover->set_child(*scrl);

  Gtk::Grid *scrl_gr = Gtk::make_managed<Gtk::Grid>();
  scrl_gr->set_halign(Gtk::Align::FILL);
  scrl_gr->set_valign(Gtk::Align::FILL);
  scrl_gr->set_expand(true);
  scrl->set_child(*scrl_gr);

  for(size_t i = 0; i < mw->genrev->size(); i++)
    {
      Glib::ustring g_group(mw->genrev->at(i).header);

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
	  std::vector<genre_item> tmpv = mw->genrev->at(i).items;
	  chexp->set_halign(Gtk::Align::START);
	  chexp->set_margin(2);
	  chexp->set_expanded(false);
	  chexp->set_label(g_group);
	  Gtk::Grid *chexp_gr = Gtk::make_managed<Gtk::Grid>();
	  chexp_gr->set_halign(Gtk::Align::CENTER);
	  chexp->set_child(*chexp_gr);

	  for(size_t j = 0; j < tmpv.size(); j++)
	    {
	      Gtk::Label *txtl = Gtk::make_managed<Gtk::Label>();
	      txtl->set_margin(2);
	      txtl->set_halign(Gtk::Align::END);
	      txtl->set_text(Glib::ustring(tmpv[j].name));
	      std::string code = tmpv[j].code;
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

  Gtk::Requisition minreq, natreq;
  scrl_gr->get_preferred_size(minreq, natreq);
  scrl->set_min_content_width(natreq.get_width());
  scrl->set_min_content_height(natreq.get_height());
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
#ifdef ML_GTK_OLD
	Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(widg);
	Glib::RefPtr<Gtk::TreeModel> model = sres->get_model();
	if(model)
	  {
	    sres->remove_all_columns();
	    sres->unset_model();
	  }
#endif
#ifndef ML_GTK_OLD
	Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(widg);
	if(sres)
	  {
	    for(size_t i = 0; i < mwl->search_res_col.size(); i++)
	      {
		sres->remove_column(mwl->search_res_col[i]);
	      }
	  }
	mwl->search_res_col.clear();
	Glib::RefPtr<Gtk::SingleSelection> ss;
	sres->set_model(ss);
	delete mwl->ms_sel_book;
	mwl->ms_sel_book = nullptr;
	mwl->list_sr = nullptr;
	mwl->ms_style_v.clear();
#endif
	genre_but->set_label(gettext("<No>"));
	genre_but->popdown();
	mwl->active_genre = "nill";
	surname_ent->set_text("");
	name_ent->set_text("");
	secname_ent->set_text("");
	booknm_ent->set_text("");
	ser_ent->set_text("");

	widg = right_grid->get_child_at(0, 7);
	Gtk::ScrolledWindow *annot_scrl =
	    dynamic_cast<Gtk::ScrolledWindow*>(widg);
	widg = annot_scrl->get_child();
	Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(widg);

	Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
	tb->set_text("");

	widg = right_grid->get_child_at(5, 7);
	Gtk::DrawingArea *drar = dynamic_cast<Gtk::DrawingArea*>(widg);

	drar->set_opacity(0.0);

	mwl->search_result_v.clear();
	mwl->cover_struct = cover_image();
      });
  left_gr->attach(*clearbut, 1, 16, 1, 1);

  return left_gr;
}

#ifdef ML_GTK_OLD
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
#endif

#ifndef ML_GTK_OLD
Glib::RefPtr<Gio::ListStore<ModelBoxes>>
CreateLeftGrid::formCollCombo()
{
  Glib::RefPtr<Gio::ListStore<ModelBoxes>> res_mod =
      Gio::ListStore<ModelBoxes>::create();
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
      res_mod->append(ModelBoxes::create(item));
    }

  return res_mod;
}
#endif

void
CreateLeftGrid::formGenreVect(std::vector<genres> *genrev)
{
  genres gs;
  gs.header = std::string(gettext("<No>"));
  genrev->push_back(gs);

  std::vector<genre_item> item_v;
  gs.header = std::string(gettext("Science Fiction & Fantasy"));
  genre_item item;
  item.code = "sf_history";
  item.name = std::string(gettext("Alternative history"));
  item_v.push_back(item);

  item.code = "sf_history";
  item.name = std::string(gettext("Alternative history"));
  item_v.push_back(item);
  item.code = "sf_action";
  item.name = std::string(gettext("Action"));
  item_v.push_back(item);
  item.code = "sf_epic";
  item.name = std::string(gettext("Epic"));
  item_v.push_back(item);
  item.code = "sf_heroic";
  item.name = std::string(gettext("Heroic"));
  item_v.push_back(item);
  item.code = "sf_detective";
  item.name = std::string(gettext("Detective"));
  item_v.push_back(item);
  item.code = "sf_cyberpunk";
  item.name = std::string(gettext("Cyberpunk"));
  item_v.push_back(item);
  item.code = "sf_space";
  item.name = std::string(gettext("Space"));
  item_v.push_back(item);
  item.code = "sf_social";
  item.name = std::string(gettext("Social-philosophical"));
  item_v.push_back(item);
  item.code = "sf_horror";
  item.name = std::string(gettext("Horror & mystic"));
  item_v.push_back(item);
  item.code = "sf_humor";
  item.name = std::string(gettext("Humor"));
  item_v.push_back(item);
  item.code = "sf_fantasy";
  item.name = std::string(gettext("Fantasy"));
  item_v.push_back(item);
  item.code = "sf";
  item.name = std::string(gettext("Science Fiction"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Detectives & Thrillers"));
  item.code = "det_classic";
  item.name = std::string(gettext("Classical detectives"));
  item_v.push_back(item);
  item.code = "det_police";
  item.name = std::string(gettext("Police Stories"));
  item_v.push_back(item);
  item.code = "det_action";
  item.name = std::string(gettext("Action"));
  item_v.push_back(item);
  item.code = "det_irony";
  item.name = std::string(gettext("Ironical detectives"));
  item_v.push_back(item);
  item.code = "det_history";
  item.name = std::string(gettext("Historical detectives"));
  item_v.push_back(item);
  item.code = "det_espionage";
  item.name = std::string(gettext("Espionage detectives"));
  item_v.push_back(item);
  item.code = "det_crime";
  item.name = std::string(gettext("Crime detectives"));
  item_v.push_back(item);
  item.code = "det_political";
  item.name = std::string(gettext("Political detectives"));
  item_v.push_back(item);
  item.code = "det_maniac";
  item.name = std::string(gettext("Maniacs"));
  item_v.push_back(item);
  item.code = "det_hard";
  item.name = std::string(gettext("Hard-boiled"));
  item_v.push_back(item);
  item.code = "thriller";
  item.name = std::string(gettext("Thrillers"));
  item_v.push_back(item);
  item.code = "detective";
  item.name = std::string(gettext("Detectives"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Prose"));
  item.code = "prose_classic";
  item.name = std::string(gettext("Classics prose"));
  item_v.push_back(item);
  item.code = "prose_history";
  item.name = std::string(gettext("Historical prose"));
  item_v.push_back(item);
  item.code = "prose_contemporary";
  item.name = std::string(gettext("Contemporary prose"));
  item_v.push_back(item);
  item.code = "prose_counter";
  item.name = std::string(gettext("Counterculture"));
  item_v.push_back(item);
  item.code = "prose_rus_classic";
  item.name = std::string(gettext("Russial classics prose"));
  item_v.push_back(item);
  item.code = "prose_su_classics";
  item.name = std::string(gettext("Soviet classics prose"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Romance"));
  item.code = "love_contemporary";
  item.name = std::string(gettext("Contemporary Romance"));
  item_v.push_back(item);
  item.code = "love_history";
  item.name = std::string(gettext("Historical Romance"));
  item_v.push_back(item);
  item.code = "love_detective";
  item.name = std::string(gettext("Detective Romance"));
  item_v.push_back(item);
  item.code = "love_short";
  item.name = std::string(gettext("Short Romance"));
  item_v.push_back(item);
  item.code = "love_erotica";
  item.name = std::string(gettext("Erotica"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Adventure"));
  item.code = "adv_western";
  item.name = std::string(gettext("Western"));
  item_v.push_back(item);
  item.code = "adv_history";
  item.name = std::string(gettext("History"));
  item_v.push_back(item);
  item.code = "adv_indian";
  item.name = std::string(gettext("Indians"));
  item_v.push_back(item);
  item.code = "adv_maritime";
  item.name = std::string(gettext("Maritime Fiction"));
  item_v.push_back(item);
  item.code = "adv_geo";
  item.name = std::string(gettext("Travel & geography"));
  item_v.push_back(item);
  item.code = "adv_animal";
  item.name = std::string(gettext("Nature & animals"));
  item_v.push_back(item);
  item.code = "adventure";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Children's"));
  item.code = "child_tale";
  item.name = std::string(gettext("Fairy Tales"));
  item_v.push_back(item);
  item.code = "child_verse";
  item.name = std::string(gettext("Verses"));
  item_v.push_back(item);
  item.code = "child_prose";
  item.name = std::string(gettext("Prose"));
  item_v.push_back(item);
  item.code = "child_sf";
  item.name = std::string(gettext("Science Fiction"));
  item_v.push_back(item);
  item.code = "child_det";
  item.name = std::string(gettext("Detectives & Thrillers"));
  item_v.push_back(item);
  item.code = "child_adv";
  item.name = std::string(gettext("Adventures"));
  item_v.push_back(item);
  item.code = "child_education";
  item.name = std::string(gettext("Educational"));
  item_v.push_back(item);
  item.code = "children";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Poetry & Dramaturgy"));
  item.code = "poetry";
  item.name = std::string(gettext("Poetry"));
  item_v.push_back(item);
  item.code = "dramaturgy";
  item.name = std::string(gettext("Dramaturgy"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Antique literature"));
  item.code = "antique_ant";
  item.name = std::string(gettext("Antique"));
  item_v.push_back(item);
  item.code = "antique_european";
  item.name = std::string(gettext("European"));
  item_v.push_back(item);
  item.code = "antique_russian";
  item.name = std::string(gettext("Old russian"));
  item_v.push_back(item);
  item.code = "antique_east";
  item.name = std::string(gettext("Old east"));
  item_v.push_back(item);
  item.code = "antique_myths";
  item.name = std::string(gettext("Myths. Legends. Epos."));
  item_v.push_back(item);
  item.code = "antique";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Scientific-educational"));
  item.code = "sci_history";
  item.name = std::string(gettext("History"));
  item_v.push_back(item);
  item.code = "sci_psychology";
  item.name = std::string(gettext("Psychology"));
  item_v.push_back(item);
  item.code = "sci_culture";
  item.name = std::string(gettext("Cultural science"));
  item_v.push_back(item);
  item.code = "sci_religion";
  item.name = std::string(gettext("Religious studies"));
  item_v.push_back(item);
  item.code = "sci_philosophy";
  item.name = std::string(gettext("Philosophy"));
  item_v.push_back(item);
  item.code = "sci_politics";
  item.name = std::string(gettext("Politics"));
  item_v.push_back(item);
  item.code = "sci_business";
  item.name = std::string(gettext("Business literature"));
  item_v.push_back(item);
  item.code = "sci_juris";
  item.name = std::string(gettext("Jurisprudence"));
  item_v.push_back(item);
  item.code = "sci_linguistic";
  item.name = std::string(gettext("Linguistics"));
  item_v.push_back(item);
  item.code = "sci_medicine";
  item.name = std::string(gettext("Medicine"));
  item_v.push_back(item);
  item.code = "sci_phys";
  item.name = std::string(gettext("Physics"));
  item_v.push_back(item);
  item.code = "sci_math";
  item.name = std::string(gettext("Mathematics"));
  item_v.push_back(item);
  item.code = "sci_chem";
  item.name = std::string(gettext("Chemistry"));
  item_v.push_back(item);
  item.code = "sci_biology";
  item.name = std::string(gettext("Biology"));
  item_v.push_back(item);
  item.code = "sci_tech";
  item.name = std::string(gettext("Technical"));
  item_v.push_back(item);
  item.code = "science";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Computers & Internet"));
  item.code = "comp_www";
  item.name = std::string(gettext("Internet"));
  item_v.push_back(item);
  item.code = "comp_programming";
  item.name = std::string(gettext("Programming"));
  item_v.push_back(item);
  item.code = "comp_hard";
  item.name = std::string(gettext("Hardware"));
  item_v.push_back(item);
  item.code = "comp_soft";
  item.name = std::string(gettext("Software"));
  item_v.push_back(item);
  item.code = "comp_db";
  item.name = std::string(gettext("Databases"));
  item_v.push_back(item);
  item.code = "comp_osnet";
  item.name = std::string(gettext("OS & Networking"));
  item_v.push_back(item);
  item.code = "computers";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Reference"));
  item.code = "ref_encyc";
  item.name = std::string(gettext("Encyclopedias"));
  item_v.push_back(item);
  item.code = "ref_dict";
  item.name = std::string(gettext("Dictionaries"));
  item_v.push_back(item);
  item.code = "ref_ref";
  item.name = std::string(gettext("Reference"));
  item_v.push_back(item);
  item.code = "ref_guide";
  item.name = std::string(gettext("Guidebooks"));
  item_v.push_back(item);
  item.code = "reference";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Nonfiction"));
  item.code = "nonf_biography";
  item.name = std::string(gettext("Biography & Memoirs"));
  item_v.push_back(item);
  item.code = "nonf_publicism";
  item.name = std::string(gettext("Publicism"));
  item_v.push_back(item);
  item.code = "nonf_criticism";
  item.name = std::string(gettext("Criticism"));
  item_v.push_back(item);
  item.code = "design";
  item.name = std::string(gettext("Art & design"));
  item_v.push_back(item);
  item.code = "nonfiction";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Religion & Inspirationn"));
  item.code = "religion_rel";
  item.name = std::string(gettext("Religion"));
  item_v.push_back(item);
  item.code = "religion_esoterics";
  item.name = std::string(gettext("Esoterics"));
  item_v.push_back(item);
  item.code = "religion_self";
  item.name = std::string(gettext("Self-improvement"));
  item_v.push_back(item);
  item.code = "religion";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Humor"));
  item.code = "humor_anecdote";
  item.name = std::string(gettext("Anecdote (funny stories)"));
  item_v.push_back(item);
  item.code = "humor_prose";
  item.name = std::string(gettext("Prose"));
  item_v.push_back(item);
  item.code = "humor_verse";
  item.name = std::string(gettext("Verses"));
  item_v.push_back(item);
  item.code = "humor";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);

  gs.header = std::string(gettext("Home & Family"));
  item.code = "home_cooking";
  item.name = std::string(gettext("Cooking"));
  item_v.push_back(item);
  item.code = "home_pets";
  item.name = std::string(gettext("Pets"));
  item_v.push_back(item);
  item.code = "home_crafts";
  item.name = std::string(gettext("Hobbies & Crafts"));
  item_v.push_back(item);
  item.code = "home_entertain";
  item.name = std::string(gettext("Entertaining"));
  item_v.push_back(item);
  item.code = "home_health";
  item.name = std::string(gettext("Health"));
  item_v.push_back(item);
  item.code = "home_garden";
  item.name = std::string(gettext("Garden"));
  item_v.push_back(item);
  item.code = "home_diy";
  item.name = std::string(gettext("Do it yourself"));
  item_v.push_back(item);
  item.code = "home_sport";
  item.name = std::string(gettext("Sports"));
  item_v.push_back(item);
  item.code = "home_sex";
  item.name = std::string(gettext("Erotica & sex"));
  item_v.push_back(item);
  item.code = "home";
  item.name = std::string(gettext("Other"));
  item_v.push_back(item);
  item_v.shrink_to_fit();
  gs.items = item_v;
  item_v.clear();
  genrev->push_back(gs);
  genrev->shrink_to_fit();
}
