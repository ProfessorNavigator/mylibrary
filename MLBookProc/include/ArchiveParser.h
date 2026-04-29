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
#ifndef ARCHIVEPARSER_H
#define ARCHIVEPARSER_H

#include <BaseID.h>
#include <LibArchive.h>
#include <MLBookProc.h>
#include <UDBElement.h>
#include <archive_entry.h>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>

/*!
 * \brief The ArchiveParser class
 *
 * This class contains methods for archives parsign. In most cases you do not
 * need to call this methods directly. Use CreateCollection and BookInfo
 * instead.
 */
class ArchiveParser : public LibArchive
{
public:
  /*!
   * \brief ArchiveParser constructor
   * \param mlbp Smart pointer to MLBookProc object.
   * \param thread_v Smart pointer to vector containing tuples with processors
   * numbers and their current busy status.
   * \param thread_v_mtx Smart pointer to mutex, locking `thread_v` vector.
   * \param thread_v_var Condition variable, locking `thread_v` vector.
   */
  ArchiveParser(
      const std::shared_ptr<MLBookProc> &mlbp,
      const std::shared_ptr<std::vector<std::tuple<unsigned, bool>>> &thread_v,
      const std::shared_ptr<std::mutex> &thread_v_mtx,
      const std::shared_ptr<std::condition_variable> &thread_v_var);

  virtual ~ArchiveParser();

  /*!
   * Parses given archive.
   *
   * \param file_path Path to archive.
   * \return Vector of UDBElement objects, containing obtained information.
   */
  std::vector<UDBElement>
  parseArchive(const std::filesystem::path &file_path);

  /*!
   * Stops all internal operations.
   */
  void
  stopAll();

private:
  void
  parseEntry(std::shared_ptr<archive> a, std::shared_ptr<archive_entry> e);

  enum FileType
  {
    FB2,
    EPUB,
    PDF,
    DJVU,
    ODT,
    TXT
  };

  UDBElement
  bufferParse(const std::string &buf, const std::string &arch_file_path,
              std::shared_ptr<archive_entry> e, const FileType &ft);

  void
  fbdProcessing();

  std::vector<UDBElement> fbd;
  std::vector<UDBElement> result;
  std::vector<std::string> unsupported;

  std::atomic<bool> cancel;

  std::shared_ptr<ArchiveParser> arch_proc;

  std::shared_ptr<std::vector<std::tuple<unsigned, bool>>> thread_v;
  std::shared_ptr<std::mutex> thread_v_mtx;
  std::shared_ptr<std::condition_variable> thread_v_var;

  int thr_num = 0;
  std::mutex thr_num_mtx;
  std::condition_variable thr_num_var;

  BaseID bid;
};

#endif // ARCHIVEPARSER_H
