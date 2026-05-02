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
#ifndef CREATECOLLECTION_H
#define CREATECOLLECTION_H

#include <ArchiveParser.h>
#include <BaseID.h>
#include <MLBookProc.h>
#include <UDBase.h>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <mutex>
#include <tuple>
#include <vector>

/*!
 * \brief The CreateCollection class
 *
 * This class contains methods for collection database creation.
 */
class CreateCollection
{
public:
  /*!
   * \brief CreateCollection constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   * \param threads_num Maximum permitted number of working threads.
   */
  CreateCollection(const std::shared_ptr<MLBookProc> &mlbp,
                   const int &threads_num = int(1));

  virtual ~CreateCollection();

  /*!
   * Creates collection database.
   *
   * \param files_and_dirs Files, directories and symlinks to be included in
   * collection.
   * \param base_path Path to result database file.
   */
  void
  createCollection(const std::vector<std::filesystem::path> &files_and_dirs,
                   const std::filesystem::path &base_path);

  /*!
   * Creates database from inpx file.
   *
   * \param path_to_inpx Path to inpx file.
   * \param base_path Path to result database file.
   */
  void
  createInpxCollection(const std::filesystem::path &path_to_inpx,
                       const std::filesystem::path &base_path);

  /*!
   * Stops all operations.
   */
  virtual void
  stopAll();

  /*!
   * Writes given base to file.
   *
   * \param base_path Path to database file.
   * \param col_base Collection database.
   */
  static void
  saveBase(const std::filesystem::path &base_path, UDBase &col_base);

  /*!
   * This callback function will be called during files collecting to indicate
   * progress if set. \a files_found - total number of found files.
   */
  std::function<void(const size_t &files_found)> signal_files_collecting;

  /*!
   * This callback function will be called during files parsing to indicate
   * progress, if set. \a processed - number of processed items, \a total -
   * total number of items to be processed.
   */
  std::function<void(double processed, double total)> signal_parsing_progress;

protected:
  /*!
   * Collects supported files and creates base template. In most cases you do
   * not need to call this method yourself.
   *
   * \param files_and_dirs Files, directories and symlinks to be included in
   * collection.
   * \param col_base Result database.
   */
  void
  filesCollecting(const std::vector<std::filesystem::path> &files_and_dirs,
                  UDBase &col_base);

  /*!
   * Counts BaseID::File objects in given vector.
   *
   * \param el_v Vector to be processed.
   * \return Quantity of BaseID::File objects.
   */
  size_t
  countFiles(const std::vector<UDBElement> &el_v);

  /*!
   * Collects all BaseID::File objects.
   *
   * \param el_v Vector to be processed.
   * \return std::tuple. First element is pointer to vector where BaseID::File
   * object has been found. Second element is objects iterator.
   */
  std::vector<std::tuple<const std::vector<UDBElement> *,
                         std::vector<UDBElement>::const_iterator>>
  getAllFiles(const std::vector<UDBElement> &el_v);

  /*!
   * Cleans given vector from repeated BaseID::File objects.
   * \param files Vector obtained from getAllFiles() method.
   */
  void
  removeDublicates(
      std::vector<std::tuple<const std::vector<UDBElement> *,
                             std::vector<UDBElement>::const_iterator>> &files);

  /*!
   * Processed given items.
   *
   * \param items Database template vector.
   */
  void
  processFiles(const std::vector<UDBElement> &items);

  /*!
   * Parses given BaseID::File object.
   *
   * \param file Pointer to BaseID::File object.
   */
  void
  fileParsing(UDBElement *file);

  /*!
   * Removes all BaseID::File objects, not containing BaseID::Book objects.
   * \param items Items to be cleaned.
   */
  void
  cleanBase(std::vector<UDBElement> &items);  

  /*!
   * Parses fb2 and fbd file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  fb2Parsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Parses epub file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  epubParsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Parses pdf file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  pdfParsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Parses djvu file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  djvuParsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Parses odt file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  odtParsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Parses txt file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  txtParsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Parses archive file.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file to be parsed.
   */
  void
  archiveParsing(UDBElement *file, const std::filesystem::path &file_path);

  /*!
   * Calcultes hash sum for given buffer.
   *
   * \param buf Buffer hash sum to be calculated for.
   * \return std::string containing raw hash sum.
   */
  std::string
  bufferHash(const std::string &buf);

  /*!
   * Calculates file hash sum.
   *
   * \param file_path Path to file hash sum to be calculated for.
   * \return std::string containing raw hash sum.
   */
  std::string
  fileHash(const std::filesystem::path &file_path);

  /*!
   * Calcultes hash sum for given BaseID::File object.
   *
   * Caller can provide \a file_path ot \a buf on his choice.
   *
   * \param file Pointer to BaseID::File object.
   * \param file_path Path to file hash sum to be calculated for.
   * \param buf Buffer hash sum to be calculted for.
   */
  void
  bufHash(UDBElement *file, const std::filesystem::path &file_path,
          const std::string &buf);

  /*!
   * Smart pointer to MlBookProc object.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::shared_ptr<MLBookProc> mlbp;

  /*!
   * Smart pointer to processors identificators buffer.
   *
   * First element of tuple - processor number. Second element indicates if
   * processor is busy by any working thread of current collection creation
   * process.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::shared_ptr<std::vector<std::tuple<unsigned, bool>>> threads_v;

  /*!
   * Smart pointer to mutex locking #threads_v.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::shared_ptr<std::mutex> threads_v_mtx;

  /*!
   * Smart pointer to conditional variable locking #threads_v.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::shared_ptr<std::condition_variable> threads_v_var;

  /*!
   * If set to true all processes will be stopped.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::atomic<bool> cancel;

  /*!
   * Vector of active archives parsing processes.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::vector<std::shared_ptr<ArchiveParser>> arch_proc;

  /*!
   * std::mutex locking #arch_proc vector.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::mutex arch_proc_mtx;

  /*!
   * Vector of files for which hash sums has been calculated erlear (in
   * RefreshCollection methods).
   *
   * First element of tuple - path to file. Second element - raw hash sum.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::vector<std::tuple<std::filesystem::path, std::string>> already_hashed;

  /*!
   * BaseID object.
   */
  BaseID bid;

  /*!
   * Number of processed items.
   *
   * \warning Do not set or modify this object yourself.
   */
  std::atomic<double> processed;

  /*!
   * Total number of items to be processed.
   *
   * \warning Do not set or modify this object yourself.
   */
  double total;
};

#endif // CREATECOLLECTION_H
