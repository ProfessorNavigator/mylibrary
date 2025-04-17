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

#ifndef CREATECOLLECTION_H
#define CREATECOLLECTION_H

#include <AuxFunc.h>
#include <FileParseEntry.h>
#include <Hasher.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#ifndef USE_OPENMP
#include <condition_variable>
#include <mutex>
#else
#include <omp.h>
#endif

/*!
 * \brief The CreateCollection class.
 *
 * This class containes methods for collection database creation.
 */
class CreateCollection : public Hasher
{
public:
  /*!
   * \brief CreateCollection constructor.
   * \param af smart pointer to AuxFunc object.
   * \param collection_path absolute path to collection base directory (if it
   * doesnt exsist, \b MLBookProc will create it).
   * \param books_path absolute path to books directory.
   * \param rar_support if \a true, rar archives will be processed, otherwise -
   * not (some rar archives can cause errors).
   * \param num_threads limit of working threads to be used.
   */
  CreateCollection(const std::shared_ptr<AuxFunc> &af,
                   const std::filesystem::path &collection_path,
                   const std::filesystem::path &books_path,
                   const bool &rar_support, const int &num_threads);

  /*!
   * \brief CreateCollection destructor.
   */
  virtual ~CreateCollection();

  /*!
   * \brief Starts collection creation.
   *
   * \note This method can throw MLException in case of errors.
   */
  void
  createCollection();

  /*!
   * \brief "Pulse" callback.
   *
   * Emitted while preliminary files collecting to show that process is not
   * frozen. Bind your method to it, if you need such information.
   */
  std::function<void()> pulse;

  /*!
   * \brief "Total bytes" callback.
   *
   * Emitted after preliminary files collecting completed to indicate total
   * quantity of bytes to be processed. Bind your method to it, if you need
   * such information.
   */
  std::function<void(const double &)> signal_total_bytes;

  /*!
   * \brief "Progress" callback.
   *
   * Emitted after each file processing completion. Indicates total quantity
   * of bytes has been processed. Bind your method to it, if you need such
   * information.
   */
  std::function<void(const double &progress)> progress;

protected:
  /*!
   * \brief CreateCollection constructor.
   *
   *\warning Do not call this constructor yourself!
   *
   * This constructor is used by RefreshCollection class.
   *
   * \param af smart pointer to AuxFunc object.
   * \param num_threads limit of working threads to be used.
   */
  CreateCollection(const std::shared_ptr<AuxFunc> &af, const int &num_threads);

  /*!
   * \brief Threads regulator.
   *
   * \warning Do not call this method yourself!
   */
  void
  threadRegulator();

  /*!
   * \brief Opens database file for writing.
   *
   * \warning Do not call this method yourself!
   */
  void
  openBaseFile();

  /*!
   * \brief Finishes database writing.
   *
   * \warning Do not call this method yourself!
   */
  void
  closeBaseFile();

  /*!
   * \brief Writes file data to database.
   *
   * \warning Do not call this method yourself!
   *
   * \param fe FileParseEntry object.
   */
  void
  write_file_to_base(const FileParseEntry &fe);

  /*!
   * \brief Absolute path to database.
   *
   * \warning Do not call or set this variable yourself!
   */
  std::filesystem::path base_path;

  /*!
   * \brief Absolute path to books directory.
   *
   * \warning Do not call or set this variable yourself!
   */
  std::filesystem::path books_path;

  /*!
   * \brief If \a true, rar archives will be processed, otherwise - not.
   *
   * \warning Do not call or set this variable yourself!
   */
  bool rar_support = false;

  /*!
   * \brief Hashed files.
   *
   * This vector is used by RefreshCollection class to indicate files, has been
   * already hashed.
   *
   * \warning Do not call or set this vector yourself!
   */
  std::vector<std::tuple<std::filesystem::path, std::string>> already_hashed;

  /*!
   * \brief "Need to parse" vector.
   *
   * This vector is used to indicate files, needed to be parsed.
   *
   * \warning Do not call or set this vector yourself!
   */
  std::vector<std::filesystem::path> need_to_parse;

#ifndef USE_OPENMP
  /*!
   * \brief Keeps quantity of bytes have been processed.
   *
   * \warning Do not call or set this variable yourself!
   */
  std::atomic<double> current_bytes;
#else
  /*!
   * \brief Keeps quantity of bytes have been processed.
   *
   * \warning Do not call or set this variable yourself!
   */
  double current_bytes = 0.0;
#endif

private:
  void
  threadFunc(const std::filesystem::path &need_to_parse);

  void
  fb2_thread(const std::filesystem::path &file_col_path,
             const std::filesystem::path &resolved);

  void
  epub_thread(const std::filesystem::path &file_col_path,
              const std::filesystem::path &resolved);

  void
  pdf_thread(const std::filesystem::path &file_col_path,
             const std::filesystem::path &resolved);

  void
  djvu_thread(const std::filesystem::path &file_col_path,
              const std::filesystem::path &resolved);

  void
  arch_thread(const std::filesystem::path &file_col_path,
              const std::filesystem::path &resolved);

  void
  book_entry_to_file_entry(std::string &file_entry,
                           const std::string &book_entry);

  std::shared_ptr<AuxFunc> af;
  int num_threads = 1;
  std::fstream base_strm;

  std::vector<void *> archp_obj;

#ifndef USE_OPENMP
  std::mutex archp_obj_mtx;
  int run_threads = 0;
  std::mutex run_threads_mtx;
  std::condition_variable run_threads_var;
  std::mutex base_strm_mtx;
#else
  omp_lock_t archp_obj_mtx;
#endif
};

#endif // CREATECOLLECTION_H
