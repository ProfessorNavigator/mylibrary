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
#ifndef BASEKEEPER_H
#define BASEKEEPER_H

#include <BaseID.h>
#include <MLBookProc.h>
#include <NotesKeeper.h>
#include <UDBase.h>
#include <filesystem>
#include <functional>
#include <shared_mutex>

/*!
 * \brief The BaseKeeper class
 *
 * This class keeps collection database and contains various methods to work
 * with this database.
 */
class BaseKeeper : public UDBase
{
public:
  /*!
   * \brief BaseKeeper constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   */
  BaseKeeper(const std::shared_ptr<MLBookProc> &mlbp);

  virtual ~BaseKeeper();

  /*!
   * Loads collection database to memory.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param base_path Path to collection database file.
   */
  void
  loadCollection(const std::filesystem::path &base_path);

  /*!
   * Returns collection books quantity.
   *
   * \return Books quantity.
   */
  size_t
  getBooksQuantity();

  /*!
   * Returns collection files quantity.
   *
   * \return Files quantity.
   */
  size_t
  getFilesQuantity();

  /*!
   * Searches books in loaded base.
   *
   * `requests` vector can contain objects of following types: BaseID::Author,
   * BaseID::BookTitle, BaseID::Sequence, BaseID::Genre.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param requests Search parameters.
   * \param coef_coincedence Coefficient of coincidence. Should be more than
   * \a 0 and less than or equal to \a 1.0.
   * \return UDBase object, containing search results (BaseID::BookSearchResult
   * type) and BaseID::CollectionInfo object.
   */
  UDBase
  searchBook(const std::vector<UDBElement> &requests,
             const double &coef_coincedence);

  /*!
   * Interrupts all search operations.
   */
  void
  cancelSearch();

  /*!
   * Returns loaded collection type.
   *
   * \note This method can throw std::exception in case of errors.
   * \return UTF-8 collection type string. Possible values: 'native', 'inpx',
   * 'legacy'.
   */
  std::string
  getCollectionType();

  /*!
   * Returns books directory. Valid only for 'inpx' and 'legacy' collections.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \return Path to collection books directory.
   */
  std::filesystem::path
  getBooksDirectory();

  /*!
   * Edits book entry in database (book file or book file path will not be
   * modified).
   *
   * \param book_search_result UDBElement of BaseID::BookSearchResult type with
   * edited values.
   */
  void
  editBookEntry(const UDBElement &book_search_result);

  /*!
   * Searches books with notes in loaded collection database.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param notes Smart pointer to NotesKeeper object.
   * \return UDBase object containing BaseID::BookSearchResult objects and
   * BaseID::CollectionInfo object.
   */
  UDBase
  searchBooksWithNotes(const std::shared_ptr<NotesKeeper> &notes);

  /*!
   * Returns all collection files.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \return UDBase object containing found BaseID::File objects.
   */
  UDBase
  getAllFiles();

  /*!
   * Collects all loaded collection authors.
   *
   * \return UDBase object containing BaseID::AuthorSearchResult objects.
   */
  UDBase
  listAllAuthors();

  /*!
   * Exports loaded collection database to file. This method should be used if
   * you want share your collection database.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param result_path Absolute path to file of result.
   */
  void
  exportBase(const std::filesystem::path &result_path);

  /*!
   * Returns loaded collection database file path.
   * \return Absolute path to loaded collection database file.
   */
  std::filesystem::path
  getCurrentCollectionBasePath();

  /*!
   * This method can be used to obtain anchor base path from file, obtained as
   * a result of exportBase() method.
   * \param base_path Path to base file.
   * \return UDBElement of BaseID::AnchorBasePath or BaseID::Error types.
   */
  static UDBElement
  getBaseAnchorFileName(const std::filesystem::path &base_path);

  /*!
   * Imports collection database. Valid only for databases obtained by
   * exportBase() method.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param source_base_path Path to base which should be imported.
   * \param coll_base_path Path to base which should be created as a result.
   * \param anchor_file Absolute path to anchor file, obtained by
   * getBaseAnchorFileName() method.
   */
  static void
  importCollectionBase(const std::filesystem::path &source_base_path,
                       const std::filesystem::path &coll_base_path,
                       const std::filesystem::path &anchor_file);

private:
  void
  loadCollectionLegacy(const std::vector<char> &buf);

  void
  parseFileEntryLegacy(const std::string &buf, const std::string &books_path);

  void
  parseBookEntryLegacy(const std::string &buf,
                       std::vector<UDBElement> &result);

  UDBase
  searchElement(std::function<void(const UDBElement &, UDBase &)>
                    search_function) override;

  void
  searchFunc(const UDBElement &el, UDBase *result,
             const std::vector<std::tuple<
                 UDBElement, std::function<bool(const UDBElement &book_el,
                                                const UDBElement &to_search)>>>
                 &l_request);

  bool
  authorSearch(const UDBElement &book_el, const UDBElement &to_search,
               const double &coef_coincidence);

  bool
  bookSearch(const UDBElement &book_el, const UDBElement &to_search,
             const double &coef_coincidence);

  bool
  sequenceSearch(const UDBElement &book_el, const UDBElement &to_search,
                 const std::string &name, const std::string &number,
                 const std::string &content, const double &coef_coincidence);

  bool
  genreSearch(const UDBElement &book_el, const UDBElement &to_search,
              const double &coef_coincidence);

  std::vector<UDBElement *>
  searchFile(const std::vector<UDBElement> &src, const std::string &file_path);

  enum Normalization
  {
    Author,
    Other
  };

  bool
  searchLineFunc(const std::string &to_search, const std::string &source,
                 const double &coef_coincidence,
                 const Normalization &variant = Normalization::Other);

  void
  normalizeString(std::string &str, const Normalization &variant);

  size_t
  booksQuantity(const std::vector<UDBElement> &items);

  size_t
  filesQuantity(const std::vector<UDBElement> &items);

  UDBase
  searchInNotes(const std::vector<UDBElement> &src,
                const std::shared_ptr<NotesKeeper> &notes);

  UDBase
  getFiles(const std::vector<UDBElement> &src);

  void
  listAuthors(const std::vector<UDBElement> &src,
              std::vector<UDBElement> *result);

  void
  setRelativePath(std::vector<UDBElement> &src,
                  const std::filesystem::path &base_path);

  static void
  setAbsolutePath(std::vector<UDBElement> &src,
                  const std::filesystem::path &base_path);

  std::shared_ptr<MLBookProc> mlbp;

  bool cancel_search = false;

  std::filesystem::path current_base_path;

  BaseID bid;

  std::shared_mutex base_mtx;
};

#endif // BASEKEEPER_H
