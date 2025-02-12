/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#include "gdkmm/monitor.h"
#include <SettingsWindow.h>
#include <fstream>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/cssprovider.h>
#include <gtkmm-4.0/gtkmm/flowbox.h>
#include <gtkmm-4.0/gtkmm/frame.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <iostream>
#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/colordialogbutton.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif
#ifdef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/colorbutton.h>
#endif

SettingsWindow::SettingsWindow(const std::shared_ptr<AuxFunc> &af,
                               Gtk::Window *parent_window)
{
  this->af = af;
  this->parent_window = parent_window;
  save_path = af->homePath();
  save_path /= std::filesystem::u8path(".config/MyLibrary");
  readSettings();
}

void
SettingsWindow::createWindow()
{
  window = new Gtk::Window;
  window->set_application(parent_window->get_application());
  window->set_title(gettext("Settings"));
  window->set_transient_for(*parent_window);
  window->set_modal(true);
  window->set_name("MLwindow");
  windowSize();

  Gtk::Box *main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  main_box->set_halign(Gtk::Align::FILL);
  main_box->set_valign(Gtk::Align::FILL);
  window->set_child(*main_box);

  Gtk::ScrolledWindow *set_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  set_scrl->set_halign(Gtk::Align::FILL);
  set_scrl->set_valign(Gtk::Align::FILL);
  set_scrl->set_expand(true);
  set_scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
  main_box->append(*set_scrl);

  Gtk::Box *scrl_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  set_scrl->set_child(*scrl_box);

  scrl_box->append(*windowsSection());

  Gtk::FlowBox *f_box = Gtk::make_managed<Gtk::FlowBox>();
  f_box->set_halign(Gtk::Align::FILL);
  f_box->set_valign(Gtk::Align::FILL);
  f_box->set_min_children_per_line(3);
  f_box->set_selection_mode();
  scrl_box->append(*f_box);

  f_box->insert(*formSection(widget_type::main_menu), -1);
  f_box->insert(*formSection(widget_type::label), -1);
  f_box->insert(*formSection(widget_type::entry), -1);
  f_box->insert(*formSection(widget_type::column_view), -1);
  f_box->insert(*formSection(widget_type::selected_label), -1);
  f_box->insert(*formSection(widget_type::text_field), -1);
  f_box->insert(*formSection(widget_type::apply_button), -1);
  f_box->insert(*formSection(widget_type::cancel_button), -1);
  f_box->insert(*formSection(widget_type::operation_button), -1);
  f_box->insert(*formSection(widget_type::remove_button), -1);
  f_box->insert(*formSection(widget_type::combo_box), -1);
  f_box->insert(*formSection(widget_type::menu_button), -1);
  f_box->insert(*formSection(widget_type::error_label), -1);
  f_box->insert(*formSection(widget_type::warning_label), -1);
  f_box->insert(*formSection(widget_type::progress_bar), -1);

  Gtk::Box *but_box
      = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
  but_box->set_halign(Gtk::Align::CENTER);
  main_box->append(*but_box);

  Gtk::Button *apply = Gtk::make_managed<Gtk::Button>();
  apply->set_margin(5);
  apply->set_halign(Gtk::Align::CENTER);
  apply->set_label(gettext("Apply"));
  apply->set_name("applyBut");
  apply->signal_clicked().connect(
      std::bind(&SettingsWindow::applySettings, this));
  but_box->append(*apply);

  Gtk::Button *apply_close = Gtk::make_managed<Gtk::Button>();
  apply_close->set_margin(5);
  apply_close->set_halign(Gtk::Align::CENTER);
  apply_close->set_label(gettext("Apply and close"));
  apply_close->set_name("applyBut");
  apply_close->signal_clicked().connect([this] {
    applySettings();
    window->close();
  });
  but_box->append(*apply_close);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_label(gettext("Cancel"));
  close->set_name("cancelBut");
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, window));
  but_box->append(*close);

  window->signal_close_request().connect(
      [this] {
        std::unique_ptr<Gtk::Window> win(window);
        win->set_visible(false);
        delete this;
        return true;
      },
      false);

  window->present();
}

Gtk::Widget *
SettingsWindow::windowsSection()
{
  Gtk::Frame *frame = Gtk::make_managed<Gtk::Frame>();
  frame->set_margin(5);

  Gtk::Box *section_box
      = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  frame->set_child(*section_box);

  auto it
      = std::find_if(settings_v.begin(), settings_v.end(), [](section &el) {
          return el.section_id == "MLwindow";
        });
  auto it_2
      = std::find_if(settings_v.begin(), settings_v.end(), [](section &el) {
          return el.section_id == "aboutDialog";
        });
  if(it != settings_v.end() && it_2 != settings_v.end())
    {
      Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
      lab->set_margin(5);
      lab->set_halign(Gtk::Align::CENTER);
      lab->set_use_markup(true);
      lab->set_markup(Glib::ustring("<b>") + gettext("Windows") + "</b>");
      lab->set_name("windowLabel");
      frame->set_label_widget(*lab);

      int row = 0;
      Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
      grid->set_halign(Gtk::Align::FILL);
      grid->set_valign(Gtk::Align::FILL);
      section_box->append(*grid);

#ifndef ML_GTK_OLD
      Glib::RefPtr<Gtk::ColorDialog> color_dialog = Gtk::ColorDialog::create();
      color_dialog->set_with_alpha(true);
#endif

      auto it_set = std::find_if(
          it->settings.begin(), it->settings.end(), [](setting &el) {
            return el.attribute_id == "background-color";
          });
      auto it_set2 = std::find_if(
          it_2->settings.begin(), it_2->settings.end(), [](setting &el) {
            return el.attribute_id == "background-color";
          });
      if(it_set != it->settings.end() && it_set2 != it_2->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Background color:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set, it_set2] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
            it_set2->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set, it_set2] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
            it_set2->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }

      it_set = std::find_if(it->settings.begin(), it->settings.end(),
                            [](setting &el) {
                              return el.attribute_id == "background-image";
                            });
      it_set2 = std::find_if(it_2->settings.begin(), it_2->settings.end(),
                             [](setting &el) {
                               return el.attribute_id == "background-image";
                             });
      if(it_set != it->settings.end() && it_set2 != it_2->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Background image:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

          Gtk::Entry *image_path = Gtk::make_managed<Gtk::Entry>();
          image_path->set_margin(5);
          image_path->set_halign(Gtk::Align::FILL);
          image_path->set_width_chars(50);
          image_path->set_hexpand(true);
          image_path->set_name("windowEntry");

          Glib::ustring val = it_set->value;
          Glib::ustring find_str("file://");
          Glib::ustring::size_type n = val.find(find_str);
          if(n != Glib::ustring::npos)
            {
              val.erase(n, find_str.size());
            }
          find_str = "url";
          n = val.find(find_str);
          if(n != Glib::ustring::npos)
            {
              val.erase(n, find_str.size());
            }
          find_str = "(";
          n = val.find(find_str);
          if(n != Glib::ustring::npos)
            {
              val.erase(n, find_str.size());
            }
          find_str = ")";
          n = val.find(find_str);
          if(n != Glib::ustring::npos)
            {
              val.erase(n, find_str.size());
            }
          find_str = "%20";
          Glib::ustring rp(" ");
          n = 0;
          while(n != Glib::ustring::npos)
            {
              n = val.find(find_str);
              if(n != Glib::ustring::npos)
                {
                  val.erase(n, find_str.size());
                  val.insert(n, rp);
                }
            }
          image_path->set_text(val);

          Glib::PropertyProxy<Glib::ustring> prop
              = image_path->property_text();
          prop.signal_changed().connect([it_set, it_set2, image_path] {
            Glib::ustring p = image_path->get_text();
            if(p.empty())
              {
                p = "none";
                it_set->value = std::string(p);
                it_set2->value = std::string(p);
              }
            else
              {
                Glib::ustring::size_type n = 0;
                Glib::ustring find_str(" ");
                Glib::ustring rp("%20");
                while(n != Glib::ustring::npos)
                  {
                    n = p.find(find_str, n);
                    if(n != Glib::ustring::npos)
                      {
                        p.erase(n, find_str.size());
                        p.insert(n, rp);
                      }
                  }
                it_set->value = "url(file://" + std::string(p) + ")";
                it_set2->value = "url(file://" + std::string(p) + ")";
              }
          });
          grid->attach(*image_path, 1, row, 1, 1);

          Gtk::Button *open = Gtk::make_managed<Gtk::Button>();
          open->set_margin(5);
          open->set_halign(Gtk::Align::CENTER);
          open->set_label(gettext("Open"));
          open->set_name("operationBut");
          open->signal_clicked().connect(
              std::bind(&SettingsWindow::fileDialog, this, image_path));
          grid->attach(*open, 2, row, 1, 1);
        }
    }

  return frame;
}

Gtk::Widget *
SettingsWindow::formSection(const widget_type &wt)
{
  Gtk::Frame *frame = Gtk::make_managed<Gtk::Frame>();
  frame->set_margin(5);

  Gtk::Box *section_box
      = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  frame->set_child(*section_box);

  std::string search_str;
  Glib::ustring sec_name;
  switch(wt)
    {
    case widget_type::apply_button:
      {
        search_str = "applyBut";
        sec_name = gettext("Apply buttons");
        break;
      }
    case widget_type::cancel_button:
      {
        search_str = "cancelBut";
        sec_name = gettext("Cancel buttons");
        break;
      }
    case widget_type::operation_button:
      {
        search_str = "operationBut";
        sec_name = gettext("Operations buttons");
        break;
      }
    case widget_type::remove_button:
      {
        search_str = "removeBut";
        sec_name = gettext("Remove buttons");
        break;
      }
    case widget_type::combo_box:
      {
        search_str = "comboBox button";
        sec_name = gettext("Combo box buttons");
        break;
      }
    case widget_type::text_field:
      {
        search_str = "textField";
        sec_name = gettext("Annotation");
        break;
      }
    case widget_type::entry:
      {
        search_str = "windowEntry";
        sec_name = gettext("Entries");
        break;
      }
    case widget_type::label:
      {
        search_str = "windowLabel";
        sec_name = gettext("Labels");
        break;
      }
    case widget_type::main_menu:
      {
        search_str = "mainMenu";
        sec_name = gettext("Main menu");
        break;
      }
    case widget_type::menu_button:
      {
        search_str = "menBut button.toggle";
        sec_name = gettext("Popup menu buttons");
        break;
      }
    case widget_type::selected_label:
      {
        search_str = "selectedLab";
        sec_name = gettext("Selected label");
        break;
      }
    case widget_type::error_label:
      {
        search_str = "badLabel";
        sec_name = gettext("\"Error\" labels");
        break;
      }
    case widget_type::warning_label:
      {
        search_str = "yellowLab";
        sec_name = gettext("\"Warning\" labels");
        break;
      }
    case widget_type::progress_bar:
      {
        search_str = "progressBars";
        sec_name = gettext("Progress bars");
        break;
      }
    case widget_type::column_view:
      {
        search_str = "tablesView listview";
        sec_name = gettext("Table views");
        break;
      }
    default:
      break;
    }

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_markup(Glib::ustring("<b>") + sec_name + "</b>");
  lab->set_name("windowLabel");
  frame->set_label_widget(*lab);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  section_box->append(*grid);

#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::ColorDialog> color_dialog = Gtk::ColorDialog::create();
  color_dialog->set_with_alpha(true);
#endif

  int row = 0;
  auto it = std::find_if(settings_v.begin(), settings_v.end(),
                         [search_str](section &el) {
                           return el.section_id == search_str;
                         });
  if(it != settings_v.end())
    {
      auto it_set = std::find_if(it->settings.begin(), it->settings.end(),
                                 [](setting &el) {
                                   return el.attribute_id == "color";
                                 });
      if(it_set != it->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Font color:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }

      it_set = std::find_if(it->settings.begin(), it->settings.end(),
                            [](setting &el) {
                              return el.attribute_id == "background-color";
                            });
      if(it_set != it->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Background color:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }

      it_set = std::find_if(it->settings.begin(), it->settings.end(),
                            [](setting &el) {
                              return el.attribute_id == "border-color";
                            });
      if(it_set != it->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Border color:"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }
    }
  std::string lstr = search_str + ":hover";
  auto it_2 = std::find_if(settings_v.begin(), settings_v.end(),
                           [lstr](section &el) {
                             return el.section_id == lstr;
                           });
  if(it_2 != settings_v.end())
    {
      auto it_set = std::find_if(it_2->settings.begin(), it_2->settings.end(),
                                 [](setting &el) {
                                   return el.attribute_id == "color";
                                 });
      if(it_set != it_2->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Font color (hover):"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }

      it_set = std::find_if(it_2->settings.begin(), it_2->settings.end(),
                            [](setting &el) {
                              return el.attribute_id == "background-color";
                            });
      if(it_set != it_2->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Background color (hover):"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }
    }

  lstr = search_str + ":active";
  auto it_3 = std::find_if(settings_v.begin(), settings_v.end(),
                           [lstr](section &el) {
                             return el.section_id == lstr;
                           });
  if(it_3 != settings_v.end())
    {
      auto it_set = std::find_if(it_3->settings.begin(), it_3->settings.end(),
                                 [](setting &el) {
                                   return el.attribute_id == "color";
                                 });
      if(it_set != it_3->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Font color (active):"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }

      it_set = std::find_if(it_3->settings.begin(), it_3->settings.end(),
                            [](setting &el) {
                              return el.attribute_id == "background-color";
                            });
      if(it_set != it_3->settings.end())
        {
          lab = Gtk::make_managed<Gtk::Label>();
          lab->set_margin(5);
          lab->set_halign(Gtk::Align::START);
          lab->set_text(gettext("Background color (active):"));
          lab->set_name("windowLabel");
          grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
          Gtk::ColorDialogButton *color
              = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
          prop.signal_changed().connect([prop, it_set] {
            Gdk::RGBA rgba = prop.get_value();
            it_set->value = std::string(rgba.to_string());
          });
#endif
#ifdef ML_GTK_OLD
          Gtk::ColorButton *color = Gtk::make_managed<Gtk::ColorButton>();
          color->set_margin(5);
          color->set_halign(Gtk::Align::START);
          color->set_use_alpha(true);
          color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
          color->signal_color_set().connect([color, it_set] {
            Gdk::RGBA rgba = color->get_rgba();
            it_set->value = std::string(rgba.to_string());
          });
#endif
          grid->attach(*color, 1, row, 1, 1);
          row++;
        }
    }

  switch(wt)
    {
    case widget_type::progress_bar:
      {
        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](section &el) {
                            return el.section_id == "progressBars trough";
                          });
        if(it != settings_v.end())
          {
            auto it_set = std::find_if(
                it->settings.begin(), it->settings.end(), [](setting &el) {
                  return el.attribute_id == "background-color";
                });
            if(it_set != it->settings.end())
              {
                lab = Gtk::make_managed<Gtk::Label>();
                lab->set_margin(5);
                lab->set_halign(Gtk::Align::START);
                lab->set_text(gettext("Progress bar background color:"));
                lab->set_name("windowLabel");
                grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
                Gtk::ColorDialogButton *color
                    = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
                prop.signal_changed().connect([prop, it_set] {
                  Gdk::RGBA rgba = prop.get_value();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
#ifdef ML_GTK_OLD
                Gtk::ColorButton *color
                    = Gtk::make_managed<Gtk::ColorButton>();
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                color->set_use_alpha(true);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->signal_color_set().connect([color, it_set] {
                  Gdk::RGBA rgba = color->get_rgba();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
                grid->attach(*color, 1, row, 1, 1);
                row++;
              }
          }
        it = std::find_if(
            settings_v.begin(), settings_v.end(), [](section &el) {
              return el.section_id == "progressBars trough progress";
            });
        if(it != settings_v.end())
          {
            auto it_set = std::find_if(
                it->settings.begin(), it->settings.end(), [](setting &el) {
                  return el.attribute_id == "background-color";
                });
            if(it_set != it->settings.end())
              {
                lab = Gtk::make_managed<Gtk::Label>();
                lab->set_margin(5);
                lab->set_halign(Gtk::Align::START);
                lab->set_text(gettext("Progress bar color:"));
                lab->set_name("windowLabel");
                grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
                Gtk::ColorDialogButton *color
                    = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
                prop.signal_changed().connect([prop, it_set] {
                  Gdk::RGBA rgba = prop.get_value();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
#ifdef ML_GTK_OLD
                Gtk::ColorButton *color
                    = Gtk::make_managed<Gtk::ColorButton>();
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                color->set_use_alpha(true);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->signal_color_set().connect([color, it_set] {
                  Gdk::RGBA rgba = color->get_rgba();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
                grid->attach(*color, 1, row, 1, 1);
                row++;
              }
          }
        break;
      }
    case widget_type::column_view:
      {
        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](section &el) {
                            return el.section_id == "tablesView button";
                          });
        if(it != settings_v.end())
          {
            auto it_set = std::find_if(it->settings.begin(),
                                       it->settings.end(), [](setting &el) {
                                         return el.attribute_id == "color";
                                       });
            if(it_set != it->settings.end())
              {
                lab = Gtk::make_managed<Gtk::Label>();
                lab->set_margin(5);
                lab->set_halign(Gtk::Align::START);
                lab->set_text(gettext("Header font color:"));
                lab->set_name("windowLabel");
                grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
                Gtk::ColorDialogButton *color
                    = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
                prop.signal_changed().connect([prop, it_set] {
                  Gdk::RGBA rgba = prop.get_value();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
#ifdef ML_GTK_OLD
                Gtk::ColorButton *color
                    = Gtk::make_managed<Gtk::ColorButton>();
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                color->set_use_alpha(true);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->signal_color_set().connect([color, it_set] {
                  Gdk::RGBA rgba = color->get_rgba();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
                grid->attach(*color, 1, row, 1, 1);
                row++;
              }

            it_set = std::find_if(
                it->settings.begin(), it->settings.end(), [](setting &el) {
                  return el.attribute_id == "background-color";
                });
            if(it_set != it->settings.end())
              {
                lab = Gtk::make_managed<Gtk::Label>();
                lab->set_margin(5);
                lab->set_halign(Gtk::Align::START);
                lab->set_text(gettext("Header background color:"));
                lab->set_name("windowLabel");
                grid->attach(*lab, 0, row, 1, 1);

#ifndef ML_GTK_OLD
                Gtk::ColorDialogButton *color
                    = Gtk::make_managed<Gtk::ColorDialogButton>(color_dialog);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                Glib::PropertyProxy<Gdk::RGBA> prop = color->property_rgba();
                prop.signal_changed().connect([prop, it_set] {
                  Gdk::RGBA rgba = prop.get_value();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
#ifdef ML_GTK_OLD
                Gtk::ColorButton *color
                    = Gtk::make_managed<Gtk::ColorButton>();
                color->set_margin(5);
                color->set_halign(Gtk::Align::START);
                color->set_use_alpha(true);
                color->set_rgba(Gdk::RGBA(Glib::ustring(it_set->value)));
                color->signal_color_set().connect([color, it_set] {
                  Gdk::RGBA rgba = color->get_rgba();
                  it_set->value = std::string(rgba.to_string());
                });
#endif
                grid->attach(*color, 1, row, 1, 1);
                row++;
              }
          }
        break;
      }
    default:
      break;
    }

  return frame;
}

void
SettingsWindow::readSettings()
{
  std::filesystem::path set_path = save_path;
  set_path /= std::filesystem::u8path("MLStyles.css");
  if(!std::filesystem::exists(set_path))
    {
      set_path = af->share_path();
      set_path /= std::filesystem::u8path("MLStyles.css");
    }
  std::fstream f;
  f.open(set_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.seekg(0, std::ios_base::end);
      source_settings.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(source_settings.data(), source_settings.size());
      f.close();

      parseSettings();
      source_settings.clear();
      source_settings.shrink_to_fit();
    }
  else
    {
      std::cout
          << "SettingsWindow::readSettings: settings file has not been opened"
          << std::endl;
    }

  auto it
      = std::find_if(settings_v.begin(), settings_v.end(), [](section &el) {
          return el.section_id == "MLwindow";
        });
  if(it != settings_v.end())
    {
      auto it_set = std::find_if(
          it->settings.begin(), it->settings.end(), [](setting &el) {
            return el.attribute_id == "background-image";
          });
      if(it_set == it->settings.end())
        {
          setting set;
          set.attribute_id = "background-image";
          set.value = "none";
          it->settings.emplace_back(set);
        }
      it_set = std::find_if(it->settings.begin(), it->settings.end(),
                            [](setting &el) {
                              return el.attribute_id == "background-size";
                            });
      if(it_set == it->settings.end())
        {
          setting set;
          set.attribute_id = "background-size";
          set.value = "cover";
          it->settings.emplace_back(set);
        }
    }
}

void
SettingsWindow::parseSettings()
{
  std::string::size_type n = 0;
  while(n != std::string::npos)
    {
      std::string find_str("#");
      n = source_settings.find(find_str, n);
      if(n != std::string::npos)
        {
          n += find_str.size();
          find_str = "{";
          std::string::size_type n2 = source_settings.find(find_str, n);
          if(n2 != std::string::npos)
            {
              section s;
              s.section_id = source_settings.substr(n, n2 - n);
              while(s.section_id.size() > 0)
                {
                  if(*s.section_id.rbegin() == 32)
                    {
                      s.section_id.pop_back();
                    }
                  else
                    {
                      break;
                    }
                }
              n = n2 + find_str.size();
              find_str = "}";
              n2 = source_settings.find(find_str, n);
              if(n2 != std::string::npos)
                {
                  parseSection(s, source_settings.substr(n, n2 - n));
                  n = n2 + find_str.size();
                  settings_v.emplace_back(s);
                }
            }
        }
    }
}

void
SettingsWindow::parseSection(section &s, const std::string &section_str)
{
  std::string::size_type n = 0;
  while(n != std::string::npos)
    {
      std::string find_str(";");
      std::string::size_type n2 = section_str.find(find_str, n);
      if(n2 != std::string::npos)
        {
          std::string raw = section_str.substr(n, n2 - n);
          n = n2 + find_str.size();
          find_str = ":";
          n2 = raw.find(find_str);
          if(n2 != std::string::npos)
            {
              setting set;
              set.attribute_id = raw.substr(0, n2);
              set.attribute_id.erase(std::remove_if(set.attribute_id.begin(),
                                                    set.attribute_id.end(),
                                                    [](char &el) {
                                                      if(el >= 0 && el <= 32)
                                                        {
                                                          return true;
                                                        }
                                                      else
                                                        {
                                                          return false;
                                                        }
                                                    }),
                                     set.attribute_id.end());
              raw.erase(0, n2 + find_str.size());
              set.value = raw;

              for(auto it = set.value.begin(); it != set.value.end();)
                {
                  switch(*it)
                    {
                    case 0 ... 8:
                    case 11 ... 31:
                      {
                        set.value.erase(it);
                        break;
                      }
                    case 9:
                    case 10:
                    case 32:
                      {
                        if(it != set.value.begin())
                          {
                            if(*(it - 1) == 32)
                              {
                                set.value.erase(it);
                              }
                            else
                              {
                                *it = 32;
                                it++;
                              }
                          }
                        else
                          {
                            set.value.erase(it);
                          }
                        break;
                      }
                    default:
                      {
                        it++;
                        break;
                      }
                    }
                }
              s.settings.emplace_back(set);
            }
        }
      else
        {
          break;
        }
    }
}

void
SettingsWindow::applySettings()
{
  auto it_ab
      = std::find_if(settings_v.begin(), settings_v.end(), [](section &el) {
          return el.section_id == "aboutDialog label";
        });
  if(it_ab != settings_v.end())
    {
      auto it = std::find_if(settings_v.begin(), settings_v.end(),
                             [](section &el) {
                               return el.section_id == "windowLabel";
                             });
      if(it != settings_v.end())
        {
          it_ab->settings = it->settings;
        }
    }
  std::filesystem::path sp
      = save_path / std::filesystem::u8path("MLStyles.css");
  std::filesystem::create_directories(save_path);
  std::filesystem::remove_all(sp);
  std::fstream f;
  f.open(sp, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      for(auto it = settings_v.begin(); it != settings_v.end(); it++)
        {
          std::string write_str = "#" + it->section_id + " {\n";
          f.write(write_str.c_str(), write_str.size());
          for(auto it_set = it->settings.begin(); it_set != it->settings.end();
              it_set++)
            {
              write_str
                  = "\t" + it_set->attribute_id + ": " + it_set->value + ";\n";
              f.write(write_str.c_str(), write_str.size());
            }
          write_str = "}\n\n";
          f.write(write_str.c_str(), write_str.size());
        }
      f.close();

      Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
      css_provider->load_from_path(sp.u8string());
      Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
#ifndef ML_GTK_OLD
      Gtk::StyleProvider::add_provider_for_display(
          disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
#ifdef ML_GTK_OLD
      Gtk::StyleContext::add_provider_for_display(
          disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
    }
}

void
SettingsWindow::fileDialog(Gtk::Entry *ent)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_title(gettext("Image"));
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> fl = Gio::File::create_for_path(af->homePath());
  fd->set_initial_folder(fl);

  std::vector<Gdk::PixbufFormat> format = Gdk::Pixbuf::get_formats();
  std::vector<Glib::ustring> fmts;
  for(auto it = format.begin(); it != format.end(); it++)
    {
      std::vector<Glib::ustring> ext = it->get_extensions();
      std::copy(ext.begin(), ext.end(), std::back_inserter(fmts));
    }

  Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> filter_list
      = Gio::ListStore<Gtk::FileFilter>::create();
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->add_pixbuf_formats();
  filter_list->append(filter);

  fd->set_filters(filter_list);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fd->open(*window,
           std::bind(&SettingsWindow::fileDialogSlot, this,
                     std::placeholders::_1, fd, ent),
           cncl);
#endif
#ifdef ML_GTK_OLD
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *window, gettext("Image"), Gtk::FileChooser::Action::OPEN, true);
  fd->set_application(window->get_application());
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

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->add_pixbuf_formats();
  fd->set_filter(filter);

  fd->signal_response().connect(std::bind(
      &SettingsWindow::fileDialogSlot, this, std::placeholders::_1, fd, ent));

  fd->signal_close_request().connect(
      [fd] {
        std::unique_ptr<Gtk::FileChooserDialog> fdl(fd);
        fdl->set_visible(false);
        return true;
      },
      false);

  fd->present();
#endif
}

void
SettingsWindow::fileDialogSlot(int respons_id, Gtk::FileChooserDialog *fd,
                               Gtk::Entry *ent)
{
  if(respons_id == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          ent->set_text(fl->get_path());
        }
    }
  fd->close();
}

#ifndef ML_GTK_OLD
void
SettingsWindow::fileDialogSlot(const Glib::RefPtr<Gio::AsyncResult> &result,
                               const Glib::RefPtr<Gtk::FileDialog> &fd,
                               Gtk::Entry *ent)
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
          std::cout << "SettingsWindow::fileDialogSlot error: " << er.what()
                    << std::endl;
        }
    }
  if(fl)
    {
      ent->set_text(fl->get_path());
    }
}
#endif

void
SettingsWindow::windowSize()
{
  Glib::RefPtr<Gdk::Surface> surf = parent_window->get_surface();
  Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
  Glib::RefPtr<Gdk::Monitor> mon = disp->get_monitor_at_surface(surf);
  Gdk::Rectangle req;
  mon->get_geometry(req);

  req.set_width(req.get_width() * mon->get_scale_factor());
  req.set_height(req.get_height() * mon->get_scale_factor());

  int w = req.get_width() * 0.55;
  int h = req.get_height() * 0.55;
  window->set_default_size(w, h);
}
