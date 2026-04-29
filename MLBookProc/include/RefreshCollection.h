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
#ifndef REFRESHCOLLECTION_H
#define REFRESHCOLLECTION_H

#include <BaseKeeper.h>
#include <CreateCollection.h>
#include <memory>

/*!
 * \brief The RefreshCollection class
 *
 * This class contains methods used for collection maintenance.
 */
class RefreshCollection : public CreateCollection
{
public:
  /*!
   * \brief RefreshCollection constructor.
   * \param mlbp Smart pointer to MLBookProc object.
   * \param threads_num Maximum number of threads to be used in operations.
   */
  RefreshCollection(const std::shared_ptr<MLBookProc> &mlbp,
                    const int &threads_num = int(1));

  virtual ~RefreshCollection();

  /*!
   * Refreshes given collection.
   *
   * Checks files existance and sizes. If size of file is not equal its
   * database entry, file will be reparsed. If \a fast_refresh set to \a false,
   * checks all files hash sums, and if file hash sum not equal its database
   * entry, file will be reparsed. If given collection type is 'legacy' or
   * 'inpx', collection will be recreated as 'native'.
   *
   * \param base_path Path to collection database file.
   * \param fast_refresh If \a true, file hash sums will not be checked.
   */
  void
  refreshCollection(const std::filesystem::path &base_path,
                    const bool &fast_refresh = bool(true));

  /*!
   * Adds files, symlinks or directories to existing collection.
   *
   * \param base_path Path to collection database file.
   * \param files_and_dirs List of files, symlinks or directories to be added
   * to collection.
   */
  void
  addFilesAndDirs(const std::filesystem::path &base_path,
                  const std::vector<std::filesystem::path> &files_and_dirs);

  /*!
   * If this callback set, it will be called to indicate files hashing
   * progress.
   *
   * \a progress - number of items already hashed, \a total - total number of
   * items to be hashed.
   */
  std::function<void(double processed, double total)> signal_file_hashed;

private:
  bool
  elementRemove(const UDBElement &el, const bool &fast_refresh);

  void
  compareVectors(std::vector<UDBElement> &new_v,
                 const std::vector<UDBElement> &old_v);

  BaseKeeper *base_keeper;

  double l_processed = 0.0;
};

#endif // REFRESHCOLLECTION_H
