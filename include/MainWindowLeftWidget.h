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
#ifndef MAINWINDOWLEFTWIDGET_H
#define MAINWINDOWLEFTWIDGET_H

#include <BaseID.h>
#include <Bases.h>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QShowEvent>
#include <QSlider>
#include <QSplitter>
#include <filesystem>
#include <mutex>
#include <thread>

class MainWindowLeftWidget : public QSplitter
{
  Q_OBJECT
public:
  MainWindowLeftWidget(QWidget *parent, const Bases &bases);

  virtual ~MainWindowLeftWidget();

  void
  removeCollection(const QString &col_name);

  void
  addCollection(const std::filesystem::path &base_path);

  void
  reloadCollection(const QString &col_name);

  void
  reloadCurrentCollection();

  void
  editBook(const UDBElement &book_search_result);

  void
  searchAuthorsBooks(const UDBElement &author_search_result,
                     const bool &sep_window); 

signals:
  void
  signalBookSearchFinished(const UDBase &base, QWidget *spw,
                           const std::filesystem::path &collection_base_path);

  void
  signalFilesSearchFinished(const UDBase &base, QWidget *spw,
                            const std::filesystem::path &collection_base_path);

  void
  signalAuthorsSearchFinished(
      const UDBase &base, QWidget *spw,
      const std::filesystem::path &collection_base_path);

  void
  signalBookSearchSepWindow(const UDBase &base, QWidget *spw,
                            const std::filesystem::path &collection_base_path);

  void
  signalClearSearchResult();

private:
  void
  createWidget();

  QWidget *
  collectionSection();

  QWidget *
  searchSection();

  QWidget *
  operationsSection();

  void
  loadCollectionsList();

  void
  setActiveCollection();

  void
  selectCollection(int index);

  QWidget *
  genrePopup();

  void
  clearSearchParameters();

  enum SearchType
  {
    Book,
    BooksWithNotes
  };

  void
  searchBook(const SearchType &type);

  void
  showFiles();

  void
  showAuthors();

  Bases bases;

  QComboBox *collections;

  std::filesystem::path current_collection_base_path;  

  std::shared_ptr<std::thread> base_loading_thr;

  QLineEdit *surname;
  QLineEdit *first_name;
  QLineEdit *second_name;
  QLineEdit *nickname;
  QLineEdit *book_name;
  QLineEdit *series;
  QLineEdit *series_number;
  QSlider *coef_coincidence;
  QPushButton *genre_button;
  std::string selected_genre;

  std::mutex *search_mtx;  

  BaseID bid;

signals:
  void
  signalCollectionLoaded(const QString &books_quantity,
                         const QString &collection_type);
};

#endif // MAINWINDOWLEFTWIDGET_H
