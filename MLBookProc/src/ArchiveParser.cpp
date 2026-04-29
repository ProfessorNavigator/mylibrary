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

#include <ArchiveParser.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <ODTParser.h>
#include <PDFParser.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <syncstream>
#include <thread>

ArchiveParser::ArchiveParser(
    const std::shared_ptr<MLBookProc> &mlbp,
    const std::shared_ptr<std::vector<std::tuple<unsigned int, bool>>>
        &thread_v,
    const std::shared_ptr<std::mutex> &thread_v_mtx,
    const std::shared_ptr<std::condition_variable> &thread_v_var)
    : LibArchive(mlbp)
{
  this->thread_v = thread_v;
  this->thread_v_mtx = thread_v_mtx;
  this->thread_v_var = thread_v_var;
  cancel.store(false, std::memory_order_relaxed);
}

ArchiveParser::~ArchiveParser()
{
  std::unique_lock<std::mutex> ullock(thr_num_mtx);
  thr_num_var.wait(ullock,
                   [this]
                     {
                       return thr_num <= 0;
                     });
}

std::vector<UDBElement>
ArchiveParser::parseArchive(const std::filesystem::path &file_path)
{
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->path = file_path;
  fd->open_mode = std::ios_base::in | std::ios_base::binary;

  std::shared_ptr<archive> a = initForReading(fd);

  int er = archive_read_set_seek_callback(a.get(), &LibArchive::seekCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a, "ArchiveParser::parseArchive:");
    }

  er = archive_read_open2(a.get(), fd.get(), &LibArchive::openCallBack,
                          &LibArchive::readCallBack, &LibArchive::skipCallback,
                          &LibArchive::closeCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a, "ArchiveParser::parseArchive:");
    }

  int retry_count = 0;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      if(cancel.load(std::memory_order_relaxed))
        {
          break;
        }

      std::shared_ptr<archive_entry> e(archive_entry_new(),
                                       [](archive_entry *e)
                                         {
                                           archive_entry_free(e);
                                         });
      er = archive_read_next_header2(a.get(), e.get());
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("ArchiveParser::parseArchive: \"") + str
                      + "\"";
              }
            else
              {
                err = std::string("ArchiveParser::parseArchive: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            parseEntry(a, e);
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
            result.clear();
            archiveError(a, "ArchiveParser::parseArchive:");
            break;
          }
        }
    }

  std::unique_lock<std::mutex> ullock(thr_num_mtx);
  thr_num_var.wait(ullock,
                   [this]
                     {
                       return thr_num <= 0;
                     });

  fbdProcessing();

  return result;
}

void
ArchiveParser::stopAll()
{
  cancel.store(true, std::memory_order_relaxed);
  if(arch_proc)
    {
      arch_proc->stopAll();
    }
}

void
ArchiveParser::parseEntry(std::shared_ptr<archive> a,
                          std::shared_ptr<archive_entry> e)
{
  const char *val = archive_entry_pathname_utf8(e.get());
  if(val == nullptr)
    {
      return void();
    }
  std::string arch_file_path(val);
  la_int64_t sz = 0;
  if(archive_entry_size_is_set(e.get()))
    {
      sz = archive_entry_size(e.get());
      if(sz < 0)
        {
          sz = 0;
        }
    }
  if(sz == 0)
    {
      return void();
    }

  std::string ext = mlbp->getExtension(arch_file_path);
  ext = mlbp->stringToLower(ext);
  std::unique_ptr<std::thread> thr;
  std::unique_lock<std::mutex> ullock(*thread_v_mtx, std::defer_lock);
  std::vector<std::tuple<unsigned, bool>>::iterator it_thr;
  if(ext == ".fb2")
    {
      std::string buf = unpackEntryToBuffer(a, e);
      ullock.lock();
      it_thr = std::find_if(thread_v->begin(), thread_v->end(),
                            [](const std::tuple<unsigned, bool> &el)
                              {
                                return std::get<1>(el);
                              });
      if(it_thr != thread_v->end())
        {
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          thr_num++;
          thr = std::unique_ptr<std::thread>(new std::thread(
              [this, buf, arch_file_path, e, it_thr]
                {
                  UDBElement book
                      = bufferParse(buf, arch_file_path, e, FileType::FB2);

                  std::scoped_lock lock(*thread_v_mtx, thr_num_mtx);
                  result.emplace_back(book);
                  thr_num--;
                  std::get<1>(*it_thr) = true;
                  thr_num_var.notify_one();
                  thread_v_var->notify_one();
                }));
        }
      else
        {
          ullock.unlock();
          UDBElement book = bufferParse(buf, arch_file_path, e, FileType::FB2);
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          result.emplace_back(book);
        }
    }
  else if(ext == ".epub")
    {
      std::string buf = unpackEntryToBuffer(a, e);

      ullock.lock();
      it_thr = std::find_if(thread_v->begin(), thread_v->end(),
                            [](const std::tuple<unsigned, bool> &el)
                              {
                                return std::get<1>(el);
                              });
      if(it_thr != thread_v->end())
        {
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          thr_num++;
          thr = std::unique_ptr<std::thread>(new std::thread(
              [this, buf, arch_file_path, e, it_thr]
                {
                  UDBElement book
                      = bufferParse(buf, arch_file_path, e, FileType::EPUB);

                  std::scoped_lock lock(*thread_v_mtx, thr_num_mtx);
                  result.emplace_back(book);
                  thr_num--;
                  std::get<1>(*it_thr) = true;
                  thr_num_var.notify_one();
                  thread_v_var->notify_one();
                }));
        }
      else
        {
          ullock.unlock();
          UDBElement book
              = bufferParse(buf, arch_file_path, e, FileType::EPUB);
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          result.emplace_back(book);
        }
    }
  else if(ext == ".pdf")
    {
      std::string buf = unpackEntryToBuffer(a, e);

      ullock.lock();
      it_thr = std::find_if(thread_v->begin(), thread_v->end(),
                            [](const std::tuple<unsigned, bool> &el)
                              {
                                return std::get<1>(el);
                              });
      if(it_thr != thread_v->end())
        {
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          thr_num++;
          thr = std::unique_ptr<std::thread>(new std::thread(
              [this, buf, arch_file_path, e, it_thr]
                {
                  UDBElement book
                      = bufferParse(buf, arch_file_path, e, FileType::PDF);

                  std::scoped_lock lock(*thread_v_mtx, thr_num_mtx);
                  result.emplace_back(book);
                  thr_num--;
                  std::get<1>(*it_thr) = true;
                  thr_num_var.notify_one();
                  thread_v_var->notify_one();
                }));
        }
      else
        {
          ullock.unlock();
          UDBElement book = bufferParse(buf, arch_file_path, e, FileType::PDF);
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          result.emplace_back(book);
        }
    }
  else if(ext == ".djvu")
    {
      std::string buf = unpackEntryToBuffer(a, e);

      ullock.lock();
      it_thr = std::find_if(thread_v->begin(), thread_v->end(),
                            [](const std::tuple<unsigned, bool> &el)
                              {
                                return std::get<1>(el);
                              });
      if(it_thr != thread_v->end())
        {
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          thr_num++;
          thr = std::unique_ptr<std::thread>(new std::thread(
              [this, buf, arch_file_path, e, it_thr]
                {
                  UDBElement book
                      = bufferParse(buf, arch_file_path, e, FileType::DJVU);

                  std::scoped_lock lock(*thread_v_mtx, thr_num_mtx);
                  result.emplace_back(book);
                  thr_num--;
                  std::get<1>(*it_thr) = true;
                  thr_num_var.notify_one();
                  thread_v_var->notify_one();
                }));
        }
      else
        {
          ullock.unlock();
          UDBElement book
              = bufferParse(buf, arch_file_path, e, FileType::DJVU);
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          result.emplace_back(book);
        }
    }
  else if(ext == ".odt")
    {
      std::string buf = unpackEntryToBuffer(a, e);

      ullock.lock();
      it_thr = std::find_if(thread_v->begin(), thread_v->end(),
                            [](const std::tuple<unsigned, bool> &el)
                              {
                                return std::get<1>(el);
                              });
      if(it_thr != thread_v->end())
        {
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          thr_num++;
          thr = std::unique_ptr<std::thread>(new std::thread(
              [this, buf, arch_file_path, e, it_thr]
                {
                  UDBElement book
                      = bufferParse(buf, arch_file_path, e, FileType::ODT);

                  std::scoped_lock lock(*thread_v_mtx, thr_num_mtx);
                  result.emplace_back(book);
                  thr_num--;
                  std::get<1>(*it_thr) = true;
                  thr_num_var.notify_one();
                  thread_v_var->notify_one();
                }));
        }
      else
        {
          ullock.unlock();
          UDBElement book = bufferParse(buf, arch_file_path, e, FileType::ODT);
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          result.emplace_back(book);
        }
    }
  else if(ext == ".txt" || ext == ".md")
    {
      UDBElement book;
      bid.setId(book, BaseID::Book);

      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str
          = std::filesystem::path(
                std::u8string(arch_file_path.begin(), arch_file_path.end()))
                .stem()
                .u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);

      el = UDBElement();
      bid.setId(el, BaseID::Date);
      if(archive_entry_atime_is_set(e.get()))
        {
          time_t tmt = archive_entry_atime(e.get());
          el.content = mlbp->timeToDate(tmt);
          book.subelements.emplace_back(el);
        }

      el = UDBElement();
      bid.setId(el, BaseID::PathInFile);
      el.content = arch_file_path;
      book.subelements.emplace_back(el);

      std::lock_guard<std::mutex> lglock(thr_num_mtx);
      result.emplace_back(book);
    }
  else if(ext == ".fbd")
    {
      std::string buf = unpackEntryToBuffer(a, e);

      ullock.lock();
      it_thr = std::find_if(thread_v->begin(), thread_v->end(),
                            [](const std::tuple<unsigned, bool> &el)
                              {
                                return std::get<1>(el);
                              });
      if(it_thr != thread_v->end())
        {
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          thr_num++;
          thr = std::unique_ptr<std::thread>(new std::thread(
              [this, buf, arch_file_path, e, it_thr]
                {
                  UDBElement book
                      = bufferParse(buf, arch_file_path, e, FileType::FB2);

                  std::scoped_lock lock(*thread_v_mtx, thr_num_mtx);
                  fbd.emplace_back(book);
                  thr_num--;
                  std::get<1>(*it_thr) = true;
                  thr_num_var.notify_one();
                  thread_v_var->notify_one();
                }));
        }
      else
        {
          ullock.unlock();
          UDBElement book = bufferParse(buf, arch_file_path, e, FileType::FB2);
          std::lock_guard<std::mutex> lglock(thr_num_mtx);
          fbd.emplace_back(book);
        }
    }
  else
    {
      if(mlbp->ifSupportedFile(arch_file_path))
        {
          std::filesystem::path tmp_dir
              = mlbp->tempDirPath() / mlbp->randomFileName();
          std::filesystem::create_directories(tmp_dir);
          std::filesystem::path arch_path
              = tmp_dir
                / std::filesystem::path(std::u8string(arch_file_path.begin(),
                                                      arch_file_path.end()));

          try
            {
              unpackEntryToDirectory(a, e, arch_path);
            }
          catch(std::exception &er)
            {
              std::osyncstream(std::cout) << "ArchiveParser::parseEntry: \""
                                          << er.what() << "\"" << std::endl;

              std::filesystem::remove_all(tmp_dir);
              return void();
            }

          std::vector<UDBElement> books;
          arch_proc = std::shared_ptr<ArchiveParser>(
              new ArchiveParser(mlbp, thread_v, thread_v_mtx, thread_v_var),
              [tmp_dir](ArchiveParser *parser)
                {
                  delete parser;
                  std::filesystem::remove_all(tmp_dir);
                });
          try
            {
              books = arch_proc->parseArchive(arch_path);
            }
          catch(std::exception &er)
            {
              std::osyncstream(std::cout) << "ArchiveParser::parseEntry: \""
                                          << er.what() << "\"" << std::endl;
            }

          for(auto it = books.begin(); it != books.end(); it++)
            {
              auto it_s = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_s == it->subelements.end())
                {
                  continue;
                }
              UDBElement el;
              bid.setId(el, BaseID::PathInFile);
              el.content = arch_file_path;
              el.subelements.push_back(*it_s);
              *it_s = el;

              std::lock_guard<std::mutex> lglock(thr_num_mtx);
              result.push_back(*it);
            }
        }
      else
        {
          unsupported.push_back(arch_file_path);
        }
    }
  if(it_thr != thread_v->end() && ullock && thr)
    {
      std::get<1>(*it_thr) = false;
#ifdef __linux
      cpu_set_t cpu;
      CPU_ZERO(&cpu);
      CPU_SET(std::get<0>(*it_thr), &cpu);
      int er = pthread_setaffinity_np(thr->native_handle(), sizeof(cpu_set_t),
                                      &cpu);
      if(er)
        {
          std::osyncstream(std::cout)
              << "ArchiveParser::parseEntry: \"" << std::strerror(er) << "\""
              << std::endl;
        }
#elif defined(_WIN32)
      GROUP_AFFINITY gaf{};
      gaf.Group = std::get<0>(*it_thr) / (sizeof(KAFFINITY) * CHAR_BIT);
      gaf.Mask = (1 << std::get<0>(*it_thr) % (sizeof(KAFFINITY) * CHAR_BIT));
      HANDLE handle = pthread_gethandle(thr->native_handle());
      if(handle != nullptr)
        {
          if(SetThreadGroupAffinity(handle, &gaf, nullptr) == 0)
            {
              std::osyncstream(std::cout)
                  << "ArchiveParser::parseEntry "
                     "SetThreadAffinityMask: \""
                  << std::strerror(GetLastError()) << "\"" << std::endl;
            }
        }
      else
        {
          std::osyncstream(std::cout)
              << "ArchiveParser::parseEntry: handle is null!" << std::endl;
        }
#endif
      thr->detach();
    }
}

UDBElement
ArchiveParser::bufferParse(const std::string &buf,
                           const std::string &arch_file_path,
                           std::shared_ptr<archive_entry> e,
                           const FileType &ft)
{
  UDBElement book;
  try
    {
      switch(ft)
        {
        case FileType::FB2:
          {
            FB2Parser parser(mlbp);
            book = parser.parseBook(buf);
            break;
          }
        case FileType::EPUB:
          {
            EPUBParser parser(mlbp);
            book = parser.parseBook(buf);
            break;
          }
        case FileType::PDF:
          {
            PDFParser parser(mlbp);
            book = parser.parseBook(buf);
            break;
          }
        case FileType::DJVU:
          {
            DJVUParser parser(mlbp);
            book = parser.parseBook(buf);
            break;
          }
        case FileType::ODT:
          {
            ODTParser parser(mlbp);
            book = parser.parseBook(buf);
            break;
          }
        default:
          break;
        }
    }
  catch(std::exception &er)
    {
      std::osyncstream(std::cout)
          << "ArchiveParser::bufferParse: " << er.what() << " "
          << arch_file_path << std::endl;
    }

  std::vector<UDBElement>::iterator it
      = std::find_if(book.subelements.begin(), book.subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::BookTitle;
                       });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          std::u8string u8str
              = std::filesystem::path(std::u8string(arch_file_path.begin(),
                                                    arch_file_path.end()))
                    .stem()
                    .u8string();
          it->content = std::string(u8str.begin(), u8str.end());
        }
    }
  else
    {
      UDBElement el;
      bid.setId(el, BaseID::BookTitle);
      std::u8string u8str
          = std::filesystem::path(
                std::u8string(arch_file_path.begin(), arch_file_path.end()))
                .stem()
                .u8string();
      el.content = std::string(u8str.begin(), u8str.end());
      book.subelements.emplace_back(el);
    }

  it = std::find_if(book.subelements.begin(), book.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Date;
                      });
  if(it != book.subelements.end())
    {
      if(it->content.empty())
        {
          if(archive_entry_atime_is_set(e.get()))
            {
              time_t tmt = archive_entry_atime(e.get());
              it->content = mlbp->timeToDate(tmt);
            }
        }
    }
  else
    {
      if(archive_entry_atime_is_set(e.get()))
        {
          UDBElement el;
          bid.setId(el, BaseID::Date);
          time_t tmt = archive_entry_atime(e.get());
          el.content = mlbp->timeToDate(tmt);
          book.subelements.emplace_back(el);
        }
    }

  UDBElement path;
  bid.setId(path, BaseID::PathInFile);
  path.content = arch_file_path;
  book.subelements.emplace_back(path);

  return book;
}

void
ArchiveParser::fbdProcessing()
{
  if(cancel.load(std::memory_order_relaxed))
    {
      return void();
    }

  for(auto it = fbd.begin(); it != fbd.end();)
    {
      auto it_fbd_path
          = std::find_if(it->subelements.begin(), it->subelements.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::PathInFile;
                           });
      if(it_fbd_path == it->subelements.end())
        {
          fbd.erase(it);
          continue;
        }

      std::filesystem::path fbd_p = std::u8string(it_fbd_path->content.begin(),
                                                  it_fbd_path->content.end());
      if(fbd_p.has_extension())
        {
          fbd_p.replace_extension("");
        }
      std::vector<UDBElement>::iterator it_result;
      std::vector<UDBElement>::iterator it_result_path;
      for(it_result = result.begin(); it_result != result.end(); it_result++)
        {
          it_result_path = std::find_if(
              it_result->subelements.begin(), it_result->subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::PathInFile;
                });
          if(it_result_path == it_result->subelements.end())
            {
              continue;
            }
          std::filesystem::path result_p = std::u8string(
              it_result_path->content.begin(), it_result_path->content.end());
          if(result_p.has_extension())
            {
              result_p.replace_extension("");
            }
          if(result_p == fbd_p)
            {
              break;
            }
        }
      if(it_result != result.end())
        {
          UDBElement el;
          bid.setId(el, BaseID::FBDPath);
          el.content = it_fbd_path->content;
          it_fbd_path->subelements.emplace_back(el);
          it_fbd_path->content = it_result_path->content;
          *it_result = *it;
          fbd.erase(it);
          continue;
        }

      std::vector<std::string>::iterator it_unsup
          = std::find_if(unsupported.begin(), unsupported.end(),
                         [fbd_p](const std::string &el)
                           {
                             std::filesystem::path p
                                 = std::u8string(el.begin(), el.end());
                             if(p.has_extension())
                               {
                                 p.replace_extension(u8"");
                               }
                             return fbd_p == p;
                           });
      if(it_unsup != unsupported.end())
        {
          UDBElement el;
          bid.setId(el, BaseID::FBDPath);
          el.content = it_fbd_path->content;
          it_fbd_path->subelements.emplace_back(el);
          it_fbd_path->content = *it_unsup;
          result.push_back(*it);
          fbd.erase(it);
          unsupported.erase(it_unsup);
          continue;
        }

      it++;
    }
}
