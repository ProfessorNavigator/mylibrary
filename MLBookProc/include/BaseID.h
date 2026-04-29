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
#ifndef BASEID_H
#define BASEID_H

#include <UDBElement.h>

/*!
 * \brief The BaseID class
 *
 * This class contains enumerator of all database object types and methods to
 * set and get this types.
 */
class BaseID
{
public:
  BaseID();

  /*!
   * \brief The ID enum
   */
  enum ID
  {
    /*!
     * Objects of this type contain collection directory path.
     */
    Dir,
    /*!
     * Objects of this type contain collection file path, FileHash,
     * FileSize and Book objects.
     */
    File,
    /*!
     * Objects of this type contain collection symlink path (only files
     * symlinks) and BaseID::File object.
     */
    Symlink,
    /*!
     * Objects of this type contain information about book. Such object can
     * include BaseID::Author, BaseID::BookTitle, BaseID::Genre, BaseID::Date,
     * BaseID::PathInFile objects. `content` variable is empty.
     */
    Book,
    /*!
     * Objects of this type contain book name.
     */
    BookTitle,
    /*!
     * Objects of this type contain source book name
     */
    SourceBookTitle,
    /*!
     * Objects of this type contain information about author. Such objects can
     * include BaseID::LastName, BaseID::FirstName, BaseID::MiddleName,
     * BaseID::Nickname, BaseID::HomePage, BaseID::EMail, and BaseID::AuthorID
     * objects. `content` varible can contain author full name in free format.
     */
    Author,
    /*!
     * Objects of this type contain source book author information (same as
     * BaseID::Author object).
     */
    SourceBookAuthor,
    /*!
     * Objects of this type contain e-book author information (same as
     * BaseID::Author object).
     */
    EbookAuthor,
    /*!
     * Objects of this type contain author surname.
     */
    LastName,
    /*!
     * Objects of this type contain author first name.
     */
    FirstName,
    /*!
     * Objects of this type contain author second name.
     */
    MiddleName,
    /*!
     * Objects of this type contain information about translator (same as
     * BaseID::Author object).
     */
    Translator,
    /*!
     * Objects of this type contain information about source book translator
     * (same as BaseID::Author object).
     */
    SourceBookTranslator,
    /*!
     * Objects of this type contain book series information. Such object must
     * contain SequenceName object and can contain SequenceNumber object.
     */
    Sequence,
    /*!
     * Objects of this type contain book series name.
     */
    SequenceName,
    /*!
     * Objects of this type contain book number in series.
     */
    SequenceNumber,
    /*!
     * Objects of this type contain source book series information (same as
     * BaseID::Sequence object).
     */
    SourceBookSequence,
    /*!
     * Objects of this type contain paper book series information (same as
     * BaseID::Sequence object).
     */
    PaperBookSequence,
    /*!
     * Objects of this type contain genre code.
     */
    Genre,
    /*!
     * Objects of this type contain source book genre code.
     */
    SourceBookGenre,
    /*!
     * Objects of this type contain date book has been published or finished.
     */
    Date,
    /*!
     * Objects of this type contain date e-book has been published.
     */
    EbookDate,
    /*!
     * Objects of this type contain book language.
     */
    Language,
    /*!
     * Objects of this type contain book source language.
     */
    SourceLanguage,
    /*!
     * Objects of this type contain source book language.
     */
    SourceBookLanguage,
    /*!
     * Objects of this type contain source book source language.
     */
    SourceBookSourceLanguage,
    /*!
     * Objects of this type contain date source book has been published or
     * finished.
     */
    SourceBookDate,
    /*!
     * Objects of this type contain book keywords.
     */
    Keywords,
    /*!
     * Objects of this type contain source book keywords.
     */
    SourceBookKeywords,
    /*!
     * Objects of this type contain book path in file (if file is archive).
     * Such object can contain another BaseID::PathInFile object ('archive in
     * archive' case) and BaseID::FBDPath object.
     */
    PathInFile,
    /*!
     * Objects of this type are returned by various BaseKeeper methods. In most
     * cases such object includes BaseID::File type object (without
     * BaseID::FileHash  and BaseID::FileSize) and BaseID::Book type object.
     */
    BookSearchResult,
    /*!
     * Objects of this type contain path to fbd file in archive.
     */
    FBDPath,
    /*!
     * Objects of this type contain source book name obtained from Dublin Core
     * file (used in epub and odt formats).
     */
    SourceBookDublinCore,
    /*!
     * Objects of this type contain name of program which has been used to
     * create e-book.
     */
    EbookProgramUsed,
    /*!
     * Objects of this type contain URL to e-book source.
     */
    EbookSourceUrl,
    /*!
     * Objects of this type contain author of the original (online) document,
     * if this is a conversion (free form).
     */
    EbookSourceOCR,
    /*!
     * Objects of this type contain e-book idetification.
     */
    EbookID,
    /*!
     * Objects of this type contain e-book version.
     */
    EbookVersion,
    /*!
     * Objects of this type contain e-book history of changes.
     */
    EbookHistory,
    /*!
     * Objects of this type contain e-book publisher (same as BaseID::Author
     * object).
     */
    EbookPublisher,
    /*!
     * Objects of this type contain paper book publisher.
     */
    PaperBookPublisher,
    /*!
     * Objects of this type contain paper book name.
     */
    PaperBookName,
    /*!
     * Objects of this type contain city where paper book has been published.
     */
    PaperBookCity,
    /*!
     * Objects of this type contain year when paper book has been published.
     */
    PaperBookYear,
    /*!
     * Objects of this type contain paper book ISBN.
     */
    PaperBookISBN,
    /*!
     * Objects of this type contain book custom info.
     */
    CustomInfo,
    /*!
     * Objects of this type contain djvu document publisher.
     */
    DjvuPublisher,
    /*!
     * Objects of this type contain BaseID::CollectionType object and can
     * contain BaseID::BooksDirectory and BaseID::InpxPath objects.
     */
    CollectionInfo,
    /*!
     * Objects of this type contain collection type. Can be 'native', 'inpx'
     * and 'legacy'.
     */
    CollectionType,
    /*!
     * Objects of this type contain book cover type. Can be 'image' (image
     * which can be automatically recognized by graphical libraries, like jpg,
     * png etc.), 'base64' (same as previous, but base64 encoded), 'text' (XML
     * text), 'txt' (plain text), 'md' (Markdown text), 'ARGB' (ARGB image),
     * 'RGB' (RGB image).
     */
    CoverType,
    /*!
     * Objects of this type contain path to books directory (used in 'inpx' and
     * 'legacy' collection types).
     */
    BooksDirectory,
    /*!
     * Objects of this type contain path to inpx file.
     */
    InpxPath,
    /*!
     * Objects of this type contain file hash (BLAKE2B).
     */
    FileHash,
    /*!
     * Objects of this type contain file size (uint64_t little endian as raw
     * bytes).
     */
    FileSize,
    /*!
     * Objects of this type contain book annotation (can contains XML markup).
     */
    Annotation,
    /*!
     * Objects of this type contain book cover page. Must contain
     * BaseID::CoverType object, can contain BaseID::CoverHeight and
     * BaseID::CoverWidth objects.
     */
    CoverPage,
    /*!
     * Objects of this type contain cover page height in pixels (little endian
     * unsigned integer as raw bytes).
     */
    CoverHeight,
    /*!
     * Objects of this type contain cover page width in pixels (little endian
     * unsigned integer as raw bytes).
     */
    CoverWidth,
    /*!
     * Objects of this type contain nickname of author (translator, publisher).
     */
    Nickname,
    /*!
     * Objects of this type contain author (translator, publisher) home page.
     */
    HomePage,
    /*!
     * Objects of this type contain author (translator, publisher) e-mail.
     */
    EMail,
    /*!
     * Objects of this type contain author (translator, publisher)
     * identification.
     */
    AuthorID,
    /*!
     * Objects of this type contain bookmark used in BookmarksKeeper. Should
     * contains BaseID::File and BaseID::Book objects.
     */
    BookMark,
    /*!
     * Objects of this type contain book notes used in NotesKeeper. Should
     * contain BaseID::File, BaseID::BookNoteFile objects. Can contain
     * BaseID::PathInFile object.
     */
    BookNote,
    /*!
     * Objects of this type contain path to book note file.
     */
    BookNoteFile,
    /*!
     * Objects of this type contain author (free form).
     */
    AuthorSearchResult,
    /*!
     * Objects of this type contain 'anchor' path, used on collection export
     * and import operations.
     */
    AnchorBasePath,
    /*!
     * Invalid object.
     */
    Error
  };

  /*!
   * Sets objects ID.
   *
   * \param result UDBElement ID to be set to.
   * \param id BaseID::ID member.
   */
  void
  setId(UDBElement &result, const ID &id);

  /*!
   *
   * Returns objects ID.
   *
   * \note This method can throw std::exception in case of errors.
   *
   * \param el UDBElement ID of which should be get.
   * \return BaseID::ID member.
   */
  ID
  getId(const UDBElement &el) const;
};

#endif // BASEID_H
