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

#include <BaseID.h>
#include <cstdint>
#include <stdexcept>

BaseID::BaseID()
{
}

void
BaseID::setId(UDBElement &result, const ID &id)
{
  std::string id_str;
  switch(id)
    {
    case ID::Dir:
      {
        int8_t val = -128;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::File:
      {
        int8_t val = -127;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Symlink:
      {
        int8_t val = -126;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Book:
      {
        int8_t val = -125;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::BookTitle:
      {
        int8_t val = -124;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookTitle:
      {
        int8_t val = -123;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Author:
      {
        int8_t val = -122;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookAuthor:
      {
        int8_t val = -121;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Sequence:
      {
        int8_t val = -120;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookAuthor:
      {
        int8_t val = -119;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookSequence:
      {
        int8_t val = -118;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PaperBookSequence:
      {
        int8_t val = -117;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Genre:
      {
        int8_t val = -116;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookGenre:
      {
        int8_t val = -115;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Date:
      {
        int8_t val = -114;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookDate:
      {
        int8_t val = -113;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PathInFile:
      {
        int8_t val = -112;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::BookSearchResult:
      {
        int8_t val = -111;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::FBDPath:
      {
        int8_t val = -110;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::LastName:
      {
        int8_t val = -109;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::FirstName:
      {
        int8_t val = -108;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::MiddleName:
      {
        int8_t val = -107;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Language:
      {
        int8_t val = -106;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceLanguage:
      {
        int8_t val = -105;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookLanguage:
      {
        int8_t val = -104;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookSourceLanguage:
      {
        int8_t val = -103;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Keywords:
      {
        int8_t val = -102;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Translator:
      {
        int8_t val = -101;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SequenceName:
      {
        int8_t val = -100;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SequenceNumber:
      {
        int8_t val = -99;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookKeywords:
      {
        int8_t val = -98;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookTranslator:
      {
        int8_t val = -97;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::SourceBookDublinCore:
      {
        int8_t val = -96;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookProgramUsed:
      {
        int8_t val = -95;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookDate:
      {
        int8_t val = -94;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookSourceUrl:
      {
        int8_t val = -93;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookSourceOCR:
      {
        int8_t val = -92;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookID:
      {
        int8_t val = -91;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookVersion:
      {
        int8_t val = -90;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookHistory:
      {
        int8_t val = -89;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EbookPublisher:
      {
        int8_t val = -88;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PaperBookPublisher:
      {
        int8_t val = -87;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PaperBookName:
      {
        int8_t val = -86;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PaperBookCity:
      {
        int8_t val = -85;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PaperBookYear:
      {
        int8_t val = -84;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::PaperBookISBN:
      {
        int8_t val = -83;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CustomInfo:
      {
        int8_t val = -82;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::DjvuPublisher:
      {
        int8_t val = -81;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CollectionInfo:
      {
        int8_t val = -80;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CollectionType:
      {
        int8_t val = -79;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CoverType:
      {
        int8_t val = -78;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::BooksDirectory:
      {
        int8_t val = -77;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::InpxPath:
      {
        int8_t val = -76;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::FileHash:
      {
        int8_t val = -75;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::FileSize:
      {
        int8_t val = -74;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Annotation:
      {
        int8_t val = -73;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CoverPage:
      {
        int8_t val = -72;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CoverHeight:
      {
        int8_t val = -71;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::CoverWidth:
      {
        int8_t val = -70;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::Nickname:
      {
        int8_t val = -69;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::HomePage:
      {
        int8_t val = -68;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::EMail:
      {
        int8_t val = -67;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::AuthorID:
      {
        int8_t val = -66;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::BookMark:
      {
        int8_t val = -65;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::BookNote:
      {
        int8_t val = -64;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::BookNoteFile:
      {
        int8_t val = -63;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::AuthorSearchResult:
      {
        int8_t val = -62;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    case ID::AnchorBasePath:
      {
        int8_t val = -61;
        id_str.push_back(*reinterpret_cast<char *>(&val));
        result.id = id_str;
        break;
      }
    default:
      break;
    }
}

BaseID::ID
BaseID::getId(const UDBElement &el) const
{
  if(el.id.empty())
    {
      throw std::runtime_error("BaseID::getId: incorrect id");
    }
  ID result;

  int8_t id = *reinterpret_cast<const int8_t *>(&el.id[0]);

  switch(id)
    {
    case -128:
      {
        result = ID::Dir;
        break;
      }
    case -127:
      {
        result = ID::File;
        break;
      }
    case -126:
      {
        result = ID::Symlink;
        break;
      }
    case -125:
      {
        result = ID::Book;
        break;
      }
    case -124:
      {
        result = ID::BookTitle;
        break;
      }
    case -123:
      {
        result = ID::SourceBookTitle;
        break;
      }
    case -122:
      {
        result = ID::Author;
        break;
      }
    case -121:
      {
        result = ID::SourceBookAuthor;
        break;
      }
    case -120:
      {
        result = ID::Sequence;
        break;
      }
    case -119:
      {
        result = ID::EbookAuthor;
        break;
      }
    case -118:
      {
        result = ID::SourceBookSequence;
        break;
      }
    case -117:
      {
        result = ID::PaperBookSequence;
        break;
      }
    case -116:
      {
        result = ID::Genre;
        break;
      }
    case -115:
      {
        result = ID::SourceBookGenre;
        break;
      }
    case -114:
      {
        result = ID::Date;
        break;
      }
    case -113:
      {
        result = ID::SourceBookDate;
        break;
      }
    case -112:
      {
        result = ID::PathInFile;
        break;
      }
    case -111:
      {
        result = ID::BookSearchResult;
        break;
      }
    case -110:
      {
        result = ID::FBDPath;
        break;
      }
    case -109:
      {
        result = ID::LastName;
        break;
      }
    case -108:
      {
        result = ID::FirstName;
        break;
      }
    case -107:
      {
        result = ID::MiddleName;
        break;
      }
    case -106:
      {
        result = ID::Language;
        break;
      }
    case -105:
      {
        result = ID::SourceLanguage;
        break;
      }
    case -104:
      {
        result = ID::SourceBookLanguage;
        break;
      }
    case -103:
      {
        result = ID::SourceBookSourceLanguage;
        break;
      }
    case -102:
      {
        result = ID::Keywords;
        break;
      }
    case -101:
      {
        result = ID::Translator;
        break;
      }
    case -100:
      {
        result = ID::SequenceName;
        break;
      }
    case -99:
      {
        result = ID::SequenceNumber;
        break;
      }
    case -98:
      {
        result = ID::SourceBookKeywords;
        break;
      }
    case -97:
      {
        result = ID::SourceBookTranslator;
        break;
      }
    case -96:
      {
        result = ID::SourceBookDublinCore;
        break;
      }
    case -95:
      {
        result = ID::EbookProgramUsed;
        break;
      }
    case -94:
      {
        result = ID::EbookDate;
        break;
      }
    case -93:
      {
        result = ID::EbookSourceUrl;
        break;
      }
    case -92:
      {
        result = ID::EbookSourceOCR;
        break;
      }
    case -91:
      {
        result = ID::EbookID;
        break;
      }
    case -90:
      {
        result = ID::EbookVersion;
        break;
      }
    case -89:
      {
        result = ID::EbookHistory;
        break;
      }
    case -88:
      {
        result = ID::EbookPublisher;
        break;
      }
    case -87:
      {
        result = ID::PaperBookPublisher;
        break;
      }
    case -86:
      {
        result = ID::PaperBookName;
        break;
      }
    case -85:
      {
        result = ID::PaperBookCity;
        break;
      }
    case -84:
      {
        result = ID::PaperBookYear;
        break;
      }
    case -83:
      {
        result = ID::PaperBookISBN;
        break;
      }
    case -82:
      {
        result = ID::CustomInfo;
        break;
      }
    case -81:
      {
        result = ID::DjvuPublisher;
        break;
      }
    case -80:
      {
        result = ID::CollectionInfo;
        break;
      }
    case -79:
      {
        result = ID::CollectionType;
        break;
      }
    case -78:
      {
        result = ID::CoverType;
        break;
      }
    case -77:
      {
        result = ID::BooksDirectory;
        break;
      }
    case -76:
      {
        result = ID::InpxPath;
        break;
      }
    case -75:
      {
        result = ID::FileHash;
        break;
      }
    case -74:
      {
        result = ID::FileSize;
        break;
      }
    case -73:
      {
        result = ID::Annotation;
        break;
      }
    case -72:
      {
        result = ID::CoverPage;
        break;
      }
    case -71:
      {
        result = ID::CoverHeight;
        break;
      }
    case -70:
      {
        result = ID::CoverWidth;
        break;
      }
    case -69:
      {
        result = ID::Nickname;
        break;
      }
    case -68:
      {
        result = ID::HomePage;
        break;
      }
    case -67:
      {
        result = ID::EMail;
        break;
      }
    case -66:
      {
        result = ID::AuthorID;
        break;
      }
    case -65:
      {
        result = ID::BookMark;
        break;
      }
    case -64:
      {
        result = ID::BookNote;
        break;
      }
    case -63:
      {
        result = ID::BookNoteFile;
        break;
      }
    case -62:
      {
        result = ID::AuthorSearchResult;
        break;
      }
    case -61:
      {
        result = ID::AnchorBasePath;
        break;
      }
    default:
      {
        result = ID::Error;
        break;
      }
    }

  return result;
}
