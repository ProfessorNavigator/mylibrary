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

#include <CoverPixBuf.h>
#include <Magick++.h>
#include <MagickModelItem.h>
#include <SaveCover.h>
#include <filesystem>
#include <functional>
#include <giomm-2.68/giomm/liststore.h>
#include <glibmm-2.68/glibmm/miscutils.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/expression.h>
#include <gtkmm-4.0/gtkmm/grid.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/signallistitemfactory.h>
#include <iostream>
#include <libintl.h>
#include <vector>

#ifndef ML_GTK_OLD
#include <giomm-2.68/giomm/cancellable.h>
#include <giomm-2.68/giomm/liststore.h>
#include <gtkmm-4.0/gtkmm/error.h>
#endif

SaveCover::SaveCover(const std::shared_ptr<BookInfoEntry> &bie,
                     Gtk::Window *parent_window)
{
  this->bie = bie;
  this->parent_window = parent_window;
  createWindow();
}

void
SaveCover::createWindow()
{
  this->set_application(parent_window->get_application());
  this->set_title(gettext("Cover saving"));
  this->set_transient_for(*parent_window);
  this->set_modal(true);
  this->set_name("MLwindow");

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  this->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_use_markup(true);
  lab->set_markup("<b>" + Glib::ustring(gettext("Format")) + "</b>");
  lab->set_name("windowLabel");
  grid->attach(*lab, 0, 0, 2, 1);

  Glib::RefPtr<Gio::ListModel> format_list = createModel();

  guint default_num = 0;
  for(guint i = 0; i < format_list->get_n_items(); i++)
    {
      Glib::RefPtr<MagickModelItem> item
          = std::dynamic_pointer_cast<MagickModelItem>(
              format_list->get_object(i));
      if(item)
        {
          if(item->info.name() == "JPEG")
            {
              default_num = i;
              break;
            }
        }
    }

  Glib::RefPtr<Gtk::SignalListItemFactory> factory
      = Gtk::SignalListItemFactory::create();

  factory->signal_setup().connect(
      [](const Glib::RefPtr<Gtk::ListItem> &list_item) {
        Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
        lab->set_halign(Gtk::Align::CENTER);
        list_item->set_child(*lab);
      });

  factory->signal_bind().connect(
      [](const Glib::RefPtr<Gtk::ListItem> &list_item) {
        Gtk::Label *lab = dynamic_cast<Gtk::Label *>(list_item->get_child());
        if(lab)
          {
            Glib::RefPtr<MagickModelItem> item
                = std::dynamic_pointer_cast<MagickModelItem>(
                    list_item->get_item());
            if(item)
              {
                Glib::ustring str(item->info.name());
                lab->set_text(str.lowercase());
              }
          }
      });

  Glib::RefPtr<Gtk::ClosureExpression<Glib::ustring>> expr
      = Gtk::ClosureExpression<Glib::ustring>::create(
          [](Glib::RefPtr<Glib::ObjectBase> obj) {
            Glib::ustring result;
            Glib::RefPtr<MagickModelItem> item
                = std::dynamic_pointer_cast<MagickModelItem>(obj);
            if(item)
              {
                Glib::ustring str(item->info.name());
                result = str.lowercase();
              }
            return result;
          });

  format = Gtk::make_managed<Gtk::DropDown>();
  format->set_margin(5);
  format->set_halign(Gtk::Align::CENTER);
  format->set_name("comboBox");
  format->set_factory(factory);
  format->set_model(format_list);
  format->set_enable_search(true);
  format->set_expression(expr);
  if(format_list->get_n_items() > 0)
    {
      format->set_selected(default_num);
    }
  grid->attach(*format, 0, 1, 2, 1);

  Gtk::Button *save = Gtk::make_managed<Gtk::Button>();
  save->set_margin(5);
  save->set_halign(Gtk::Align::CENTER);
  save->set_name("applyBut");
  save->set_label(gettext("Save"));
  save->signal_clicked().connect(std::bind(&SaveCover::saveDialog, this));
  grid->attach(*save, 0, 5, 1, 1);

  Gtk::Button *cancel = Gtk::make_managed<Gtk::Button>();
  cancel->set_margin(5);
  cancel->set_halign(Gtk::Align::CENTER);
  cancel->set_name("cancelBut");
  cancel->set_label(gettext("Cancel"));
  cancel->signal_clicked().connect(std::bind(&Gtk::Window::close, this));
  grid->attach(*cancel, 1, 5, 1, 1);
}

void
SaveCover::saveDialog()
{
#ifndef ML_GTK_OLD
  Glib::RefPtr<Gtk::FileDialog> fd = Gtk::FileDialog::create();
  fd->set_title(gettext("Cover saving path"));
  fd->set_modal(true);

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(Glib::get_home_dir());
  fd->set_initial_folder(initial);

  Glib::ustring name;
  Glib::RefPtr<MagickModelItem> item
      = std::dynamic_pointer_cast<MagickModelItem>(
          format->get_selected_item());
  if(item)
    {
      name = "Cover";

      Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
      filter->add_mime_type(item->info.mimeType());
      Glib::ustring str(item->info.name());
      filter->set_name(str.lowercase());

      Glib::RefPtr<Gio::ListStore<Gtk::FileFilter>> f_list
          = Gio::ListStore<Gtk::FileFilter>::create();
      f_list->append(filter);
      fd->set_filters(f_list);

      fd->set_default_filter(filter);
    }
  fd->set_initial_name(name);

  fd->save(*this, std::bind(&SaveCover::saveDialogResult, this,
                            std::placeholders::_1, fd));
#else
  Gtk::FileChooserDialog *fd
      = new Gtk::FileChooserDialog(*this, gettext("Cover saving path"),
                                   Gtk::FileChooser::Action::SAVE, true);
  fd->set_application(this->get_application());
  fd->set_modal(true);
  fd->set_name("MLwindow");

  Gtk::Button *but
      = fd->add_button(gettext("Cancel"), Gtk::ResponseType::CANCEL);
  but->set_margin(5);
  but->set_name("cancelBut");

  but = fd->add_button(gettext("Save"), Gtk::ResponseType::ACCEPT);
  but->set_margin(5);
  but->set_name("applyBut");

  Glib::RefPtr<Gio::File> initial
      = Gio::File::create_for_path(Glib::get_home_dir());
  fd->set_current_folder(initial);

  Glib::ustring name;
  Glib::RefPtr<MagickModelItem> item
      = std::dynamic_pointer_cast<MagickModelItem>(
          format->get_selected_item());
  if(item)
    {
      Glib::ustring str(item->info.name());
      name = "Cover." + str.lowercase();

      Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
      filter->add_mime_type(item->info.mimeType());
      filter->set_name(str.lowercase());
      fd->add_filter(filter);
      fd->set_filter(filter);
    }
  fd->set_current_name(name);

  fd->signal_response().connect(std::bind(&SaveCover::saveDialogResult, this,
                                          std::placeholders::_1, fd));

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

#ifndef ML_GTK_OLD
void
SaveCover::saveDialogResult(const Glib::RefPtr<Gio::AsyncResult> &result,
                            const Glib::RefPtr<Gtk::FileDialog> &fd)
{
  Glib::RefPtr<Gio::File> fl;
  try
    {
      fl = fd->save_finish(result);
    }
  catch(Gtk::DialogError &er)
    {
      if(er.code() == Gtk::DialogError::Code::FAILED)
        {
          std::cout << "SaveCover::save_dialog_result error: " << er.what()
                    << std::endl;
        }
    }
  if(fl)
    {
      saveFunc(fl);
    }
}
#else
void
SaveCover::saveDialogResult(int resp, Gtk::FileChooserDialog *fd)
{
  if(resp == Gtk::ResponseType::ACCEPT)
    {
      Glib::RefPtr<Gio::File> fl = fd->get_file();
      if(fl)
        {
          saveFunc(fl);
        }
    }
  fd->close();
}
#endif

Glib::RefPtr<Gio::ListModel>
SaveCover::createModel()
{
  Glib::RefPtr<Gio::ListStore<MagickModelItem>> result
      = Gio::ListStore<MagickModelItem>::create();

  std::vector<Magick::CoderInfo> list;
  try
    {
      Magick::coderInfoList(&list, Magick::CoderInfo::AnyMatch,
                            Magick::CoderInfo::TrueMatch,
                            Magick::CoderInfo::AnyMatch);
    }
  catch(Magick::Exception &er)
    {
      std::cout << "SaveCover::createModel: " << er.what() << std::endl;
    }

  for(auto it = list.begin(); it != list.end(); it++)
    {
      if(!it->mimeType().empty())
        {
          Glib::RefPtr<MagickModelItem> item = MagickModelItem::create(*it);
          result->append(item);
        }
    }

  return result;
}

void
SaveCover::saveFunc(const Glib::RefPtr<Gio::File> &fl)
{
  bool result = false;
  Glib::RefPtr<MagickModelItem> item
      = std::dynamic_pointer_cast<MagickModelItem>(
          format->get_selected_item());
  if(item)
    {
      std::filesystem::path save_path
          = std::filesystem::u8path(fl->get_path());
      CoverPixBuf pb(bie);
      result = pb.saveImage(save_path, item->info.name());
    }

  this->unset_child();
  this->set_default_size(1, 1);

  Gtk::Grid *grid = Gtk::make_managed<Gtk::Grid>();
  grid->set_halign(Gtk::Align::FILL);
  grid->set_valign(Gtk::Align::FILL);
  grid->set_expand(true);
  this->set_child(*grid);

  Gtk::Label *lab = Gtk::make_managed<Gtk::Label>();
  lab->set_margin(5);
  lab->set_halign(Gtk::Align::CENTER);
  lab->set_expand(true);
  lab->set_name("windowLabel");
  if(result)
    {
      lab->set_text(gettext("Cover successfully saved!"));
    }
  else
    {
      lab->set_wrap(true);
      lab->set_wrap_mode(Pango::WrapMode::WORD);
      lab->set_max_width_chars(50);
      lab->set_width_chars(50);
      lab->set_justify(Gtk::Justification::CENTER);
      lab->set_text(Glib::ustring(gettext("Error!")));
    }
  grid->attach(*lab, 0, 0, 1, 1);

  Gtk::Button *close = Gtk::make_managed<Gtk::Button>();
  close->set_margin(5);
  close->set_halign(Gtk::Align::CENTER);
  close->set_name("operationBut");
  close->set_label(gettext("Close"));
  close->signal_clicked().connect(std::bind(&Gtk::Window::close, this));
  grid->attach(*close, 0, 1, 1, 1);
}
