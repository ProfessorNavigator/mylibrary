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

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QWidget>
#include <SettingsManager.h>
#include <algorithm>
#include <iostream>
#include <sstream>

SettingsManager::SettingsManager()
{
  loadSettings();
}

SettingsManager::SettingsManager(const SettingsManager &other)
{
  styles = other.styles;
  styles_path = other.styles_path;
  app_font = other.app_font;
  font_path = other.font_path;
  default_font = other.default_font;
}

QString
SettingsManager::getStyleAttributeValue(const std::string &id,
                                        const std::string &attribute_name)
{
  QString result;

  std::vector<UDBElement> *raw_base = styles.getRawBase();
  auto it = std::find_if(raw_base->begin(), raw_base->end(),
                         [id](const UDBElement &el)
                           {
                             return el.id == id;
                           });
  if(it == raw_base->end())
    {
      return result;
    }

  auto it_attr = std::find_if(it->subelements.begin(), it->subelements.end(),
                              [attribute_name](const UDBElement &el)
                                {
                                  return el.id == attribute_name;
                                });
  if(it_attr == it->subelements.end())
    {
      return result;
    }

  result = it_attr->content.c_str();

  return result;
}

void
SettingsManager::setStyleAttributeValue(const std::string &id,
                                        const std::string &attribute_name,
                                        const std::string &value)
{
  std::vector<UDBElement> *raw_base = styles.getRawBase();
  auto it = std::find_if(raw_base->begin(), raw_base->end(),
                         [id](const UDBElement &el)
                           {
                             return el.id == id;
                           });
  if(it == raw_base->end())
    {
      return void();
    }

  auto it_attr = std::find_if(it->subelements.begin(), it->subelements.end(),
                              [attribute_name](const UDBElement &el)
                                {
                                  return el.id == attribute_name;
                                });
  if(it_attr != it->subelements.end())
    {
      it_attr->content = value;
    }
}

void
SettingsManager::replaceStyleAttribute(const std::string &id,
                                       const std::string &attribute_name,
                                       const std::string &new_attribute_name,
                                       const std::string &value)
{
  std::vector<UDBElement> *raw_base = styles.getRawBase();
  auto it = std::find_if(raw_base->begin(), raw_base->end(),
                         [id](const UDBElement &el)
                           {
                             return el.id == id;
                           });
  if(it == raw_base->end())
    {
      return void();
    }

  auto it_attr = std::find_if(it->subelements.begin(), it->subelements.end(),
                              [attribute_name](const UDBElement &el)
                                {
                                  return el.id == attribute_name;
                                });
  if(it_attr != it->subelements.end())
    {
      it_attr->id = new_attribute_name;
      it_attr->content = value;
    }
}

void
SettingsManager::addStyleAttribute(const std::string &id,
                                   const std::string &attribute_name,
                                   const std::string &value)
{
  std::vector<UDBElement> *raw_base = styles.getRawBase();
  auto it = std::find_if(raw_base->begin(), raw_base->end(),
                         [id](const UDBElement &el)
                           {
                             return el.id == id;
                           });
  if(it == raw_base->end())
    {
      return void();
    }

  auto it_attr = std::find_if(it->subelements.begin(), it->subelements.end(),
                              [attribute_name](const UDBElement &el)
                                {
                                  return el.id == attribute_name;
                                });
  if(it_attr == it->subelements.end())
    {
      UDBElement el;
      el.id = attribute_name;
      el.content = value;
      it->subelements.emplace_back(el);
    }
  else
    {
      it_attr->content = value;
    }
}

void
SettingsManager::removeStyleAttribute(const std::string &id,
                                      const std::string &attribute_name)
{
  std::vector<UDBElement> *raw_base = styles.getRawBase();
  auto it = std::find_if(raw_base->begin(), raw_base->end(),
                         [id](const UDBElement &el)
                           {
                             return el.id == id;
                           });
  if(it == raw_base->end())
    {
      return void();
    }

  it->subelements.erase(std::remove_if(it->subelements.begin(),
                                       it->subelements.end(),
                                       [attribute_name](const UDBElement &el)
                                         {
                                           return el.id == attribute_name;
                                         }),
                        it->subelements.end());
}

QColor
SettingsManager::stringToColor(const std::string &str)
{
  QColor result;

  std::string find_str("rgba");
  std::string::size_type n = str.find(find_str);
  if(n == std::string::npos)
    {
      find_str = "rgb";
      n = str.find(find_str);
      if(n == std::string::npos)
        {
          std::cout << "SettingsManager::stringToColor: not a color"
                    << std::endl;
          return result;
        }
    }

  std::string l_str(str.begin() + n + find_str.size(), str.end());

  find_str = "(";
  n = l_str.find(find_str);
  if(n == std::string::npos)
    {
      std::cout << "SettingsManager::stringToColor: incorrect color format"
                << std::endl;
      return result;
    }

  l_str.erase(0, n + find_str.size());

  find_str = ",";
  int count = 0;
  while(count < 4 && l_str.size() > 0)
    {
      n = l_str.find(find_str);
      if(n == std::string::npos)
        {
          find_str = ")";
          n = l_str.find(find_str);
          if(n == std::string::npos)
            {
              break;
            }
        }
      std::string num_str(l_str.begin(), l_str.begin() + n);
      normalizeString(num_str);
      l_str.erase(0, n + find_str.size());

      std::stringstream strm;
      strm.imbue(std::locale("C"));
      strm.str(num_str);
      int color;
      strm >> color;
      switch(count)
        {
        case 0:
          {
            result.setRed(color);
            break;
          }
        case 1:
          {
            result.setGreen(color);
            break;
          }
        case 2:
          {
            result.setBlue(color);
            break;
          }
        case 3:
          {
            result.setAlpha(color);
            break;
          }
        default:
          break;
        }
      count++;
    }

  return result;
}

void
SettingsManager::applySettings()
{
  QString buf;

  std::vector<UDBElement> *raw_base = styles.getRawBase();
  for(auto it = raw_base->begin(); it != raw_base->end(); it++)
    {
      QString l_buf("#");
      l_buf += it->id.c_str();
      l_buf += " {\n";
      for(auto it_attr = it->subelements.begin();
          it_attr != it->subelements.end(); it_attr++)
        {
          l_buf += "\t";
          l_buf += it_attr->id.c_str();
          l_buf += ": ";
          l_buf += it_attr->content.c_str();
          l_buf += ";\n";
        }
      l_buf += "}";

      if(!buf.isEmpty())
        {
          buf += "\n\n";
        }
      buf += l_buf;
    }
  buf += "\n";

  qApp->setStyleSheet(buf); 

  std::filesystem::create_directories(styles_path.parent_path());
  std::filesystem::remove_all(styles_path);

  std::unique_ptr<QFile> f(new QFile(styles_path));
  if(f->open(QIODeviceBase::WriteOnly | QIODeviceBase::Text))
    {
      f->write(buf.toUtf8());
      f->close();
    }

  qApp->setFont(app_font);

  QList<QWidget *> w_list = qApp->allWidgets();
  for(qsizetype i = 0; i < w_list.size(); i++)
    {
      QFont font = w_list[i]->font();
      font.setFamily(app_font.family());
      font.setPointSize(app_font.pointSize());
      w_list[i]->setFont(font);
    }

  std::filesystem::create_directories(font_path.parent_path());
  std::filesystem::remove_all(font_path);

  f = std::unique_ptr<QFile>(new QFile(font_path));
  if(f->open(QIODeviceBase::WriteOnly))
    {
      QByteArray arr = app_font.toString().toUtf8();
      f->write(arr);
      f->close();
    }
}

void
SettingsManager::resetToDefault()
{
  std::filesystem::remove_all(styles_path);
  std::filesystem::remove_all(font_path);
  styles.clearBase();
  qApp->setFont(default_font);
  loadSettings();
}

void
SettingsManager::resetColorsToSystemDefault()
{
  QString str;
  std::filesystem::remove_all(styles_path);
  std::filesystem::create_directories(styles_path.parent_path());
  QFile f(styles_path);
  if(f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      f.write(str.toUtf8());
      f.close();
    }
  styles.clearBase();
  qApp->setStyleSheet("");
}

QFont
SettingsManager::getFont()
{
  return app_font;
}

void
SettingsManager::setFont(const QFont &font)
{
  app_font = font;
}

void
SettingsManager::loadSettings()
{
  std::string str = QDir::homePath().toStdString();
  styles_path = std::u8string(str.begin(), str.end());
  styles_path /= std::filesystem::path(u8".config");
  styles_path /= std::filesystem::path(u8"MyLibrary");
  styles_path /= std::filesystem::path(u8"styles.css");

  std::unique_ptr<QFile> f(new QFile(styles_path));
  if(f->open(QIODeviceBase::ReadOnly | QIODeviceBase::Text))
    {
      QByteArray arr = f->readAll();
      f->close();
      QString buf(arr);

      qApp->setStyleSheet(buf);

      createStylesDatabase(buf.toStdString());
    }
  else
    {
      f = std::unique_ptr<QFile>(new QFile(":/styles/styles.css"));
      if(f->open(QIODeviceBase::ReadOnly | QIODeviceBase::Text))
        {
          QByteArray arr = f->readAll();
          f->close();
          QString buf(arr);
          qApp->setStyleSheet(buf);

          createStylesDatabase(buf.toStdString());
        }
    }

  std::filesystem::path def_font = std::u8string(str.begin(), str.end());
  def_font /= std::filesystem::path(u8".config");
  def_font /= std::filesystem::path(u8"MyLibrary");
  def_font /= std::filesystem::path(u8"default_font");
  if(std::filesystem::exists(def_font))
    {
      f = std::unique_ptr<QFile>(new QFile(def_font));
      if(f->open(QIODeviceBase::ReadOnly))
        {
          QByteArray arr = f->readAll();
          f->close();
          QString buf(arr);
          default_font.fromString(buf);
        }
      else
        {
          default_font = qApp->font();
        }
    }
  else
    {
      default_font = qApp->font();
      std::filesystem::create_directories(def_font.parent_path());
      QString qstr = default_font.toString();
      f = std::unique_ptr<QFile>(new QFile(def_font));
      if(f->open(QIODeviceBase::WriteOnly))
        {
          f->write(qstr.toUtf8());
          f->close();
        }
    }

  font_path = std::u8string(str.begin(), str.end());
  font_path /= std::filesystem::path(u8".config");
  font_path /= std::filesystem::path(u8"MyLibrary");
  font_path /= std::filesystem::path(u8"font_settings");
  f = std::unique_ptr<QFile>(new QFile(font_path));
  if(f->open(QIODeviceBase::ReadOnly))
    {
      QByteArray arr = f->readAll();
      f->close();
      QString qstr(arr);
      app_font.fromString(qstr);
    }
  else
    {
      app_font = qApp->font();
    }

  qApp->setFont(app_font);
}

void
SettingsManager::createStylesDatabase(const std::string &buf)
{
  std::string style_scope_start("{");
  std::string style_scope_end("}");
  std::string double_space("  ");
  std::string::size_type n = 0;
  while(n < buf.size())
    {
      std::string::size_type n2 = buf.find(style_scope_start, n);
      if(n2 == std::string::npos)
        {
          break;
        }
      std::string str(buf.begin() + n, buf.begin() + n2);
      n = n2 + style_scope_start.size();

      while(str.size() > 0)
        {
          char ch = *str.rbegin();
          if(ch >= 0 && ch <= ' ')
            {
              str.pop_back();
            }
          else
            {
              break;
            }
        }
      for(auto it = str.begin(); it != str.end();)
        {
          if(*it >= 0 && *it <= ' ')
            {
              str.erase(it);
            }
          else if(*it == '#')
            {
              str.erase(it);
            }
          else
            {
              break;
            }
        }

      std::string::size_type n_dub = 0;
      while(n_dub < str.size())
        {
          n_dub = str.find(double_space);
          if(n_dub != std::string::npos)
            {
              str.erase(str.begin() + n_dub);
            }
        }

      if(str.empty())
        {
          break;
        }
      UDBElement style;
      style.id = str;

      n2 = buf.find(style_scope_end, n);
      if(n2 == std::string::npos)
        {
          break;
        }

      str = std::string(buf.begin() + n, buf.begin() + n2);
      n = n2 + style_scope_end.size();

      style.subelements = parseStyle(str);
      if(style.subelements.size() > 0)
        {
          styles.addElement(style);
        }
    }
  styles.shrinkToFit();
}

std::vector<UDBElement>
SettingsManager::parseStyle(const std::string &buf)
{
  std::vector<UDBElement> result;

  std::string item_end(";");
  std::string::size_type n = 0;
  while(n < buf.size())
    {
      std::string::size_type n2 = buf.find(item_end, n);
      if(n2 == std::string::npos)
        {
          break;
        }
      std::string str(buf.begin() + n, buf.begin() + n2);
      n = n2 + item_end.size();

      UDBElement el;
      parseStyleItem(str, el);

      if(!el.id.empty() && !el.content.empty())
        {
          result.emplace_back(el);
        }
    }

  return result;
}

void
SettingsManager::parseStyleItem(const std::string &buf, UDBElement &result)
{
  std::string sep(":");
  std::string::size_type n = buf.find(sep);
  if(n == std::string::npos)
    {
      return void();
    }
  result.id = std::string(buf.begin(), buf.begin() + n);
  n += sep.size();

  normalizeString(result.id);

  if(n >= buf.size())
    {
      return void();
    }

  result.content = std::string(buf.begin() + n, buf.end());

  normalizeString(result.content);
}

void
SettingsManager::normalizeString(std::string &str)
{
  while(str.size() > 0)
    {
      char ch = *str.rbegin();
      if(ch >= 0 && ch <= ' ')
        {
          str.pop_back();
        }
      else
        {
          break;
        }
    }

  for(auto it = str.begin(); it != str.end();)
    {
      char ch = *it;
      if(ch >= 0 && ch <= ' ')
        {
          str.erase(it);
        }
      else
        {
          break;
        }
    }
}

std::string
SettingsManager::getAttributeName(const std::string &val)
{
  std::string result;
  std::string find_str = ":";

  std::string::size_type n = val.find(find_str);
  if(n == std::string::npos)
    {
      return result;
    }

  result = val.substr(0, n);

  while(result.size() > 0)
    {
      char ch = *result.rbegin();
      if(ch >= 0 && ch <= ' ')
        {
          result.pop_back();
        }
      else
        {
          break;
        }
    }

  for(auto it = result.begin(); it != result.end();)
    {
      char ch = *it;
      if(ch >= 0 && ch <= ' ')
        {
          result.erase(it);
        }
      else
        {
          break;
        }
    }

  return result;
}
