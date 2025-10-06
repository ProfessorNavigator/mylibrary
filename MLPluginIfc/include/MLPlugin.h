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
#ifndef MLPLUGIN_H
#define MLPLUGIN_H

#include <AuxFunc.h>
#include <gtkmm-4.0/gtkmm/window.h>

/*!
 * \mainpage MLPluginIfc
 *
 * \b MLPluginIfc library provides interfaces to create plugins for <A
 * HREF="https://github.com/ProfessorNavigator/mylibrary">MyLibrary</A>.
 *
 * To start add cmake package MLPluginIfc to your project.
 *
 * \code{.unparsed}
 * find_package(MLPluginIfc)
 * if(MLPluginIfc_FOUND)
 *  target_link_libraries(myproject
 *    PRIVATE MLPluginIfc::mlpluginifc
 *    PRIVATE MLBookProc::mlbookproc
 *  )
 * endif()
 * \endcode
 *
 * \note MLPluginIfc uses <A HREF="https://gtkmm.gnome.org/en/">gtkmm</A> as
 * dependency. If version of gtkmm is less then 4.10, MLPluginIfc sets
 * ML_GTK_OLD build variable.
 *
 * See MLPlugin for details and code example.
 */

/*!
 * \brief The MLPlugin class
 *
 * MLPlugin is base class for plugins creation. To create plugin inherit
 * your plugin base class from MLPlugin and override createWindow() method.
 * Also set #plugin_name and #plugin_description if needed. Your plugin base
 * header file must include C function create (see example).
 *
 * \include examples/ExamplePlugin.h
 * \include examples/ExamplePlugin.cpp
 */
class MLPlugin
{
public:
  /*!
   * \brief MLPlugin constructor
   * \param af_ptr pointer to std::shared_ptr<AuxFunc> object.
   */
  MLPlugin(void *af_ptr);

  virtual ~MLPlugin();

  /*!
   * \brief Virtual function.
   *
   * You should override this method to create your plugin window. New window
   * should be transient for parent_window. It is also strongly recommended to
   * make new window modal (see example in class description).
   *
   * \param parent_window pointer to parent window object
   */
  virtual void
  createWindow(Gtk::Window *parent_window);

  /*!
   * \brief Returns plugin name if set.
   * \return <A
   * HREF="https://gnome.pages.gitlab.gnome.org/glibmm/classGlib_1_1ustring.html">Glib</A>
   * string containing plugin name
   */
  Glib::ustring
  getPluginName();

  /*!
   * \brief Returns plugin description.
   * \return <A
   * HREF="https://gnome.pages.gitlab.gnome.org/glibmm/classGlib_1_1ustring.html">Glib</A>
   * string containing plugin name
   */
  Glib::ustring
  getPluginDescription();

protected:
  /*!
   * \brief Pointer to AuxFunc object.
   *
   * AuxFunc class contains various useful methods (see \b MLBookProc
   * documentation).
   */
  std::shared_ptr<AuxFunc> af;

  /*!
   * \brief Plugin name
   */
  Glib::ustring plugin_name;
  /*!
   * \brief Plugin description
   */
  Glib::ustring plugin_description;
};

#endif // MLPLUGIN_H
