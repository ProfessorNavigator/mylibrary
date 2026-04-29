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
#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

int
main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for(const QString &locale : uiLanguages)
    {
      const QString baseName = "MyLibrary_" + QLocale(locale).name();
      if(translator.load(":/i18n/" + baseName))
        {
          a.installTranslator(&translator);
          break;
        }
    }

  QIcon icon(":/icons/mylibrary.svg");
  a.setWindowIcon(icon);

  MainWindow w;
  w.show();
  return a.exec();
}
