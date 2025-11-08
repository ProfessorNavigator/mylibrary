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

#ifndef ARCHPARSER_H
#define ARCHPARSER_H

#include <AuxFunc.h>
#include <BookParseEntry.h>
#include <LibArchive.h>
#include <SelfRemovingPath.h>
#include <archive.h>
#include <filesystem>
#include <memory>
#include <vector>

#ifndef USE_OPENMP
#include <atomic>
#include <condition_variable>
#include <mutex>
#else
#include <omp.h>
#endif

/*!
 * \brief The ARCHParser class.
 *
 * Class for archives parsing. In most cases you do not need to use this class
 * directly. Use CreateCollection or RefreshCollection instead.
 */
class ARCHParser : public LibArchive
{
public:
#ifdef USE_OPENMP
  /*!
   * \brief ARCHParser constructor.
   * \param af smart pointer to AuxFunc object.
   * \param rar_support if \a true, ARCHParser will parse rar archives,
   * otherwise not.
   */
  ARCHParser(const std::shared_ptr<AuxFunc> &af, const bool &rar_support);
#else
  ARCHParser(const std::shared_ptr<AuxFunc> &af, const bool &rar_support,
             const int &processor_num);
#endif

  /*!
   * \brief ARCHParser destructor.
   */
  virtual ~ARCHParser();

  /*!
   * \brief This method carries out actual parsing.
   *
   * Call this method to start archive parsing.
   * \param filepath absolute path to archive.
   * \return Vector of BookParseEntry objects.
   */
  std::vector<BookParseEntry>
  arch_parser(const std::filesystem::path &filepath);

  /*!
   * \brief Stops all operations.
   */
  void
  stopAll();

private:
  void
  arch_process(const std::shared_ptr<archive> &a);

  void
  unpack_entry(const std::filesystem::path &ch_p,
               const std::shared_ptr<archive> &a,
               const std::shared_ptr<archive_entry> &e);

  void
  check_for_fbd();

  static void
  signalHandler(int sig);

  std::shared_ptr<AuxFunc> af;
  bool rar_support = false;

  std::vector<BookParseEntry> result;
  std::vector<BookParseEntry> fbd;

  std::vector<ARCHParser *> archp_obj;

#ifndef USE_OPENMP
  std::mutex archp_obj_mtx;
  std::atomic<bool> cancel;

  bool extra_run = false;
  std::mutex extra_run_mtx;
  std::condition_variable extra_run_var;

  int processor_num = -1;
#else
  bool cancel = false;
  omp_lock_t archp_obj_mtx;
#endif

  std::filesystem::path arch_path;
};

#endif // ARCHPARSER_H
