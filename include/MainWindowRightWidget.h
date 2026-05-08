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
#ifndef MAINWINDOWRIGHTWIDGET_H
#define MAINWINDOWRIGHTWIDGET_H

#include <BaseID.h>
#include <Bases.h>
#include <BookInfo.h>
#include <CoverWidget.h>
#include <FormatAnnotation.h>
#include <OpenBook.h>
#include <QComboBox>
#include <QLineEdit>
#include <QMetaObject>
#include <QPaintEvent>
#include <QPushButton>
#include <QTableView>
#include <QTextBrowser>
#include <QToolButton>
#include <QWidget>
#include <ReplaceTagItem.h>
#include <SettingsManager.h>
#include <TableView.h>
#include <UDBase.h>
#include <condition_variable>
#include <memory>

class MainWindowRightWidget : public QWidget
{
  Q_OBJECT
public:
  MainWindowRightWidget(QWidget *parent, const Bases &bases,
                        const std::shared_ptr<SettingsManager> &settings);

  virtual ~MainWindowRightWidget();

  void
  setBookSearchResult(const UDBase &search_result, QWidget *spw,
                      const std::filesystem::path &collection_base_path);

  void
  setFilesSearchResult(const UDBase &search_result, QWidget *spw,
                       const std::filesystem::path &collection_base_path);

  void
  setAuthorsSearchResult(const UDBase &search_result, QWidget *spw,
                         const std::filesystem::path &collection_base_path);

  void
  clearSearchResult();

signals:
  void
  signalEditBook(const UDBElement &book_search_result);

  void
  signalSearchAuthorsBooks(const UDBElement &author_search_result,
                           const bool &separate_window);

  void
  signalShowBooksWindow(const UDBase &search_result, QWidget *spw,
                        const std::filesystem::path &collection_base_path);

private:
  void
  createWidget();

  void
  resizeColumns();

  void
  setFiltersBook();

  void
  setFiltersFiles();

  void
  setFiltersAuthors();

  void
  setFilter();

  void
  removeFilter();

  QList<QAction *>
  bookActions(const Qt::ItemFlags &editable);

  QList<QAction *>
  fileActions();

  QList<QAction *>
  authorActions();

  void
  getBookInfo(const QModelIndex &index);

  void
  formTagReplacementTable(
      std::vector<ReplaceTagItem> &replacement_table,
      std::vector<std::tuple<std::string, std::string>> &symbols_replacement);

  void
  openBookCallback(const std::filesystem::path &p);

  void
  bookSaveDialog();

  void
  saveBookFunc(const QString &save_path, const UDBElement &bsr);

  void
  addBookToBookMarks(const UDBElement &book_search_result);

  void
  showBooks(const bool &separate_window);

  void
  paintEvent(QPaintEvent *event) override;

  Bases bases;
  std::shared_ptr<SettingsManager> settings;

  TableView *search_view;

  int search_view_width = 0;

  QPushButton *operations;

  QComboBox *filter_type;

  QLineEdit *filter_string;

  enum SearchResultType
  {
    Books,
    Files,
    Authors,
    None
  };

  SearchResultType current_type = SearchResultType::None;

  QTextBrowser *annotation;

  BookInfo *book_info;

  std::shared_ptr<FormatAnnotation> format_annotation;

  CoverWidget *cover;

  OpenBook *open_book = nullptr;

  std::filesystem::path book_open_dir;

  QMetaObject::Connection search_result_doubleclicked;
  QMetaObject::Connection search_result_singleclicked;
  QMetaObject::Connection search_result_resize;

  BaseID bid;

  std::filesystem::path collection_base_path;

  int thr_num = 0;
  std::mutex thr_num_mtx;
  std::condition_variable thr_num_var;

signals:
  void
  signalShowBooks(const UDBase &search_result, QWidget *spw,
                  const std::filesystem::path &collection_base_path);  
};

#endif // MAINWINDOWRIGHTWIDGET_H
