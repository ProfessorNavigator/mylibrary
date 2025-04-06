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

#ifndef REFRESHCOLLECTION_H
#define REFRESHCOLLECTION_H

#include <AuxFunc.h>
#include <BaseKeeper.h>
#include <BookBaseEntry.h>
#include <BookMarks.h>
#include <CreateCollection.h>
#include <FileParseEntry.h>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef USE_OPENMP
#include <atomic>
#include <mutex>
#endif
#ifdef USE_OPENMP
#include <omp.h>
#endif

/*!
 * \brief The RefreshCollection class
 *
 * This class contains various methods for collection database refreshing in
 * case of any changes were made to collection files.
 */
class RefreshCollection : public CreateCollection
{
public:
  /*!
   * \brief RefreshCollection constructor.
   *
   * \note See also set_rar_support() method.
   * \param af smart pointer to AuxFunc object.
   * \param collection_name collection name.
   * \param num_threads number of threads to be used (see also
   * CreateCollection::CreateCollection()).
   * \param remove_empty if \a true, empty directories and files will be
   * removed.
   * \param fast_refresh if \a true, file hashes calculations will not be
   * carried out (hashes of all collection files will be recalculated
   * otherwise).
   * \param refresh_bookmarks if \a true, bookmarks pointing to absent books
   * will be removed.
   * \param bookmarks smart pointer to BookMarks object.
   */
  RefreshCollection(const std::shared_ptr<AuxFunc> &af,
                    const std::string &collection_name, const int &num_threads,
                    const bool &remove_empty, const bool &refresh_bookmarks,
                    const bool &fast_refresh,
                    const std::shared_ptr<BookMarks> &bookmarks);

  /*!
   * \brief RefreshCollection destructor.
   */
  virtual ~RefreshCollection();

  /*!
   * \brief "Total bytes to hash" signal.
   *
   * Emitted after files for refreshing have been collected, to indicate
   * total quantity bytes to be hashed. Bind your method to \b
   * total_bytes_to_hash, if you need such information.
   */
  std::function<void(const double &total_hash)> total_bytes_to_hash;

  /*!
   * \brief "Total bytes hashed" signal.
   *
   * Emitted after file has been hashed, to indicate total quantity of
   * bytes have been hashed. Bind your method to \b bytes_hashed, if you need
   * such information.
   */
  std::function<void(const double &hashed)> bytes_hashed;

  /*!
   * \brief Refreshes whole collection.
   *
   * Caries out collection refreshing.
   * \note This method can throw MLException in case of errors.
   */
  void
  refreshCollection();

  /*!
   * \brief Refreshes iformation about particular file.
   * \param bbe BookBaseEntry object (see BaseKeeper::searchBook()).
   */
  void
  refreshFile(const BookBaseEntry &bbe);

  /*!
   * \brief Replaces information in database.
   *
   * Use this method, if you need to edit database entries manually.
   * \param bbe_old existing BookBaseEntry (see BaseKeeper::searchBook()).
   * \param bbe_new BookBaseEntry containing new information.
   * \return Returns \a true, if operation has been successful.
   */
  bool
  editBook(const BookBaseEntry &bbe_old, const BookBaseEntry &bbe_new);

  /*!
   * \brief Refreshes information in database about particular book.
   * \param bbe BookBaseEntry object (see BaseKeeper::searchBook()).
   * \return Returns \a true, if operation has been successful.
   */
  bool
  refreshBook(const BookBaseEntry &bbe);

  /*!
   * \brief Enables support of rar archives.
   *
   * Set \b rar_support to \a true, if you need to parse rar archives (see also
   * CreateCollection::CreateCollection()).
   * \param rar_support if \a true, rar archives will be parsed.
   */
  void
  set_rar_support(const bool &rar_support);

private:
  std::filesystem::path
  get_base_path(const std::string &collection_name);

  std::filesystem::path
  get_books_path();

  void
  compaire_vectors(std::vector<FileParseEntry> &base,
                   std::vector<std::filesystem::path> &books_files);

  bool
  compare_function1(const std::filesystem::path &book_path,
                    const FileParseEntry &ent);

  bool
  compare_function2(const FileParseEntry &ent,
                    const std::filesystem::path &book_path);

  void
  check_hashes(std::vector<FileParseEntry> *base,
               std::vector<std::filesystem::path> *books_files);

  void
  hash_thread(const std::filesystem::path &file_to_hash,
              std::vector<FileParseEntry> *base);

  void
  refreshBookMarks(const std::shared_ptr<BaseKeeper> &bk);

  std::shared_ptr<AuxFunc> af;
  int num_threads = 1;
  bool remove_empty = false;
  bool fast_refresh = false;
  bool refresh_bookmarks = false;
  std::shared_ptr<BookMarks> bookmarks;

  std::string collection_name;

#ifndef USE_OPENMP
  std::atomic<uintmax_t> bytes_summ;

  std::mutex basemtx;
  std::mutex already_hashedmtx;
  std::mutex need_to_parsemtx;

  std::mutex newthrmtx;
  std::condition_variable continue_hashing;
  int run_threads = 0;
#endif

#ifdef USE_OPENMP
  uintmax_t bytes_summ = 0;
  omp_lock_t basemtx;
  omp_lock_t already_hashedmtx;
  omp_lock_t need_to_parsemtx;
#endif
};

#endif // REFRESHCOLLECTION_H
