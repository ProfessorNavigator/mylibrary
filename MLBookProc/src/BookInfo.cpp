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

#include <BookInfo.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <LibArchive.h>
#include <ODTParser.h>
#include <PDFParser.h>
#include <TXTParser.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

BookInfo::BookInfo(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
}

UDBase
BookInfo::getBookInfo(const UDBElement &book_search_result)
{
  UDBase result;

  std::vector<UDBElement>::const_iterator it
      = std::find_if(book_search_result.subelements.begin(),
                     book_search_result.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::File;
                       });
  if(it == book_search_result.subelements.end())
    {
      return result;
    }

  std::filesystem::path p
      = std::u8string(it->content.begin(), it->content.end());
  if(std::filesystem::is_symlink(p))
    {
      p = std::filesystem::read_symlink(p);
    }

  result = bookInfo(p, book_search_result);

  return result;
}

void
BookInfo::setDPI(const double &horizontal_dpi, const double &vertical_dpi)
{
  this->horizontal_dpi = horizontal_dpi;
  this->vertical_dpi = vertical_dpi;
}

double
BookInfo::getHorizontalDPI()
{
  return horizontal_dpi;
}

double
BookInfo::getVerticalDPI()
{
  return vertical_dpi;
}

UDBase
BookInfo::bookInfo(const std::filesystem::path &p, const UDBElement &path)
{
  UDBase result;

  std::string ext = mlbp->getExtension(p);
  ext = mlbp->stringToLower(ext);
  if(ext == ".fb2" || ext == ".fbd")
    {
      std::fstream f;
      f.open(p, std::ios_base::in | std::ios_base::binary);
      if(!f.is_open())
        {
          return result;
        }
      f.seekg(0, std::ios_base::end);
      std::string buf;
      buf.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      FB2Parser parser(mlbp);
      try
        {
          result = parser.getBookInfo(buf);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::bookInfo: \"" << er.what() << "\""
                    << std::endl;
        }
    }
  else if(ext == ".epub")
    {
      std::fstream f;
      f.open(p, std::ios_base::in | std::ios_base::binary);
      if(!f.is_open())
        {
          return result;
        }
      f.seekg(0, std::ios_base::end);
      std::string buf;
      buf.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      EPUBParser parser(mlbp);
      try
        {
          result = parser.getBookInfo(buf);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::bookInfo: \"" << er.what() << "\""
                    << std::endl;
        }
    }
  else if(ext == ".pdf")
    {
      std::fstream f;
      f.open(p, std::ios_base::in | std::ios_base::binary);
      if(!f.is_open())
        {
          return result;
        }
      f.seekg(0, std::ios_base::end);
      std::string buf;
      buf.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      PDFParser parser(mlbp);
      parser.setDPI(horizontal_dpi, vertical_dpi);
      try
        {
          result = parser.getBookInfo(buf);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::bookInfo: \"" << er.what() << "\""
                    << std::endl;
        }
    }
  else if(ext == ".djvu")
    {
      std::fstream f;
      f.open(p, std::ios_base::in | std::ios_base::binary);
      if(!f.is_open())
        {
          return result;
        }
      f.seekg(0, std::ios_base::end);
      std::string buf;
      buf.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      DJVUParser parser(mlbp);
      try
        {
          result = parser.getBookInfo(buf);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::bookInfo: \"" << er.what() << "\""
                    << std::endl;
        }
    }
  else if(ext == ".odt")
    {
      std::fstream f;
      f.open(p, std::ios_base::in | std::ios_base::binary);
      if(!f.is_open())
        {
          return result;
        }
      f.seekg(0, std::ios_base::end);
      std::string buf;
      buf.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      ODTParser parser(mlbp);
      try
        {
          result = parser.getBookInfo(buf);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::bookInfo: \"" << er.what() << "\""
                    << std::endl;
        }
    }
  else if(ext == ".txt" || ext == ".md")
    {
      TXTParser parser(mlbp);
      try
        {
          result = parser.getBookInfo(p);
        }
      catch(std::exception &er)
        {
          std::cout << "BookInfo::bookInfo: \"" << er.what() << "\""
                    << std::endl;
        }
    }
  else
    {
      BaseID::ID id = bid.getId(path);
      switch(id)
        {
        case BaseID::BookSearchResult:
        case BaseID::BookMark:
          {
            std::vector<UDBElement>::const_iterator it = std::find_if(
                path.subelements.begin(), path.subelements.end(),
                [this](const UDBElement &el)
                  {
                    return bid.getId(el) == BaseID::Book;
                  });
            if(it == path.subelements.end())
              {
                return result;
              }

            std::vector<UDBElement>::const_iterator it_p
                = std::find_if(it->subelements.begin(), it->subelements.end(),
                               [this](const UDBElement &el)
                                 {
                                   return bid.getId(el) == BaseID::PathInFile;
                                 });
            if(it_p == it->subelements.end())
              {
                return result;
              }

            std::unique_ptr<std::filesystem::path,
                            std::function<void(std::filesystem::path *)>>
                tmp_dir(new std::filesystem::path(mlbp->tempDirPath()
                                                  / mlbp->randomFileName()),
                        [](std::filesystem::path *p)
                          {
                            std::filesystem::remove_all(*p);
                            delete p;
                          });

            LibArchive la(mlbp);
            std::filesystem::path res_p;
            std::vector<UDBElement>::const_iterator it_fbd = std::find_if(
                it_p->subelements.begin(), it_p->subelements.end(),
                [this](const UDBElement &el)
                  {
                    return bid.getId(el) == BaseID::FBDPath;
                  });
            if(it_fbd != it_p->subelements.end())
              {
                if(ext == ".zip")
                  {
                    res_p = la.unpackZipFileToDirectory(p, it_fbd->content,
                                                        *tmp_dir);
                  }
                else
                  {
                    res_p = la.unpackFileToDirectory(p, it_fbd->content,
                                                     *tmp_dir);
                  }
                result = bookInfo(res_p, UDBElement());
                return result;
              }

            if(ext == ".zip")
              {
                res_p
                    = la.unpackZipFileToDirectory(p, it_p->content, *tmp_dir);
              }
            else
              {
                res_p = la.unpackFileToDirectory(p, it_p->content, *tmp_dir);
              }

            UDBElement path_l;
            std::vector<UDBElement>::const_iterator it_p2 = std::find_if(
                it_p->subelements.begin(), it_p->subelements.end(),
                [this](const UDBElement &el)
                  {
                    return bid.getId(el) == BaseID::PathInFile;
                  });
            if(it_p2 != it_p->subelements.end())
              {
                path_l = *it_p2;
              }
            result = bookInfo(res_p, path_l);
            break;
          }
        case BaseID::PathInFile:
          {
            std::unique_ptr<std::filesystem::path,
                            std::function<void(std::filesystem::path *)>>
                tmp_dir(new std::filesystem::path(mlbp->tempDirPath()
                                                  / mlbp->randomFileName()),
                        [](std::filesystem::path *p)
                          {
                            std::filesystem::remove_all(*p);
                            delete p;
                          });

            LibArchive la(mlbp);
            std::filesystem::path res_p;
            std::vector<UDBElement>::const_iterator it_fbd = std::find_if(
                path.subelements.begin(), path.subelements.end(),
                [this](const UDBElement &el)
                  {
                    return bid.getId(el) == BaseID::FBDPath;
                  });
            if(it_fbd != path.subelements.end())
              {
                if(ext == ".zip")
                  {
                    res_p = la.unpackZipFileToDirectory(p, it_fbd->content,
                                                        *tmp_dir);
                  }
                else
                  {
                    res_p = la.unpackFileToDirectory(p, it_fbd->content,
                                                     *tmp_dir);
                  }
                result = bookInfo(res_p, UDBElement());
                return result;
              }

            if(ext == ".zip")
              {
                res_p = la.unpackZipFileToDirectory(p, path.content, *tmp_dir);
              }
            else
              {
                res_p = la.unpackFileToDirectory(p, path.content, *tmp_dir);
              }

            UDBElement path_l;
            std::vector<UDBElement>::const_iterator it_p2 = std::find_if(
                path.subelements.begin(), path.subelements.end(),
                [this](const UDBElement &el)
                  {
                    return bid.getId(el) == BaseID::PathInFile;
                  });
            if(it_p2 != path.subelements.end())
              {
                path_l = *it_p2;
              }
            result = bookInfo(res_p, path_l);
            break;
          }
        default:
          break;
        }
    }

  return result;
}
