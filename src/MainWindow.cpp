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

#include <AddBookGui.h>
#include <BookMarksGui.h>
#include <CreateCollectionGui.h>
#include <EmptyCollectionGui.h>
#include <ExportCollectionGui.h>
#include <ImportCollectionGui.h>
#include <Magick++.h>
#include <MainWindow.h>
#include <RefreshCollectionGui.h>
#include <RemoveCollectionGui.h>
#include <SettingsWindow.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <giomm-2.68/giomm/file.h>
#include <giomm-2.68/giomm/menu.h>
#include <giomm-2.68/giomm/menuitem.h>
#include <giomm-2.68/giomm/simpleaction.h>
#include <giomm-2.68/giomm/simpleactiongroup.h>
#include <gtkmm-4.0/gdkmm/display.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gdkmm/rectangle.h>
#include <gtkmm-4.0/gdkmm/surface.h>
#include <gtkmm-4.0/gdkmm/texture.h>
#include <gtkmm-4.0/gtkmm/aboutdialog.h>
#include <gtkmm-4.0/gtkmm/cssprovider.h>
#include <gtkmm-4.0/gtkmm/dropdown.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/settings.h>
#include <gtkmm-4.0/gtkmm/stringlist.h>
#include <iostream>
#include <libintl.h>

MainWindow::MainWindow(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;

  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  Glib::RefPtr<Gdk::Display> disp = get_display();
#ifndef ML_GTK_OLD
  css_provider->load_from_string(loadStyles());
  Gtk::StyleProvider::add_provider_for_display(
      disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#else
  css_provider->load_from_data(loadStyles());
  Gtk::StyleContext::add_provider_for_display(
      disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
  notes = std::make_shared<NotesKeeper>(af);
  bookmarks = std::make_shared<BookMarks>(af);
#ifdef USE_PLUGINS
  plugins_keeper = std::make_shared<PluginsKeeper>(this, af);
#endif

  mlbookproc_docs_path = af->share_path();
  mlbookproc_docs_path /= std::filesystem::u8path("doc");
  mlbookproc_docs_path /= std::filesystem::u8path("MLBookProc");
  mlbookproc_docs_path /= std::filesystem::u8path("html");
  mlbookproc_docs_path /= std::filesystem::u8path("index.html");
  if(!std::filesystem::exists(mlbookproc_docs_path))
    {
      mlbookproc_docs_path = mlbookproc_docs_path.parent_path().parent_path()
                             / std::filesystem::u8path("pdf");
      mlbookproc_docs_path /= std::filesystem::u8path("refman.pdf");
    }

  mlpluginifc_docs_path = af->share_path();
  mlpluginifc_docs_path /= std::filesystem::u8path("doc");
  mlpluginifc_docs_path /= std::filesystem::u8path("MLPluginIfc");
  mlpluginifc_docs_path /= std::filesystem::u8path("html");
  mlpluginifc_docs_path /= std::filesystem::u8path("index.html");
  if(!std::filesystem::exists(mlpluginifc_docs_path))
    {
      mlpluginifc_docs_path = mlpluginifc_docs_path.parent_path().parent_path()
                              / std::filesystem::u8path("pdf");
      mlpluginifc_docs_path /= std::filesystem::u8path("refman.pdf");
    }

  Glib::RefPtr<Gtk::Settings> settings = this->get_settings();
  Glib::PropertyProxy<Glib::ustring> them_name
      = settings->property_gtk_theme_name();
  them_name.set_value(Glib::ustring("Adwaita"));

  formMainWindow();
}

MainWindow::~MainWindow()
{
  delete lg;
  delete rg;
  std::filesystem::remove_all(temp_background_path);
}

void
MainWindow::formMainWindow()
{
  set_title("MyLibrary");
  set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  set_child(*grid);

  createMainMenuActionGroup();

  Gtk::PopoverMenuBar *menu_bar = createMainMenu();
  grid->attach(*menu_bar, 0, 0, 1, 1);

  main_pane = Gtk::make_managed<Gtk::Paned>();
  main_pane->set_halign(Gtk::Align::FILL);
  main_pane->set_valign(Gtk::Align::FILL);
  main_pane->set_expand(true);
  grid->attach(*main_pane, 0, 1, 1, 1);

  lg = new LeftGrid(af, this, notes);
  Gtk::Grid *left_grid = lg->createGrid();
  main_pane->set_start_child(*left_grid);

  rg = new RightGrid(af, this, bookmarks, notes);
  Gtk::Grid *right_grid = rg->createGrid();
  main_pane->set_end_child(*right_grid);
  lg->clear_search_result = std::bind(&RightGrid::clearSearchResult, rg);

  lg->search_result_show
      = std::bind(&RightGrid::searchResultShow, rg, std::placeholders::_1,
                  RightGrid::MainWindow);

  lg->search_result_show_separate_window
      = std::bind(&RightGrid::searchResultShow, rg, std::placeholders::_1,
                  RightGrid::SeparateWindow);

  lg->search_result_show_files = std::bind(&RightGrid::searchResultShowFiles,
                                           rg, std::placeholders::_1);

  lg->search_result_authors = std::bind(&RightGrid::searchResultShowAuthors,
                                        rg, std::placeholders::_1);

  rg->get_current_collection_name
      = std::bind(&MainWindow::getCurrentCollectionName, this);

  rg->reload_collection_base = [this](const std::string &col_name) {
    lg->reloadCollection(col_name);
  };

  rg->search_books_callback
      = std::bind(&LeftGrid::searchAuth, lg, std::placeholders::_1,
                  LeftGrid::ShowBooks::MainWindow);

  rg->search_books_callback_separate_window
      = std::bind(&LeftGrid::searchAuth, lg, std::placeholders::_1,
                  LeftGrid::ShowBooks::SeparateWindow);

#ifdef USE_PLUGINS
  plugins_keeper->signal_reload_collection_list
      = std::bind(&LeftGrid::reloadCollectionList, lg);
#endif

  signal_realize().connect(std::bind(&MainWindow::setMainWindowSizes, this));

  signal_close_request().connect(
      std::bind(&MainWindow::mainWindowCloseFunc, this), false);
}

Gtk::PopoverMenuBar *
MainWindow::createMainMenu()
{
  Gtk::PopoverMenuBar *bar = Gtk::make_managed<Gtk::PopoverMenuBar>();
  bar->set_name("mainMenu");
  bar->set_halign(Gtk::Align::START);
  bar->set_margin(5);

  Glib::RefPtr<Gio::Menu> menu_model = Gio::Menu::create();

  bar->set_menu_model(menu_model);

  Glib::RefPtr<Gio::Menu> collections_menu = Gio::Menu::create();
  Glib::RefPtr<Gio::MenuItem> item
      = Gio::MenuItem::create(gettext("Collections"), collections_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("Create collection"),
                               "main_menu.create_collection");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Remove collection"),
                               "main_menu.remove_collection");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Refresh collection"),
                               "main_menu.refresh_collection");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Add books to collection"),
                               "main_menu.add_book");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Add directories with books"),
                               "main_menu.add_directory");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Export collection base"),
                               "main_menu.export_collection");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Import collection base"),
                               "main_menu.import_collection");
  collections_menu->append_item(item);

  item = Gio::MenuItem::create(gettext("Create empty collection"),
                               "main_menu.empty_collection");
  collections_menu->append_item(item);

  Glib::RefPtr<Gio::Menu> book_marks_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(gettext("Bookmarks"), book_marks_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("Show bookmarks"),
                               "main_menu.book_marks");
  book_marks_menu->append_item(item);

#ifdef USE_PLUGINS
  Glib::RefPtr<Gio::Menu> plugins_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(gettext("Plugins"), plugins_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("Show plugins"),
                               "main_menu.plugins_keeper");
  plugins_menu->append_item(item);
#endif

  Glib::RefPtr<Gio::Menu> settings_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(gettext("Settings"), settings_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("Settings"), "main_menu.settings_win");
  settings_menu->append_item(item);

  Glib::RefPtr<Gio::Menu> about_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(gettext("About"), about_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("About MyLibrary"),
                               "main_menu.about_dialog");
  about_menu->append_item(item);

  if(std::filesystem::exists(mlbookproc_docs_path))
    {
      item = Gio::MenuItem::create(gettext("MLBookProc documentation"),
                                   "main_menu.mlbookproc_doc");
      about_menu->append_item(item);
    }

  if(std::filesystem::exists(mlpluginifc_docs_path))
    {
      item = Gio::MenuItem::create(gettext("MLPluginIfc documentation"),
                                   "main_menu.mlpluginifc_doc");
      about_menu->append_item(item);
    }

  return bar;
}

void
MainWindow::setMainWindowSizes()
{
  std::filesystem::path szpath = af->homePath();
  szpath /= std::filesystem::u8path(".cache")
            / std::filesystem::u8path("MyLibrary")
            / std::filesystem::u8path("mwsizes");
  int32_t height, width;
  double pos;
  std::fstream f;
  f.open(szpath, std::ios_base::in);
  if(f.is_open())
    {
      std::string rs;
      f.seekg(0, std::ios_base::end);
      rs.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(rs.data(), rs.size());
      f.close();

      if(rs.size() >= 16)
        {
          size_t read = 0;
          std::memcpy(&width, &rs[read], sizeof(width));
          read += sizeof(width);
          std::memcpy(&height, &rs[read], sizeof(height));
          read += sizeof(height);
          std::memcpy(&pos, &rs[read], sizeof(pos));
        }
      else
        {
          width = -1;
          height = -1;
          pos = 0.35;
        }
      set_default_size(static_cast<int>(width), static_cast<int>(height));
      if(width > 0)
        {
          main_pane->set_position(static_cast<int>(width) * pos);
        }
    }
  else
    {
      Glib::RefPtr<Gdk::Surface> surf = get_surface();
      Glib::RefPtr<Gdk::Display> disp = get_display();
      Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
      Gdk::Rectangle req;
      mon->get_geometry(req);

      req.set_width(req.get_width() * mon->get_scale_factor());
      req.set_height(req.get_height() * mon->get_scale_factor());

      width = req.get_width();
      height = req.get_height();
      set_default_size(width * 0.75, height * 0.75);
      main_pane->set_position(width * 0.35);
    }
}

void
MainWindow::createMainMenuActionGroup()
{
  Glib::RefPtr<Gio::SimpleActionGroup> main_menu_actions
      = Gio::SimpleActionGroup::create();

  main_menu_actions->add_action("create_collection", [this] {
    CreateCollectionGui *ccg = new CreateCollectionGui(af, this);
    ccg->add_new_collection
        = std::bind(&LeftGrid::add_new_collection, lg, std::placeholders::_1);
    ccg->createWindow();
  });

  main_menu_actions->add_action("remove_collection", [this] {
    RemoveCollectionGui *rcg = new RemoveCollectionGui(af, this, notes);
    rcg->collection_removed = std::bind(&MainWindow::collectionRemoveSlot,
                                        this, std::placeholders::_1);
    rcg->createWindow();
  });

  main_menu_actions->add_action("refresh_collection", [this] {
    RefreshCollectionGui *rfcg
        = new RefreshCollectionGui(af, this, bookmarks, notes);
    rfcg->collection_refreshed = [this](const std::string &col_name) {
      if(lg->reloadCollection(col_name))
        {
          rg->clearSearchResult();
        }
    };
    rfcg->createWindow();
  });

  main_menu_actions->add_action("book_marks", [this] {
    BookMarksGui *bmg = new BookMarksGui(af, bookmarks, notes, this);
    bmg->createWindow();
  });

  main_menu_actions->add_action("add_book", [this] {
    AddBookGui *abg = new AddBookGui(af, this, bookmarks, false);
    abg->books_added = [this](const std::string &col_name) {
      lg->reloadCollection(col_name);
    };
    abg->createWindow();
  });

  main_menu_actions->add_action("export_collection", [this] {
    ExportCollectionGui *ecg = new ExportCollectionGui(af, this);
    ecg->createWindow();
  });

  main_menu_actions->add_action("import_collection", [this] {
    ImportCollectionGui *icg = new ImportCollectionGui(af, this);
    icg->signal_success
        = std::bind(&LeftGrid::add_new_collection, lg, std::placeholders::_1);
    icg->createWindow();
  });

  main_menu_actions->add_action("empty_collection", [this] {
    EmptyCollectionGui *ecg = new EmptyCollectionGui(af, this);
    ecg->signal_success
        = std::bind(&LeftGrid::add_new_collection, lg, std::placeholders::_1);
    ecg->createWindow();
  });

  main_menu_actions->add_action("add_directory", [this] {
    AddBookGui *abg = new AddBookGui(af, this, bookmarks, true);
    abg->books_added = [this](const std::string &col_name) {
      lg->reloadCollection(col_name);
    };
    abg->createWindow();
  });

  main_menu_actions->add_action("settings_win", [this] {
    SettingsWindow *sw = new SettingsWindow(af, this);

    sw->signal_new_background_path = [this](const std::filesystem::path &p) {
      std::filesystem::remove_all(temp_background_path);
      temp_background_path = p;
    };

    sw->signal_coef_coincedence = std::bind(&LeftGrid::setCoefOfCoincedence,
                                            lg, std::placeholders::_1);

    sw->signal_close_request().connect(
        [sw] {
          std::unique_ptr<SettingsWindow> s(sw);
          s->set_visible(false);
          return true;
        },
        false);
    sw->present();
  });

  main_menu_actions->add_action("about_dialog",
                                std::bind(&MainWindow::aboutDialog, this));

#ifdef USE_PLUGINS
  main_menu_actions->add_action(
      "plugins_keeper",
      std::bind(&PluginsKeeper::createWindow, plugins_keeper));
#endif

  main_menu_actions->add_action("mlbookproc_doc", [this] {
    if(std::filesystem::exists(mlbookproc_docs_path))
      {
        af->open_book_callback(mlbookproc_docs_path);
      }
  });

  main_menu_actions->add_action("mlpluginifc_doc", [this] {
    if(std::filesystem::exists(mlpluginifc_docs_path))
      {
        af->open_book_callback(mlpluginifc_docs_path);
      }
  });

  insert_action_group("main_menu", main_menu_actions);
}

bool
MainWindow::mainWindowCloseFunc()
{
  int32_t width, height;
  width = static_cast<int32_t>(get_width());
  height = static_cast<int32_t>(get_height());

  int pane_pos = main_pane->get_position();
  double pos = static_cast<double>(pane_pos) / static_cast<double>(width);

  std::filesystem::path szpath = af->homePath();
  szpath /= std::filesystem::u8path(".cache")
            / std::filesystem::u8path("MyLibrary")
            / std::filesystem::u8path("mwsizes");
  std::filesystem::create_directories(szpath.parent_path());
  std::filesystem::remove_all(szpath);

  std::fstream f;
  f.open(szpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      f.write(reinterpret_cast<char *>(&width), sizeof(width));
      f.write(reinterpret_cast<char *>(&height), sizeof(height));
      f.write(reinterpret_cast<char *>(&pos), sizeof(pos));

      f.close();
    }
  set_visible(false);

  return true;
}

void
MainWindow::collectionRemoveSlot(const std::string &filename)
{
  Gtk::DropDown *col_sel = lg->get_collection_select();
  Glib::RefPtr<Gtk::StringObject> sel
      = std::dynamic_pointer_cast<Gtk::StringObject>(
          col_sel->get_selected_item());
  Glib::RefPtr<Gtk::StringList> list
      = std::dynamic_pointer_cast<Gtk::StringList>(col_sel->get_model());
  if(list)
    {
      Glib::ustring search(filename);
      for(guint i = 0; i < list->get_n_items(); i++)
        {
          if(list->get_string(i) == search)
            {
              list->remove(i);
              break;
            }
        }
      if(sel)
        {
          if(sel->get_string() == search)
            {
              lg->clearCollectionBase();
              lg->clear_all_search_fields();
              rg->clearSearchResult();
              sel = std::dynamic_pointer_cast<Gtk::StringObject>(
                  col_sel->get_selected_item());
              if(sel)
                {
                  lg->reloadCollection(std::string(sel->get_string()));
                }
            }
        }
    }
}

std::string
MainWindow::getCurrentCollectionName()
{
  std::string result;
  Gtk::DropDown *dd = lg->get_collection_select();
  if(dd)
    {
      Glib::RefPtr<Gtk::StringObject> obj
          = std::dynamic_pointer_cast<Gtk::StringObject>(
              dd->get_selected_item());
      if(obj)
        {
          result = std::string(obj->get_string());
        }
    }
  return result;
}

void
MainWindow::aboutDialog()
{
  Gtk::AboutDialog *about = new Gtk::AboutDialog;
  about->set_application(get_application());
  about->set_transient_for(*this);
  about->set_modal(true);
  about->set_name("aboutDialog");

  about->set_program_name("MyLibrary");

  std::filesystem::path icon_p = af->share_path();
  icon_p /= std::filesystem::u8path("MyLibrary");
  icon_p /= std::filesystem::u8path("mylibrary.svg");

  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(icon_p.u8string());

  Glib::RefPtr<Gdk::Texture> icon_t = Gdk::Texture::create_from_file(fl);

  std::vector<Glib::ustring> credits_people;
  credits_people.push_back("Felix <f11091877@gmail.com>");
  about->add_credit_section(gettext("Icon designed by: "), credits_people);

  about->set_logo(icon_t);

  about->set_version("4.2");

  about->set_website("https://github.com/ProfessorNavigator/mylibrary");

  about->set_copyright(
      "Copyright 2022-2025 Yury Bobylev <bobilev_yury@mail.ru>");

  about->set_license_type(Gtk::License::GPL_3_0_ONLY);

  Glib::ustring abbuf
      = Glib::ustring(gettext("MyLibrary is a home librarian"))
        + Glib::ustring("\n\n") + Glib::ustring(gettext("Author Yury Bobylev"))
        + Glib::ustring(" <bobilev_yury@mail.ru>.\n\n")
        + Glib::ustring(gettext("Program uses next libraries:"))
        + Glib::ustring(
            "\n"
            "GTK https://www.gtk.org/\n"
            "libarchive https://libarchive.org\n"
            "libgcrypt https://www.gnupg.org/software/libgcrypt/\n"
            "libgpg-error https://www.gnupg.org/software/libgpg-error/\n"
            "ICU https://icu.unicode.org/\n"
            "Poppler https://poppler.freedesktop.org/\n"
            "DjVuLibre https://djvu.sourceforge.net/\n"
            "Magick++ https://imagemagick.org/Magick++/");
#ifdef USE_TBB
  abbuf += "\noneTBB https://github.com/uxlfoundation/oneTBB";
#endif
  about->set_comments(abbuf);

  about->signal_close_request().connect(
      [about] {
        std::unique_ptr<Gtk::AboutDialog> ab(about);
        ab->set_visible(false);
        return true;
      },
      false);

  about->present();
}

std::string
MainWindow::loadStyles()
{
  std::string result;
  std::filesystem::path styles_path
      = af->homePath() / std::filesystem::u8path(".config")
        / std::filesystem::u8path("MyLibrary")
        / std::filesystem::u8path("MLStyles.css");
  if(!std::filesystem::exists(styles_path))
    {
      styles_path = af->share_path();
      styles_path /= std::filesystem::u8path("MyLibrary");
      styles_path /= std::filesystem::u8path("MLStyles.css");
    }

  std::fstream f;
  f.open(styles_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.seekg(0, std::ios_base::end);
      result.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(result.data(), result.size());
      f.close();
    }

  std::string find_str = "#MLwindow";
  std::string::size_type n = result.find(find_str);
  if(n == std::string::npos)
    {
      return result;
    }
  n += find_str.size();

  find_str = "}";
  std::string::size_type n2 = result.find(find_str, n);
  if(n2 == std::string::npos)
    {
      return result;
    }

  find_str = "background-image:";
  n = result.find(find_str, n);
  if(n >= n2 || n == std::string::npos)
    {
      return result;
    }

  n += find_str.size();
#ifdef __linux
  find_str = "file://";
#endif
#ifdef _WIN32
  find_str = "file:///";
#endif
  n = result.find(find_str, n);
  if(n >= n2 || n == std::string::npos)
    {
      return result;
    }

  n += find_str.size();

  find_str = ")";

  std::string::size_type n3 = result.find(find_str, n);
  if(n3 >= n2 || n3 == std::string::npos)
    {
      return result;
    }

  std::string back_p(result.begin() + n, result.begin() + n3);
  std::filesystem::path p = std::filesystem::u8path(back_p);
  if(!std::filesystem::exists(p))
    {
      return result;
    }

  Magick::Image img;
  try
    {
      img.read(p.string());
    }
  catch(Magick::Exception &er)
    {
      std::cout << "MainWindow::loadStyles: " << er.what() << std::endl;
      return result;
    }

  temp_background_path = af->temp_path();
  temp_background_path
      /= std::filesystem::u8path(af->randomFileName() + ".png");

  try
    {
      img.write(temp_background_path.string());
    }
  catch(Magick::Exception &er)
    {
      std::cout << "MainWindow::loadStyles: " << er.what() << std::endl;
      return result;
    }

  result.erase(result.begin() + n, result.begin() + n3);
  find_str = temp_background_path.u8string();
  result.insert(result.begin() + n, find_str.begin(), find_str.end());

  return result;
}
