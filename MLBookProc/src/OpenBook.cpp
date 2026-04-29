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

#include <OpenBook.h>
#include <algorithm>

OpenBook::OpenBook(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
  la = new LibArchive(mlbp);
  supported_archives = mlbp->getSupportedArchivesTypesUnpacking();
}

OpenBook::~OpenBook()
{
  delete la;
}

void
OpenBook::openBook(
    const UDBElement &book_search_result,
    const std::filesystem::path &unpacking_directory,
    std::function<void(const std::filesystem::path &)> open_call_back)
{
  if(!open_call_back.operator bool())
    {
      return void();
    }
  unpack_dir = unpacking_directory;
  call_count = 0;

  std::vector<UDBElement>::const_iterator it
      = std::find_if(book_search_result.subelements.begin(),
                     book_search_result.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::File;
                       });
  if(it == book_search_result.subelements.end())
    {
      return void();
    }
  std::filesystem::path p
      = std::u8string(it->content.begin(), it->content.end());

  it = std::find_if(book_search_result.subelements.begin(),
                    book_search_result.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Book;
                      });
  if(it == book_search_result.subelements.end())
    {
      return void();
    }
  std::vector<UDBElement>::const_iterator it_p
      = std::find_if(it->subelements.begin(), it->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  if(it_p == it->subelements.end())
    {
      openBook(p, open_call_back, UDBElement());
    }
  else
    {
      openBook(p, open_call_back, *it_p);
    }
}

void
OpenBook::openBook(
    const std::filesystem::path &p,
    std::function<void(const std::filesystem::path &)> open_call_back,
    const UDBElement &path)
{
  call_count++;
  std::string ext = mlbp->getExtension(p);
  ext = mlbp->stringToLower(ext);
  std::string find_str(".");
  std::string::size_type n = ext.find(find_str);
  if(n != std::string::npos)
    {
      ext.erase(0, n + find_str.size());
    }

  auto it
      = std::find(supported_archives.begin(), supported_archives.end(), ext);
  if(it == supported_archives.end())
    {
      if(call_count == 1)
        {
          open_call_back(p);
        }
      else
        {
          std::filesystem::path book_p = unpack_dir / p.filename();
          std::filesystem::create_directories(book_p.parent_path());
          std::filesystem::remove_all(book_p);
          std::filesystem::rename(p, book_p);
          open_call_back(book_p);
        }
    }
  else
    {
      std::unique_ptr<std::filesystem::path,
                      std::function<void(std::filesystem::path *)>>
          tmp_p(new std::filesystem::path(mlbp->tempDirPath()
                                          / mlbp->randomFileName()),
                [](std::filesystem::path *p)
                  {
                    std::filesystem::remove_all(*p);
                    delete p;
                  });
      if(bid.getId(path) == BaseID::PathInFile)
        {
          std::filesystem::path res;
          if(ext == "zip")
            {
              res = la->unpackZipFileToDirectory(p, path.content, *tmp_p);
            }
          else
            {
              res = la->unpackFileToDirectory(p, path.content, *tmp_p);
            }
          std::vector<UDBElement>::const_iterator it_p
              = std::find_if(path.subelements.begin(), path.subelements.end(),
                             [this](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::PathInFile;
                               });
          if(it_p == path.subelements.end())
            {
              openBook(res, open_call_back, UDBElement());
            }
          else
            {
              openBook(res, open_call_back, *it_p);
            }
        }
      else
        {
          if(call_count == 1)
            {
              open_call_back(p);
            }
          else
            {
              std::filesystem::path book_p = unpack_dir / p.filename();
              std::filesystem::create_directories(book_p.parent_path());
              std::filesystem::remove_all(book_p);
              std::filesystem::rename(p, book_p);
              open_call_back(book_p);
            }
        }
    }
}
