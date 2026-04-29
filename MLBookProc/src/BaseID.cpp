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
        id_str.push_back(-128);
        result.id = id_str;
        break;
      }
    case ID::File:
      {
        id_str.push_back(-127);
        result.id = id_str;
        break;
      }
    case ID::Symlink:
      {
        id_str.push_back(-126);
        result.id = id_str;
        break;
      }
    case ID::Book:
      {
        id_str.push_back(-125);
        result.id = id_str;
        break;
      }
    case ID::BookTitle:
      {
        id_str.push_back(-124);
        result.id = id_str;
        break;
      }
    case ID::SourceBookTitle:
      {
        id_str.push_back(-123);
        result.id = id_str;
        break;
      }
    case ID::Author:
      {
        id_str.push_back(-122);
        result.id = id_str;
        break;
      }
    case ID::SourceBookAuthor:
      {
        id_str.push_back(-121);
        result.id = id_str;
        break;
      }
    case ID::Sequence:
      {
        id_str.push_back(-120);
        result.id = id_str;
        break;
      }
    case ID::EbookAuthor:
      {
        id_str.push_back(-119);
        result.id = id_str;
        break;
      }
    case ID::SourceBookSequence:
      {
        id_str.push_back(-118);
        result.id = id_str;
        break;
      }
    case ID::PaperBookSequence:
      {
        id_str.push_back(-117);
        result.id = id_str;
        break;
      }
    case ID::Genre:
      {
        id_str.push_back(-116);
        result.id = id_str;
        break;
      }
    case ID::SourceBookGenre:
      {
        id_str.push_back(-115);
        result.id = id_str;
        break;
      }
    case ID::Date:
      {
        id_str.push_back(-114);
        result.id = id_str;
        break;
      }
    case ID::SourceBookDate:
      {
        id_str.push_back(-113);
        result.id = id_str;
        break;
      }
    case ID::PathInFile:
      {
        id_str.push_back(-112);
        result.id = id_str;
        break;
      }
    case ID::BookSearchResult:
      {
        id_str.push_back(-111);
        result.id = id_str;
        break;
      }
    case ID::FBDPath:
      {
        id_str.push_back(-110);
        result.id = id_str;
        break;
      }
    case ID::LastName:
      {
        id_str.push_back(-109);
        result.id = id_str;
        break;
      }
    case ID::FirstName:
      {
        id_str.push_back(-108);
        result.id = id_str;
        break;
      }
    case ID::MiddleName:
      {
        id_str.push_back(-107);
        result.id = id_str;
        break;
      }
    case ID::Language:
      {
        id_str.push_back(-106);
        result.id = id_str;
        break;
      }
    case ID::SourceLanguage:
      {
        id_str.push_back(-105);
        result.id = id_str;
        break;
      }
    case ID::SourceBookLanguage:
      {
        id_str.push_back(-104);
        result.id = id_str;
        break;
      }
    case ID::SourceBookSourceLanguage:
      {
        id_str.push_back(-103);
        result.id = id_str;
        break;
      }
    case ID::Keywords:
      {
        id_str.push_back(-102);
        result.id = id_str;
        break;
      }
    case ID::Translator:
      {
        id_str.push_back(-101);
        result.id = id_str;
        break;
      }
    case ID::SequenceName:
      {
        id_str.push_back(-100);
        result.id = id_str;
        break;
      }
    case ID::SequenceNumber:
      {
        id_str.push_back(-99);
        result.id = id_str;
        break;
      }
    case ID::SourceBookKeywords:
      {
        id_str.push_back(-98);
        result.id = id_str;
        break;
      }
    case ID::SourceBookTranslator:
      {
        id_str.push_back(-97);
        result.id = id_str;
        break;
      }
    case ID::SourceBookDublinCore:
      {
        id_str.push_back(-96);
        result.id = id_str;
        break;
      }
    case ID::EbookProgramUsed:
      {
        id_str.push_back(-95);
        result.id = id_str;
        break;
      }
    case ID::EbookDate:
      {
        id_str.push_back(-94);
        result.id = id_str;
        break;
      }
    case ID::EbookSourceUrl:
      {
        id_str.push_back(-93);
        result.id = id_str;
        break;
      }
    case ID::EbookSourceOCR:
      {
        id_str.push_back(-92);
        result.id = id_str;
        break;
      }
    case ID::EbookID:
      {
        id_str.push_back(-91);
        result.id = id_str;
        break;
      }
    case ID::EbookVersion:
      {
        id_str.push_back(-90);
        result.id = id_str;
        break;
      }
    case ID::EbookHistory:
      {
        id_str.push_back(-89);
        result.id = id_str;
        break;
      }
    case ID::EbookPublisher:
      {
        id_str.push_back(-88);
        result.id = id_str;
        break;
      }
    case ID::PaperBookPublisher:
      {
        id_str.push_back(-87);
        result.id = id_str;
        break;
      }
    case ID::PaperBookName:
      {
        id_str.push_back(-86);
        result.id = id_str;
        break;
      }
    case ID::PaperBookCity:
      {
        id_str.push_back(-85);
        result.id = id_str;
        break;
      }
    case ID::PaperBookYear:
      {
        id_str.push_back(-84);
        result.id = id_str;
        break;
      }
    case ID::PaperBookISBN:
      {
        id_str.push_back(-83);
        result.id = id_str;
        break;
      }
    case ID::CustomInfo:
      {
        id_str.push_back(-82);
        result.id = id_str;
        break;
      }
    case ID::DjvuPublisher:
      {
        id_str.push_back(-81);
        result.id = id_str;
        break;
      }
    case ID::CollectionInfo:
      {
        id_str.push_back(-80);
        result.id = id_str;
        break;
      }
    case ID::CollectionType:
      {
        id_str.push_back(-79);
        result.id = id_str;
        break;
      }
    case ID::CoverType:
      {
        id_str.push_back(-78);
        result.id = id_str;
        break;
      }
    case ID::BooksDirectory:
      {
        id_str.push_back(-77);
        result.id = id_str;
        break;
      }
    case ID::InpxPath:
      {
        id_str.push_back(-76);
        result.id = id_str;
        break;
      }
    case ID::FileHash:
      {
        id_str.push_back(-75);
        result.id = id_str;
        break;
      }
    case ID::FileSize:
      {
        id_str.push_back(-74);
        result.id = id_str;
        break;
      }
    case ID::Annotation:
      {
        id_str.push_back(-73);
        result.id = id_str;
        break;
      }
    case ID::CoverPage:
      {
        id_str.push_back(-72);
        result.id = id_str;
        break;
      }
    case ID::CoverHeight:
      {
        id_str.push_back(-71);
        result.id = id_str;
        break;
      }
    case ID::CoverWidth:
      {
        id_str.push_back(-70);
        result.id = id_str;
        break;
      }
    case ID::Nickname:
      {
        id_str.push_back(-69);
        result.id = id_str;
        break;
      }
    case ID::HomePage:
      {
        id_str.push_back(-68);
        result.id = id_str;
        break;
      }
    case ID::EMail:
      {
        id_str.push_back(-67);
        result.id = id_str;
        break;
      }
    case ID::AuthorID:
      {
        id_str.push_back(-66);
        result.id = id_str;
        break;
      }
    case ID::BookMark:
      {
        id_str.push_back(-65);
        result.id = id_str;
        break;
      }
    case ID::BookNote:
      {
        id_str.push_back(-64);
        result.id = id_str;
        break;
      }
    case ID::BookNoteFile:
      {
        id_str.push_back(-63);
        result.id = id_str;
        break;
      }
    case ID::AuthorSearchResult:
      {
        id_str.push_back(-62);
        result.id = id_str;
        break;
      }
    case ID::AnchorBasePath:
      {
        id_str.push_back(-61);
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

  switch(el.id[0])
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
