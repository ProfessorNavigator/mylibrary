#include <ExamplePlugin.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>

#ifdef USE_OPENMP
#include <iostream>
#include <omp.h>
#endif

ExamplePlugin::ExamplePlugin(void *af_ptr) : MLPlugin(af_ptr)
{
  plugin_name = "Example plugin";
  plugin_description = "Small example plugin";
}

void
ExamplePlugin::createWindow(Gtk::Window *parent_window)
{
  Gtk::Window *window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(plugin_name);
  window->set_name("MLwindow");
  window->set_transient_for(*parent_window);
  window->set_modal(true);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  window->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text("Example plugin");
  grid->attach(*lab, 0, 0, 1, 1);

#ifdef ML_GTK_OLD
  // Legacy gtkmm code
#endif

  window->signal_close_request().connect(
      [window] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        return true;
      },
      false);

  window->present();

#ifdef USE_OPENMP
#pragma omp masked
  {
    omp_event_handle_t event;
#pragma omp task detach(event)
    {
      std::cout << "My detached thread" << std::endl;
      omp_fulfill_event(event);
    }
  }
#endif
}
