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

#include <BookInfo.h>
#include <BookParseEntry.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <ODTParser.h>
#include <PDFParser.h>
#include <SelfRemovingPath.h>
#include <TXTParser.h>
#include <XMLTextEncoding.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>

BookInfo::BookInfo(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
}

std::shared_ptr<BookInfoEntry>
BookInfo::get_book_info(const BookBaseEntry &bbe)
{
  return getBookInfo(bbe);
}

std::shared_ptr<BookInfoEntry>
BookInfo::getBookInfo(const BookBaseEntry &bbe)
{
  std::shared_ptr<BookInfoEntry> result;
  std::string ext;
  std::error_code ec;
  std::filesystem::file_status fstat
      = std::filesystem::symlink_status(bbe.file_path, ec);
  if(ec)
    {
      throw std::runtime_error("BookInfo::getBookInfo error: \"" + ec.message()
                               + "\" " + bbe.file_path.u8string());
    }
  if(fstat.type() == std::filesystem::file_type::symlink)
    {
      std::filesystem::path resolved
          = std::filesystem::read_symlink(bbe.file_path);
      ext = resolved.extension().u8string();
    }
  else
    {
      ext = bbe.file_path.extension().u8string();
    }
  ext = af->stringToLower(ext);
  if(ext == ".fb2" || ext == ".fbd")
    {
      std::fstream f;
      f.open(bbe.file_path, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
        {
          std::string book;
          f.seekg(0, std::ios_base::end);
          book.resize(f.tellg());
          f.seekg(0, std::ios_base::beg);
          f.read(book.data(), book.size());
          f.close();

          FB2Parser fb2(af);
          try
            {
              result = fb2.fb2BookInfo(book);
            }
          catch(std::exception &er)
            {
              std::cout << "BookInfo::getBookInfo: \"" << er.what() << "\""
                        << std::endl;
            }
        }
    }
  else if(ext == ".epub")
    {
      EPUBParser epub(af);
      try
        {
          result = epub.epubBookInfo(bbe.file_path);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::getBookInfo: \"" << er.what() << "\""
                    << std::endl;
          result = std::make_shared<BookInfoEntry>();
        }
    }
  else if(ext == ".pdf")
    {
      std::fstream f;
      f.open(bbe.file_path, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
        {
          std::string book;
          f.seekg(0, std::ios_base::end);
          book.resize(f.tellg());
          f.seekg(0, std::ios_base::beg);
          f.read(book.data(), book.size());
          f.close();

          PDFParser pdf(af);
          result = pdf.pdf_annotation_n_cover(book, h_dpi, v_dpi);
        }
    }
  else if(ext == ".djvu")
    {
      DJVUParser djvu(af);
      result = djvu.djvu_book_info(bbe.file_path);
    }
  else if(ext == ".odt")
    {
      ODTParser odt(af);
      result = odt.odtBookInfo(bbe.file_path);
    }
  else if(ext == ".txt" || ext == ".md")
    {      
      TXTParser txt(af);
      result = txt.txtBookInfo(bbe.file_path);
    }
  else
    {
      result = getFromArchive(bbe, ext);
    }

  return result;
}

void
BookInfo::set_dpi(const double &h_dpi, const double &v_dpi)
{
  setDpi(h_dpi, v_dpi);
}

void
BookInfo::setDpi(const double &h_dpi, const double &v_dpi)
{
  this->h_dpi = h_dpi;
  this->v_dpi = v_dpi;
}

std::shared_ptr<BookInfoEntry>
BookInfo::getFromArchive(const BookBaseEntry &bbe, const std::string &ext)
{
  std::shared_ptr<BookInfoEntry> result;
  LibArchive la(af);
  std::filesystem::path tmp = af->tempPath();
  tmp /= std::filesystem::u8path(af->randomFileName());
  SelfRemovingPath p(tmp);
  BookBaseEntry bber = bbe;
  std::string::size_type n;
  std::string unpack_path;
  std::string sstr = "\n";
  n = bbe.bpe.book_path.find(sstr);
  if(n != std::string::npos)
    {
      unpack_path = bber.bpe.book_path.substr(0, n);
      bber.bpe.book_path.erase(0, n + sstr.size());
    }
  else
    {
      unpack_path = bber.bpe.book_path;
    }
  if(ext == ".zip")
    {
      std::vector<ArchEntry> files;
      la.fileNames(bbe.file_path, files);
      std::string::size_type n;
      sstr = "\\";
      for(auto it = files.begin(); it != files.end(); it++)
        {
          if(it->filename.size() > 0)
            {
              n = 0;
              for(;;)
                {
                  n = it->filename.find(sstr, n);
                  if(n != std::string::npos)
                    {
                      it->filename.erase(n, sstr.size());
                      it->filename.insert(n, "/");
                    }
                  else
                    {
                      break;
                    }
                }
            }
        }
      std::string search_p;
      std::string conv_nm;
      for(int32_t i = -1; i < af->get_charset_conv_quantity(); i++)
        {
          if(i < 0)
            {
              search_p = unpack_path;
            }
          else
            {
              conv_nm = af->get_converter_by_number(i);
              XMLTextEncoding::convertToEncoding(unpack_path, search_p,
                                                 "UTF-8", conv_nm);
            }
          auto it = std::find_if(files.begin(), files.end(),
                                 [search_p](ArchEntry &el)
                                   {
                                     return el.filename == search_p;
                                   });
          if(it != files.end())
            {
              std::filesystem::path ch_fbd;
              bool encoding = false;
              if(i < 0)
                {
                  ch_fbd = std::filesystem::u8path(it->filename);
                  ch_fbd.replace_extension(".fbd");
                }
              else
                {
                  std::string res;
                  XMLTextEncoding::convertToEncoding(it->filename, res,
                                                     conv_nm, "UTF-8");
                  ch_fbd = std::filesystem::u8path(res);
                  ch_fbd.replace_extension(".fbd");
                  encoding = true;
                }
              auto it2 = std::find_if(files.begin(), files.end(),
                                      std::bind(&BookInfo::compareFunc, this,
                                                std::placeholders::_1,
                                                encoding, conv_nm, ch_fbd));
              if(it2 != files.end())
                {
                  bber.file_path
                      = la.unpackByPosition(bbe.file_path, tmp, *it2);
                }
              else
                {
                  bber.file_path
                      = la.unpackByPosition(bbe.file_path, tmp, *it);
                }

              BookInfo *bi = new BookInfo(af);
              result = bi->getBookInfo(bber);
              delete bi;
              break;
            }
        }
    }
  else
    {
      std::filesystem::path ch_fbd = std::filesystem::u8path(unpack_path);
      std::string ext = ch_fbd.extension().u8string();
      ext = af->stringToLower(ext);
      if(ext != ".fb2")
        {
          std::vector<ArchEntry> files;
          la.fileNamesStream(bbe.file_path, files);
          ch_fbd.replace_extension(".fbd");
          auto it2 = std::find_if(files.begin(), files.end(),
                                  std::bind(&BookInfo::compareFunc, this,
                                            std::placeholders::_1, false, "",
                                            ch_fbd));
          if(it2 != files.end())
            {
              bber.file_path = la.unpackByFileNameStream(bbe.file_path, tmp,
                                                         it2->filename);
              BookInfo *bi = new BookInfo(af);
              result = bi->getBookInfo(bber);
              delete bi;
            }
          else
            {
              bber.file_path
                  = la.unpackByFileNameStream(bbe.file_path, tmp, unpack_path);
              BookInfo *bi = new BookInfo(af);
              result = bi->getBookInfo(bber);
              delete bi;
            }
        }
      else
        {
          bber.file_path
              = la.unpackByFileNameStream(bbe.file_path, tmp, unpack_path);
          BookInfo *bi = new BookInfo(af);
          result = bi->getBookInfo(bber);
          delete bi;
        }
    }
  return result;
}

bool
BookInfo::compareFunc(const ArchEntry &ent, const bool &encoding,
                       const std::string &conv_nm,
                       const std::filesystem::path &ch_fbd)
{
  std::string val;
  if(encoding)
    {
      XMLTextEncoding::convertToEncoding(ent.filename, val, conv_nm, "UTF-8");
    }
  else
    {
      val = ent.filename;
    }
  std::filesystem::path ch_p = std::filesystem::u8path(val);
  std::string ext = af->stringToLower(ch_p.extension().u8string());
  if(ext == ch_fbd.extension().u8string() && ch_fbd.stem() == ch_p.stem())
    {
      return true;
    }
  else
    {
      return false;
    }
}
