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
#include <Magick++.h>
#include <SettingsWindow.h>
#include <fstream>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gdkmm/monitor.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/cssprovider.h>
#include <gtkmm-4.0/gtkmm/flowbox.h>
#include <gtkmm-4.0/gtkmm/frame.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/notebook.h>
#include <gtkmm-4.0/gtkmm/scale.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/separator.h>
#include <iostream>
#include <libintl.h>
#include <sstream>

#ifndef ML_GTK_OLD
#include <gtkmm-4.0/gtkmm/colordialogbutton.h>
#include <gtkmm-4.0/gtkmm/error.h>
#else
#include <gtkmm-4.0/gtkmm/colorbutton.h>
#endif

SettingsWindow::SettingsWindow(const std::shared_ptr<AuxFunc> &af,
                               Gtk::Window *parent_window)
{
  this->af = af;
  this->parent_window = parent_window;
  save_path = af->homePath();
  save_path /= std::filesystem::u8path(".config")
               / std::filesystem::u8path("MyLibrary");
  readSettings();
  readSearchSettings();
  createWindow();
}

void
SettingsWindow::createWindow()
{
  this->set_application(parent_window->get_application());
  this->set_title(gettext("Settings"));
  this->set_transient_for(*parent_window);
  this->set_modal(true);
  this->set_name("MLwindow");
  windowSize();

  Gtk::Box *main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  main_box->set_halign(Gtk::Align::FILL);
  main_box->set_valign(Gtk::Align::FILL);
  this->set_child(*main_box);

  Gtk::Notebook *note_book = Gtk::make_managed<Gtk::Notebook>();
  note_book->set_margin(5);
  note_book->set_halign(Gtk::Align::FILL);
  note_book->set_valign(Gtk::Align::FILL);
  note_book->set_name("MLnotebook");
  main_box->append(*note_book);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_name("windowLabel");
  lab->set_markup(Glib::ustring("<b>") + gettext("Search settings") + "</b>");
  note_book->append_page(*searchTab(), *lab);

  lab = Gtk::make_managed<Gtk::Label>();
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_use_markup(true);
  lab->set_name("windowLabel");
  lab->set_markup(Glib::ustring("<b>") + gettext("Color settings") + "</b>");
  note_book->append_page(*colorTab(), *lab);

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
    this->close();
  });
  but_box->append(*apply_close);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_label(gettext("Cancel"));
  close->set_name("cancelBut");
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, this));
  but_box->append(*close);
}

Gtk::Widget *
SettingsWindow::colorTab()
{
  Gtk::ScrolledWindow *set_scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
  set_scrl->set_halign(Gtk::Align::FILL);
  set_scrl->set_valign(Gtk::Align::FILL);
  set_scrl->set_expand(true);
  set_scrl->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

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
  f_box->insert(*formSection(widget_type::frames), -1);

  return set_scrl;
}

Gtk::Widget *
SettingsWindow::searchTab()
{
  Gtk::Box *box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  box->set_halign(Gtk::Align::FILL);
  box->set_valign(Gtk::Align::FILL);
  box->set_expand(true);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::START);
  lab->set_name("windowLabel");
  lab->set_text(gettext("Сoefficient of coincedence for search operations:"));
  box->append(*lab);

  Gtk::Scale *coef_coincidence = Gtk::make_managed<Gtk::Scale>();
  coef_coincidence->set_margin(5);
  coef_coincidence->set_halign(Gtk::Align::FILL);
  coef_coincidence->set_name("MLscale");
  coef_coincidence->set_range(0.0, 100.0);
  coef_coincidence->set_value(coef_coincedence_val * 100.0);
  coef_coincidence->add_mark(25.0, Gtk::PositionType::BOTTOM, "25%");
  coef_coincidence->add_mark(50.0, Gtk::PositionType::BOTTOM, "50%");
  coef_coincidence->add_mark(75.0, Gtk::PositionType::BOTTOM, "75%");
  coef_coincidence->set_draw_value(true);
  coef_coincidence->signal_value_changed().connect([this, coef_coincidence] {
    coef_coincedence_val = coef_coincidence->get_value() * 0.01;
  });
  std::shared_ptr<std::stringstream> strm(new std::stringstream);
  strm->imbue(std::locale("C"));
  coef_coincidence->set_format_value_func([strm](const double &val) {
    strm->clear();
    strm->str("");
    *strm << std::fixed << std::setprecision(1) << val;
    return Glib::ustring(strm->str()) + "%";
  });
  box->append(*coef_coincidence);

  return box;
}

Gtk::Widget *
SettingsWindow::windowsSection()
{
  Gtk::Frame *frame = Gtk::make_managed<Gtk::Frame>();
  frame->set_margin(5);
  frame->set_name("MLframe");

  Gtk::Box *section_box
      = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
  frame->set_child(*section_box);

  auto it
      = std::find_if(settings_v.begin(), settings_v.end(), [](section &el) {
          return el.section_id == "MLwindow";
        });
  if(it != settings_v.end())
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
#else
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
                              return el.attribute_id == "background-image";
                            });
      if(it_set != it->settings.end())
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
#ifdef __linux
          Glib::ustring find_str("file://");
#endif
#ifdef _WIN32
          Glib::ustring find_str("file:///");
#endif
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
#ifdef _WIN32
          find_str = ":";
          n = val.find(find_str);
          if(n != Glib::ustring::npos)
            {
              rp = val.substr(0, n);
              val.erase(0, n);
              val = rp.uppercase() + val;
            }
          n = 0;
          find_str = "/";
          rp = "\\";
          while(n != Glib::ustring::npos)
            {
              n = val.find(find_str);
              if(n != Glib::ustring::npos)
                {
                  val.erase(n, find_str.size());
                  val.insert(n, rp);
                }
            }
#endif
          image_path->set_text(val);

          Glib::PropertyProxy<Glib::ustring> prop
              = image_path->property_text();
          prop.signal_changed().connect([it_set, image_path] {
            Glib::ustring p = image_path->get_text();
            if(p.empty())
              {
                p = "none";
                it_set->value = std::string(p);
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
#ifdef __linux
                it_set->value = "url(file://" + std::string(p) + ")";
#endif
#ifdef _WIN32
                n = 0;
                find_str = "\\";
                rp = "/";
                while(n != Glib::ustring::npos)
                  {
                    n = p.find(find_str);
                    if(n != Glib::ustring::npos)
                      {
                        p.erase(n, find_str.size());
                        p.insert(n, rp);
                      }
                  }
                find_str = ":";

                n = p.find(":");
                if(n != Glib::ustring::npos)
                  {
                    Glib::ustring lower = p.substr(0, n);
                    p.erase(0, n);
                    p = lower.lowercase() + p;
                  }
                it_set->value = "url(file:///" + std::string(p) + ")";
#endif
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
  frame->set_name("MLframe");

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
    case widget_type::frames:
      {
        search_str = "MLframe";
        sec_name = gettext("Frames");
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
#else
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
#else
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
#else
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
#else
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
#else
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
#else
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
#else
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
#else
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
#else
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
#else
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
#else
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

        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](section &el) {
                            return el.section_id == "selectedLab";
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
                lab->set_text(gettext("Selected line font color:"));
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
#else
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
                lab->set_text(gettext("Selected line background color:"));
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
#else
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
      set_path /= std::filesystem::u8path("MyLibrary");
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
SettingsWindow::readSearchSettings()
{
  std::filesystem::path p = af->homePath();
  p /= std::filesystem::u8path(".config");
  p /= std::filesystem::u8path("MyLibrary");
  p /= std::filesystem::u8path("SearchSettings");
  std::fstream f;
  f.open(p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t sz = sizeof(coef_coincedence_val);
      f.seekg(0, std::ios_base::end);
      size_t fsz = f.tellg();
      if(fsz >= sz)
        {
          f.seekg(0, std::ios_base::beg);
          f.read(reinterpret_cast<char *>(&coef_coincedence_val), sz);
          ByteOrder bo;
          bo.set_little(coef_coincedence_val);
          coef_coincedence_val = bo;
        }
      f.close();
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
  applySearchSettings();
  {
    auto it = std::find_if(settings_v.begin(), settings_v.end(),
                           [](const section &el) {
                             return el.section_id == "windowLabel";
                           });
    if(it != settings_v.end())
      {
        std::vector<setting> sv = it->settings;

        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](const section &el) {
                            return el.section_id == "MLscale marks.bottom";
                          });
        if(it != settings_v.end())
          {
            it->settings = sv;
          }
        else
          {
            section sc;
            sc.section_id = "MLscale marks.bottom";
            sc.settings = sv;
            settings_v.push_back(sc);
          }

        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](const section &el) {
                            return el.section_id == "MLscale value";
                          });
        if(it != settings_v.end())
          {
            it->settings = sv;
          }
        else
          {
            section sc;
            sc.section_id = "MLscale value";
            sc.settings = sv;
            settings_v.push_back(sc);
          }

        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](const section &el) {
                            return el.section_id == "MLnotebook header.top";
                          });
        if(it != settings_v.end())
          {
            it->settings = sv;
          }
        else
          {
            section sc;
            sc.section_id = "MLnotebook header.top";
            sc.settings = sv;
            settings_v.push_back(sc);
          }

        it = std::find_if(settings_v.begin(), settings_v.end(),
                          [](const section &el) {
                            return el.section_id == "MLnotebook stack";
                          });
        if(it == settings_v.end())

          {
            section sc;
            sc.section_id = "MLnotebook stack";
            setting set;
            set.attribute_id = "background-color";
            set.value = "rgba(255, 255, 255, 0)";
            sc.settings.push_back(set);
            settings_v.push_back(sc);
          }
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
      Glib::RefPtr<Gdk::Display> disp = parent_window->get_display();
#ifndef ML_GTK_OLD
      css_provider->load_from_string(loadStyles(sp));
      Gtk::StyleProvider::add_provider_for_display(
          disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#else
      css_provider->load_from_data(loadStyles(sp));
      Gtk::StyleContext::add_provider_for_display(
          disp, css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
    }
}

void
SettingsWindow::applySearchSettings()
{
  std::filesystem::path p = af->homePath();
  p /= std::filesystem::u8path(".config");
  p /= std::filesystem::u8path("MyLibrary");
  p /= std::filesystem::u8path("SearchSettings");
  std::filesystem::create_directories(p.parent_path());
  std::filesystem::remove_all(p);
  std::fstream f;
  f.open(p, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      double cc = coef_coincedence_val;
      ByteOrder bo(cc);
      bo.get_little(cc);
      f.write(reinterpret_cast<char *>(&cc), sizeof(cc));
      f.close();
    }
  if(signal_coef_coincedence)
    {
      signal_coef_coincedence(coef_coincedence_val);
    }
}

void
SettingsWindow::fileDialog(Gtk::Entry *ent)
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_title(gettext("Image"));
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> fl
      = Gio::File::create_for_path(af->homePath().u8string());
  fd->set_initial_folder(fl);

  Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> filter_list
      = Gio::ListStore<Gtk::FileFilter>::create();

  std::vector<Magick::CoderInfo> list;
  try
    {
      Magick::coderInfoList(&list, Magick::CoderInfo::TrueMatch,
                            Magick::CoderInfo::AnyMatch,
                            Magick::CoderInfo::AnyMatch);
    }
  catch(Magick::Exception &er)
    {
      std::cout << "SettingsWindow::fileDialog: " << er.what() << std::endl;
    }

  Glib::RefPtr<Gtk::FileFilter> default_filter = Gtk::FileFilter::create();
  default_filter->set_name(gettext("All images"));
  filter_list->append(default_filter);

  for(auto it = list.begin(); it != list.end(); it++)
    {
      if(!it->mimeType().empty())
        {
          Glib::ustring str(it->name());
          Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
          filter->set_name(str.lowercase());
          filter->add_mime_type(it->mimeType());
          filter_list->append(filter);
          default_filter->add_mime_type(it->mimeType());
        }
    }

  fd->set_filters(filter_list);
  fd->set_default_filter(default_filter);

  Glib::RefPtr<Gio::Cancellable> cncl = Gio::Cancellable::create();
  fd->open(*this,
           std::bind(&SettingsWindow::fileDialogSlot, this,
                     std::placeholders::_1, fd, ent),
           cncl);
#else
  Gtk::FileChooserDialog *fd = new Gtk::FileChooserDialog(
      *this, gettext("Image"), Gtk::FileChooser::Action::OPEN, true);
  fd->set_application(this->get_application());
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

  std::vector<Magick::CoderInfo> list;
  try
    {
      Magick::coderInfoList(&list, Magick::CoderInfo::TrueMatch,
                            Magick::CoderInfo::AnyMatch,
                            Magick::CoderInfo::AnyMatch);
    }
  catch(Magick::Exception &er)
    {
      std::cout << "SettingsWindow::fileDialog: " << er.what() << std::endl;
    }

  Glib::RefPtr<Gtk::FileFilter> default_filter = Gtk::FileFilter::create();
  default_filter->set_name(gettext("All images"));
  fd->add_filter(default_filter);

  for(auto it = list.begin(); it != list.end(); it++)
    {
      if(!it->mimeType().empty())
        {
          Glib::ustring str(it->name());
          Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
          filter->set_name(str.lowercase());
          filter->add_mime_type(it->mimeType());
          fd->add_filter(filter);
          default_filter->add_mime_type(it->mimeType());
        }
    }

  fd->set_filter(default_filter);

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

#ifdef ML_GTK_OLD
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
#endif

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
  this->set_default_size(w, h);
}

std::string
SettingsWindow::loadStyles(const std::filesystem::path &sp)
{
  std::string result;

  std::fstream f;
  f.open(sp, std::ios_base::in | std::ios_base::binary);
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
      std::cout << "SettingsWindow::loadStyles: " << er.what() << std::endl;
      return result;
    }

  std::filesystem::path temp_background_path = af->temp_path();
  temp_background_path
      /= std::filesystem::u8path(af->randomFileName() + ".png");

  try
    {
      img.write(temp_background_path.string());
    }
  catch(Magick::Exception &er)
    {
      std::cout << "SettingsWindow::loadStyles: " << er.what() << std::endl;
      return result;
    }

  result.erase(result.begin() + n, result.begin() + n3);
  find_str = temp_background_path.u8string();
  result.insert(result.begin() + n, find_str.begin(), find_str.end());

  if(signal_new_background_path)
    {
      signal_new_background_path(temp_background_path);
    }

  return result;
}
