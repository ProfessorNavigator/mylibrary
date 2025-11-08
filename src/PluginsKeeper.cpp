/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <ByteOrder.h>
#include <PluginsKeeper.h>
#include <fstream>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/flowbox.h>
#include <gtkmm-4.0/gtkmm/frame.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <libintl.h>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/error.h>
#endif

PluginsKeeper::PluginsKeeper(Gtk::Window *parent_window,
                             const std::shared_ptr<AuxFunc> &af)
{
  this->parent_window = parent_window;
  this->af = af;
  loadPlugins();
}

PluginsKeeper::~PluginsKeeper()
{
  savePlugins();
}

void
PluginsKeeper::createWindow()
{
  main_window = new Gtk::Window;
  main_window->set_application(parent_window->get_application());
  main_window->set_title(gettext("Plugins"));
  main_window->set_transient_for(*parent_window);
  main_window->set_modal(true);
  main_window->set_name("MLwindow");
  setWindowSizes();

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  main_window->set_child(*grid);

  Gtk::ScrolledWindow *scrl = new Gtk::ScrolledWindow;
  scrl->set_margin(5);
  scrl->set_halign(Gtk::Align::FILL);
  scrl->set_valign(Gtk::Align::FILL);
  scrl->set_expand(true);
  scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  grid->attach(*scrl, 0, 0, 2, 1);

  Gtk::FlowBox *flbx = Gtk::make_managed<Gtk::FlowBox>();
  flbx->set_selection_mode(Gtk::SelectionMode::NONE);
  flbx->set_max_children_per_line(1);
  scrl->set_child(*flbx);

  for(size_t i = 0; i < plugin_list.size(); i++)
    {
      Gtk::Frame *frm = Gtk::make_managed<Gtk::Frame>();
      frm->set_margin(5);
      frm->set_halign(Gtk::Align::CENTER);
      frm->set_valign(Gtk::Align::START);
      frm->set_name("MLframe");

      Gtk::Box *fr_box
          = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
      frm->set_child(*fr_box);

      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_name("windowLabel");
      lab->set_use_markup(true);
      Glib::ustring p_nm = plugin_list[i]->plugin_ptr->getPluginName();
      lab->set_markup("<b>" + p_nm + "</b>");
      fr_box->append(*lab);

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_name("windowLabel");
      lab->set_justify(Gtk::Justification::FILL);
      lab->set_text(
          gettext(plugin_list[i]->plugin_ptr->getPluginDescription().c_str()));
      fr_box->append(*lab);

      textdomain("MyLibrary");

      lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::START);
      lab->set_name("windowLabel");
      lab->set_text(plugin_list[i]->plugin_path.u8string());
      fr_box->append(*lab);

      Gtk::Button *launch = Gtk::make_managed<Gtk::Button>();
      launch->set_margin(5);
      launch->set_halign(Gtk::Align::CENTER);
      launch->set_name("applyBut");
      launch->set_label(gettext("Launch"));
      std::shared_ptr<plugin> pl = plugin_list[i];
      launch->signal_clicked().connect([pl, this] {
        pl->plugin_ptr->createWindow(main_window);
      });
      fr_box->append(*launch);

      Gtk::Button *remove = Gtk::make_managed<Gtk::Button>();
      remove->set_margin(5);
      remove->set_halign(Gtk::Align::CENTER);
      remove->set_name("removeBut");
      remove->set_label(gettext("Disable"));
      remove->signal_clicked().connect(std::bind(
          &PluginsKeeper::removeConfirmationWindow, this, frm, flbx));
      fr_box->append(*remove);

      flbx->insert(*frm, -1);
      plugin_list[i]->widg = frm;
    }

  Gtk::Button *add = Gtk::make_managed<Gtk::Button>();
  add->set_margin(5);
  add->set_halign(Gtk::Align::CENTER);
  add->set_name("applyBut");
  add->set_label(gettext("Add"));
  add->signal_clicked().connect(
      std::bind(&PluginsKeeper::addPluginDialog, this, flbx));
  grid->attach(*add, 0, 1, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("cancelBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, main_window));
  grid->attach(*close, 1, 1, 1, 1);

  main_window->signal_close_request().connect(
      [this] {
        std::unique_ptr<Gtk::Window> win(main_window);
        main_window = nullptr;
        win->set_visible(false);
        for(auto it = plugin_list.begin(); it != plugin_list.end(); it++)
          {
            (*it)->widg = nullptr;
          }
        textdomain("MyLibrary");
        if(signal_reload_collection_list)
          {
            signal_reload_collection_list();
          }
        return true;
      },
      false);

  main_window->present();
}

void
PluginsKeeper::setWindowSizes()
{
  Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
  Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);

  req.set_width(req.get_width() * mon->get_scale_factor());
  req.set_height(req.get_height() * mon->get_scale_factor());

  int width = static_cast<int>(req.get_width());
  int height = static_cast<int>(req.get_height());
  main_window->set_default_size(width * 0.5, height * 0.5);
}

void
PluginsKeeper::loadPlugins()
{
  std::filesystem::path base_path = af->homePath();
  base_path /= std::filesystem::u8path(".config/MyLibrary/plugins_base");
  std::fstream f;
  f.open(base_path);
  if(f.is_open())
    {
      std::string base_str;
      f.seekg(0, std::ios_base::end);
      base_str.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(base_str.data(), base_str.size());
      f.close();

      try
        {
          parseRawBase(base_str);
        }
      catch(std::exception &e)
        {
          std::cout << e.what() << std::endl;
        }
    }
}

void
PluginsKeeper::parseRawBase(const std::string &raw_base)
{
  uint64_t val64;
  size_t sz_64 = sizeof(val64);
  size_t rb = 0;
  size_t lim = raw_base.size();
  ByteOrder bo;
  while(rb < lim)
    {
      if(rb + sz_64 <= lim)
        {
          std::memcpy(&val64, &raw_base[rb], sz_64);
          rb += sz_64;
          bo.set_little(val64);
          val64 = bo;
        }
      else
        {
          throw std::runtime_error(
              "PluginsKeeper::parseRawBase incorrect entry size");
        }
      size_t sz = static_cast<size_t>(val64);
      if(rb + sz <= lim)
        {
          std::filesystem::path pp
              = std::filesystem::u8path(raw_base.substr(rb, sz));
          rb += sz;
          loadPlugin(pp);
        }
      else
        {
          throw std::runtime_error(
              "PluginsKeeper::parseRawBase incorrect entry");
        }
    }
}

void
PluginsKeeper::loadPlugin(const std::filesystem::path &pp)
{
#ifdef __linux
  void *handle = dlopen(pp.c_str(), RTLD_NOW);
#endif
#ifdef _WIN32
  HINSTANCE handle = LoadLibraryA(TEXT(pp.string().c_str()));
#endif
  if(handle)
    {
      std::shared_ptr<plugin> plg(new plugin);
      plg->handle = handle;
      plg->plugin_path = pp;
      MLPlugin *(*create_ptr)(void *af_ptr);
#ifdef __linux
      create_ptr
          = reinterpret_cast<MLPlugin *(*)(void *)>(dlsym(handle, "create"));
#endif
#ifdef _WIN32
      create_ptr = reinterpret_cast<MLPlugin *(*)(void *)>(
          GetProcAddress(handle, "create"));
#endif
      if(create_ptr)
        {
          MLPlugin *pl_ptr = create_ptr(&af);
          textdomain("MyLibrary");
          if(pl_ptr)
            {
              plg->plugin_ptr = pl_ptr;
              plugin_list.push_back(plg);
            }
          else
            {
              std::cout
                  << "PluginsKeeper::loadPlugin: error on plugin creation "
                  << pp << std::endl;
            }
        }
      else
        {
          std::cout << "PluginsKeeper::loadPlugin: plugin creation function "
                       "not found "
                    << pp << std::endl;
        }
    }
  else
    {
#ifdef __linux
      std::cout << "PluginsKeeper::loadPlugin " << pp << ": " << dlerror()
                << std::endl;
#endif
#ifdef _WIN32
      std::cout << "PluginsKeeper::loadPlugin " << pp << " loading error"
                << std::endl;
#endif
    }
}

void
PluginsKeeper::savePlugins()
{
  std::filesystem::path base_path = af->homePath();
  base_path /= std::filesystem::u8path(".config/MyLibrary/plugins_base");
  std::filesystem::create_directories(base_path.parent_path());
  std::filesystem::remove_all(base_path);

  if(plugin_list.size() > 0)
    {
      std::fstream f;
      f.open(base_path, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
        {
          uint64_t val64;
          size_t sz_64 = sizeof(val64);
          ByteOrder bo;
          for(auto it = plugin_list.begin(); it != plugin_list.end(); it++)
            {
              std::string entry = (*it)->plugin_path.u8string();
              val64 = static_cast<uint64_t>(entry.size());
              bo = val64;
              bo.get_little(val64);
              f.write(reinterpret_cast<char *>(&val64), sz_64);
              f.write(entry.c_str(), entry.size());
            }
          f.close();
        }
    }
}

void
PluginsKeeper::removeConfirmationWindow(Gtk::Widget *widg, Gtk::Widget *flbx)
{
  textdomain("MyLibrary");

  Gtk::Window *window = new Gtk::Window;
  window->set_application(main_window->get_application());
  window->set_title(gettext("Confirmation"));
  window->set_transient_for(*main_window);
  window->set_name("MLwindow");
  window->set_modal(true);
  window->set_default_size(1, 1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_name("windowLabel");
  lab->set_text(gettext("Are you sure?"));
  grid->attach(*lab, 0, 0, 2, 1);

  Gtk::Button *yes = Gtk::make_managed<Gtk::Button>();
  yes->set_margin(5);
  yes->set_halign(Gtk::Align::CENTER);
  yes->set_name("removeBut");
  yes->set_label(gettext("Yes"));
  yes->signal_clicked().connect([this, widg, flbx, window] {
    reinterpret_cast<Gtk::FlowBox *>(flbx)->remove(*widg);
    plugin_list.erase(std::remove_if(plugin_list.begin(), plugin_list.end(),
                                     [widg](std::shared_ptr<plugin> &el) {
                                       return el->widg == widg;
                                     }),
                      plugin_list.end());
    savePlugins();
    window->close();
  });
  grid->attach(*yes, 0, 1, 1, 1);

  Gtk::Button *no = Gtk::make_managed<Gtk::Button>();
  no->set_margin(5);
  no->set_halign(Gtk::Align::CENTER);
  no->set_name("cancelBut");
  no->set_label(gettext("No"));
  no->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  grid->attach(*no, 1, 1, 1, 1);

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();
}

void
PluginsKeeper::addPluginDialog(Gtk::Widget *flbx)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> fl
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_initial_folder(fl);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();

  fd->open(*main_window,
           std::bind(&PluginsKeeper::addPluginDialogSlot, this,
                     std::placeholders::_1, fd, flbx),
           cncl);
#else
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *main_window, gettext("Plugin"), Gtk::FileChooser::Action::OPEN, true);
  fd->set_application(main_window->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Open"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_current_folder(initial);

  fd->signal_response().connect(std::bind(&PluginsKeeper::addPluginDialogSlot,
                                          this, std::placeholders::_1, fd,
                                          flbx));

  fd->signal_close_request().connect(
      [fd] {
        std::shared_ptr<Gtk::FileChooserDialog> fdl(fd);
        fdl->set_visible(false);
        return true;
      },
      false);

  fd->present();
#endif
}

#ifdef ML_GTK_OLD
void
PluginsKeeper::addPluginDialogSlot(int respons_id, Gtk::FileChooserDialog *fd,
                                   Gtk::Widget *flbx)
{
  if(respons_id == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      addPluginDialogSlotProc(fl, flbx);
    }
  fd->close();
}
#endif

void
PluginsKeeper::addPluginDialogSlotProc(const Glib::RefPtr<Gio::File> &fl,
                                       Gtk::Widget *flbx)
{
  if(fl)
    {
      std::filesystem::path pp = std::filesystem::u8path(fl->get_path());
      size_t sz = plugin_list.size();
      loadPlugin(pp);
      if(sz < plugin_list.size())
        {
          savePlugins();

          Gtk::Frame *frm = Gtk::make_managed<Gtk::Frame>();
          frm->set_margin(5);
          frm->set_halign(Gtk::Align::CENTER);
          frm->set_valign(Gtk::Align::START);
          frm->set_name("MLframe");

          Gtk::Box *fr_box
              = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
          frm->set_child(*fr_box);

          Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_name("windowLabel");
          lab->set_use_markup(true);
          Glib::ustring p_nm
              = (*plugin_list.rbegin())->plugin_ptr->getPluginName();
          lab->set_markup("<b>" + p_nm + "</b>");
          fr_box->append(*lab);

          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_name("windowLabel");
          lab->set_justify(Gtk::Justification::FILL);
          lab->set_text(gettext((*plugin_list.rbegin())
                                    ->plugin_ptr->getPluginDescription()
                                    .c_str()));
          fr_box->append(*lab);

          textdomain("MyLibrary");

          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_name("windowLabel");
          lab->set_text((*plugin_list.rbegin())->plugin_path.u8string());
          fr_box->append(*lab);

          Gtk::Button *launch = Gtk::make_managed<Gtk::Button>();
          launch->set_margin(5);
          launch->set_halign(Gtk::Align::CENTER);
          launch->set_name("applyBut");
          launch->set_label(gettext("Launch"));
          std::shared_ptr<plugin> pl = *plugin_list.rbegin();
          launch->signal_clicked().connect([pl, this] {
            pl->plugin_ptr->createWindow(main_window);
          });
          fr_box->append(*launch);

          Gtk::Button *remove = Gtk::make_managed<Gtk::Button>();
          remove->set_margin(5);
          remove->set_halign(Gtk::Align::CENTER);
          remove->set_name("removeBut");
          remove->set_label(gettext("Disable"));
          remove->signal_clicked().connect(std::bind(
              &PluginsKeeper::removeConfirmationWindow, this, frm, flbx));
          fr_box->append(*remove);

          reinterpret_cast<Gtk::FlowBox *>(flbx)->insert(*frm, -1);
          (*plugin_list.rbegin())->widg = frm;
        }
    }
}

#ifndef ML_GTK_OLD
void
PluginsKeeper::addPluginDialogSlot(
    const Glib::RefPtr<Gio::AsyncResult> &result,
    const Glib::RefPtr<Gtk::FileDialog> &fd, Gtk::Widget *flbx)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->open_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::Code::FAILED)
        {
          std::cout << "SettiPluginsKeeper::addPluginDialogSlot error: "
                    << er.what() << std::endl;
        }
    }
  addPluginDialogSlotProc(fl, flbx);
}
#endif
