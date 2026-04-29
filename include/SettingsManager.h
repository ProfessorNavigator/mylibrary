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
#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QColor>
#include <QFont>
#include <QString>
#include <UDBase.h>
#include <filesystem>

class SettingsManager
{
public:
  SettingsManager();

  SettingsManager(const SettingsManager &other);

  QString
  getStyleAttributeValue(const std::string &id,
                         const std::string &attribute_name);

  void
  setStyleAttributeValue(const std::string &id,
                         const std::string &attribute_name,
                         const std::string &value);

  void
  replaceStyleAttribute(const std::string &id,
                        const std::string &attribute_name,
                        const std::string &new_attribute_name,
                        const std::string &value);

  void
  addStyleAttribute(const std::string &id, const std::string &attribute_name,
                    const std::string &value);

  void
  removeStyleAttribute(const std::string &id,
                       const std::string &attribute_name);

  QColor
  stringToColor(const std::string &str);

  void
  applySettings();

  void
  resetToDefault();

  void
  resetColorsToSystemDefault();

  QFont
  getFont();

  void
  setFont(const QFont &font);

private:
  void
  loadSettings();

  void
  createStylesDatabase(const std::string &buf);

  std::vector<UDBElement>
  parseStyle(const std::string &buf);

  void
  parseStyleItem(const std::string &buf, UDBElement &result);

  void
  normalizeString(std::string &str);

  std::string
  getAttributeName(const std::string &val);

  UDBase styles;

  std::filesystem::path styles_path;

  QFont app_font;
  std::filesystem::path font_path;

  QFont default_font;
};

#endif // SETTINGSMANAGER_H
