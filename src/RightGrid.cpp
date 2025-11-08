/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <BookInfoGui.h>
#include <BooksWindow.h>
#include <CopyBookGui.h>
#include <EditBookGui.h>
#include <FullSizeCover.h>
#include <NotesGui.h>
#include <RemoveBookGui.h>
#include <RightGrid.h>
#include <SaveCover.h>
#include <TransferBookGui.h>
#include <filesystem>
#include <giomm-2.68/giomm/liststore.h>
#include <giomm-2.68/giomm/menuitem.h>
#include <giomm-2.68/giomm/simpleaction.h>
#include <giomm-2.68/giomm/simpleactiongroup.h>
#include <gtkmm-4.0/gdkmm/display.h>
#include <gtkmm-4.0/gdkmm/general.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gdkmm/surface.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/eventcontrollerkey.h>
#include <gtkmm-4.0/gtkmm/gestureclick.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/linkbutton.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/separator.h>
#include <gtkmm-4.0/gtkmm/singleselection.h>
#include <gtkmm-4.0/gtkmm/sortlistmodel.h>
#include <gtkmm-4.0/gtkmm/textchildanchor.h>
#include <gtkmm-4.0/gtkmm/textiter.h>
#include <gtkmm-4.0/gtkmm/textmark.h>
#include <iostream>
#include <libintl.h>
#include <thread>

RightGrid::RightGrid(const std::shared_ptr<AuxFunc> &af,
                     Gtk::Window *main_window,
                     const std::shared_ptr<BookMarks> &bookmarks,
                     const std::shared_ptr<NotesKeeper> &notes,
                     const ShowVariant &sv)
{
  this->af = af;
  this->main_window = main_window;
  this->bookmarks = bookmarks;
  this->notes = notes;
  show_variant = sv;
  bi = new BookInfo(af);
  formatter = new FormatAnnotation(af);
  std::vector<ReplaceTagItem> tag_replacement_table;
  std::vector<std::tuple<std::string, std::string>> symbols_replacement;
  BookInfoGui::formReplacementTable(tag_replacement_table,
                                    symbols_replacement);
  formatter->setTagReplacementTable(tag_replacement_table,
                                    symbols_replacement);
  open_book = new OpenBook(af);
  bookOperationsActionGroup();
  coverOperationsActionGroup();
}

RightGrid::~RightGrid()
{
  delete srs;
  delete bi;
  delete formatter;
  delete open_book;
}

Gtk::Grid *
RightGrid::createGrid()
{
  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);

  grid->signal_realize().connect(std::bind(&RightGrid::getDpi, this));

  int height = 4;
  Gtk::ScrolledWindow *search_res_scrl
      = Gtk::make_managed<Gtk::ScrolledWindow>();
  search_res_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                              Gtk::PolicyType::AUTOMATIC);
  search_res_scrl->set_halign(Gtk::Align::FILL);
  search_res_scrl->set_valign(Gtk::Align::FILL);
  search_res_scrl->set_expand(true);
  search_res_scrl->set_margin(5);
  search_res_scrl->set_has_frame(true);
  grid->attach(*search_res_scrl, 0, 0, 1, height);

  search_res = Gtk::make_managed<Gtk::ColumnView>();
  search_res->set_halign(Gtk::Align::FILL);
  search_res->set_valign(Gtk::Align::FILL);
  search_res->set_reorderable(true);
  search_res->set_single_click_activate(true);
  search_res->set_name("tablesView");
  Glib::PropertyProxy<bool> row_sep
      = search_res->property_show_row_separators();
  row_sep.set_value(true);
  Glib::PropertyProxy<bool> column_sep
      = search_res->property_show_column_separators();
  column_sep.set_value(true);
  search_res->signal_activate().connect(
      std::bind(&RightGrid::slotRowActivated, this, std::placeholders::_1));
  search_res_scrl->set_child(*search_res);

  srs = new SearchResultShow(af, search_res);

  bookMenu(menu_sr);

  Gtk::PopoverMenu *pop_menu = Gtk::make_managed<Gtk::PopoverMenu>();
  pop_menu->set_menu_model(menu_sr);
  pop_menu->set_parent(*search_res);
  search_res->signal_unrealize().connect(
      [pop_menu]
        {
          pop_menu->unparent();
        });

  Glib::RefPtr<Gtk::GestureClick> clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect(
      std::bind(&RightGrid::showPopupMenu, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3, pop_menu));
  search_res->add_controller(clck);

  Gtk::Grid *ops_grid = Gtk::make_managed<Gtk::Grid>();
  ops_grid->set_halign(Gtk::Align::FILL);
  ops_grid->set_valign(Gtk::Align::FILL);
  grid->attach(*ops_grid, 0, height, 1, 1);
  height++;

  book_ops = Gtk::make_managed<Gtk::MenuButton>();
  book_ops->set_margin(5);
  book_ops->set_halign(Gtk::Align::START);
  book_ops->set_valign(Gtk::Align::START);
  book_ops->set_label(gettext("Book operations"));
  book_ops->set_name("menBut");
  book_ops->set_menu_model(menu_sr);
  ops_grid->attach(*book_ops, 0, 0, 1, 1);

  Gtk::Separator *sep
      = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::VERTICAL);
  ops_grid->attach(*sep, 1, 0, 1, 1);

  Gtk::Box *filter_box = Gtk::make_managed<Gtk::Box>();
  filter_box->set_margin_start(10);
  ops_grid->attach(*filter_box, 2, 0, 1, 1);

  Gtk::Label *filter_lab = Gtk::make_managed<Gtk::Label>();
  filter_lab->set_margin(5);
  filter_lab->set_halign(Gtk::Align::START);
  filter_lab->set_text(gettext("Filtering:"));
  filter_lab->set_name("windowLabel");
  filter_box->append(*filter_lab);

  Gtk::Entry *filter_ent = Gtk::make_managed<Gtk::Entry>();
  filter_ent->set_margin(5);
  filter_ent->set_has_frame(true);
  filter_ent->set_name("windowEntry");
  filter_ent->set_width_chars(20);
  Glib::RefPtr<Gtk::EventControllerKey> key
      = Gtk::EventControllerKey::create();
  key->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
  key->signal_key_pressed().connect(
      [this, filter_ent](guint keyval, guint, Gdk::ModifierType)
        {
          if(keyval == GDK_KEY_Return)
            {
              Glib::RefPtr<Gtk::EntryBuffer> buf = filter_ent->get_buffer();
              if(filter_selection->is_visible())
                {
                  srs->filterBooks(buf->get_text(),
                                   filter_selection->get_selected());
                }
              else
                {
                  srs->filterFiles(buf->get_text());
                }
              return true;
            }
          return false;
        },
      false);
  filter_ent->add_controller(key);
  filter_box->append(*filter_ent);

  Glib::RefPtr<Gtk::StringList> filter_list
      = Gtk::StringList::create(std::vector<Glib::ustring>());
  Glib::ustring col = gettext("Column");
  filter_list->append(col + ": \'" + gettext("Author") + "\'");
  filter_list->append(col + ": \'" + gettext("Book") + "\'");
  filter_list->append(col + ": \'" + gettext("Series") + "\'");
  filter_list->append(col + ": \'" + gettext("Genre") + "\'");
  filter_list->append(col + ": \'" + gettext("Date") + "\'");

  filter_selection = Gtk::make_managed<Gtk::DropDown>();
  filter_selection->set_margin(5);
  filter_selection->set_name("comboBox");
  filter_selection->set_model(filter_list);
  filter_box->append(*filter_selection);

  Gtk::Button *filter_but = Gtk::make_managed<Gtk::Button>();
  filter_but->set_margin(5);
  filter_but->set_halign(Gtk::Align::START);
  filter_but->set_name("operationBut");
  filter_but->set_label(gettext("Filter"));
  filter_but->signal_clicked().connect(
      [this, filter_ent]
        {
          Glib::RefPtr<Gtk::EntryBuffer> buf = filter_ent->get_buffer();
          if(filter_selection->is_visible())
            {
              srs->filterBooks(buf->get_text(),
                               filter_selection->get_selected());
            }
          else
            {
              srs->filterFiles(buf->get_text());
            }
        });
  filter_box->append(*filter_but);

  Gtk::Button *clear_filter_but = Gtk::make_managed<Gtk::Button>();
  clear_filter_but->set_margin(5);
  clear_filter_but->set_halign(Gtk::Align::START);
  clear_filter_but->set_name("cancelBut");
  clear_filter_but->set_label(gettext("Clear filter"));
  clear_filter_but->signal_clicked().connect(
      [this, filter_ent]
        {
          Glib::RefPtr<Gtk::EntryBuffer> buf = filter_ent->get_buffer();
          Glib::ustring txt("");
          buf->set_text(txt);
          srs->filterFiles(txt);
          for(guint i = 0; i < 5; i++)
            {
              srs->filterBooks(txt, i);
            }
        });
  filter_box->append(*clear_filter_but);

  Gtk::Grid *annot_cover_grid = Gtk::make_managed<Gtk::Grid>();
  annot_cover_grid->set_halign(Gtk::Align::FILL);
  annot_cover_grid->set_valign(Gtk::Align::FILL);
  annot_cover_grid->set_expand(true);
  grid->attach(*annot_cover_grid, 0, height, 1, 1);

  int width = 6;
  Gtk::ScrolledWindow *annotation_scrl
      = Gtk::make_managed<Gtk::ScrolledWindow>();
  annotation_scrl->set_policy(Gtk::PolicyType::AUTOMATIC,
                              Gtk::PolicyType::AUTOMATIC);
  annotation_scrl->set_margin(5);
  annotation_scrl->set_halign(Gtk::Align::FILL);
  annotation_scrl->set_valign(Gtk::Align::FILL);
  annotation_scrl->set_expand(true);
  annotation_scrl->set_has_frame(true);
  annot_cover_grid->attach(*annotation_scrl, 0, 0, width, 1);

  annotation = Gtk::make_managed<Gtk::TextView>();
  annotation->set_halign(Gtk::Align::FILL);
  annotation->set_valign(Gtk::Align::FILL);
  annotation->set_expand(true);
  annotation->set_wrap_mode(Gtk::WrapMode::WORD);
  annotation->set_editable(false);
  annotation->set_justification(Gtk::Justification::FILL);
  annotation->set_top_margin(5);
  annotation->set_right_margin(5);
  annotation->set_bottom_margin(5);
  annotation->set_left_margin(5);
  annotation->set_name("textField");
  annotation_scrl->set_child(*annotation);

  cover_area = Gtk::make_managed<Gtk::DrawingArea>();
  cover_area->set_margin(5);
  cover_area->set_halign(Gtk::Align::FILL);
  cover_area->set_valign(Gtk::Align::FILL);
  cover_area->set_expand(true);
  cover_area->set_draw_func(
      std::bind(&RightGrid::coverDraw, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  annot_cover_grid->attach(*cover_area, width, 0, 1, 1);

  Glib::RefPtr<Gio::Menu> menu = coverMenu();
  pop_menu = Gtk::make_managed<Gtk::PopoverMenu>();
  pop_menu->set_menu_model(menu);
  pop_menu->set_parent(*cover_area);
  cover_area->signal_unrealize().connect(
      [pop_menu]
        {
          pop_menu->unparent();
        });

  clck = Gtk::GestureClick::create();
  clck->set_button(1);
  clck->signal_pressed().connect(
      [this](int, double, double)
        {
          coverFullSize();
        });
  cover_area->add_controller(clck);

  clck = Gtk::GestureClick::create();
  clck->set_button(3);
  clck->signal_pressed().connect(
      std::bind(&RightGrid::showCoverPopupMenu, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3, pop_menu));
  cover_area->add_controller(clck);

  created_grid = grid;

  return grid;
}

Gtk::ColumnView *
RightGrid::getSearchResult()
{
  return search_res;
}

void
RightGrid::clearSearchResult()
{
  srs->clearSearchResult();
  Glib::RefPtr<Gtk::TextBuffer> tb = Gtk::TextBuffer::create();
  annotation->set_buffer(tb);
  bie.reset();
  cover_buf.clearImage();
  cover_area->queue_draw();
  current_collection.clear();
}

void
RightGrid::searchResultShow(const std::vector<BookBaseEntry> &result,
                            const ShowVariant &sv)
{
  switch(sv)
    {
    case ShowVariant::MainWindow:
      {
        Gtk::Window *window = new Gtk::Window;
        window->set_application(main_window->get_application());
        window->set_transient_for(*main_window);
        window->set_name("MLwindow");
        window->set_modal(true);
        window->set_deletable(false);

        Gtk::Box *box
            = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        box->set_halign(Gtk::Align::FILL);
        box->set_valign(Gtk::Align::FILL);
        window->set_child(*box);

        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_margin(5);
        lab->set_halign(Gtk::Align::CENTER);
        lab->set_valign(Gtk::Align::CENTER);
        lab->set_name("windowLabel");
        lab->set_text(gettext("Sorting..."));
        box->append(*lab);

        window->signal_close_request().connect(
            [window, this]
              {
                std::unique_ptr<Gtk::Window> win(window);
                win->set_visible(false);
                return true;
              },
            false);

        Glib::Dispatcher *result_disp = new Glib::Dispatcher;
        result_disp->connect(
            [this, result, result_disp, window]
              {
                std::unique_ptr<Glib::Dispatcher> disp(result_disp);
                srs->searchResultShow(result);
                if(get_current_collection_name)
                  {
                    current_collection = get_current_collection_name();
                  }
                bookMenu(menu_sr);
                filter_selection->set_visible(true);
                book_ops->set_label(gettext("Book operations"));
                window->close();
              });

        window->present();

        std::thread thr(
            [result_disp]
              {
                result_disp->emit();
              });
        thr.detach();
        break;
      }
    case ShowVariant::SeparateWindow:
      {
        BooksWindow *bw = new BooksWindow(
            main_window, af, get_current_collection_name(), bookmarks, notes);
        bw->set_default_size(created_grid->get_width(),
                             created_grid->get_height());
        bw->createWindow(result);
        bw->signal_close_request().connect(
            [bw]
              {
                std::unique_ptr<BooksWindow> ubw(bw);
                ubw->set_visible(false);
                return true;
              },
            false);
        bw->present();
        break;
      }
    default:
      break;
    }
}

void
RightGrid::searchResultShowFiles(const std::vector<FileParseEntry> &result)
{
  srs->searchResultShow(result);
  if(get_current_collection_name)
    {
      current_collection = get_current_collection_name();
    }
  filesMenu(menu_sr);
  filter_selection->set_visible(false);
  book_ops->set_label(gettext("File operations"));
}

void
RightGrid::searchResultShowAuthors(const std::vector<std::string> &result)
{
  srs->searchResultShow(result);
  if(get_current_collection_name)
    {
      current_collection = get_current_collection_name();
    }
  authMenu(menu_sr);
  filter_selection->set_visible(false);
  book_ops->set_label(gettext("Operations"));
}

void
RightGrid::slotRowActivated(guint pos)
{
  if(pos != GTK_INVALID_LIST_POSITION)
    {
      Glib::RefPtr<Gtk::SingleSelection> sel
          = std::dynamic_pointer_cast<Gtk::SingleSelection>(
              search_res->get_model());
      if(sel)
        {
          Glib::RefPtr<Gtk::SortListModel> sort_model
              = std::dynamic_pointer_cast<Gtk::SortListModel>(
                  sel->get_model());
          if(sort_model)
            {
              Glib::RefPtr<SearchResultModelItem> item
                  = std::dynamic_pointer_cast<SearchResultModelItem>(
                      sort_model->get_object(pos));
              if(item)
                {
                  std::cout
                      << "Selected file: " << item->bbe.file_path.u8string()
                      << std::endl;
                  srs->select_item(item);
                  setAnnotationNCover(item);
                }
              else
                {
                  Glib::RefPtr<SearchResultModelItemFL> item_fl
                      = std::dynamic_pointer_cast<SearchResultModelItemFL>(
                          sort_model->get_object(pos));
                  if(item_fl)
                    {
                      std::cout
                          << "Selected file: " << item_fl->entry.file_rel_path
                          << std::endl;
                      srs->select_item(item_fl);
                    }
                  else
                    {
                      Glib::RefPtr<SearchResultModelItemAuth> item_auth
                          = std::dynamic_pointer_cast<
                              SearchResultModelItemAuth>(
                              sort_model->get_object(pos));
                      if(item_auth)
                        {
                          std::cout << "Selected author: " << item_auth->auth
                                    << std::endl;
                          srs->select_item(item_auth);
                        }
                    }
                }
            }
        }
    }
}

void
RightGrid::setAnnotationNCover(const Glib::RefPtr<SearchResultModelItem> &item)
{
  bie.reset();
  cover_buf = CoverPixBuf();
  try
    {
      bie = bi->getBookInfo(item->bbe);
    }
  catch(std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }
  Glib::RefPtr<Gtk::TextBuffer> buffer = Gtk::TextBuffer::create();
  if(bie)
    {
      formatter->removeEscapeSequences(bie->annotation);
      std::string reserve_annot = bie->annotation;
      formatter->replaceTags(bie->annotation);
      formatter->finalCleaning(bie->annotation);

      buffer->insert_markup(buffer->begin(), Glib::ustring(bie->annotation));
      if(buffer->size() == 0)
        {
          formatter->removeAllTags(reserve_annot);
          formatter->finalCleaning(reserve_annot);
          buffer->set_text(Glib::ustring(reserve_annot));
        }
      cover_buf = CoverPixBuf(bie, formatter);
    }
  annotation->set_buffer(buffer);
  annotationParseHttp(buffer);

  cover_area->queue_draw();
}

void
RightGrid::annotationParseHttp(Glib::RefPtr<Gtk::TextBuffer> &annotation)
{
  struct self_remove
  {
    Glib::RefPtr<Gtk::TextBuffer> tb;
    Glib::RefPtr<Gtk::TextMark> mark;
    ~self_remove()
    {
      if(tb && mark)
        {
          tb->delete_mark(mark);
        }
    }
  };
  Glib::ustring sstr;
  for(int i = 1; i < 3; i++)
    {
      switch(i)
        {
        case 1:
          {
            sstr = "http://";
            break;
          }
        case 2:
          {
            sstr = "https://";
            break;
          }
        default:
          {
            sstr.clear();
            break;
          }
        }
      Gtk::TextIter begin = annotation->begin();
      bool found = false;
      for(;;)
        {
          Gtk::TextIter b1, e1;
          found = begin.forward_search(sstr, Gtk::TextSearchFlags::TEXT_ONLY,
                                       b1, e1);
          if(found)
            {
              Gtk::TextIter b2, e2;
              found = e1.forward_search(" ", Gtk::TextSearchFlags::TEXT_ONLY,
                                        b2, e2);
              self_remove markb;
              markb.tb = annotation;
              markb.mark = annotation->create_mark(b1, true);
              self_remove marke;
              marke.tb = annotation;
              Glib::ustring http;
              if(found)
                {
                  marke.mark = annotation->create_mark(e2, false);
                  http = annotation->get_text(b1, e2);
                  annotation->erase(b1, e2);
                  begin = marke.mark->get_iter();
                }
              else
                {
                  http = annotation->get_text(b1, annotation->end());
                  annotation->erase(b1, annotation->end());
                  marke.mark
                      = annotation->create_mark(annotation->end(), false);
                }
              Glib::RefPtr<Gtk::TextChildAnchor> anchor
                  = annotation->create_child_anchor(markb.mark->get_iter());
              Gtk::LinkButton *lb = Gtk::make_managed<Gtk::LinkButton>();
              lb->set_label(http);
              lb->set_uri(http);
              lb->set_visited(false);
              this->annotation->add_child_at_anchor(*lb, anchor);
              begin = marke.mark->get_iter();
            }
          else
            {
              break;
            }
        }
    }
}

void
RightGrid::coverDraw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                     int height)
{
  if(cover_buf)
    {
      Cairo::RefPtr<Cairo::ImageSurface> surf
          = cover_buf.getSurface(width, height);
      if(surf)
        {
          double x = 0.0;
          if(width > surf->get_width())
            {
              x = static_cast<double>(width - surf->get_width()) * 0.5;
            }
          double y = 0.0;
          if(height > surf->get_height())
            {
              y = static_cast<double>(height - surf->get_height()) * 0.5;
            }
          cr->set_source(surf, x, y);
          cr->paint();
        }
    }
  else
    {
      cr->set_source_rgba(255.0, 255.0, 255.0, 0.0);
      cr->rectangle(0.0, 0.0, static_cast<double>(width),
                    static_cast<double>(height));
      cr->fill();
    }
}

void
RightGrid::getDpi()
{
  Glib::RefPtr<Gdk::Display> disp = main_window->get_display();
  Glib::RefPtr<Gdk::Surface> surf = main_window->get_surface();
  Glib::RefPtr<Gdk::Monitor> monitor = disp->get_monitor_at_surface(surf);

  Gdk::Rectangle rec;
  monitor->get_geometry(rec);
  double y_pix
      = static_cast<double>(rec.get_height() * monitor->get_scale_factor());
  double x_pix
      = static_cast<double>(rec.get_width() * monitor->get_scale_factor());
  double x_mm = static_cast<double>(monitor->get_width_mm());
  double y_mm = static_cast<double>(monitor->get_height_mm());
  x_mm = x_mm * 0.039370079;
  y_mm = y_mm * 0.039370079;
  bi->setDpi(x_pix / x_mm, y_pix / y_mm);
}

void
RightGrid::bookOperationsActionGroup()
{
  Glib::RefPtr<Gio::SimpleActionGroup> book_actions
      = Gio::SimpleActionGroup::create();

  book_actions->add_action("open_book",
                           std::bind(&RightGrid::openBookAction, this));

  book_actions->add_action("create_bookmark",
                           [this]
                             {
                               auto item = srs->get_selected_item();
                               if(item)
                                 {
                                   int variant = bookmarks->createBookMark(
                                       current_collection, item->bbe);
                                   bookmarksSaveResultDialog(variant);
                                 }
                             });

  book_actions->add_action("book_info",
                           [this]
                             {
                               auto item = srs->get_selected_item();
                               if(item)
                                 {
                                   BookInfoGui *big
                                       = new BookInfoGui(af, main_window, bie);
                                   big->creatWindow(item->bbe);
                                 }
                             });

  book_actions->add_action("copy_book",
                           [this]
                             {
                               auto item = srs->get_selected_item();
                               if(item)
                                 {
                                   CopyBookGui *cbg = new CopyBookGui(
                                       af, main_window, item->bbe);
                                   cbg->createWindow();
                                 }
                             });

  book_actions->add_action("remove_book",
                           std::bind(&RightGrid::bookRemoveAction, this));

  book_actions->add_action(
      "edit_book",
      [this]
        {
          auto item = srs->get_selected_item();
          if(item)
            {
              EditBookGui *ebg = new EditBookGui(
                  af, main_window, bookmarks, current_collection, item->bbe);
              ebg->successfully_edited_signal = std::bind(
                  &RightGrid::editBookSuccessSlot, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);
              ebg->createWindow();
            }
        });

  book_actions->add_action("move_to_another_col",
                           std::bind(&RightGrid::transferBookAction, this));

  book_actions->add_action("book_notes",
                           std::bind(&RightGrid::bookNotesAction, this));

  main_window->insert_action_group("book_ops", book_actions);

  Glib::RefPtr<Gio::SimpleActionGroup> file_actions
      = Gio::SimpleActionGroup::create();

  file_actions->add_action(
      "list_books",
      [this]
        {
          Glib::RefPtr<SearchResultModelItemFL> f_item
              = srs->get_selected_item_file();
          if(f_item)
            {
              std::vector<BookBaseEntry> bbe_v;
              for(auto it = f_item->entry.books.begin();
                  it != f_item->entry.books.end(); it++)
                {
                  BookBaseEntry bbe(*it, f_item->entry.file_rel_path);
                  bbe_v.emplace_back(bbe);
                }
              srs->searchResultShow(bbe_v);
              book_ops->set_label(gettext("Book operations"));
              bookMenu(menu_sr);
              filter_selection->set_visible(true);
            }
        });

  file_actions->add_action(
      "list_books_separate_window",
      [this]
        {
          Glib::RefPtr<SearchResultModelItemFL> f_item
              = srs->get_selected_item_file();
          if(f_item)
            {
              BooksWindow *bw = new BooksWindow(main_window, af,
                                                get_current_collection_name(),
                                                bookmarks, notes);
              bw->set_default_size(created_grid->get_width(),
                                   created_grid->get_height());
              std::vector<BookBaseEntry> bbe_v;
              for(auto it = f_item->entry.books.begin();
                  it != f_item->entry.books.end(); it++)
                {
                  BookBaseEntry bbe(*it, f_item->entry.file_rel_path);
                  bbe_v.emplace_back(bbe);
                }
              bw->createWindow(bbe_v);
              bw->signal_close_request().connect(
                  [bw]
                    {
                      std::unique_ptr<BooksWindow> ubw(bw);
                      ubw->set_visible(false);
                      return true;
                    },
                  false);
              bw->present();
            }
        });

  main_window->insert_action_group("file_ops", file_actions);

  Glib::RefPtr<Gio::SimpleActionGroup> auth_actions
      = Gio::SimpleActionGroup::create();

  auth_actions->add_action(
      "list_books",
      [this]
        {
          Glib::RefPtr<SearchResultModelItemAuth> auth_item
              = srs->get_selected_item_auth();
          if(auth_item)
            {
              if(search_books_callback)
                {
                  search_books_callback(auth_item->auth);
                }
            }
        });

  auth_actions->add_action(
      "list_books_separate_window",
      [this]
        {
          Glib::RefPtr<SearchResultModelItemAuth> auth_item
              = srs->get_selected_item_auth();
          if(auth_item)
            {
              if(search_books_callback_separate_window)
                {
                  search_books_callback_separate_window(auth_item->auth);
                }
            }
        });

  main_window->insert_action_group("auth_ops", auth_actions);
}

void
RightGrid::bookMenu(Glib::RefPtr<Gio::Menu> &result)
{
  if(result)
    {
      result->remove_all();
    }
  else
    {
      result = Gio::Menu::create();
    }

  Glib::RefPtr<Gio::MenuItem> item
      = Gio::MenuItem::create(gettext("Open book"), "book_ops.open_book");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Create bookmark"),
                               "book_ops.create_bookmark");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Book info"), "book_ops.book_info");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Save book as..."),
                               "book_ops.copy_book");
  result->append_item(item);

  if(show_variant == ShowVariant::MainWindow)
    {
      item = Gio::MenuItem::create(gettext("Remove book"),
                                   "book_ops.remove_book");
      result->append_item(item);
    }

  item = Gio::MenuItem::create(gettext("Edit book entry"),
                               "book_ops.edit_book");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Notes"), "book_ops.book_notes");
  result->append_item(item);

  if(show_variant == ShowVariant::MainWindow)
    {
      item = Gio::MenuItem::create(gettext("Move to another collection"),
                                   "book_ops.move_to_another_col");
      result->append_item(item);
    }
}

void
RightGrid::filesMenu(Glib::RefPtr<Gio::Menu> &result)
{
  if(result)
    {
      result->remove_all();
    }
  else
    {
      result = Gio::Menu::create();
    }

  Glib::RefPtr<Gio::MenuItem> item
      = Gio::MenuItem::create(gettext("Show books"), "file_ops.list_books");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Show books in separate window"),
                               "file_ops.list_books_separate_window");
  result->append_item(item);
}

void
RightGrid::authMenu(Glib::RefPtr<Gio::Menu> &result)
{
  if(result)
    {
      result->remove_all();
    }
  else
    {
      result = Gio::Menu::create();
    }

  Glib::RefPtr<Gio::MenuItem> item
      = Gio::MenuItem::create(gettext("Show books"), "auth_ops.list_books");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Show books in separate window"),
                               "auth_ops.list_books_separate_window");
  result->append_item(item);
}

void
RightGrid::showPopupMenu(int, double x, double y, Gtk::PopoverMenu *pop_menu)
{
  Gdk::Rectangle rec(static_cast<int>(x), static_cast<int>(y), 1, 1);
  Glib::RefPtr<Gtk::SingleSelection> sing_sel
      = std::dynamic_pointer_cast<Gtk::SingleSelection>(
          search_res->get_model());
  if(sing_sel)
    {
      Glib::RefPtr<SearchResultModelItem> item
          = std::dynamic_pointer_cast<SearchResultModelItem>(
              sing_sel->get_selected_item());
      if(item)
        {
          pop_menu->set_pointing_to(rec);
          pop_menu->popup();
          srs->select_item(item);
          setAnnotationNCover(item);
        }
      else
        {
          Glib::RefPtr<SearchResultModelItemFL> f_item
              = std::dynamic_pointer_cast<SearchResultModelItemFL>(
                  sing_sel->get_selected_item());
          if(f_item)
            {
              pop_menu->set_pointing_to(rec);
              pop_menu->popup();
              srs->select_item(f_item);
            }
          else
            {
              Glib::RefPtr<SearchResultModelItemAuth> auth_item
                  = std::dynamic_pointer_cast<SearchResultModelItemAuth>(
                      sing_sel->get_selected_item());
              if(auth_item)
                {
                  pop_menu->set_pointing_to(rec);
                  pop_menu->popup();
                  srs->select_item(auth_item);
                }
            }
        }
    }
}

void
RightGrid::coverOperationsActionGroup()
{
  Glib::RefPtr<Gio::SimpleActionGroup> cover_actions
      = Gio::SimpleActionGroup::create();

  cover_actions->add_action("full_size",
                            std::bind(&RightGrid::coverFullSize, this));

  cover_actions->add_action("save_cover",
                            std::bind(&RightGrid::saveCover, this));

  main_window->insert_action_group("cover_ops", cover_actions);
}

Glib::RefPtr<Gio::Menu>
RightGrid::coverMenu()
{
  Glib::RefPtr<Gio::Menu> result = Gio::Menu::create();

  Glib::RefPtr<Gio::MenuItem> item = Gio::MenuItem::create(
      gettext("Show full size"), "cover_ops.full_size");
  result->append_item(item);

  item = Gio::MenuItem::create(gettext("Save cover"), "cover_ops.save_cover");
  result->append_item(item);

  return result;
}

void
RightGrid::showCoverPopupMenu(int, double x, double y,
                              Gtk::PopoverMenu *pop_menu)
{
  if(bie)
    {
      if(!bie->cover.empty()
         && bie->cover_type != BookInfoEntry::cover_types::error)
        {
          Gdk::Rectangle rec(static_cast<int>(x), static_cast<int>(y), 1, 1);
          pop_menu->set_pointing_to(rec);
          pop_menu->popup();
        }
    }
}

void
RightGrid::coverFullSize()
{
  if(cover_buf)
    {
      FullSizeCover *fsc = new FullSizeCover(main_window, cover_buf);
      fsc->signal_close_request().connect(
          [fsc]
            {
              std::unique_ptr<FullSizeCover> f(fsc);
              f->set_visible(false);
              return true;
            },
          false);
      fsc->present();
    }
}

void
RightGrid::saveCover()
{
  if(bie)
    {
      if(!bie->cover.empty()
         && bie->cover_type != BookInfoEntry::cover_types::error)
        {
          SaveCover *sc = new SaveCover(bie, main_window, af);
          sc->signal_close_request().connect(
              [sc]
                {
                  std::unique_ptr<SaveCover> s(sc);
                  s->set_visible(false);
                  return true;
                },
              false);
          sc->present();
        }
    }
}

void
RightGrid::bookRemoveAction()
{
  auto item = srs->get_selected_item();
  if(item && get_current_collection_name)
    {
      std::string col_name = get_current_collection_name();
      if(!col_name.empty())
        {
          RemoveBookGui *rbg = new RemoveBookGui(af, main_window, item->bbe,
                                                 col_name, bookmarks, notes);
          rbg->remove_callback = [this, item, col_name](const BookBaseEntry &)
            {
              bie.reset();
              Glib::RefPtr<Gtk::TextBuffer> buf = Gtk::TextBuffer::create();
              annotation->set_buffer(buf);
              bie.reset();
              cover_buf.clearImage();
              cover_area->queue_draw();
              srs->removeBook(item);
              if(reload_collection_base)
                {
                  reload_collection_base(col_name);
                }
            };
          rbg->createWindow();
        }
    }
}

void
RightGrid::bookNotesAction()
{
  auto item = srs->get_selected_item();
  if(item && get_current_collection_name)
    {
      std::string col_name = get_current_collection_name();
      if(!col_name.empty())
        {
          NotesGui *ngui = new NotesGui(main_window, notes);
          ngui->creatWindow(col_name, item->bbe);
        }
    }
}

void
RightGrid::bookmarksSaveResultDialog(const int &variant)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Message"));
  window->set_transient_for(*main_window);
  window->set_modal(true);
  window->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_hexpand(true);
  lab->set_name("windowLabel");
  switch(variant)
    {
    case -1:
      {
        lab->set_text(gettext("Error! Bookmark has not been saved!"));
        break;
      }
    case 0:
      {
        lab->set_text(gettext("Book is already in bookmarks"));
        break;
      }
    case 1:
      {
        lab->set_text(gettext("Bookmark successfully saved"));
        break;
      }
    default:
      {
        delete window;
        return void();
      }
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*close, 0, 1, 1, 1);

  window->signal_close_request().connect(
      [window]
        {
          std::unique_ptr<Gtk::Window> win(window);
          win->set_visible(false);
          return true;
        },
      false);

  window->present();
}

void
RightGrid::editBookSuccessSlot(const BookBaseEntry &bbe_old,
                               const BookBaseEntry &bbe_new,
                               const std::string &col_name)
{
  auto model = srs->get_model();
  if(model)
    {
      for(guint i = 0; i < model->get_n_items(); i++)
        {
          auto item = model->get_item(i);
          if(item->bbe == bbe_old)
            {
              item->bbe = bbe_new;
              model->remove(i);
              model->insert(i, item);
              break;
            }
        }
    }
  if(reload_collection_base)
    {
      reload_collection_base(col_name);
    }
}

void
RightGrid::openBookAction()
{
  auto item = srs->get_selected_item();
  if(item)
    {
      try
        {
          std::filesystem::path tmp = af->temp_path();
          tmp /= std::filesystem::u8path("MyLibraryReading");
          std::filesystem::remove_all(tmp);
          open_book->open_book(item->bbe, false, tmp, false,
                               std::bind(&AuxFunc::open_book_callback,
                                         af.get(), std::placeholders::_1));
        }
      catch(std::exception &er)
        {
          std::cout << er.what() << std::endl;
        }
    }
}

void
RightGrid::transferBookAction()
{
  auto item = srs->get_selected_item();
  if(item)
    {
      TransferBookGui *tbg = new TransferBookGui(
          af, bookmarks, notes, item->bbe, current_collection, main_window);
      tbg->success_signal
          = [this, item](const BookBaseEntry &, const std::string &col_name)
        {
          bie.reset();
          Glib::RefPtr<Gtk::TextBuffer> buf = Gtk::TextBuffer::create();
          annotation->set_buffer(buf);
          cover_area->queue_draw();
          srs->removeBook(item);
          if(reload_collection_base)
            {
              reload_collection_base(col_name);
            }
        };
      tbg->createWindow();
    }
}
