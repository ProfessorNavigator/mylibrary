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

#ifndef BASEKEEPER_H
#define BASEKEEPER_H

#include <AuxFunc.h>
#include <BookBaseEntry.h>
#include <FileParseEntry.h>
#include <NotesBaseEntry.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <atomic>
#endif

/*!
 * \brief The BaseKeeper class.
 *
 * This class is intended to keep and operate collections databases.
 * loadCollection() method should be called first.
 */
class BaseKeeper
{
public:
  /*!
   * \brief BaseKeeper constructor.
   * \param af smart pointer to AuxFunc object.
   */
  BaseKeeper(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief BaseKeeper destructor.
   */
  virtual ~BaseKeeper();

  /*!
   * \brief Loads collection database to memory.
   *
   * \note This method can throw MLException in case of errors.
   *
   * \param col_name collection name.
   */
  void
  loadCollection(const std::string &col_name);

  /*!
   * \brief Searches book in collection.
   *
   * BookBaseEntry object must be provided as search request. It is necessary
   * to fill in any field in the inner BookParseEntry object to receive
   * particular result. Otherwise complete collection book list will be
   * returned.
   *
   * \param search BookBaseEntry object. If author search is needed
   * bpe.book_author field must be in the following form: "Surname\7First
   * Name\7Second name".
   * \param coef_coincedence Required coefficent of coincedence for search
   * result. Allowed values are from \a 0.0 to \a 1.0. Default value is \a 0.7.
   * If value is greater then 1.0, it means "exact match". In that case this
   * method will return only results with the exact match for all search fields
   * (empty search parameters are ignored).
   * \return Vector of BookBaseEntry objects, containing search results.
   */
  std::vector<BookBaseEntry>
  searchBook(const BookBaseEntry &search,
             const double &coef_coincedence = double(0.7));

  /*!
   * \brief Lists all authors, found in collection.
   * \return Vector containing UTF-8 author's names strings.
   */
  std::vector<std::string>
  collectionAuthors();

  /*!
   * \brief Lists all books of current collection, which have notes.
   * \param notes vector of notes (see NotesKeeper class documentation).
   * \return Vector of books with notes.
   */
  std::vector<BookBaseEntry>
  booksWithNotes(const std::vector<NotesBaseEntry> &notes);

  /*!
   * \brief Stops all search operations.
   */
  void
  stopSearch();

  /*!
   * \brief Unloads collection base from memory.
   */
  void
  clearBase();

  /*!
   * \brief Returns copy of inner database vector.
   * \return Database vector.
   */
  std::vector<FileParseEntry>
  get_base_vector();

  /*!
   * \brief Returns absolute path to directory containing collection books.
   *
   * This method can be called without collection loading to
   * memory.
   * \note This method can throw MLException in case of errors.
   *
   * \param collection_name collection name.
   * \param af smart pointer to AuxFunc object.
   * \return Absolute path to books directory.
   */
  static std::filesystem::path
  get_books_path(const std::string &collection_name,
                 const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief Returns total quantity of books in loaded collection.
   *
   * \note If this method is called during collection loading, it will
   * block calling thread execution until collection is loaded.
   *
   * \return Total books quantity in loaded collection.
   */
  size_t
  getBooksQuantity();

  /*!
   * \brief collectionAuthors() progress callback
   *
   * collectionAuthors() method execution can take some time. This callback
   * indicates progress.
   * \a progr is current progress in conventional units. \a sz is total
   * quantity of conventional units to be processed.
   */
  std::function<void(const double &progr, const double &sz)> auth_show_progr;

private:
  FileParseEntry
  readFileEntry(const std::string &base, size_t &rb);

  std::vector<BookParseEntry>
  readBookEntry(const std::string &entry, size_t &rb);

  void
  parseBookEntry(const std::string &e, std::string &read_val, size_t &rb);

  bool
  searchLineFunc(const std::string &to_search, const std::string &source,
                 const double &coef_coincidence);

  bool
  searchSurname(const BookBaseEntry &search,
                std::vector<BookBaseEntry> &result,
                const double &coef_coincidence);
  bool
  searchFirstName(const BookBaseEntry &search,
                  std::vector<BookBaseEntry> &result,
                  const double &coef_coincidence);

  bool
  searchLastName(const BookBaseEntry &search,
                 std::vector<BookBaseEntry> &result,
                 const double &coef_coincidence);

  void
  searchBook(const BookBaseEntry &search, std::vector<BookBaseEntry> &result,
             const double &coef_coincidence);

  void
  searchSeries(const BookBaseEntry &search, std::vector<BookBaseEntry> &result,
               const double &coef_coincidence);

  void
  searchGenre(const BookBaseEntry &search, std::vector<BookBaseEntry> &result,
              const double &coef_coincidence);

  bool
  exactMatchSearchFunc(const BookBaseEntry &el, const BookBaseEntry &search);

  std::shared_ptr<AuxFunc> af;

  std::vector<FileParseEntry> base;
  std::string collection_name;
  std::filesystem::path collection_path;
  size_t books_in_base = 0;
#ifdef USE_OPENMP
  omp_lock_t basemtx;
  bool cancel_search;
#else
  std::mutex basemtx;
  std::atomic<bool> cancel_search;
#endif
};

#endif // BASEKEEPER_H
