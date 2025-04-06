/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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
#ifndef NOTESKEEPER_H
#define NOTESKEEPER_H

#include <AuxFunc.h>
#include <NotesBaseEntry.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif
#ifndef USE_OPENMP
#include <mutex>
#endif

/*!
 * \brief The NotesKeeper class.
 *
 * This class contains various methods for collections notes operating. It is
 * recommended to start from loadBase() method. To create new note call
 * getNote() and editNote() methods. getNote() method also can be used to
 * obtain note for particular book in collection. editNote() method can be used
 * to remove note.
 */
class NotesKeeper
{
public:
  /*!
   * \brief NotesKeeper constructor.
   * \param af smart pointer to AuxFunc object.
   */
  NotesKeeper(const std::shared_ptr<AuxFunc> &af);

  /*!
   * \brief NotesKeeper destructor.
   */
  virtual ~NotesKeeper();

  /*!
   * \brief Loads notes base to memory.
   *
   * This method should be called before any other methods of this class.
   */
  void
  loadBase();

  /*!
   * \brief Edits note.
   *
   * If note file does not exist, this method will creat it and add
   * NotesBaseEntry object to base. If note file exists, it will be
   * overwritten. If note string is empty, note file will be removed, and
   * NotesBaseEntry object will be removed from base.
   * \param nbe NotesBaseEntry object (see getNote() and
   * getNotesForCollection()).
   * \param note note text.
   */
  void
  editNote(const NotesBaseEntry &nbe, const std::string &note);

  /*!
   * \brief Gets NotesBaseEntry object from base or creats it.
   *
   * If note exists, returns its NotesBaseEntry object. If note does not exist,
   * creats new NotesBaseEntry object.
   * \param collection_name collection name, book came from.
   * \param book_file_full_path book file absolute path.
   * \param book_path book path in file (if any, empty string otherwise).
   * \return NotesBaseEntry object for note.
   */
  NotesBaseEntry
  getNote(const std::string &collection_name,
          const std::filesystem::path &book_file_full_path,
          const std::string &book_path);

  /*!
   * \brief Returns content of note file.
   *
   * Note file contains header and note text. Header contains collection name,
   * book file absolute path, book path in file (if any). Header is separated
   * from note text by "\n\n" sequence.
   * \param nbe NotesBaseEntry object (see getNote() and
   * getNotesForCollection()).
   * \return String containing note file raw content.
   */
  std::string
  readNote(const NotesBaseEntry &nbe);

  /*!
   * \brief Returns note text (if any).
   * \param nbe NotesBaseEntry object (see getNote() and
   * getNotesForCollection()).
   * \return String containing note text.
   */
  std::string
  readNoteText(const NotesBaseEntry &nbe);

  /*!
   * \brief Removes notes.
   *
   * It is recommended to use this method after book removing from collection.
   *
   * \warning If \b nbe parametr \b book_file_full_path contains path to rar
   * archive, this method will remove notes for all books in archive.
   * \param nbe NotesBaseEntry object (see getNote() and
   * getNotesForCollection()).
   * \param reserve_directory absolute path to directory for reserve copies of
   * notes to be removed. If directory does not exist, it will be created.
   * \param make_reserve if set to \a true, removeNotes() will create reserve
   * copies of notes to be removed.
   */
  void
  removeNotes(const NotesBaseEntry &nbe,
              const std::filesystem::path &reserve_directory,
              const bool &make_reserve);

  /*!
   * \brief Removes notes for all books in particular collection.
   * \param collection_name collection name.
   * \param reserve_directory absolute path to directory for reserve copies of
   * notes to be removed. If directory does not exist, it will be created.
   * \param make_reserve if set to \a true, removeCollection() will create
   * reserve copies of notes to be removed.
   */
  void
  removeCollection(const std::string &collection_name,
                   const std::filesystem::path &reserve_directory,
                   const bool &make_reserve);

  /*!
   * \brief Compares notes base and collection base and removes notes for
   * absent books.
   * \param collection_name collection to compare bases.
   * \param reserve_directory absolute path to directory for reserve copies of
   * notes to be removed. If directory does not exist, it will be created.
   * \param make_reserve if set to \a true, refreshCollection() will create
   * reserve copies of notes to be removed.
   */
  void
  refreshCollection(const std::string &collection_name,
                    const std::filesystem::path &reserve_directory,
                    const bool &make_reserve);

  /*!
   * \brief Returns all notes for particular collection.
   * \param collection_name collection name.
   * \return Vector of NotesBaseEntry objects.
   */
  std::vector<NotesBaseEntry>
  getNotesForCollection(const std::string &collection_name);

private:
  void
  parseRawBase(const std::string &raw_base);

  void
  parseEntry(const std::string &entry);

  void
  saveBase();

  std::shared_ptr<AuxFunc> af;
  std::filesystem::path base_directory_path;

  std::vector<NotesBaseEntry> base;
#ifndef USE_OPENMP
  std::mutex base_mtx;
#endif
#ifdef USE_OPENMP
  omp_lock_t base_mtx;
#endif
};

#endif // NOTESKEEPER_H
