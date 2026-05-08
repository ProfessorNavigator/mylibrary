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
#ifndef BOOKDETAILSWINDOW_H
#define BOOKDETAILSWINDOW_H

#include <BaseID.h>
#include <CoverWidget.h>
#include <FormatAnnotation.h>
#include <GenreBase.h>
#include <QLayout>
#include <QPaintEvent>
#include <QShowEvent>
#include <QWidget>
#include <SettingsManager.h>
#include <UDBase.h>
#include <memory>

class BookDetailsWindow : public QWidget
{
  Q_OBJECT
public:
  BookDetailsWindow(QWidget *parent, const UDBase &info,
                    const UDBElement &book_search_result,
                    const std::shared_ptr<FormatAnnotation> &format_annotation,
                    const std::shared_ptr<GenreBase> &genre_base,
                    const std::shared_ptr<SettingsManager> &settings);

  void
  createWindow();

private:
  void
  showEvent(QShowEvent *event) override;

  QLayout *
  bookSection();

  void
  bookTitle(QLayout *layout, const std::vector<UDBElement> &book, int &row,
            const BaseID::ID &search);

  void
  bookAuthor(QLayout *layout, const std::vector<UDBElement> &book, int &row,
             const BaseID::ID &search);

  void
  bookSequence(QLayout *layout, const std::vector<UDBElement> &book, int &row,
               const BaseID::ID &search);

  void
  bookGenre(QLayout *layout, const std::vector<UDBElement> &book, int &row,
            const BaseID::ID &search);

  void
  bookDate(QLayout *layout, const std::vector<UDBElement> &book, int &row,
           const BaseID::ID &search);

  void
  addField(QLayout *layout, int &row, const BaseID::ID &search,
           const QString &name);

  void
  bookTranslator(QLayout *layout, int &row, const BaseID::ID &search);

  QLayout *
  srcBookSection();

  QLayout *
  ebookSection();

  void
  ebookHistory(QLayout *layout, int &row);

  void
  ebookPublisher(QLayout *layout, int &row);

  QLayout *
  paperSection();

  QLayout *
  customInfoSection();

  QLayout *
  fileInfoSection();

  void
  paintEvent(QPaintEvent *event) override;

  UDBase info;
  UDBElement book_search_result;
  std::shared_ptr<FormatAnnotation> format_annotation;
  std::shared_ptr<GenreBase> genre_base;
  std::shared_ptr<SettingsManager> settings;

  UDBElement cover_el;
  CoverWidget *cover = nullptr;

  BaseID bid;
};

#endif // BOOKDETAILSWINDOW_H
