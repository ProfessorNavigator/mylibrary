/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <Bases.h>
#include <QWidget>
#include <filesystem>

/*!
 * \mainpage MLPlugin
 *
 * \b MLPlugin library provides interfaces to create plugins for <a
 * href="https://github.com/ProfessorNavigator/mylibrary">MyLibrary</A>.
 *
 * To start add cmake package MLPlugin to your project.
 *
 * \code{.unparsed}
 * find_package(MLPlugin REQUIRED)
 * target_link_libraries(my_target
 *    PRIVATE MLPlugin::MLPlugin
 *    PRIVATE MLBookProc::MLBookProc
 *  )
 * \endcode
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
   * \brief MLPlugin constructor.
   * \param bases pointer to Bases object.
   * \param plugin_path Pointer to std::filesystem::path object.
   */
  MLPlugin(void *bases, void *plugin_path);

  /*!
   *
   * This method will be called when user requests to show plugin window.
   *
   * \param parent Pointer to parent window widget.
   */
  virtual void
  createWindow(QWidget *parent);

  /*!
   * Returns plugin name set in #plugin_name.
   * \return Value of #plugin_name.
   */
  QString
  getPluginName();

  /*!
   * Returns plugin description set on #plugin_description.
   *
   * \return Value of #plugin_description.
   */
  QString
  getPluginDescription();

  /*!
   * Returns plugin library file path.
   * \return Absolute path, set in #plugin_path.
   */
  std::filesystem::path
  getPluginPath();

  /*!
   * Returns smart pointer to MLBookProc object, set in internal #bases object.
   * \return Smart pointer to MLBookProc object.
   */
  std::shared_ptr<MLBookProc>
  getMLBookProc();

protected:
  /*!
   * Bases object.
   *
   * \warning Do not try to set or modify this object.
   */
  Bases bases;

  /*!
   * Path to plugin library.
   *
   * \warning Do not try to set or modify this object.
   */
  std::filesystem::path plugin_path;

  /*!
   * Plugin name. You can set or modify this object on your need.
   */
  QString plugin_name;

  /*!
   * Plugin description. You can set or modify this object on your need.
   */
  QString plugin_description;
};

#endif // MLPLUGIN_H
