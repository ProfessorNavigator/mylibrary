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

#include <BookOpWindows.h>

BookOpWindows::BookOpWindows(MainWindow *mw)
{
  this->mw = mw;
}

BookOpWindows::~BookOpWindows()
{

}

#ifndef ML_GTK_OLD
void
BookOpWindows::searchBook(Gtk::DropDown *coll_nm, Gtk::Entry *surname_ent,
			  Gtk::Entry *name_ent, Gtk::Entry *secname_ent,
			  Gtk::Entry *booknm_ent, Gtk::Entry *ser_ent)
{
  mw->search_result_v.clear();
  auto mod_box = std::dynamic_pointer_cast<ModelBoxes>(
      coll_nm->get_selected_item());
  if(!mod_box)
    {
      return void();
    }
  std::string collnm(mod_box->menu_line);
  std::string surnm(surname_ent->get_text());
  std::string name(name_ent->get_text());
  std::string secname(secname_ent->get_text());
  std::string book(booknm_ent->get_text());
  std::string series(ser_ent->get_text());
  std::string genre(mw->active_genre);

  int *search_cancel = new int(0);
  SearchBook *sb = new SearchBook(collnm, surnm, name, secname, book, series,
				  genre, &mw->prev_search_nm, &mw->base_v,
				  &mw->search_result_v, search_cancel);

  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_name("MLwindow");
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
  cancel->set_name("cancelBut");
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  MainWindow *mwl = mw;
  cancel->signal_clicked().connect([search_cancel]
  {
    *search_cancel = 1;
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  Glib::Dispatcher *search_compl_disp = new Glib::Dispatcher();
  search_compl_disp->connect([window, mwl, search_compl_disp]
  {
    CreateRightGrid crgr(mwl);
    crgr.searchResultShow();
    window->close();
    delete search_compl_disp;
  });

  window->signal_close_request().connect([window]
  {

    window->set_visible(false);
    delete window;
    return true;
  },
					 false);

  window->present();

  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }

  std::thread *thr = new std::thread([sb, mwl, search_compl_disp, search_cancel]
  {
    mwl->searchmtx->lock();
    sb->searchBook();
    sb->cleanSearchV();
    mwl->searchmtx->unlock();
    search_compl_disp->emit();
    delete sb;
    delete search_cancel;
  });
  thr->detach();
  delete thr;
}

void
BookOpWindows::bookRemoveWin(Gtk::Window *win)
{
  Glib::RefPtr<Gtk::AlertDialog> msg = Gtk::AlertDialog::create();
  msg->set_modal(true);
  msg->set_message(gettext("This action will remove book from "
			   "collection and file system. Continue?"));
  std::vector<Glib::ustring> labels;
  labels.push_back(gettext("No"));
  labels.push_back(gettext("Yes"));
  msg->set_buttons(labels);
  msg->set_cancel_button(0);
  msg->set_default_button(0);
  MainWindow *mwl = mw;
  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  msg->choose(*mwl, [mwl]
  (Glib::RefPtr<Gio::AsyncResult> &result)
    {
      int resp;
      auto obj = result->get_source_object_base();
      auto msg = std::dynamic_pointer_cast<Gtk::AlertDialog>(obj);
      if(msg)
	{
	  resp = msg->choose_finish(result);
	}
      else
	{
	  return void();
	}
      if(resp == 1)
	{
	  BookOpWindows bopw(mwl);
	  bopw.bookRemoveVar1();
	}
    },
	      cncl);
}
#endif

#ifdef ML_GTK_OLD
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

  int *search_cancel = new int(0);
  SearchBook *sb = new SearchBook(collnm, surnm, name, secname, book, series,
				  genre, &mw->prev_search_nm, &mw->base_v,
				  &mw->search_result_v, search_cancel);

  Gtk::Window *window = new Gtk::Window;
  window->set_application(mw->get_application());
  window->set_name("MLwindow");
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
  cancel->set_name("cancelBut");
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_margin(5);
  cancel->set_label(gettext("Cancel"));
  MainWindow *mwl = mw;
  cancel->signal_clicked().connect([search_cancel]
  {
    *search_cancel = 1;
  });
  grid->attach(*cancel, 0, 1, 1, 1);

  Glib::Dispatcher *search_compl_disp = new Glib::Dispatcher();
  search_compl_disp->connect([window, mwl, search_compl_disp]
  {
    CreateRightGrid crgr(mwl);
    crgr.searchResultShow(1, nullptr);

    window->close();
    delete search_compl_disp;
  });

  window->signal_close_request().connect([window]
  {
    window->hide();

    delete window;
    return true;
  },
					 false);

  window->present();

  std::thread *thr = new std::thread([sb, mwl, search_compl_disp, search_cancel]
  {
    mwl->searchmtx->lock();
    sb->searchBook();
    sb->cleanSearchV();
    mwl->searchmtx->unlock();
    search_compl_disp->emit();
    delete sb;
    delete search_cancel;
  });
  thr->detach();
  delete thr;
}

void
BookOpWindows::bookRemoveWin(int variant, Gtk::Window *win, Gtk::TreeView *sres)
{
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  Glib::ustring msgtxt;
	  Gtk::Window *trans = nullptr;
	  if(variant == 1)
	    {
	      msgtxt =
		  Glib::ustring(
		      gettext(
			  "This action will remove book from collection and file system. Continue?"));
	      trans = mw;
	    }
	  else if(variant == 2)
	    {
	      msgtxt =
		  Glib::ustring(
		      gettext(
			  "This action will remove book from bookmarks. Continue?"));
	      trans = win;
	    }

	  if(trans)
	    {
	      Gtk::MessageDialog *msg = new Gtk::MessageDialog(
		  *trans, msgtxt, false, Gtk::MessageType::QUESTION,
		  Gtk::ButtonsType::YES_NO, true);
	      msg->set_application(mw->get_application());

	      MainWindow *mwl = mw;
	      if(variant == 1)
		{
		  msg->signal_response().connect([mwl, msg]
		  (int resp)
		    {
		      if(resp == Gtk::ResponseType::YES)
			{
			  BookOpWindows bopw(mwl);
			  bopw.bookRemoveVar1(msg);
			}
		      else if(resp == Gtk::ResponseType::NO)
			{
			  msg->close();
			}
		    });
		}
	      else if(variant == 2)
		{
		  msg->signal_response().connect([msg, win, mwl]
		  (int resp)
		    {
		      if(resp == Gtk::ResponseType::NO)
			{
			  msg->close();
			}
		      else if(resp == Gtk::ResponseType::YES)
			{
			  BookOpWindows bopw(mwl);
			  bopw.bookRemoveVar2(win);
			  msg->close();
			}
		    });
		}
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
    }
}

void
BookOpWindows::bookRemoveVar1(Gtk::MessageDialog *mess)
{
  if(mess)
    {
      mess->close();
    }
  MainWindow *mwl = mw;
  Gtk::MessageDialog *msg = new Gtk::MessageDialog(
      *mwl, gettext("Removing... It will take some time"), false,
      Gtk::MessageType::INFO, Gtk::ButtonsType::NONE, true);

  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
  Gtk::ComboBoxText *collect_box = dynamic_cast<Gtk::ComboBoxText*>(left_gr
      ->get_child_at(0, 1));

  int *cncl = new int(0);
  RefreshCollection *rc = new RefreshCollection(
      std::string(collect_box->get_active_text()), 1, false, cncl);

  Glib::Dispatcher *disp_file_nexists = new Glib::Dispatcher();
  disp_file_nexists->connect([mwl]
  {
    AuxWindows aw(mwl);
    aw.errorWin(7, mwl);
  });
  rc->collection_not_exists = [disp_file_nexists]
  {
    disp_file_nexists->emit();
  };

  Glib::Dispatcher *disp_finished = new Glib::Dispatcher();
  disp_finished->connect([msg, mwl, disp_finished, disp_file_nexists]
  {
    mwl->prev_search_nm.clear();
    Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
    Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
    Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
    Gtk::ScrolledWindow *sres_scrl =
	dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
    Glib::RefPtr<Gtk::Adjustment> adj = sres_scrl->get_vadjustment();
    double pos = adj->get_value();
    CreateRightGrid crgr(mwl);
    crgr.searchResultShow(1, nullptr);
    msg->close();
    Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
    while(mc->pending())
      {
	mc->iteration(true);
      }
    adj->set_value(pos);
    delete disp_file_nexists;
    delete disp_finished;
  });

  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(right_grid
      ->get_child_at(0, 0));
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();
  std::thread *thr = nullptr;
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  size_t id;
	  iter->get_value(0, id);
	  std::string filename = (mwl->search_result_v[id - 1]).path_to_book;
	  std::string lsfnm = filename;
	  lsfnm.erase(
	      0, lsfnm.find("<archpath>") + std::string("<archpath>").size());
	  lsfnm = lsfnm.substr(0, lsfnm.find("</archpath>"));
	  std::filesystem::path ch_p = std::filesystem::u8path(lsfnm);
	  if(ch_p.extension().u8string() == ".rar")
	    {
	      delete disp_finished;
	      delete disp_file_nexists;
	      delete msg;
	      disp_file_nexists = new Glib::Dispatcher();
	      disp_file_nexists->connect([mwl]
	      {
		AuxWindows aw(mwl);
		aw.errorWin(7, mwl);
	      });
	      rc->collection_not_exists = [disp_file_nexists]
	      {
		disp_file_nexists->emit();
	      };
	      msg = new Gtk::MessageDialog(
		  *mwl,
		  gettext(
		      "Book is in rar archive and cannot be remove separately."
		      " Remove whole archive?"),
		  false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO,
		  true);
	      msg->signal_response().connect(
		  [ch_p, msg, rc, mwl, sres, iter, disp_file_nexists]
		  (int resp)
		    {
		      if(resp == Gtk::ResponseType::YES)
			{
			  mwl->search_result_v.erase(std::remove_if(mwl->search_result_v.begin(), mwl->search_result_v.end(), [ch_p](auto &el)
				    {
				      std::string::size_type n;
				      std::string ptb = el.path_to_book;
				      n = ptb.find(ch_p.u8string());
				      if(n != std::string::npos)
					{
					  return true;
					}
				      else
					{
					  return false;
					}
				    }), mwl->search_result_v.end());

			  rc->removeRar(ch_p.u8string());
			  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
			  while(mc->pending())
			    {
			      mc->iteration(true);
			    }
			  delete disp_file_nexists;

			  mwl->prev_search_nm.clear();
			  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
			  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
			  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
			  Gtk::DrawingArea *drar =
			  dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(1, 2));
			  drar->set_opacity(0.0);
			  Gtk::ScrolledWindow *annot_scrl =
			  dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0,
				  2));
			  Gtk::TextView *annot =
			  dynamic_cast<Gtk::TextView*>(annot_scrl->get_child());
			  Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
			  tb->set_text("");
			  Gtk::ScrolledWindow *sres_scrl =
			  dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
			  Glib::RefPtr<Gtk::Adjustment> adj = sres_scrl->get_vadjustment();
			  double pos = adj->get_value();
			  CreateRightGrid crgr(mwl);
			  crgr.searchResultShow(1, nullptr);
			  while(mc->pending())
			    {
			      mc->iteration(true);
			    }
			  adj->set_value(pos);
			  msg->close();
			}
		      else if(resp == Gtk::ResponseType::NO)
			{
			  delete disp_file_nexists;
			  msg->close();
			}
		    });

	      msg->signal_close_request().connect([msg, rc, cncl]
	      {
		msg->hide();
		delete rc;
		delete cncl;
		delete msg;
		return true;
	      },
						  false);
	      msg->present();
	      return void();
	    }
	  else
	    {
	      mwl->search_result_v.erase(mwl->search_result_v.begin() + id - 1);
	      Gtk::DrawingArea *drar =
		  dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(1, 2));
	      drar->set_opacity(0.0);
	      Gtk::ScrolledWindow *annot_scrl =
		  dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0,
									      2));
	      Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(annot_scrl
		  ->get_child());
	      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
	      tb->set_text("");
	      thr = new std::thread([rc, disp_finished, filename]
	      {
		rc->removeBook(filename);
		disp_finished->emit();
		delete rc;
	      });
	    }
	}
    }
  else
    {
      delete msg;
      delete disp_finished;
      delete disp_file_nexists;
      delete cncl;
      delete rc;
      return void();
    }

  msg->signal_close_request().connect([msg, disp_file_nexists, cncl]
  {
    msg->hide();
    delete cncl;
    delete msg;
    return true;
  },
				      false);
  msg->present();
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  if(thr)
    {
      thr->detach();
      delete thr;
    }
}
#endif

#ifndef ML_GTK_OLD
void
BookOpWindows::bookRemoveVar1()
{
  MainWindow *mwl = mw;
  std::thread *thr = nullptr;

  Gtk::Window *msg = new Gtk::Window;
  msg->set_application(mw->get_application());
  msg->set_name("MLwindow");
  msg->set_transient_for(*mw);
  msg->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::CENTER);
  grid->set_valign(Gtk::Align::CENTER);
  msg->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_valign(Gtk::Align::CENTER);
  lab->set_text(gettext("Removing... It will take some time"));

  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *left_gr = dynamic_cast<Gtk::Grid*>(pn->get_start_child());
  Gtk::DropDown *collect_box = dynamic_cast<Gtk::DropDown*>(left_gr
      ->get_child_at(0, 1));
  if(!collect_box)
    {
      return void();
    }
  auto obj_coll = collect_box->get_selected_item();
  auto mod_box = std::dynamic_pointer_cast<ModelBoxes>(obj_coll);
  std::string collname;
  if(mod_box)
    {
      collname = std::string(mod_box->menu_line);
    }
  else
    {
      return void();
    }
  int *cncl = new int(0);
  RefreshCollection *rc = new RefreshCollection(collname, 1, false, cncl);

  Glib::Dispatcher *disp_file_nexists = new Glib::Dispatcher();
  disp_file_nexists->connect([mwl]
  {
    AuxWindows aw(mwl);
    aw.errorWin(7, mwl);
  });
  rc->collection_not_exists = [disp_file_nexists]
  {
    disp_file_nexists->emit();
  };

  Glib::Dispatcher *disp_finished = new Glib::Dispatcher();
  disp_finished->connect([msg, mwl, disp_finished, disp_file_nexists]
  {
    mwl->prev_search_nm.clear();
    Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
    Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
    Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
    Gtk::ScrolledWindow *sres_scrl =
	dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
    Gtk::ColumnView *sres =
	dynamic_cast<Gtk::ColumnView*>(sres_scrl->get_child());
    Glib::RefPtr<Gtk::SingleSelection> selection;
    if(sres)
      {
	auto sl = sres->get_model();
	if(sl)
	  {
	    selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(sl);
	  }
      }
    Glib::RefPtr<Glib::ObjectBase> obj;
    if(selection)
      {
	if(mwl->ms_sel_book)
	  {
	    auto lm = selection->get_model();
	    if(lm)
	      {
		if(lm->get_n_items() > *(mwl->ms_sel_book))
		  {
		    obj = lm->get_object(*(mwl->ms_sel_book));
		  }
	      }
	  }
	else
	  {
	    obj = selection->get_selected_item();
	  }
      }
    if(obj)
      {
	auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
	if(mod_col && mwl->list_sr)
	  {
	    auto sr = mwl->list_sr->find(mod_col);
	    if(std::get<0>(sr))
	      {
		mwl->list_sr->remove(std::get<1>(sr));
		auto v = mwl->ms_style_v;
		v.erase(std::remove_if(v.begin(), v.end(), [mod_col]
		(auto &el)
		  {
		    return el.item == mod_col;
		  }),
			v.end());
	      }
	  }
      }
    msg->close();
    delete disp_file_nexists;
    delete disp_finished;
  });

  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(right_grid
      ->get_child_at(0, 0));
  Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(sres_scrl->get_child());
  auto sm = sres->get_model();
  Glib::RefPtr<Gtk::SingleSelection> selection = std::dynamic_pointer_cast<
      Gtk::SingleSelection>(sm);
  Glib::RefPtr<Glib::ObjectBase> obj;
  if(selection)
    {
      if(mw->ms_sel_book)
	{
	  auto lm = selection->get_model();
	  if(lm)
	    {
	      if(lm->get_n_items() > *(mw->ms_sel_book))
		{
		  obj = lm->get_object(*(mw->ms_sel_book));
		}
	    }
	}
      else
	{
	  obj = selection->get_selected_item();
	}
    }
  if(obj)
    {
      auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
      if(mod_col)
	{
	  std::string filename(mod_col->path);
	  std::string lsfnm = filename;
	  lsfnm.erase(
	      0, lsfnm.find("<archpath>") + std::string("<archpath>").size());
	  lsfnm = lsfnm.substr(0, lsfnm.find("</archpath>"));
	  std::filesystem::path ch_p = std::filesystem::u8path(lsfnm);
	  if(ch_p.extension().u8string() == ".rar")
	    {
	      mwl->prev_search_nm.clear();
	      delete disp_file_nexists;
	      delete disp_finished;
	      delete msg;
	      disp_file_nexists = new Glib::Dispatcher();
	      disp_file_nexists->connect([mwl]
	      {
		AuxWindows aw(mwl);
		aw.errorWin(7, mwl);
	      });
	      rc->collection_not_exists = [disp_file_nexists]
	      {
		disp_file_nexists->emit();
	      };
	      Glib::RefPtr<Gtk::AlertDialog> msg_rar =
		  Gtk::AlertDialog::create();
	      msg_rar->set_modal(true);
	      msg_rar->set_message(
		  gettext(
		      "Book is in rar archive and cannot be remove separately."
		      " Remove whole archive?"));
	      std::vector<Glib::ustring> labels;
	      labels.push_back(gettext("No"));
	      labels.push_back(gettext("Yes"));
	      msg_rar->set_buttons(labels);
	      msg_rar->set_cancel_button(0);
	      msg_rar->set_default_button(0);
	      Glib::RefPtr<Gio::Cancellable> cancel =
		  Gio::Cancellable::create();
	      msg_rar->choose(
		  *mwl,
		  [ch_p, rc, mwl, sres, disp_file_nexists, cncl]
		  (Glib::RefPtr<Gio::AsyncResult> &result)
		    {
		      int resp;
		      auto obj = result->get_source_object_base();
		      auto msg_rar = std::dynamic_pointer_cast<Gtk::AlertDialog>(obj);
		      if(msg_rar)
			{
			  resp = msg_rar->choose_finish(result);
			}
		      else
			{
			  return void();
			}
		      if(resp == 1)
			{
			  rc->removeRar(ch_p.u8string());
			  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
			  while(mc->pending())
			    {
			      mc->iteration(true);
			    }
			  delete disp_file_nexists;

			  mwl->prev_search_nm.clear();
			  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mwl->get_child());
			  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
			  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
			  Gtk::DrawingArea *drar =
			  dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(1, 2));
			  drar->set_opacity(0.0);
			  Gtk::ScrolledWindow *annot_scrl =
			  dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0,
				  2));
			  Gtk::TextView *annot =
			  dynamic_cast<Gtk::TextView*>(annot_scrl->get_child());
			  Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
			  tb->set_text("");
			  std::pair<bool, unsigned int> sr;
			  auto s_val =
			  ModelColumns::create("", "", "", "", "", ch_p.u8string());
			  for(;;)
			    {
			      sr = mwl->list_sr->find(s_val, [](auto &el1, auto &el2)
				    {
				      Glib::ustring::size_type n;
				      n = el1->path.find(el2->path);
				      if(n == Glib::ustring::npos)
					{
					  return false;
					}
				      else
					{
					  return true;
					}
				    });
			      if(std::get<0>(sr))
				{
				  auto mod_col = mwl->list_sr->get_item(std::get<1>(sr));
				  mwl->list_sr->remove(std::get<1>(sr));
				  auto v = mwl->ms_style_v;
				  v.erase(std::remove_if(v.begin(), v.end(), [mod_col]
					  (auto &el)
					    {
					      return el.item == mod_col;
					    }),
				      v.end());
				}
			      else
				{
				  break;
				}
			    }
			}
		      else if(resp == 0)
			{
			  delete disp_file_nexists;
			}
		      delete rc;
		      delete cncl;
		    },
		  cancel);
	      return void();
	    }
	  else
	    {
	      Gtk::DrawingArea *drar =
		  dynamic_cast<Gtk::DrawingArea*>(right_grid->get_child_at(1, 2));
	      drar->set_opacity(0.0);
	      Gtk::ScrolledWindow *annot_scrl =
		  dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0,
									      2));
	      Gtk::TextView *annot = dynamic_cast<Gtk::TextView*>(annot_scrl
		  ->get_child());
	      Glib::RefPtr<Gtk::TextBuffer> tb = annot->get_buffer();
	      tb->set_text("");
	      thr = new std::thread([rc, disp_finished, filename]
	      {
		rc->removeBook(filename);
		disp_finished->emit();
		delete rc;
	      });
	    }
	}
    }
  else
    {
      delete msg;
      delete disp_finished;
      delete disp_file_nexists;
      delete rc;
      delete cncl;
      return void();
    }

  msg->signal_close_request().connect([msg, cncl]
  {
    msg->set_visible(false);
    delete cncl;
    delete msg;
    return true;
  },
				      false);
  msg->present();
  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
  while(mc->pending())
    {
      mc->iteration(true);
    }
  if(thr)
    {
      thr->detach();
      delete thr;
    }
}
#endif

#ifdef ML_GTK_OLD
void
BookOpWindows::bookRemoveVar2(Gtk::Window *win)
{

  Glib::RefPtr<Gtk::TreeSelection> selection = mw->bm_tv->get_selection();
  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  size_t id;
	  iter->get_value(0, id);
	  mw->bookmark_v.erase(mw->bookmark_v.begin() + id - 1);
	  Glib::RefPtr<Gtk::ListStore> liststore = std::dynamic_pointer_cast<
	      Gtk::ListStore>(mw->bm_tv->get_model());
	  liststore->erase(iter);
	  Gtk::Grid *gr = dynamic_cast<Gtk::Grid*>(win->get_child());
	  Gtk::ScrolledWindow *scrl = dynamic_cast<Gtk::ScrolledWindow*>(gr
	      ->get_child_at(0, 0));
	  Glib::RefPtr<Gtk::Adjustment> adj = scrl->get_vadjustment();
	  double pos = adj->get_value();
	  std::string filename;
	  AuxFunc af;
	  af.homePath(&filename);
	  filename = filename + "/.MyLibrary/Bookmarks";
	  std::filesystem::path filepath = std::filesystem::u8path(filename);
	  if(std::filesystem::exists(filepath))
	    {
	      std::filesystem::remove(filepath);
	    }
	  std::fstream f;
	  f.open(filepath, std::ios_base::out | std::ios_base::binary);
	  if(f.is_open())
	    {
	      for(size_t i = 0; i < mw->bookmark_v.size(); i++)
		{
		  std::string line = (mw->bookmark_v[i]).authors + "<?>";
		  line = line + (mw->bookmark_v[i]).book + "<?>";
		  line = line + (mw->bookmark_v[i]).series + "<?>";
		  line = line + (mw->bookmark_v[i]).genre + "<?>";
		  line = line + (mw->bookmark_v[i]).date + "<?>";
		  line = line + (mw->bookmark_v[i]).path_to_book + "<?L>";
		  f.write(line.c_str(), line.size());
		}
	      f.close();
	    }
	  CreateRightGrid crgr(mw);
	  crgr.searchResultShow(2, win);
	  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
	  while(mc->pending())
	    {
	      mc->iteration(true);
	    }
	  adj->set_value(pos);
	}
    }
}
#endif

#ifndef ML_GTK_OLD
void
BookOpWindows::fileInfo(int variant)
{
  Glib::RefPtr<Glib::ObjectBase> obj;
  Gtk::Window *parent_window = nullptr;
  switch(variant)
    {
    case 1:
      {
	Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
	Gtk::Paned *pn =
	    dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
	Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
	Gtk::ScrolledWindow *sres_scrl =
	    dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
	Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(sres_scrl
	    ->get_child());
	Glib::RefPtr<Gtk::SingleSelection> selection;
	if(sres)
	  {
	    auto sm = sres->get_model();
	    selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
	  }
	if(selection)
	  {
	    if(mw->ms_sel_book)
	      {
		auto lm = selection->get_model();
		if(lm)
		  {
		    if(lm->get_n_items() > *(mw->ms_sel_book))
		      {
			obj = lm->get_object(*(mw->ms_sel_book));
		      }
		  }
	      }
	  }
	parent_window = mw;
	break;
      }
    case 2:
      {
	if(mw->bm_col_view)
	  {
	    Glib::RefPtr<Gtk::SelectionModel> model =
		mw->bm_col_view->get_model();
	    Glib::RefPtr<Gtk::SingleSelection> ss = std::dynamic_pointer_cast<
		Gtk::SingleSelection>(model);
	    if(ss)
	      {
		if(mw->bm_sel_book)
		  {
		    auto lm = ss->get_model();
		    if(lm)
		      {
			if(lm->get_n_items() > *(mw->bm_sel_book))
			  {
			    obj = lm->get_object(*(mw->bm_sel_book));
			  }
		      }
		  }
	      }
	    Gtk::Widget *widg = mw->bm_col_view->get_parent();
	    if(widg)
	      {
		widg = widg->get_parent();
		if(widg)
		  {
		    parent_window =
			dynamic_cast<Gtk::Window*>(widg->get_parent());
		  }
	      }
	  }
	break;
      }
    default:
      break;
    }

  if(obj)
    {
      auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
      if(mod_col)
	{
	  Gtk::Window *window = new Gtk::Window;
	  window->set_application(mw->get_application());
	  window->set_title(gettext("Book file info"));
	  if(parent_window)
	    {
	      window->set_transient_for(*parent_window);
	    }
	  window->set_name("MLwindow");

	  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
	  grid->set_halign(Gtk::Align::FILL);
	  grid->set_valign(Gtk::Align::FILL);
	  window->set_child(*grid);

	  AuxFunc af;
	  std::filesystem::path filepath;
	  std::string filename(mod_col->path);
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
	      if(std::filesystem::exists(filepath))
		{
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
			  Glib::ustring("<i>")
			      + Glib::ustring(std::get<1>(*itinfv))
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
			  Glib::ustring("<i>")
			      + Glib::ustring(std::get<1>(*itinfv))
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
			  Glib::ustring("<i>")
			      + Glib::ustring(std::get<1>(*itinfv))
			      + Glib::ustring("</i> ")
			      + Glib::ustring(gettext("bytes")));
		    }
		  grid->attach(*sz_lb2, 1, 3, 1, 1);
		}
	      else
		{
		  window->set_default_size(1, 1);
		  Gtk::Label *warn_lb = Gtk::make_managed<Gtk::Label>();
		  warn_lb->set_margin(5);
		  warn_lb->set_halign(Gtk::Align::START);
		  warn_lb->set_use_markup(true);
		  warn_lb->set_markup(
		      Glib::ustring("<i>")
			  + Glib::ustring(gettext("File does not exists!"))
			  + Glib::ustring("</i>"));
		  grid->attach(*warn_lb, 0, 0, 1, 1);
		}
	    }
	  else
	    {
	      filepath = std::filesystem::u8path(filename);
	      filepath.make_preferred();
	      if(std::filesystem::exists(filepath))
		{
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
		      Glib::ustring("<b>")
			  + Glib::ustring(gettext("File size: "))
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
	      else
		{
		  window->set_default_size(1, 1);
		  Gtk::Label *warn_lb = Gtk::make_managed<Gtk::Label>();
		  warn_lb->set_margin(5);
		  warn_lb->set_halign(Gtk::Align::START);
		  warn_lb->set_use_markup(true);
		  warn_lb->set_markup(
		      Glib::ustring("<i>")
			  + Glib::ustring(gettext("File does not exists!"))
			  + Glib::ustring("</i>"));
		  grid->attach(*warn_lb, 0, 0, 1, 1);
		}
	    }
	  window->signal_close_request().connect([window]
	  {
#ifdef ML_GTK_OLD
	    window->hide();
#endif
#ifndef ML_GTK_OLD
						   window->set_visible(false);
#endif
						   delete window;
						   return true;
						 },
						 false);
	  window->present();
	}
    }
}

void
BookOpWindows::editBook()
{
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(right_grid
      ->get_child_at(0, 0));
  Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(sres_scrl->get_child());
  Glib::RefPtr<Gtk::SingleSelection> selection;
  if(sres)
    {
      auto sl = sres->get_model();
      if(sl)
	{
	  selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(sl);
	}
    }
  Glib::RefPtr<Glib::ObjectBase> obj;
  if(selection)
    {
      if(mw->ms_sel_book)
	{
	  auto lm = selection->get_model();
	  if(lm)
	    {
	      if(lm->get_n_items() > *(mw->ms_sel_book))
		{
		  obj = lm->get_object(*(mw->ms_sel_book));
		}
	    }
	}
      else
	{
	  obj = selection->get_selected_item();
	}
    }
  std::vector<std::tuple<std::string, std::string>> *bookv = new std::vector<
      std::tuple<std::string, std::string>>();

  if(obj)
    {
      auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
      if(mod_col)
	{
	  std::tuple<std::string, std::string> ttup;
	  std::get<0>(ttup) = "Author";
	  std::get<1>(ttup) = std::string(mod_col->author);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "Book";
	  std::get<1>(ttup) = std::string(mod_col->book);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "Series";
	  std::get<1>(ttup) = std::string(mod_col->series);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "Genre";
	  CreateRightGrid crg(mw);
	  std::get<1>(ttup) = crg.genre_str(mod_col->genre);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "Date";
	  std::get<1>(ttup) = std::string(mod_col->date);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "filepath";
	  std::get<1>(ttup) = std::string(mod_col->path);
	  bookv->push_back(ttup);

	  Gtk::Window *window = new Gtk::Window;
	  window->set_application(mw->get_application());
	  window->set_title(gettext("Book entry editor"));
	  window->set_transient_for(*mw);
	  window->set_name("MLwindow");
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
	  scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
			   Gtk::PolicyType::AUTOMATIC);
	  Gtk::Grid *scrl_gr = Gtk::make_managed<Gtk::Grid>();
	  scrl_gr->set_halign(Gtk::Align::START);

	  Gtk::Popover *popover = Gtk::make_managed<Gtk::Popover>();
	  popover->set_child(*scrl);
	  genre_but->set_popover(*popover);

	  int maxl = 0;
	  Gtk::Expander *maxexp = nullptr;
	  for(size_t i = 0; i < mw->genrev->size(); i++)
	    {
	      Glib::ustring g_group(mw->genrev->at(i).header);

	      if(i == 0)
		{
		  Gtk::Label *txtl = Gtk::make_managed<Gtk::Label>();
		  txtl->set_margin(2);
		  txtl->set_halign(Gtk::Align::START);
		  txtl->set_text(g_group);
		  Glib::RefPtr<Gtk::GestureClick> clck =
		      Gtk::GestureClick::create();
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
		  std::vector<genre_item> tmpv = mw->genrev->at(i).items;
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
		      txtl->set_text(Glib::ustring(tmpv[j].name));
		      std::string code = tmpv[j].code;
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
	  save->set_name("applyBut");
	  save->set_halign(Gtk::Align::CENTER);
	  save->set_margin(5);
	  save->set_label(gettext("Save"));
	  MainWindow *mwl = mw;
#ifdef ML_GTK_OLD
	  save->signal_clicked().connect([bookv, mwl, window]
	  {
	    BookOpWindows bopw(mwl);
	    bopw.bookSaveRestore(window, bookv, 1);
	  });
#endif
#ifndef ML_GTK_OLD
	  save->signal_clicked().connect([bookv, mwl, window, mod_col]
	  {
	    BookOpWindows bopw(mwl);
	    bopw.bookSaveRestore(window, bookv, 1, mod_col);
	  });
#endif
	  grid->attach(*save, 0, 12, 1, 1);

	  Gtk::Button *restore = Gtk::make_managed<Gtk::Button>();
	  restore->set_name("operationBut");
	  restore->set_halign(Gtk::Align::CENTER);
	  restore->set_margin(5);
	  restore->set_label(gettext("Restore"));
#ifdef ML_GTK_OLD
	  restore->signal_clicked().connect([bookv, mwl, window]
	  {
	    BookOpWindows bopw(mwl);
	    bopw.bookSaveRestore(window, bookv, 2);
	  });
#endif
#ifndef ML_GTK_OLD
	  restore->signal_clicked().connect([bookv, mwl, window, mod_col]
	  {
	    BookOpWindows bopw(mwl);
	    bopw.bookSaveRestore(window, bookv, 2, mod_col);
	  });
#endif
	  grid->attach(*restore, 1, 12, 1, 1);

	  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
	  cancel->set_name("cancelBut");
	  cancel->set_halign(Gtk::Align::CENTER);
	  cancel->set_margin(5);
	  cancel->set_label(gettext("Cancel"));
	  cancel->signal_clicked().connect([window]
	  {
	    window->close();
	  });
	  grid->attach(*cancel, 2, 12, 1, 1);

	  window->signal_close_request().connect([window, bookv]
	  {
	    bookv->clear();
	    delete bookv;
#ifdef ML_GTK_OLD
	    window->hide();
#endif
#ifndef ML_GTK_OLD
						   window->set_visible(false);
#endif
						   delete window;
						   return true;
						 },
						 false);
	  window->present();
	}
      else
	{
	  delete bookv;
	}
    }
  else
    {
      delete bookv;
    }
}
#endif

#ifdef ML_GTK_OLD
void
BookOpWindows::fileInfo(int variant)
{
  Glib::RefPtr<Gtk::TreeSelection> selection;
  Gtk::Window *parent_window = nullptr;
  switch(variant)
    {
    case 1:
      {
	Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
	Gtk::Paned *pn =
	    dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
	Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
	Gtk::ScrolledWindow *sres_scrl =
	    dynamic_cast<Gtk::ScrolledWindow*>(right_grid->get_child_at(0, 0));
	Gtk::TreeView *sres =
	    dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
	selection = sres->get_selection();
	parent_window = mw;
	break;
      }
    case 2:
      {
	selection = mw->bm_tv->get_selection();
	Gtk::Widget *widg = mw->bm_tv->get_parent();
	if(widg)
	  {
	    widg = widg->get_parent();
	    if(widg)
	      {
		parent_window = dynamic_cast<Gtk::Window*>(widg->get_parent());
	      }
	  }
	break;
      }
    default:
      break;
    }

  if(selection)
    {
      Gtk::TreeModel::iterator iter = selection->get_selected();
      if(iter)
	{
	  Gtk::Window *window = new Gtk::Window;
	  window->set_application(mw->get_application());
	  window->set_title(gettext("Book file info"));
	  if(parent_window)
	    {
	      window->set_transient_for(*parent_window);
	    }
	  window->set_name("MLwindow");

	  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
	  grid->set_halign(Gtk::Align::FILL);
	  grid->set_valign(Gtk::Align::FILL);
	  window->set_child(*grid);

	  size_t id;
	  AuxFunc af;
	  std::filesystem::path filepath;
	  iter->get_value(0, id);
	  std::string filename;
	  switch(variant)
	    {
	    case 1:
	      {
		filename = (mw->search_result_v[id - 1]).path_to_book;
		break;
	      }
	    case 2:
	      {
		filename = mw->bookmark_v[id - 1].path_to_book;
		break;
	      }
	    default:
	      break;
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
	      filepath = std::filesystem::u8path(archpath);
	      filepath.make_preferred();
	      if(std::filesystem::exists(filepath))
		{
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
			  Glib::ustring("<i>")
			      + Glib::ustring(std::get<1>(*itinfv))
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
			  Glib::ustring("<i>")
			      + Glib::ustring(std::get<1>(*itinfv))
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
			  Glib::ustring("<i>")
			      + Glib::ustring(std::get<1>(*itinfv))
			      + Glib::ustring("</i> ")
			      + Glib::ustring(gettext("bytes")));
		    }
		  grid->attach(*sz_lb2, 1, 3, 1, 1);
		}
	      else
		{
		  window->set_default_size(1, 1);
		  Gtk::Label *warn_lb = Gtk::make_managed<Gtk::Label>();
		  warn_lb->set_margin(5);
		  warn_lb->set_halign(Gtk::Align::START);
		  warn_lb->set_use_markup(true);
		  warn_lb->set_markup(
		      Glib::ustring("<i>")
			  + Glib::ustring(gettext("File does not exists!"))
			  + Glib::ustring("</i>"));
		  grid->attach(*warn_lb, 0, 0, 1, 1);
		}
	    }
	  else
	    {
	      filepath = std::filesystem::u8path(filename);
	      filepath.make_preferred();
	      if(std::filesystem::exists(filepath))
		{
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
		      Glib::ustring("<b>")
			  + Glib::ustring(gettext("File size: "))
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
	      else
		{
		  window->set_default_size(1, 1);
		  Gtk::Label *warn_lb = Gtk::make_managed<Gtk::Label>();
		  warn_lb->set_margin(5);
		  warn_lb->set_halign(Gtk::Align::START);
		  warn_lb->set_use_markup(true);
		  warn_lb->set_markup(
		      Glib::ustring("<i>")
			  + Glib::ustring(gettext("File does not exists!"))
			  + Glib::ustring("</i>"));
		  grid->attach(*warn_lb, 0, 0, 1, 1);
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
    }
}

void
BookOpWindows::editBook()
{
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(mw->get_child());
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(main_grid->get_child_at(0, 1));
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(pn->get_end_child());
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(right_grid
      ->get_child_at(0, 0));
  Gtk::TreeView *sres = dynamic_cast<Gtk::TreeView*>(sres_scrl->get_child());
  Glib::RefPtr<Gtk::TreeSelection> selection = sres->get_selection();

  std::vector<std::tuple<std::string, std::string>> *bookv = new std::vector<
      std::tuple<std::string, std::string>>();

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
	  std::get<1>(ttup) = (mw->search_result_v[id - 1]).genre;
	  bookv->push_back(ttup);

	  val.clear();
	  iter->get_value(5, val);
	  std::get<0>(ttup) = "Date";
	  std::get<1>(ttup) = std::string(val);
	  bookv->push_back(ttup);

	  std::get<0>(ttup) = "filepath";
	  std::get<1>(ttup) = (mw->search_result_v[id - 1]).path_to_book;
	  bookv->push_back(ttup);

	  Gtk::Window *window = new Gtk::Window;
	  window->set_application(mw->get_application());
	  window->set_title(gettext("Book entry editor"));
	  window->set_transient_for(*mw);
	  window->set_name("MLwindow");
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
	  scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
			   Gtk::PolicyType::AUTOMATIC);
	  Gtk::Grid *scrl_gr = Gtk::make_managed<Gtk::Grid>();
	  scrl_gr->set_halign(Gtk::Align::START);

	  Gtk::Popover *popover = Gtk::make_managed<Gtk::Popover>();
	  popover->set_child(*scrl);
	  genre_but->set_popover(*popover);

	  int maxl = 0;
	  Gtk::Expander *maxexp = nullptr;
	  for(size_t i = 0; i < mw->genrev->size(); i++)
	    {
	      Glib::ustring g_group(mw->genrev->at(i).header);

	      if(i == 0)
		{
		  Gtk::Label *txtl = Gtk::make_managed<Gtk::Label>();
		  txtl->set_margin(2);
		  txtl->set_halign(Gtk::Align::START);
		  txtl->set_text(g_group);
		  Glib::RefPtr<Gtk::GestureClick> clck =
		      Gtk::GestureClick::create();
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
		  std::vector<genre_item> tmpv = mw->genrev->at(i).items;
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
		      txtl->set_text(Glib::ustring(tmpv[j].name));
		      std::string code = tmpv[j].code;
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
	  save->set_name("applyBut");
	  save->set_halign(Gtk::Align::CENTER);
	  save->set_margin(5);
	  save->set_label(gettext("Save"));
	  MainWindow *mwl = mw;
	  save->signal_clicked().connect(
	      [bookv, auth_ent, book_ent, series_ent, genre_ent, date_ent, mwl,
	       window]
	      {
		BookOpWindows bopw(mwl);
		bopw.bookSaveRestore(window, bookv, 1);
	      });
	  grid->attach(*save, 0, 12, 1, 1);

	  Gtk::Button *restore = Gtk::make_managed<Gtk::Button>();
	  restore->set_name("operationBut");
	  restore->set_halign(Gtk::Align::CENTER);
	  restore->set_margin(5);
	  restore->set_label(gettext("Restore"));
	  restore->signal_clicked().connect(
	      [bookv, auth_ent, book_ent, series_ent, genre_ent, date_ent, mwl,
	       window]
	      {
		BookOpWindows bopw(mwl);
		bopw.bookSaveRestore(window, bookv, 2);
	      });
	  grid->attach(*restore, 1, 12, 1, 1);

	  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
	  cancel->set_name("cancelBut");
	  cancel->set_halign(Gtk::Align::CENTER);
	  cancel->set_margin(5);
	  cancel->set_label(gettext("Cancel"));
	  cancel->signal_clicked().connect([window]
	  {
	    window->close();
	  });
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
	  window->present();
	}
      else
	{
	  delete bookv;
	}
    }
  else
    {
      delete bookv;
    }
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
	  new std::vector<std::tuple<std::string, std::string>>();
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
      window->set_name("MLwindow");
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
						    false, cncl);
      MainWindow *mwl = mw;
      Glib::Dispatcher *disp_corrected = new Glib::Dispatcher();
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
	  return el.path_to_book == searchstr;
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
		(*itsv).authors = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Book";
	      });
	    if(itnewb != newbase->end())
	      {
		(*itsv).book = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Series";
	      });
	    if(itnewb != newbase->end())
	      {
		(*itsv).series = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Genre";
	      });
	    if(itnewb != newbase->end())
	      {
		(*itsv).genre = std::get<1>(*itnewb);
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Date";
	      });
	    if(itnewb != newbase->end())
	      {
		(*itsv).date = std::get<1>(*itnewb);
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
	crgr.searchResultShow(1, nullptr);
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
	close->set_name("applyBut");
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
      yes->set_name("applyBut");
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
      no->set_name("cancelBut");
      no->set_halign(Gtk::Align::CENTER);
      no->set_margin(5);
      no->set_label(gettext("No"));
      no->signal_clicked().connect([window]
      {
	window->close();
      });
      grid->attach(*no, 1, 1, 1, 1);

      window->signal_close_request().connect(
	  [window, newbase, disp_corrected, cncl, rc]
	  {
	    newbase->clear();
	    delete newbase;
	    delete cncl;
	    delete rc;
	    delete disp_corrected;
	    window->hide();
	    delete window;
	    return true;
	  },
	  false);
      window->present();
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
#endif

#ifndef ML_GTK_OLD
void
BookOpWindows::bookSaveRestore(
    Gtk::Window *win, std::vector<std::tuple<std::string, std::string>> *bookv,
    int variant, Glib::RefPtr<ModelColumns> mod_col)
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
	  new std::vector<std::tuple<std::string, std::string>>();
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
      window->set_name("MLwindow");
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
						    false, cncl);
      MainWindow *mwl = mw;
      Glib::Dispatcher *disp_corrected = new Glib::Dispatcher();
      disp_corrected->connect([window, win, mwl, bookv, newbase, mod_col]
      {
	if(mod_col && mwl->list_sr)
	  {
	    auto sr = mwl->list_sr->find(mod_col);
	    if(std::get<0>(sr))
	      {
		mwl->list_sr->remove(std::get<1>(sr));
	      }
	    auto itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Author";
	      });
	    if(itnewb != newbase->end())
	      {
		mod_col->author = Glib::ustring(std::get<1>(*itnewb));
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Book";
	      });
	    if(itnewb != newbase->end())
	      {
		mod_col->book = Glib::ustring(std::get<1>(*itnewb));
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Series";
	      });
	    if(itnewb != newbase->end())
	      {
		mod_col->series = Glib::ustring(std::get<1>(*itnewb));
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Genre";
	      });
	    if(itnewb != newbase->end())
	      {
		mod_col->genre = Glib::ustring(std::get<1>(*itnewb));
	      }

	    itnewb = std::find_if(newbase->begin(), newbase->end(), []
	    (auto &el)
	      {
		return std::get<0>(el) == "Date";
	      });
	    if(itnewb != newbase->end())
	      {
		mod_col->date = Glib::ustring(std::get<1>(*itnewb));
	      }
	    mwl->list_sr->insert(std::get<1>(sr), mod_col);
	  }
	mwl->prev_search_nm.clear();

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
	close->set_name("applyBut");
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
      yes->set_name("applyBut");
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
      no->set_name("cancelBut");
      no->set_halign(Gtk::Align::CENTER);
      no->set_margin(5);
      no->set_label(gettext("No"));
      no->signal_clicked().connect([window]
      {
	window->close();
      });
      grid->attach(*no, 1, 1, 1, 1);

      window->signal_close_request().connect(
	  [window, newbase, disp_corrected, cncl, rc]
	  {
	    newbase->clear();
	    delete newbase;
	    delete cncl;
	    delete rc;
	    delete disp_corrected;
#ifdef ML_GTK_OLD
	    window->hide();
#endif
#ifndef ML_GTK_OLD
	    window->set_visible(false);
#endif
	    delete window;
	    return true;
	  },
	  false);
      window->present();
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

#ifndef ML_GTK_OLD
void
BookOpWindows::openBook()
{
  Glib::RefPtr<Gtk::SingleSelection> selection;
  Gtk::Widget *widg = mw->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(0, 0);
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
  widg = sres_scrl->get_child();
  Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(widg);
  if(sres)
    {
      auto sm = sres->get_model();
      selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(sm);
    }
  Glib::RefPtr<Glib::ObjectBase> obj;
  if(selection)
    {
      if(mw->ms_sel_book)
	{
	  auto lm = selection->get_model();
	  if(lm)
	    {
	      if(lm->get_n_items() > *(mw->ms_sel_book))
		{
		  obj = lm->get_object(*(mw->ms_sel_book));
		}
	    }
	}
      else
	{
	  obj = selection->get_selected_item();
	}
    }
  if(obj)
    {
      auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
      if(mod_col)
	{
	  openBookFunc(mod_col->path);
	}
    }
}

void
BookOpWindows::openBookFunc(Glib::ustring input_path)
{
  AuxFunc af;
  std::filesystem::path filepath;
  std::string filename(input_path);
  std::string::size_type n;
  n = filename.find("<zip>");
  if(n != std::string::npos)
    {
      std::string archpath = filename;
      archpath.erase(
	  0, archpath.find("<archpath>") + std::string("<archpath>").size());
      archpath = archpath.substr(0, archpath.find("</archpath>"));
      std::string ind_str = filename;
      ind_str.erase(0, ind_str.find("<index>") + std::string("<index>").size());
      ind_str = ind_str.substr(0, ind_str.find("</index>"));
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << ind_str;
      int index;
      strm >> index;
      std::string outfolder = af.temp_path();
      outfolder = outfolder + "/MyLibraryForReading";
      std::filesystem::path ffr = std::filesystem::u8path(outfolder);
      if(std::filesystem::exists(ffr))
	{
	  std::filesystem::remove_all(ffr);
	}
      std::filesystem::path chext = std::filesystem::u8path(archpath);
      if(chext.extension().u8string() == ".zip")
	{
	  af.unpackByIndex(archpath, outfolder, index);
	}
      else
	{
	  af.unpackByIndexNonZip(archpath, outfolder, index);
	}
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
      std::cout << "Book open command result code: " << check << std::endl;
#endif
#ifdef _WIN32
      ShellExecuteA (0, af.utf8to ("open").c_str (), commstr.c_str (), 0, 0, 0);
#endif
    }
}
#endif

void
BookOpWindows::copyTo(Gtk::ColumnView *sres, int variant, Gtk::Window *win)
{
  auto sm = sres->get_model();
  Glib::RefPtr<Gtk::SingleSelection> selection = std::dynamic_pointer_cast<
      Gtk::SingleSelection>(sm);
  Glib::RefPtr<Glib::ObjectBase> obj;
  if(variant == 1)
    {
      if(selection)
	{
	  if(mw->ms_sel_book)
	    {
	      auto lm = selection->get_model();
	      if(lm)
		{
		  if(lm->get_n_items() > *(mw->ms_sel_book))
		    {
		      obj = lm->get_object(*(mw->ms_sel_book));
		    }
		}
	    }
	  else
	    {
	      obj = selection->get_selected_item();
	    }
	}
    }
  if(obj)
    {
      auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
      if(mod_col)
	{
	  copyToFunc(mod_col->path, mod_col->author, mod_col->book, win);
	}
    }
}

void
BookOpWindows::copyToFunc(Glib::ustring input_path, Glib::ustring author,
			  Glib::ustring booknm, Gtk::Window *win)
{
  AuxFunc af;
  std::filesystem::path filepath;
  bool archive = false;
  std::string filename(input_path);
  std::string::size_type n;
  n = filename.find("<zip>");
  if(n != std::string::npos)
    {
      std::string archpath = filename;
      archpath.erase(
	  0, archpath.find("<archpath>") + std::string("<archpath>").size());
      archpath = archpath.substr(0, archpath.find("</archpath>"));
      std::string ind_str = filename;
      ind_str.erase(0, ind_str.find("<index>") + std::string("<index>").size());
      ind_str = ind_str.substr(0, ind_str.find("</index>"));
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << ind_str;
      int index;
      strm >> index;
      std::string outfolder = af.temp_path();
      outfolder = outfolder + "/" + af.randomFileName();
      std::filesystem::path ffr = std::filesystem::u8path(outfolder);
      if(std::filesystem::exists(ffr))
	{
	  std::filesystem::remove_all(ffr);
	}
      std::filesystem::path ch_zip = std::filesystem::u8path(archpath);
      if(ch_zip.extension().u8string() == ".zip")
	{
	  af.unpackByIndex(archpath, outfolder, index);
	}
      else
	{
	  af.unpackByIndexNonZip(archpath, outfolder, index);
	}
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
      BookOpWindows bopw(mw);
      bopw.saveDialog(filepath, author, booknm, archive, win);
    }
}
#endif

#ifdef ML_GTK_OLD
void
BookOpWindows::openBook(int variant)
{
  Glib::RefPtr<Gtk::TreeSelection> selection;
  if(variant == 1)
    {
      Gtk::Widget *widg = mw->get_child();
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
      selection = mw->bm_tv->get_selection();
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
	      filename = (mw->search_result_v[id - 1]).path_to_book;
	    }
	  else if(variant == 2)
	    {
	      filename = (mw->bookmark_v[id - 1]).path_to_book;
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
	      std::string outfolder = af.temp_path();
	      outfolder = outfolder + "/MyLibraryForReading";
	      std::filesystem::path ffr = std::filesystem::u8path(outfolder);
	      if(std::filesystem::exists(ffr))
		{
		  std::filesystem::remove_all(ffr);
		}
	      std::filesystem::path chext = std::filesystem::u8path(archpath);
	      if(chext.extension().u8string() == ".zip")
		{
		  af.unpackByIndex(archpath, outfolder, index);
		}
	      else
		{
		  af.unpackByIndexNonZip(archpath, outfolder, index);
		}
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
BookOpWindows::copyTo(Gtk::TreeView *sres, int variant, Gtk::Window *win)
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
	  std::string filename, author, book;
	  if(variant == 1)
	    {
	      filename = (mw->search_result_v[id - 1]).path_to_book;
	      author = (mw->search_result_v[id - 1]).authors;
	      book = (mw->search_result_v[id - 1]).book;
	    }
	  else if(variant == 2)
	    {
	      filename = (mw->bookmark_v[id - 1]).path_to_book;
	      author = (mw->bookmark_v[id - 1]).authors;
	      book = (mw->bookmark_v[id - 1]).book;
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
	      std::string outfolder = af.temp_path();
	      outfolder = outfolder + "/" + af.randomFileName();
	      std::filesystem::path ffr = std::filesystem::u8path(outfolder);
	      if(std::filesystem::exists(ffr))
		{
		  std::filesystem::remove_all(ffr);
		}
	      std::filesystem::path ch_zip = std::filesystem::u8path(archpath);
	      if(ch_zip.extension().u8string() == ".zip")
		{
		  af.unpackByIndex(archpath, outfolder, index);
		}
	      else
		{
		  af.unpackByIndexNonZip(archpath, outfolder, index);
		}
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
	      BookOpWindows bopw(mw);
	      bopw.saveDialog(filepath, author, book, archive, win);
	    }
	}
    }
}
#endif

void
BookOpWindows::saveDialog(std::filesystem::path filepath, std::string author,
			  std::string booknm, bool archive, Gtk::Window *win)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fchd = Gtk::FileDialog::create();
  fchd->set_title(gettext("Save as..."));
  fchd->set_modal(true);
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fchd->set_initial_folder(fl);
  if(!author.empty() && !booknm.empty())
    {
      std::string bp = author + " - " + booknm;
      bp = filename + "/" + bp + filepath.extension().u8string();
      fl = Gio::File::create_for_path(bp);
    }
  else
    {
      fl = Gio::File::create_for_path(filepath.u8string());
    }
  fchd->set_initial_file(fl);
  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  MainWindow *mwl = mw;
  fchd->save(*win, [mwl, archive, filepath, win]
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
	  fl = fchd->save_finish(result);
	}
      catch(Glib::Error &e)
	{
	  if(e.code() != Gtk::DialogError::DISMISSED)
	    {
	      std::cout << "saveDialog: " << e.what() << std::endl;
	    }
	}
      if(fl)
	{
	  std::string loc = fl->get_path();
	  std::filesystem::path outpath = std::filesystem::u8path(loc);
	  if(std::filesystem::exists(outpath))
	    {
	      std::filesystem::remove_all(outpath);
	    }
	  std::filesystem::copy(filepath, outpath);
	  AuxWindows aw(mwl);
	  aw.errorWin(9, win);
	  Glib::RefPtr<Glib::MainContext> mc = Glib::MainContext::get_default();
	  while(mc->pending())
	    {
	      mc->iteration(true);
	    }
	}
      if(archive)
	{
	  if(std::filesystem::exists(filepath.parent_path()))
	    {
	      std::filesystem::remove_all(filepath.parent_path());
	    }
	}
    },
	     cncl);
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fchd = new Gtk::FileChooserDialog(
      *win, gettext("Save as..."), Gtk::FileChooser::Action::SAVE, false);
  fchd->set_application(mw->get_application());
  fchd->set_modal(true);
  fchd->set_name("MLwindow");

  Gtk::Box *cont = fchd->get_content_area();
  cont->set_margin(5);

  if(!author.empty() && !booknm.empty())
    {
      std::string bp = author + " - " + booknm
	  + filepath.extension().u8string();
      fchd->set_current_name(Glib::ustring(bp));
    }
  else
    {
      fchd->set_current_name(Glib::ustring(filepath.filename().u8string()));
    }

  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(filename);
  fchd->set_current_folder(fl);

  Gtk::Button *cancel = fchd->add_button(gettext("Cancel"),
					 Gtk::ResponseType::CANCEL);
  cancel->set_name("cancelBut");
  cancel->set_margin(5);

  Gtk::Requisition min, nat;
  fchd->get_preferred_size(min, nat);

  Gtk::Button *save = fchd->add_button(gettext("Save"),
				       Gtk::ResponseType::ACCEPT);
  save->set_margin_bottom(5);
  save->set_margin_end(5);
  save->set_margin_top(5);
  save->set_margin_start(nat.get_width() - 15);
  save->set_name("applyBut");

  MainWindow *mwl = mw;
  fchd->signal_response().connect([fchd, filepath, mwl, win]
  (int resp_id)
    {
      if(resp_id == Gtk::ResponseType::CANCEL)
	{
	  fchd->close();
	}
      else if (resp_id == Gtk::ResponseType::ACCEPT)
	{
	  Glib::RefPtr<Gio::File> fl = fchd->get_file();
	  std::string loc;
	  if(fl)
	    {
	      loc = fl->get_path();
	      std::filesystem::path outpath = std::filesystem::u8path(loc);
	      if(std::filesystem::exists(outpath))
		{
		  std::filesystem::remove_all(outpath);
		}
	      std::filesystem::copy(filepath, outpath);
	      AuxWindows aw(mwl);
	      aw.errorWin(9, win);
	      fchd->close();
	    }
	}
    });

  fchd->signal_close_request().connect([fchd, archive, filepath]
  {
    if(archive)
      {
	if(std::filesystem::exists(filepath.parent_path()))
	  {
	    std::filesystem::remove_all(filepath.parent_path());
	  }
      }
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
BookOpWindows::createBookmark()
{
  Gtk::Widget *widg = mw->get_child();
  Gtk::Grid *main_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = main_grid->get_child_at(0, 1);
  Gtk::Paned *pn = dynamic_cast<Gtk::Paned*>(widg);
  widg = pn->get_end_child();
  Gtk::Grid *right_grid = dynamic_cast<Gtk::Grid*>(widg);
  widg = right_grid->get_child_at(0, 0);
  Gtk::ScrolledWindow *sres_scrl = dynamic_cast<Gtk::ScrolledWindow*>(widg);
  widg = sres_scrl->get_child();
  Gtk::ColumnView *sres = dynamic_cast<Gtk::ColumnView*>(widg);
  auto sl = sres->get_model();
  if(sl)
    {
      Glib::RefPtr<Gtk::SingleSelection> selection = std::dynamic_pointer_cast<
	  Gtk::SingleSelection>(sl);
      if(selection)
	{
	  auto obj = selection->get_selected_item();
	  if(obj)
	    {
	      auto mod_col = std::dynamic_pointer_cast<ModelColumns>(obj);
	      if(mod_col)
		{
		  AuxFunc af;
		  std::string filename;
		  af.homePath(&filename);
		  filename = filename + "/.MyLibrary/Bookmarks";
		  std::filesystem::path filepath = std::filesystem::u8path(
		      filename);
		  filename = std::string(mod_col->path);
		  std::string bookline;
		  bookline = std::string(mod_col->author);
		  bookline = bookline + "<?>" + std::string(mod_col->book);
		  bookline = bookline + "<?>" + std::string(mod_col->series);
		  CreateRightGrid crg(mw);
		  bookline = bookline + "<?>"
		      + std::string(crg.genre_str(mod_col->genre));
		  bookline = bookline + "<?>" + std::string(mod_col->date);
		  bookline = bookline + "<?>" + filename;
		  bookline = bookline + "<?L>";
		  std::fstream f;
		  f.open(
		      filepath,
		      std::ios_base::out | std::ios_base::app
			  | std::ios_base::binary);
		  f.write(bookline.c_str(), bookline.size());
		  f.close();
		  AuxWindows aw(mw);
		  aw.errorWin(8, mw);
		}
	    }
	}
    }
}
#endif
