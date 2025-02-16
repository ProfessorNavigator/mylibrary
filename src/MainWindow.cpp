/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <AddBookGui.h>
#include <BookMarksGui.h>
#include <CreateCollectionGui.h>
#include <EmptyCollectionGui.h>
#include <ExportCollectionGui.h>
#include <ImportCollectionGui.h>
#include <MainWindow.h>
#include <RefreshCollectionGui.h>
#include <RemoveCollectionGui.h>
#include <SettingsWindow.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/rectangle.h>
#include <gdkmm/surface.h>
#include <gdkmm/texture.h>
#include <giomm/menu.h>
#include <giomm/menuitem.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>
#include <glib/gtypes.h>
#include <glibmm/refptr.h>
#include <glibmm/signalproxy.h>
#include <glibmm/ustring.h>
#include <gtk/gtkstyleprovider.h>
#include <gtk/gtktypes.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/application.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/dropdown.h>
#include <gtkmm/enums.h>
#include <gtkmm/grid.h>
#include <gtkmm/object.h>
#include <gtkmm/stringlist.h>
#include <libintl.h>
#include <sigc++/connection.h>

MainWindow::MainWindow(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  std::filesystem::path styles_path
      = af->homePath()
        / std::filesystem::u8path(".config/MyLibrary/MLStyles.css");
  if(!std::filesystem::exists(styles_path))
    {
      styles_path = af->share_path();
      styles_path /= std::filesystem::u8path("MLStyles.css");
    }
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  css_provider->load_from_path(styles_path.u8string());
  Glib::RefPtr<Gdk::Display> disp = get_display();
#ifndef ML_GTK_OLD
  Gtk::StyleProvider::add_provider_for_display(
      disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
#ifdef ML_GTK_OLD
  Gtk::StyleContext::add_provider_for_display(
      disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
  bookmarks = std::make_shared<BookMarks>(af);
  formMainWindow();
}

MainWindow::~MainWindow()
{
  delete lg;
  delete rg;
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

  lg = new LeftGrid(af, this);
  Gtk::Grid *left_grid = lg->createGrid();
  main_pane->set_start_child(*left_grid);

  rg = new RightGrid(af, this, bookmarks);
  Gtk::Grid *right_grid = rg->createGrid();
  main_pane->set_end_child(*right_grid);
  lg->clear_search_result = std::bind(&RightGrid::clearSearchResult, rg);

  lg->search_result_show
      = std::bind(&RightGrid::search_result_show, rg, std::placeholders::_1);

  lg->search_result_show_files = std::bind(
      &RightGrid::search_result_show_files, rg, std::placeholders::_1);

  rg->get_current_collection_name
      = std::bind(&MainWindow::get_current_collection_name, this);

  rg->reload_collection_base = [this](const std::string &col_name) {
    lg->reloadCollection(col_name);
  };

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

  Glib::RefPtr<Gio::Menu> settings_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(gettext("Settings"), settings_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("Color settings"),
                               "main_menu.settings_win");
  settings_menu->append_item(item);

  Glib::RefPtr<Gio::Menu> about_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(gettext("About"), about_menu);
  menu_model->append_item(item);

  item = Gio::MenuItem::create(gettext("About MyLibrary"),
                               "main_menu.about_dialog");
  about_menu->append_item(item);

  return bar;
}

void
MainWindow::setMainWindowSizes()
{
  std::filesystem::path szpath = af->homePath();
  szpath /= std::filesystem::u8path(".cache/MyLibrary/mwsizes");
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

      width = static_cast<int32_t>(req.get_width());
      height = static_cast<int32_t>(req.get_height());
      int w = static_cast<int>(width * 0.75);
      set_default_size(w, static_cast<int>(height * 0.75));
      main_pane->set_position(w * 0.35);
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
    RemoveCollectionGui *rcg = new RemoveCollectionGui(af, this);
    rcg->collection_removed = std::bind(&MainWindow::collectionRemoveSlot,
                                        this, std::placeholders::_1);
    rcg->createWindow();
  });

  main_menu_actions->add_action("refresh_collection", [this] {
    RefreshCollectionGui *rfcg = new RefreshCollectionGui(af, this, bookmarks);
    rfcg->collection_refreshed = [this](const std::string &col_name) {
      if(lg->reloadCollection(col_name))
        {
          rg->clearSearchResult();
        }
    };
    rfcg->createWindow();
  });

  main_menu_actions->add_action("book_marks", [this] {
    BookMarksGui *bmg = new BookMarksGui(af, bookmarks, this);
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
    sw->createWindow();
  });

  main_menu_actions->add_action("about_dialog",
                                std::bind(&MainWindow::about_dialog, this));

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
  szpath /= std::filesystem::u8path(".cache/MyLibrary/mwsizes");
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
  Glib::RefPtr<Gtk::StringList> list
      = std::dynamic_pointer_cast<Gtk::StringList>(col_sel->get_model());
  if(list)
    {
      Glib::ustring search(filename);
      guint selected = col_sel->get_selected();
      if(list->get_string(selected) == search)
        {
          lg->clearCollectionBase();
          lg->clear_all_search_fields();
          rg->clearSearchResult();
        }
      for(guint i = 0; i < list->get_n_items(); i++)
        {
          if(list->get_string(i) == search)
            {
              list->remove(i);
              break;
            }
        }
    }
}

std::string
MainWindow::get_current_collection_name()
{
  std::string result;
  Gtk::DropDown *dd = lg->get_collection_select();
  if(dd)
    {
      guint sel = dd->get_selected();
      auto model = std::dynamic_pointer_cast<Gtk::StringList>(dd->get_model());
      if(sel != GTK_INVALID_LIST_POSITION && model)
        {
          result = std::string(model->get_string(sel));
        }
    }
  return result;
}

void
MainWindow::about_dialog()
{
  Gtk::AboutDialog *about = new Gtk::AboutDialog;
  about->set_application(get_application());
  about->set_transient_for(*this);
  about->set_modal(true);
  about->set_name("aboutDialog");

  about->set_program_name("MyLibrary");

  std::filesystem::path icon_p = af->share_path();
  icon_p /= std::filesystem::u8path("mylibrary.svg");

  Glib::RefPtr<Gdk::Pixbuf> icon
      = Gdk::Pixbuf::create_from_file(icon_p.u8string());

  Glib::RefPtr<Gdk::Texture> icon_t = Gdk::Texture::create_for_pixbuf(icon);

  std::vector<Glib::ustring> credits_people;
  credits_people.push_back("Felix <f11091877@gmail.com>");
  about->add_credit_section(gettext("Icon designed by: "), credits_people);

  about->set_logo(icon_t);

  about->set_version("3.2");

  about->set_website("https://github.com/ProfessorNavigator/mylibrary");

  about->set_copyright(
      "Copyright 2022-2025 Yury Bobylev <bobilev_yury@mail.ru>");

  about->set_license_type(Gtk::License::GPL_3_0);

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
            "DjVuLibre https://djvu.sourceforge.net/");
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
