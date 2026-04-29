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

#include <RefreshCollection.h>
#include <RemoveBook.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>

RemoveBook::RemoveBook(const std::shared_ptr<MLBookProc> &mlbp)
    : LibArchive(mlbp)
{
}

void
RemoveBook::removeBook(const std::filesystem::path &base_path,
                       const UDBElement &book_search_result)
{
  auto it_fl = std::find_if(book_search_result.subelements.begin(),
                            book_search_result.subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::File;
                              });
  if(it_fl == book_search_result.subelements.end())
    {
      throw std::runtime_error(
          "RemoveBook::removeBook: cannot find file path");
    }

  std::filesystem::path book_fl
      = std::u8string(it_fl->content.begin(), it_fl->content.end());

  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });

  if(it_book == book_search_result.subelements.end())
    {
      throw std::runtime_error("RemoveBook::removeBook: not a book entry");
    }

  std::string ext = mlbp->getExtension(it_fl->content);
  ext = mlbp->stringToLower(ext);
  std::shared_ptr<RefreshCollection> refr(
      new RefreshCollection(mlbp, std::thread::hardware_concurrency()));
  refr->signal_parsing_progress = signal_parsing_progress;
  if(ext == ".rar")
    {
      std::filesystem::remove_all(book_fl);
      refr->refreshCollection(base_path);
    }
  else
    {
      std::vector<std::string> arch_type
          = std::move(mlbp->getSupportedArchivesTypesPacking());

      std::string find_str(".");
      std::string::size_type n = ext.find(find_str);
      if(n != std::string::npos)
        {
          ext.erase(n, find_str.size());
        }

      auto it_sup = std::find(arch_type.begin(), arch_type.end(), ext);
      if(it_sup == arch_type.end())
        {
          std::filesystem::remove_all(book_fl);
          refr->refreshCollection(base_path);
        }
      else
        {
          auto it_path = std::find_if(
              it_book->subelements.begin(), it_book->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::PathInFile;
                });
          if(it_path == it_book->subelements.end())
            {
              throw std::runtime_error(
                  "RemoveBook::removeBook: cannot find path in archive");
            }
          size_t count = removeFromArchive(*it_path, book_fl);
          if(count == 0)
            {
              std::filesystem::remove_all(book_fl);
            }
          refr->refreshCollection(base_path);
        }
    }
}

size_t
RemoveBook::removeFromArchive(const UDBElement &path,
                              const std::filesystem::path &archive_path)
{
  size_t result = 0;

  std::string fbd;
  auto it_fbd = std::find_if(path.subelements.begin(), path.subelements.end(),
                             [this](const UDBElement &el)
                               {
                                 return bid.getId(el) == BaseID::FBDPath;
                               });
  if(it_fbd != path.subelements.end())
    {
      fbd = it_fbd->content;
    }

  std::shared_ptr<LibArchiveFileData> fd_read(new LibArchiveFileData);
  fd_read->path = archive_path;
  fd_read->open_mode = std::ios_base::in | std::ios_base::binary;

  std::shared_ptr<archive> a_read = initForReading(fd_read);

  int er = archive_read_set_seek_callback(a_read.get(),
                                          &LibArchive::seekCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a_read, "RemoveBook::removeFromArchive:");
    }

  er = archive_read_open2(a_read.get(), fd_read.get(),
                          &LibArchive::openCallBack, &LibArchive::readCallBack,
                          &LibArchive::skipCallback,
                          &LibArchive::closeCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a_read, "RemoveBook::removeFromArchive:");
    }

  std::shared_ptr<LibArchiveFileData> fd_write(new LibArchiveFileData);
  fd_write->path = archive_path.parent_path() / mlbp->randomFileName();
  while(std::filesystem::exists(fd_write->path))
    {
      fd_write->path = archive_path.parent_path() / mlbp->randomFileName();
    }
  fd_write->open_mode = std::ios_base::out | std::ios_base::binary;

  std::shared_ptr<archive> a_write = initForWriting(fd_write);

  er = archive_write_set_format_filter_by_ext(a_write.get(),
                                              archive_path.string().c_str());
  if(er != ARCHIVE_OK)
    {
      archiveError(a_write, "RemoveBook::removeFromArchive:");
    }

  er = archive_write_open(
      a_write.get(), fd_write.get(), &LibArchive::openCallBack,
      &LibArchive::writeCallback, &LibArchive::closeCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a_write, "RemoveBook::removeFromArchive:");
    }

  int retry_count = 0;
  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      er = archive_read_next_header2(a_read.get(), e.get());
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a_read.get());
            std::string err;
            if(str)
              {
                err = std::string("LLibArchive::unpackFileToBuffer: \"") + str
                      + "\"";
              }
            else
              {
                err = std::string("LLibArchive::unpackFileToBuffer: ")
                      + std::strerror(archive_errno(a_read.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            const char *val = archive_entry_pathname_utf8(e.get());
            if(val)
              {
                std::string path_in_arch(val);
                if(path_in_arch == path.content)
                  {
                    auto it = std::find_if(
                        path.subelements.begin(), path.subelements.end(),
                        [this](const UDBElement &el)
                          {
                            return bid.getId(el) == BaseID::PathInFile;
                          });
                    if(it != path.subelements.end())
                      {
                        std::string ext = mlbp->getExtension(it->content);
                        ext = mlbp->stringToLower(ext);
                        if(ext != ".rar")
                          {
                            std::filesystem::path random
                                = archive_path.parent_path()
                                  / mlbp->randomFileName();
                            while(std::filesystem::exists(random))
                              {
                                random = archive_path.parent_path()
                                         / mlbp->randomFileName();
                              }
                            std::filesystem::create_directories(random);
                            std::shared_ptr<std::filesystem::path> tmp_p(
                                new std::filesystem::path(random),
                                [](std::filesystem::path *p)
                                  {
                                    std::filesystem::remove_all(*p);
                                    delete p;
                                  });
                            random = *tmp_p
                                     / std::filesystem::path(
                                           std::u8string(path.content.begin(),
                                                         path.content.end()))
                                           .filename();

                            unpackEntryToDirectory(a_read, e, random);

                            size_t res = removeFromArchive(*it, random);
                            if(res > 0)
                              {
                                std::filesystem::perms perms
                                    = std::filesystem::perms::none;
                                if(archive_entry_perm_is_set(e.get()))
                                  {
                                    perms = getPermissionsFromEntry(e);
                                  }
                                writeFile(a_write, random, path.content,
                                          perms);
                                result++;
                              }
                          }
                      }
                  }
                else if(path_in_arch != fbd)
                  {
                    std::string buf
                        = std::move(unpackEntryToBuffer(a_read, e));
                    writeBufferToArchive(a_write, e, buf);
                    result++;
                  }
              }
            break;
          }
        case ARCHIVE_EOF:
          {
            break;
          }
        case ARCHIVE_RETRY:
          {
            retry_count++;
            break;
          }
        default:
          {
            archiveError(a_read, "LibArchive::unpackToBuffer:");
            break;
          }
        }
      archive_entry_clear(e.get());
    }

  a_read.reset();
  a_write.reset();
  if(result > 0)
    {
      std::filesystem::remove_all(fd_read->path);
      std::filesystem::rename(fd_write->path, fd_read->path);
    }
  else
    {
      std::filesystem::remove_all(fd_write->path);
    }

  return result;
}