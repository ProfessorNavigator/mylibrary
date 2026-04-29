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

#include <ByteOrder.h>
#include <LibArchive.h>
#include <XMLTextEncoding.h>
#include <algorithm>
#include <archive_entry.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>

LibArchive::LibArchive(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
}

LibArchive::~LibArchive()
{
}

void
LibArchive::listFilesInArchive(
    const std::filesystem::path &archive_path,
    std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result)
{
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->path = archive_path;
  fd->open_mode = std::ios_base::in | std::ios_base::binary;

  std::shared_ptr<archive> a = initForReading(fd);

  int er = archive_read_set_seek_callback(a.get(), &LibArchive::seekCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::listFilesInArchive:");
    }

  er = archive_read_open2(a.get(), fd.get(), &LibArchive::openCallBack,
                          &LibArchive::readCallBack, &LibArchive::skipCallback,
                          &LibArchive::closeCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::listFilesInArchive:");
    }

  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });
  int retry_count = 0;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      er = archive_read_next_header2(a.get(), e.get());
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("LibArchive::listFilesInArchive: \"") + str
                      + "\"";
              }
            else
              {
                err = std::string("LibArchive::listFilesInArchive: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            const char *val = archive_entry_pathname_utf8(e.get());
            la_int64_t sz = 0;
            if(archive_entry_size_is_set(e.get()))
              {
                sz = archive_entry_size(e.get());
                if(sz < 0)
                  {
                    sz = 0;
                  }
              }
            if(val)
              {
                result.push_back(std::make_tuple(
                    std::string(val), static_cast<uint64_t>(sz), 0));
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
            result.clear();
            archiveError(a, "LibArchive::listFilesInArchive:");
            break;
          }
        }
      archive_entry_clear(e.get());
    }
}

void
LibArchive::listFilesInArchiveBuffer(
    const std::string &buffer,
    std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result)
{
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->source_buffer = buffer;

  std::shared_ptr<archive> a = initForReading(fd);

  int er = archive_read_set_seek_callback(a.get(), &LibArchive::seekCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::listFilesInArchiveBuffer:");
    }

  er = archive_read_open2(a.get(), fd.get(), &LibArchive::openCallBack,
                          &LibArchive::readCallBack, &LibArchive::skipCallback,
                          &LibArchive::closeCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::listFilesInArchiveBuffer:");
    }

  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });
  int retry_count = 0;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      er = archive_read_next_header2(a.get(), e.get());
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("LibArchive::listFilesInArchiveBuffer: \"")
                      + str + "\"";
              }
            else
              {
                err = std::string("LibArchive::listFilesInArchiveBuffer: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            const char *val = archive_entry_pathname_utf8(e.get());
            la_int64_t sz = 0;
            if(archive_entry_size_is_set(e.get()))
              {
                sz = archive_entry_size(e.get());
                if(sz < 0)
                  {
                    sz = 0;
                  }
              }
            if(val)
              {
                result.push_back(std::make_tuple(
                    std::string(val), static_cast<uint64_t>(sz), 0));
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
            result.clear();
            archiveError(a, "LibArchive::listFilesInArchiveBuffer:");
            break;
          }
        }
      archive_entry_clear(e.get());
    }
}

void
LibArchive::listFilesInZip(
    const std::filesystem::path &archive_path,
    std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result)
{
  std::shared_ptr<std::fstream> f(new std::fstream,
                                  [](std::fstream *f)
                                    {
                                      delete f;
                                    });
  f->open(archive_path, std::ios_base::in | std::ios_base::binary);
  if(!f->is_open())
    {
      std::string err = "LibArchive::listFilesInZip: cannot open file ";
      std::u8string u8str = archive_path.u8string();
      err += std::string(u8str.begin(), u8str.end());
      throw std::runtime_error(err);
    }

  f->seekg(0, std::ios_base::end);
  uint64_t fsz = static_cast<uint64_t>(f->tellg());

  std::string central_directory;
  try
    {
      getCentralDirectory(f, fsz, central_directory);

      parseCentralDirectory(central_directory, result);
    }
  catch(std::exception &er)
    {
      f.reset();
      std::cout << er.what() << std::endl;
      result.clear();
      listFilesInArchive(archive_path, result);
    }
}

void
LibArchive::listFilesInZipBuffer(
    const std::string &buffer,
    std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result)
{
  std::shared_ptr<std::stringstream> str(new std::stringstream,
                                         [](std::stringstream *str)
                                           {
                                             delete str;
                                           });
  str->str(buffer);

  str->seekg(0, std::ios_base::end);
  uint64_t fsz = static_cast<uint64_t>(str->tellg());

  std::string central_directory;
  try
    {
      getCentralDirectory(str, fsz, central_directory);

      parseCentralDirectory(central_directory, result);
    }
  catch(std::exception &er)
    {
      str.reset();
      std::cout << er.what() << std::endl;
      result.clear();
      listFilesInArchiveBuffer(buffer, result);
    }
}

std::filesystem::path
LibArchive::unpackFileToDirectory(const std::filesystem::path &archive_path,
                                  const std::string &filename,
                                  const std::filesystem::path &directory,
                                  const size_t &offset)
{
  std::filesystem::path result;
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->path = archive_path;
  fd->open_mode = std::ios_base::in | std::ios_base::binary;
  fd->start_offset = offset;

  result = unpackToDirectory(fd, filename, directory);

  return result;
}

std::filesystem::path
LibArchive::unpackBufferFileToDirectory(const std::string &buffer,
                                        const std::string &filename,
                                        const std::filesystem::path &directory,
                                        const size_t &offset)
{
  std::filesystem::path result;
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->source_buffer = buffer;
  fd->start_offset = offset;

  result = unpackToDirectory(fd, filename, directory);

  return result;
}

std::filesystem::path
LibArchive::unpackZipFileToDirectory(const std::filesystem::path &archive_path,
                                     const std::string &filename,
                                     const std::filesystem::path &directory)
{
  std::filesystem::path result;

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> list;
  listFilesInZip(archive_path, list);
  auto it_res = std::find_if(
      list.begin(), list.end(),
      [filename](const std::tuple<std::string, uint64_t, uint64_t> &el)
        {
          return std::get<0>(el) == filename;
        });

  if(it_res != list.end())
    {
      result = unpackFileToDirectory(archive_path, filename, directory,
                                     std::get<2>(*it_res));
    }
  else
    {
      result = unpackFileToDirectory(archive_path, filename, directory);
    }

  return result;
}

std::filesystem::path
LibArchive::unpackZipBufferFileToDirectory(
    const std::string &buffer, const std::string &filename,
    const std::filesystem::path &directory)
{
  std::filesystem::path result;

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> list;
  listFilesInZipBuffer(buffer, list);
  auto it_res = std::find_if(
      list.begin(), list.end(),
      [filename](const std::tuple<std::string, uint64_t, uint64_t> &el)
        {
          return std::get<0>(el) == filename;
        });
  if(it_res != list.end())
    {
      result = unpackBufferFileToDirectory(buffer, filename, directory,
                                           std::get<2>(*it_res));
    }

  return result;
}

std::string
LibArchive::unpackFileToBuffer(const std::filesystem::path &archive_path,
                               const std::string &filename,
                               const size_t &offset)
{
  std::string result;
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->path = archive_path;
  fd->open_mode = std::ios_base::in | std::ios_base::binary;
  fd->start_offset = offset;

  unpackToBuffer(fd, filename, result);

  return result;
}

std::string
LibArchive::unpackBufferFileToBuffer(const std::string &buffer,
                                     const std::string &filename,
                                     const size_t &offset)
{
  std::string result;
  std::shared_ptr<LibArchiveFileData> fd(new LibArchiveFileData);
  fd->source_buffer = buffer;
  fd->start_offset = offset;

  unpackToBuffer(fd, filename, result);

  return result;
}

std::string
LibArchive::unpackZipFileToBuffer(const std::filesystem::path &archive_path,
                                  const std::string &filename)
{
  std::string result;

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> list;
  listFilesInZip(archive_path, list);
  auto it_res = std::find_if(
      list.begin(), list.end(),
      [filename](const std::tuple<std::string, uint64_t, uint64_t> &el)
        {
          return std::get<0>(el) == filename;
        });

  if(it_res != list.end())
    {
      result
          = unpackFileToBuffer(archive_path, filename, std::get<2>(*it_res));
    }

  return result;
}

std::string
LibArchive::unpackZipBufferFileToBuffer(const std::string &buffer,
                                        const std::string &filename)
{
  std::string result;

  std::vector<std::tuple<std::string, uint64_t, uint64_t>> list;
  listFilesInZipBuffer(buffer, list);
  auto it_res = std::find_if(
      list.begin(), list.end(),
      [filename](const std::tuple<std::string, uint64_t, uint64_t> &el)
        {
          return std::get<0>(el) == filename;
        });

  if(it_res != list.end())
    {
      result
          = unpackBufferFileToBuffer(buffer, filename, std::get<2>(*it_res));
    }

  return result;
}

void
LibArchive::writeToArchive(const std::filesystem::path &source_object,
                           const std::filesystem::path &archive_path,
                           std::string name_in_archive,
                           const std::filesystem::perms &perms,
                           const bool &overwrite_existing)
{
  if(!std::filesystem::exists(source_object))
    {
      throw std::runtime_error(
          "LibArchive::writeToArchive: source object does not exist!");
    }
  if(name_in_archive.empty())
    {
      std::u8string u8str = source_object.filename().u8string();
      name_in_archive = std::string(u8str.begin(), u8str.end());
    }

  if(std::filesystem::path::preferred_separator == '\\')
    {
      for(auto it = name_in_archive.begin(); it != name_in_archive.end(); it++)
        {
          if(*it == std::filesystem::path::preferred_separator)
            {
              *it = '/';
            }
        }
    }

  if(overwrite_existing)
    {
      std::filesystem::remove_all(archive_path);
      std::filesystem::create_directories(archive_path.parent_path());
    }

  bool exist = std::filesystem::exists(archive_path);
  std::shared_ptr<LibArchiveFileData> fd_write(new LibArchiveFileData);
  fd_write->path = archive_path;
  if(exist)
    {
      while(std::filesystem::exists(fd_write->path))
        {
          fd_write->path = archive_path.parent_path() / mlbp->randomFileName();
        }
    }
  fd_write->open_mode = std::ios_base::out | std::ios_base::binary;

  std::shared_ptr<archive> a_write = initForWriting(fd_write);

  int er = archive_write_set_format_filter_by_ext(
      a_write.get(), archive_path.string().c_str());
  if(er != ARCHIVE_OK)
    {
      archiveError(a_write, "LibArchive::writeToArchive:");
    }

  er = archive_write_open(
      a_write.get(), fd_write.get(), &LibArchive::openCallBack,
      &LibArchive::writeCallback, &LibArchive::closeCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a_write, "LibArchive::writeToArchive:");
    }

  std::vector<std::string> names;
  if(exist)
    {
      std::shared_ptr<LibArchiveFileData> fd_read(new LibArchiveFileData);
      fd_read->path = archive_path;
      fd_read->open_mode = std::ios_base::in | std::ios_base::binary;

      std::shared_ptr<archive> a_read = initForReading(fd_read);
      er = archive_read_set_seek_callback(a_read.get(),
                                          &LibArchive::seekCallback);
      if(er != ARCHIVE_OK)
        {
          archiveError(a_read, "LibArchive::writeToArchive:");
        }

      er = archive_read_open2(
          a_read.get(), fd_read.get(), &LibArchive::openCallBack,
          &LibArchive::readCallBack, &LibArchive::skipCallback,
          &LibArchive::closeCallback);

      if(er != ARCHIVE_OK)
        {
          archiveError(a_read, "LibArchive::writeToArchive:");
        }

      std::shared_ptr<archive_entry> e(archive_entry_new(),
                                       [](archive_entry *e)
                                         {
                                           archive_entry_free(e);
                                         });
      int retry_count = 0;
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
                    err = std::string("LibArchive::writeToArchive: \"") + str
                          + "\"";
                  }
                else
                  {
                    err = std::string("LibArchive::writeToArchive: ")
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
                    std::string p_in_arch(val);
                    if(p_in_arch == name_in_archive)
                      {
                        a_write.reset();
                        fd_write->f.reset();
                        std::filesystem::remove_all(fd_write->path);
                        throw std::runtime_error("LibArchive::writeToArchive: "
                                                 "file already in archive!");
                      }
                    names.emplace_back(p_in_arch);
                  }

                std::string buf = unpackEntryToBuffer(a_read, e);
                if(buf.size() > 0)
                  {
                    writeBufferToArchive(a_write, e, buf);
                  }
                else
                  {
                    er = archive_write_header(a_write.get(), e.get());
                    if(er != ARCHIVE_OK)
                      {
                        archiveError(a_write, "LibArchive::writeToArchive:");
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
                archiveError(a_read, "LibArchive::writeToArchive:");
                break;
              }
            }
          archive_entry_clear(e.get());
        }
    }

  std::filesystem::file_status status
      = std::filesystem::symlink_status(source_object);
  switch(status.type())
    {
    case std::filesystem::file_type::directory:
      {
        for(auto &dir_it :
            std::filesystem::recursive_directory_iterator(source_object))
          {
            std::filesystem::path p = dir_it.path();
            status = std::filesystem::symlink_status(p);
            switch(status.type())
              {
              case std::filesystem::file_type::regular:
                {
                  std::filesystem::path relative
                      = p.lexically_relative(source_object);
                  relative
                      = std::filesystem::path(std::u8string(
                            name_in_archive.begin(), name_in_archive.end()))
                        / relative;
                  std::u8string u8str = relative.u8string();
                  auto it = std::find(names.begin(), names.end(),
                                      std::string(u8str.begin(), u8str.end()));
                  if(it != names.end())
                    {
                      a_write.reset();
                      fd_write->f.reset();
                      std::filesystem::remove_all(fd_write->path);
                      throw std::runtime_error("LibArchive::writeToArchive: "
                                               "file already in archive!");
                    }
                  writeFile(a_write, p,
                            std::string(u8str.begin(), u8str.end()), perms);
                  break;
                }
              case std::filesystem::file_type::symlink:
                {
                  std::filesystem::path relative
                      = p.lexically_relative(source_object);
                  relative
                      = std::filesystem::path(std::u8string(
                            name_in_archive.begin(), name_in_archive.end()))
                        / relative;
                  std::vector<
                      std::tuple<std::filesystem::path, std::filesystem::path>>
                      paths = symlinkWriteResolver(relative, p);
                  for(auto it = paths.begin(); it != paths.end(); it++)
                    {
                      std::u8string u8str = std::get<0>(*it).u8string();
                      auto it_n
                          = std::find(names.begin(), names.end(),
                                      std::string(u8str.begin(), u8str.end()));
                      if(it_n != names.end())
                        {
                          a_write.reset();
                          fd_write->f.reset();
                          std::filesystem::remove_all(fd_write->path);
                          throw std::runtime_error(
                              "LibArchive::writeToArchive: "
                              "file already in archive!");
                        }
                      writeFile(a_write, std::get<1>(*it),
                                std::string(u8str.begin(), u8str.end()),
                                perms);
                    }
                  break;
                }
              default:
                break;
              }
          }
        break;
      }
    case std::filesystem::file_type::regular:
      {
        auto it_n = std::find(names.begin(), names.end(), name_in_archive);
        if(it_n != names.end())
          {
            a_write.reset();
            fd_write->f.reset();
            std::filesystem::remove_all(fd_write->path);
            throw std::runtime_error("LibArchive::writeToArchive: "
                                     "file already in archive!");
          }
        writeFile(a_write, source_object, name_in_archive, perms);
        break;
      }
    case std::filesystem::file_type::symlink:
      {
        std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
            paths = symlinkWriteResolver(
                std::filesystem::path(std::u8string(name_in_archive.begin(),
                                                    name_in_archive.end())),
                source_object);
        for(auto it = paths.begin(); it != paths.end(); it++)
          {
            std::u8string u8str = std::get<0>(*it).u8string();
            auto it_n = std::find(names.begin(), names.end(),
                                  std::string(u8str.begin(), u8str.end()));
            if(it_n != names.end())
              {
                a_write.reset();
                fd_write->f.reset();
                std::filesystem::remove_all(fd_write->path);
                throw std::runtime_error("LibArchive::writeToArchive: "
                                         "file already in archive!");
              }
            writeFile(a_write, std::get<1>(*it),
                      std::string(u8str.begin(), u8str.end()), perms);
          }
        break;
      }
    default:
      break;
    }
  if(exist)
    {
      a_write.reset();
      fd_write->f.reset();
      std::filesystem::remove_all(archive_path);
      std::filesystem::rename(fd_write->path, archive_path);
    }
}

void
LibArchive::writeBufferObjectToArchive(
    const std::string &buffer, const std::filesystem::path &archive_path,
    std::string name_in_archive, const std::filesystem::perms &perms,
    const bool &overwrite_existing)
{
  if(name_in_archive.empty())
    {
      throw std::runtime_error(
          "LibArchive::writeBufferToArchive: name_in_archive is empty!");
    }

  if(std::filesystem::path::preferred_separator == '\\')
    {
      for(auto it = name_in_archive.begin(); it != name_in_archive.end(); it++)
        {
          if(*it == std::filesystem::path::preferred_separator)
            {
              *it = '/';
            }
        }
    }

  if(overwrite_existing)
    {
      std::filesystem::remove_all(archive_path);
      std::filesystem::create_directories(archive_path.parent_path());
    }

  bool exist = std::filesystem::exists(archive_path);
  std::shared_ptr<LibArchiveFileData> fd_write(new LibArchiveFileData);
  fd_write->path = archive_path;
  if(exist)
    {
      while(std::filesystem::exists(fd_write->path))
        {
          fd_write->path = archive_path.parent_path() / mlbp->randomFileName();
        }
    }
  fd_write->open_mode = std::ios_base::out | std::ios_base::binary;

  std::shared_ptr<archive> a_write = initForWriting(fd_write);

  int er = archive_write_set_format_filter_by_ext(
      a_write.get(), archive_path.string().c_str());
  if(er != ARCHIVE_OK)
    {
      archiveError(a_write, "LibArchive::writeBufferToArchive:");
    }

  er = archive_write_open(
      a_write.get(), fd_write.get(), &LibArchive::openCallBack,
      &LibArchive::writeCallback, &LibArchive::closeCallback);
  if(er != ARCHIVE_OK)
    {
      archiveError(a_write, "LibArchive::writeBufferToArchive:");
    }

  std::vector<std::string> names;
  if(exist)
    {
      std::shared_ptr<LibArchiveFileData> fd_read(new LibArchiveFileData);
      fd_read->path = archive_path;
      fd_read->open_mode = std::ios_base::in | std::ios_base::binary;

      std::shared_ptr<archive> a_read = initForReading(fd_read);
      er = archive_read_set_seek_callback(a_read.get(),
                                          &LibArchive::seekCallback);
      if(er != ARCHIVE_OK)
        {
          archiveError(a_read, "LibArchive::writeBufferToArchive:");
        }

      er = archive_read_open2(
          a_read.get(), fd_read.get(), &LibArchive::openCallBack,
          &LibArchive::readCallBack, &LibArchive::skipCallback,
          &LibArchive::closeCallback);

      if(er != ARCHIVE_OK)
        {
          archiveError(a_read, "LibArchive::writeBufferToArchive:");
        }

      std::shared_ptr<archive_entry> e(archive_entry_new(),
                                       [](archive_entry *e)
                                         {
                                           archive_entry_free(e);
                                         });
      int retry_count = 0;
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
                    err = std::string("LibArchive::writeBufferToArchive: \"")
                          + str + "\"";
                  }
                else
                  {
                    err = std::string("LibArchive::writeBufferToArchive: ")
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
                    std::string p_in_arch(val);
                    if(p_in_arch == name_in_archive)
                      {
                        a_write.reset();
                        fd_write->f.reset();
                        std::filesystem::remove_all(fd_write->path);
                        throw std::runtime_error(
                            "LibArchive::writeBufferToArchive: "
                            "file already in archive!");
                      }
                    names.emplace_back(p_in_arch);
                  }

                std::string buf = unpackEntryToBuffer(a_read, e);
                if(buf.size() > 0)
                  {
                    writeBufferToArchive(a_write, e, buf);
                  }
                else
                  {
                    er = archive_write_header(a_write.get(), e.get());
                    if(er != ARCHIVE_OK)
                      {
                        archiveError(a_write,
                                     "LibArchive::writeBufferToArchive:");
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
                archiveError(a_read, "LibArchive::writeBufferToArchive:");
                break;
              }
            }
          archive_entry_clear(e.get());
        }
    }

  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });

  if(std::filesystem::path::preferred_separator == '\\')
    {
      for(auto it = name_in_archive.begin(); it != name_in_archive.end(); it++)
        {
          if(*it == std::filesystem::path::preferred_separator)
            {
              *it = '/';
            }
        }
    }

  archive_entry_set_pathname_utf8(e.get(), name_in_archive.c_str());

  archive_entry_set_perm(e.get(), static_cast<__LA_MODE_T>(perms));

  archive_entry_set_filetype(e.get(), AE_IFREG);

  archive_entry_set_size(e.get(), static_cast<la_int64_t>(buffer.size()));

  auto sytem_clock_tp = std::chrono::system_clock::now();
  time_t lwt = std::chrono::system_clock::to_time_t(sytem_clock_tp);
  archive_entry_set_mtime(e.get(), lwt, 0);

  writeBufferToArchive(a_write, e, buffer);

  if(exist)
    {
      a_write.reset();
      fd_write->f.reset();
      std::filesystem::remove_all(archive_path);
      std::filesystem::rename(fd_write->path, archive_path);
    }
}

std::shared_ptr<archive>
LibArchive::initForReading(const std::shared_ptr<LibArchiveFileData> &fd)
{
  std::shared_ptr<archive> result(archive_read_new(),
                                  [fd](archive *a)
                                    {
                                      archive_read_free(a);
                                    });
  int er = archive_read_support_filter_all(result.get());
  if(er != ARCHIVE_OK)
    {
      archiveError(result, "LibArchive::openForReading:");
    }

  er = archive_read_support_format_all(result.get());
  if(er != ARCHIVE_OK)
    {
      archiveError(result, "LibArchive::openForReading:");
    }

  return result;
}

std::shared_ptr<archive>
LibArchive::initForWriting(const std::shared_ptr<LibArchiveFileData> &fd)
{
  std::shared_ptr<archive> result
      = std::shared_ptr<archive>(archive_write_new(),
                                 [](archive *a)
                                   {
                                     archive_free(a);
                                   });

  return result;
}

void
LibArchive::archiveError(const std::shared_ptr<archive> &a,
                         const std::string &message)
{
  const char *str = archive_error_string(a.get());
  if(str)
    {
      std::string err = message + " \"" + str + "\"";
      throw std::runtime_error(err);
    }
  else
    {
      std::string err = message + " " + std::strerror(archive_errno(a.get()));
      throw std::runtime_error(err);
    }
}

int
LibArchive::openCallBack(archive *a, void *client_data)
{
  int result = ARCHIVE_FATAL;

  LibArchiveFileData *fd = reinterpret_cast<LibArchiveFileData *>(client_data);
  if(fd->source_buffer.empty())
    {
      std::shared_ptr<std::fstream> f(new std::fstream);
      f->open(fd->path, fd->open_mode);
      if(f->is_open())
        {
          f->seekg(0, std::ios_base::end);
          fd->file_size = static_cast<size_t>(f->tellg());
          f->seekg(fd->start_offset, std::ios_base::beg);
          fd->f = f;
          result = ARCHIVE_OK;
        }
      else
        {
          archive_set_error(a, ENOENT, "%s", "File has not been opened!");
        }
    }
  else
    {
      std::shared_ptr<std::stringstream> strm(new std::stringstream);
      strm->str(fd->source_buffer);
      strm->seekg(0, std::ios_base::end);
      fd->file_size = static_cast<size_t>(strm->tellg());
      strm->seekg(fd->start_offset, std::ios_base::beg);
      fd->f = strm;
      result = ARCHIVE_OK;
    }

  return result;
}

la_ssize_t
LibArchive::readCallBack(archive *a, void *client_data, const void **buffer)
{
  la_ssize_t result = 0;
  LibArchiveFileData *fd = reinterpret_cast<LibArchiveFileData *>(client_data);

  if(fd->f)
    {
      if(fd->f->good())
        {
          size_t rb = static_cast<size_t>(fd->f->tellg());
          size_t left = fd->file_size - rb;
          if(left > fd->buffer_size)
            {
              fd->f->read(fd->buffer, fd->buffer_size);
              result = static_cast<la_ssize_t>(fd->buffer_size);
            }
          else
            {
              result = static_cast<la_ssize_t>(left);
              if(left > 0)
                {
                  fd->f->read(fd->buffer, left);
                }
            }
          *buffer = fd->buffer;
        }
      else if(!fd->f->eof())
        {
          archive_set_error(a, EIO, "%s", "Bad stream condition!");
        }
    }
  else
    {
      archive_set_error(a, EINVAL, "%s", "Stream object is null!");
    }

  return result;
}

la_int64_t
LibArchive::skipCallback(archive *a, void *client_data, la_int64_t request)
{
  la_int64_t result = 0;

  LibArchiveFileData *fd = reinterpret_cast<LibArchiveFileData *>(client_data);

  if(fd->f)
    {
      if(fd->f->good())
        {
          fd->f->seekg(request, std::ios_base::cur);
          result = request;
        }
      else if(!fd->f->eof())
        {
          archive_set_error(a, EIO, "%s", "Bad stream condition!");
        }
    }
  else
    {
      archive_set_error(a, EINVAL, "%s", "Stream object is null!");
    }

  return result;
}

int
LibArchive::closeCallback(archive *a, void *client_data)
{
  int result = ARCHIVE_FATAL;

  LibArchiveFileData *fd = reinterpret_cast<LibArchiveFileData *>(client_data);

  if(fd->f)
    {
      fd->f.reset();
      result = ARCHIVE_OK;
    }
  else
    {
      archive_set_error(a, EINVAL, "%s", "Stream object is null!");
    }

  return result;
}

la_int64_t
LibArchive::seekCallback(archive *a, void *client_data, la_int64_t offset,
                         int whence)
{
  la_int64_t result = 0;

  LibArchiveFileData *fd = reinterpret_cast<LibArchiveFileData *>(client_data);

  if(fd->f)
    {
      if(fd->f->good())
        {
          switch(whence)
            {
            case SEEK_SET:
              {
                fd->f->seekg(offset, std::ios_base::beg);
                result = static_cast<la_int64_t>(fd->f->tellg());
                break;
              }
            case SEEK_CUR:
              {
                fd->f->seekg(offset, std::ios_base::cur);
                result = static_cast<la_int64_t>(fd->f->tellg());
                break;
              }
            case SEEK_END:
              {

                fd->f->seekg(offset, std::ios_base::end);
                result = static_cast<la_int64_t>(fd->f->tellg());
                break;
              }
            default:
              {
                result = ARCHIVE_FATAL;
                break;
              }
            }
        }
      else
        {
          archive_set_error(a, EIO, "%s", "Stream error!");
        }
    }
  else
    {
      archive_set_error(a, EINVAL, "%s", "Stream object is null!");
    }

  return result;
}

la_ssize_t
LibArchive::writeCallback(archive *a, void *client_data, const void *buffer,
                          size_t length)
{
  la_ssize_t result = -1;
  LibArchiveFileData *fd = reinterpret_cast<LibArchiveFileData *>(client_data);
  if(fd != nullptr)
    {
      if(fd->f)
        {
          fd->f->write(reinterpret_cast<const char *>(buffer), length);
          result = static_cast<la_ssize_t>(length);
        }
      else
        {
          archive_set_error(a, EPERM, "%s", "Cannot write to stream!");
        }
    }
  else
    {
      archive_set_error(a, EINVAL, "%s", "Stream object is null!");
    }

  return result;
}

void
LibArchive::getCentralDirectory(std::shared_ptr<std::istream> f,
                                const uint64_t &fsz, std::string &result)
{
  std::tuple<uint64_t, uint64_t> cd_tup = parseEOCDRecord(f, fsz);
  if(std::get<0>(cd_tup) == 0 || std::get<1>(cd_tup) == 0)
    {
      throw std::runtime_error(
          "LibArchive::getCentralDirectory: cannot find central directory");
    }
  if(std::get<0>(cd_tup) + std::get<1>(cd_tup) > fsz)
    {
      throw std::runtime_error(
          "LibArchive::getCentralDirectory: incorrect zip file");
    }

  result.resize(std::get<1>(cd_tup));
  f->clear(std::ios_base::eofbit);
  f->seekg(std::get<0>(cd_tup), std::ios_base::beg);
  f->read(result.data(), result.size());
}

std::tuple<uint64_t, uint64_t>
LibArchive::parseEOCDRecord(std::shared_ptr<std::istream> f,
                            const uint64_t &fsz)
{
  if(fsz < 22)
    {
      throw std::runtime_error(
          "LibArchive::parseEOCDRecord: incorrect file size");
    }

  std::tuple<uint64_t, uint64_t> result = std::make_tuple(0, 0);

  uint32_t eocd_signature = 101010256;
  ByteOrder bo(eocd_signature);
  bo.getLittle(eocd_signature);

  uint32_t val;
  size_t sz_32 = sizeof(val);
  uint64_t offset = fsz - 22;
  while(offset >= 0)
    {
      f->seekg(offset, std::ios_base::beg);
      f->read(reinterpret_cast<char *>(&val), sz_32);
      if(val == eocd_signature)
        {
          break;
        }
      if(offset == 0)
        {
          throw std::runtime_error(
              "LibArchive::parseEOCDRecord: not a zip file");
        }
      offset--;
    }

  std::string eocd;
  eocd.resize(fsz - offset);
  f->read(eocd.data(), eocd.size());

  uint32_t cd_size = 0;
  size_t lim = 8 + sz_32;
  char *ptr = reinterpret_cast<char *>(&cd_size);
  for(size_t i = 8; i < lim; i++)
    {
      ptr[i - 8] = eocd[i];
    }

  if(cd_size == 0xffffffff)
    {
      result
          = parseEOCDRecordZip64(f, fsz, static_cast<uint64_t>(eocd.size()));
      return result;
    }
  bo.setLittle(cd_size);
  cd_size = bo;

  uint32_t cd_offset = 0;
  ptr = reinterpret_cast<char *>(&cd_offset);
  lim = 12 + sz_32;
  for(size_t i = 12; i < lim; i++)
    {
      ptr[i - 12] = eocd[i];
    }
  if(cd_offset == 0xffffffff)
    {
      result
          = parseEOCDRecordZip64(f, fsz, static_cast<uint64_t>(eocd.size()));
      return result;
    }
  bo.setLittle(cd_offset);
  cd_offset = bo;

  std::get<0>(result) = static_cast<uint64_t>(cd_offset);
  std::get<1>(result) = static_cast<uint64_t>(cd_size);

  return result;
}

std::tuple<uint64_t, uint64_t>
LibArchive::parseEOCDRecordZip64(std::shared_ptr<std::istream> f,
                                 const uint64_t &fsz,
                                 const uint64_t &eocd_record_size)
{
  if(fsz < eocd_record_size + 76)
    {
      throw std::runtime_error(
          "LibArchive::parseEOCDRecordZip64: incorrect zip file");
    }
  std::tuple<uint64_t, uint64_t> result = std::make_tuple(0, 0);
  uint64_t offset = fsz - eocd_record_size - 76;

  uint32_t signature = 101075792;
  ByteOrder bo(signature);
  bo.getLittle(signature);
  uint32_t val;
  f->clear(std::ios_base::eofbit);
  while(offset >= 0)
    {
      f->seekg(offset, std::ios_base::beg);
      f->read(reinterpret_cast<char *>(&val), sizeof(val));
      if(val == signature)
        {
          break;
        }
      if(offset == 0)
        {
          throw std::runtime_error(
              "LibArchive::parseEOCDRecordZip64: not zip file");
        }
      offset--;
    }
  uint64_t eocd_size;
  f->read(reinterpret_cast<char *>(&eocd_size), sizeof(eocd_size));
  bo.setLittle(eocd_size);
  eocd_size = bo;
  if(eocd_size + static_cast<uint64_t>(f->tellg()) > fsz)
    {
      throw std::runtime_error(
          "LibArchive::parseEOCDRecordZip64: incorrect eocd");
    }
  std::string eocd;
  eocd.resize(eocd_size);
  f->read(eocd.data(), eocd.size());

  uint64_t cd_size;
  char *ptr = reinterpret_cast<char *>(&cd_size);
  for(size_t i = 28; i < 28 + sizeof(cd_size); i++)
    {
      ptr[i - 28] = eocd[i];
    }
  bo.setLittle(cd_size);
  cd_size = bo;

  uint64_t cd_offset;
  ptr = reinterpret_cast<char *>(&cd_offset);
  for(size_t i = 36; i < 36 + sizeof(cd_offset); i++)
    {
      ptr[i - 36] = eocd[i];
    }
  bo.setLittle(cd_offset);
  cd_offset = bo;

  std::get<0>(result) = cd_offset;
  std::get<1>(result) = cd_size;

  return result;
}

void
LibArchive::parseCentralDirectory(
    const std::string &central_directory,
    std::vector<std::tuple<std::string, uint64_t, uint64_t>> &result)
{
  size_t rb = 0;
  uint32_t signature = 33639248;
  ByteOrder bo(signature);
  bo.getLittle(signature);
  uint32_t val32;
  size_t sz_32 = sizeof(val32);
  uint16_t val16;
  size_t sz_16 = sizeof(val16);
  size_t cd_sz = central_directory.size();
  char *ptr;
  size_t sum;

  uint16_t utf8bit = 1;
  utf8bit = utf8bit << 10;
  bo = utf8bit;
  bo.getLittle(utf8bit);

  uint16_t n;
  uint16_t m;
  uint16_t k;

  while(rb < cd_sz)
    {
      std::tuple<std::string, uint64_t, uint64_t> res
          = std::make_tuple("", 0, 0);

      sum = rb;
      if(sum + sz_32 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (1)");
        }
      ptr = reinterpret_cast<char *>(&val32);
      for(size_t i = sum; i < sum + sz_32; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      if(val32 != signature)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (2)");
        }

      sum = rb + 8;
      if(sum + sz_16 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (3)");
        }
      ptr = reinterpret_cast<char *>(&val16);
      for(size_t i = sum; i < sum + sz_16; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      bool utf8 = false;
      if(uint16_t(val16 & utf8bit) != 0)
        {
          utf8 = true;
        }

      sum = rb + 20;
      if(sum + sz_32 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (4)");
        }
      ptr = reinterpret_cast<char *>(&val32);
      for(size_t i = sum; i < sum + sz_32; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      int zip64 = 0;
      if(val32 == 0xffffffff)
        {
          zip64 = zip64 | ZIP64_COMPRESSED;
        }
      else
        {
          bo.setLittle(val32);
          val32 = bo;
          std::get<1>(res) = static_cast<uint64_t>(val32);
        }

      sum = rb + 24;
      if(sum + sz_32 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (5)");
        }
      ptr = reinterpret_cast<char *>(&val32);
      for(size_t i = sum; i < sum + sz_32; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      if(val32 == 0xffffffff)
        {
          zip64 = zip64 | ZIP64_UNCOMPRESSED;
        }

      sum = rb + 28;
      if(sum + sz_16 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (6)");
        }
      ptr = reinterpret_cast<char *>(&val16);
      for(size_t i = sum; i < sum + sz_16; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      bo.setLittle(val16);
      n = bo;

      sum = rb + 30;
      if(sum + sz_16 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (7)");
        }
      ptr = reinterpret_cast<char *>(&val16);
      for(size_t i = sum; i < sum + sz_16; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      bo.setLittle(val16);
      m = bo;

      sum = rb + 32;
      if(sum + sz_16 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (8)");
        }
      ptr = reinterpret_cast<char *>(&val16);
      for(size_t i = sum; i < sum + sz_16; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      bo.setLittle(val16);
      k = bo;

      sum = rb + 42;
      if(sum + sz_32 > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (9)");
        }
      ptr = reinterpret_cast<char *>(&val32);
      for(size_t i = sum; i < sum + sz_32; i++)
        {
          ptr[i - sum] = central_directory[i];
        }
      if(val32 == 0xffffffff)
        {
          zip64 = zip64 | ZIP64_OFFSET;
        }
      else
        {
          bo.setLittle(val32);
          val32 = bo;
          std::get<2>(res) = static_cast<uint64_t>(val32);
        }

      sum = rb + 46;
      size_t nm_sz = static_cast<size_t>(n);
      if(sum + nm_sz > cd_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseCentralDirectory: incorrect file record (10)");
        }
      std::get<0>(res).reserve(nm_sz);
      std::copy(central_directory.begin() + sum,
                central_directory.begin() + sum + nm_sz,
                std::back_inserter(std::get<0>(res)));
      if(!utf8)
        {
          std::vector<std::string> code_pages
              = XMLTextEncoding::detectStringEncoding(std::get<0>(res));
          if(code_pages.size() > 0)
            {
              std::string r;
              XMLTextEncoding::convertToEncoding(std::get<0>(res), r,
                                                 code_pages[0], "UTF-8");
              std::get<0>(res) = r;
            }
        }

      if(zip64 != 0)
        {
          sum = rb + 46 + static_cast<size_t>(n);
          size_t extra_sz = static_cast<size_t>(m);
          if(sum + extra_sz > cd_sz)
            {
              throw std::runtime_error("LibArchive::parseCentralDirectory: "
                                       "incorrect file record (11)");
            }
          std::string extra;
          extra.reserve(extra_sz);
          std::copy(central_directory.begin() + sum,
                    central_directory.begin() + sum + extra_sz,
                    std::back_inserter(extra));
          parseExtraField(extra, std::get<1>(res), std::get<2>(res), zip64);
        }
      else
        {
          sum = rb + 42;
          if(sum + sz_32 > cd_sz)
            {
              throw std::runtime_error("LibArchive::parseCentralDirectory: "
                                       "incorrect file record (12)");
            }
          ptr = reinterpret_cast<char *>(&val32);
          for(size_t i = sum; i < sum + sz_32; i++)
            {
              ptr[i - sum] = central_directory[i];
            }
          bo.setLittle(val32);
          val32 = bo;
          std::get<2>(res) = static_cast<uint64_t>(val32);
        }

      result.emplace_back(res);
      rb += 46 + static_cast<size_t>(n) + static_cast<size_t>(m)
            + static_cast<size_t>(k);
    }
}

void
LibArchive::parseExtraField(const std::string &extra, uint64_t &compressed_sz,
                            uint64_t &offset, const int &mask)
{
  uint16_t header_id = 1;
  ByteOrder bo(header_id);
  bo.getLittle(header_id);
  uint16_t val16;
  size_t sz_16 = sizeof(val16);
  size_t rb = 0;
  size_t extra_sz = extra.size();
  size_t sum;
  size_t sz_64 = sizeof(uint64_t);
  char *ptr;
  while(rb < extra_sz)
    {
      sum = rb + sz_16;
      if(sum > extra_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseExtraField: incorrect header id");
        }
      ptr = reinterpret_cast<char *>(&val16);
      for(size_t i = rb; i < sum; i++)
        {
          ptr[i - rb] = extra[i];
        }
      rb += sz_16;
      bool found = false;
      if(header_id == val16)
        {
          found = true;
        }

      sum = rb + sz_16;
      if(sum > extra_sz)
        {
          throw std::runtime_error(
              "LibArchive::parseExtraField: incorrect data size");
        }
      ptr = reinterpret_cast<char *>(&val16);
      for(size_t i = rb; i < sum; i++)
        {
          ptr[i - rb] = extra[i];
        }
      rb += sz_16;
      bo.setLittle(val16);
      val16 = bo;

      if(found)
        {
          size_t lsum = rb;
          if(int(mask & ZIP64_UNCOMPRESSED) != 0)
            {
              lsum += sz_64;
            }
          if(int(mask & ZIP64_COMPRESSED) != 0)
            {
              sum = lsum + sz_64;
              if(sum > extra_sz)
                {
                  throw std::runtime_error("LibArchive::parseExtraField: "
                                           "incorrect compressed data size");
                }
              ptr = reinterpret_cast<char *>(&compressed_sz);
              for(size_t i = lsum; i < sum; i++)
                {
                  ptr[i - lsum] = extra[i];
                }
              bo.setLittle(compressed_sz);
              compressed_sz = bo;
              lsum += sz_64;
            }

          if(int(mask & ZIP64_OFFSET) != 0)
            {
              sum = lsum + sz_64;
              if(sum > extra_sz)
                {
                  throw std::runtime_error("LibArchive::parseExtraField: "
                                           "incorrect offset");
                }
              ptr = reinterpret_cast<char *>(&offset);
              for(size_t i = lsum; i < sum; i++)
                {
                  ptr[i - lsum] = extra[i];
                }
              bo.setLittle(offset);
              offset = bo;
              lsum += sz_64;
            }
        }
      rb += static_cast<size_t>(val16);
    }
}

std::filesystem::path
LibArchive::unpackToDirectory(std::shared_ptr<LibArchiveFileData> fd,
                              const std::string &filename,
                              const std::filesystem::path &directory)
{
  std::filesystem::path result;
  std::shared_ptr<archive> a = initForReading(fd);

  int er;
  if(fd->start_offset == 0)
    {
      er = archive_read_set_seek_callback(a.get(), &LibArchive::seekCallback);
      if(er != ARCHIVE_OK)
        {
          archiveError(a, "LibArchive::unpackToDirectory:");
        }
    }

  er = archive_read_open2(a.get(), fd.get(), &LibArchive::openCallBack,
                          &LibArchive::readCallBack, &LibArchive::skipCallback,
                          &LibArchive::closeCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::unpackToDirectory:");
    }

  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });
  int retry_count = 0;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      er = archive_read_next_header2(a.get(), e.get());
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("LibArchive::unpackToDirectory: \"") + str
                      + "\"";
              }
            else
              {
                err = std::string("LibArchive::unpackToDirectory: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            const char *val = archive_entry_pathname_utf8(e.get());
            if(val)
              {
                if(std::string(val) == filename)
                  {

                    result = directory
                             / std::u8string(
                                 reinterpret_cast<const char8_t *>(val));
                    std::filesystem::create_directories(result.parent_path());

                    unpackEntryToDirectory(a, e, result);

                    retry_count = 3;
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
            archiveError(a, "LibArchive::unpackToDirectory:");
            break;
          }
        }
      archive_entry_clear(e.get());
    }

  return result;
}

void
LibArchive::unpackToBuffer(std::shared_ptr<LibArchiveFileData> fd,
                           const std::string &filename, std::string &result)
{
  std::shared_ptr<archive> a = initForReading(fd);

  int er;
  if(fd->start_offset == 0)
    {
      er = archive_read_set_seek_callback(a.get(), &LibArchive::seekCallback);
      if(er != ARCHIVE_OK)
        {
          archiveError(a, "LibArchive::unpackToBuffer:");
        }
    }

  er = archive_read_open2(a.get(), fd.get(), &LibArchive::openCallBack,
                          &LibArchive::readCallBack, &LibArchive::skipCallback,
                          &LibArchive::closeCallback);

  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::unpackToBuffer:");
    }

  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });
  int retry_count = 0;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      er = archive_read_next_header2(a.get(), e.get());
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("LLibArchive::unpackFileToBuffer: \"") + str
                      + "\"";
              }
            else
              {
                err = std::string("LLibArchive::unpackFileToBuffer: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            const char *val = archive_entry_pathname_utf8(e.get());
            if(val)
              {
                if(std::string(val) == filename)
                  {
                    result = unpackEntryToBuffer(a, e);
                    retry_count = 3;
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
            archiveError(a, "LibArchive::unpackToBuffer:");
            break;
          }
        }
      archive_entry_clear(e.get());
    }
}

void
LibArchive::unpackEntryToDirectory(std::shared_ptr<archive> a,
                                   std::shared_ptr<archive_entry> e,
                                   const std::filesystem::path &file_path)
{
  if(archive_entry_filetype_is_set(e.get()))
    {
      switch(archive_entry_filetype(e.get()))
        {
        case AE_IFREG:
          {
            break;
          }
        case AE_IFDIR:
          {
            std::filesystem::create_directories(file_path);
            return void();
          }
        case AE_IFLNK:
          {
            const char *t = archive_entry_symlink_utf8(e.get());
            if(t)
              {
                std::filesystem::path target
                    = std::u8string(reinterpret_cast<const char8_t *>(t));
                std::error_code ec;
                std::filesystem::create_symlink(target, file_path, ec);
                if(ec)
                  {
                    std::cout << "LibArchive::unpackEntryToDirectory: \""
                              << ec.message() << "\"" << std::endl;
                  }
              }
            return void();
          }
        default:
          return void();
        }
    }
  else
    {
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
          std::filesystem::create_directories(file_path);
          return void();
        }
    }

  int er = ARCHIVE_OK;
  int retry_count = 0;
  std::unique_ptr<std::fstream, std::function<void(std::fstream *)>> f(
      new std::fstream,
      [](std::fstream *f)
        {
          if(f->is_open())
            {
              f->close();
            }
          delete f;
        });
  f->open(file_path, std::ios_base::out | std::ios_base::binary);
  if(!f->is_open())
    {
      std::cout << "LibArchive::unpackEntryToDirectory: cannot create file "
                << file_path << std::endl;
      return void();
    }
  size_t sz;
  la_int64_t offset;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      const char *buf;
      sz = 0;
      offset = 0;
      er = archive_read_data_block(
          a.get(), reinterpret_cast<const void **>(&buf), &sz, &offset);
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("LibArchive::unpackEntryToDirectory: \"")
                      + str + "\"";
              }
            else
              {
                err = std::string("LibArchive::unpackEntryToDirectory: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            f->seekg(offset, std::ios_base::beg);
            f->write(buf, sz);
            break;
          }
        case ARCHIVE_RETRY:
          {
            retry_count++;
            break;
          }
        case ARCHIVE_EOF:
          {
            break;
          }
        default:
          {
            archiveError(a, "LibArchive::unpackEntryToDirectory:");
            break;
          }
        }
    }
  std::filesystem::perms perms = getPermissionsFromEntry(e);
  std::error_code ec;
  std::filesystem::permissions(file_path, perms, ec);
  if(ec)
    {
      std::cout << "LibArchive::unpackEntryToDirectory: " << ec.message()
                << std::endl;
    }
}

std::string
LibArchive::unpackEntryToBuffer(std::shared_ptr<archive> a,
                                std::shared_ptr<archive_entry> e)
{
  std::string result;
  if(archive_entry_filetype_is_set(e.get()))
    {
      switch(archive_entry_filetype(e.get()))
        {
        case AE_IFREG:
          {
            break;
          }
        default:
          return result;
        }
    }
  else
    {
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
          return result;
        }
    }

  int er = ARCHIVE_OK;
  int retry_count = 0;
  size_t sz;
  la_int64_t offset;
  while(er >= ARCHIVE_WARN && er <= ARCHIVE_OK && retry_count < 3)
    {
      const char *buf;
      sz = 0;
      offset = 0;
      er = archive_read_data_block(
          a.get(), reinterpret_cast<const void **>(&buf), &sz, &offset);
      switch(er)
        {
        case ARCHIVE_WARN:
          {
            const char *str = archive_error_string(a.get());
            std::string err;
            if(str)
              {
                err = std::string("LibArchive::unpackEntryToBuffer: \"") + str
                      + "\"";
              }
            else
              {
                err = std::string("LibArchive::unpackEntryToBuffer: ")
                      + std::strerror(archive_errno(a.get()));
              }
            std::cout << err << std::endl;
          }
        case ARCHIVE_OK:
          {
            retry_count = 0;
            result.resize(offset);
            for(size_t i = 0; i < sz; i++)
              {
                result.push_back(buf[i]);
              }
            break;
          }
        case ARCHIVE_RETRY:
          {
            retry_count++;
            break;
          }
        case ARCHIVE_EOF:
          {
            break;
          }
        default:
          {
            archiveError(a, "LibArchive::unpackEntryToBuffer:");
            break;
          }
        }
    }

  return result;
}

void
LibArchive::writeFile(std::shared_ptr<archive> a,
                      const std::filesystem::path &path,
                      std::string name_in_archive,
                      const std::filesystem::perms &perms)
{
  std::shared_ptr<archive_entry> e(archive_entry_new(),
                                   [](archive_entry *e)
                                     {
                                       archive_entry_free(e);
                                     });

  if(std::filesystem::path::preferred_separator == '\\')
    {
      for(auto it = name_in_archive.begin(); it != name_in_archive.end(); it++)
        {
          if(*it == std::filesystem::path::preferred_separator)
            {
              *it = '/';
            }
        }
    }

  archive_entry_set_pathname_utf8(e.get(), name_in_archive.c_str());

  if(perms == std::filesystem::perms::none)
    {
      std::filesystem::file_status f_stat
          = std::filesystem::symlink_status(path);
      archive_entry_set_perm(e.get(),
                             static_cast<__LA_MODE_T>(f_stat.permissions()));
    }
  else
    {
      archive_entry_set_perm(e.get(), static_cast<__LA_MODE_T>(perms));
    }

  archive_entry_set_filetype(e.get(), AE_IFREG);

  archive_entry_set_size(
      e.get(), static_cast<la_int64_t>(std::filesystem::file_size(path)));

  std::filesystem::file_time_type last_write
      = std::filesystem::last_write_time(path);
  auto sytem_clock_tp
      = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          last_write - std::filesystem::file_time_type::clock::now()
          + std::chrono::system_clock::now());
  time_t lwt = std::chrono::system_clock::to_time_t(sytem_clock_tp);
  archive_entry_set_mtime(e.get(), lwt, 0);

  int er = archive_write_header(a.get(), e.get());
  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::writeFile:");
    }

  std::unique_ptr<std::fstream, std::function<void(std::fstream *)>> f(
      new std::fstream,
      [](std::fstream *f)
        {
          if(f->is_open())
            {
              f->close();
            }
          delete f;
        });
  f->open(path, std::ios_base::in | std::ios_base::binary);

  if(!f->is_open())
    {
      throw std::runtime_error(
          "LibArchive::writeFile: cannot open source file");
    }

  f->seekg(0, std::ios_base::end);
  size_t fsz = static_cast<size_t>(f->tellg());
  f->seekg(0, std::ios_base::beg);
  size_t rb = 0;
  std::string buf;
  size_t diff;
  size_t buf_sz = 4194304;

  la_ssize_t wb;
  size_t pos;
  while(rb < fsz)
    {
      diff = fsz - rb;
      if(diff > buf_sz)
        {
          buf.resize(buf_sz);
        }
      else
        {
          buf.resize(diff);
        }
      f->read(buf.data(), buf.size());
      rb += buf.size();

      pos = 0;
      while(pos < buf.size())
        {
          wb = archive_write_data(
              a.get(), reinterpret_cast<const void *>(buf.c_str() + pos),
              buf.size() - pos);
          if(wb <= 0)
            {
              throw std::runtime_error(
                  "LibArchive::writeFile: error on writing file to archive");
            }
          else
            {
              pos += static_cast<size_t>(wb);
            }
        }
    }
}

std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
LibArchive::symlinkWriteResolver(const std::filesystem::path &relative,
                                 const std::filesystem::path &symlink)
{
  std::vector<std::tuple<std::filesystem::path, std::filesystem::path>> result;

  std::error_code ec;
  std::filesystem::path resolved = std::filesystem::read_symlink(symlink, ec);
  if(ec)
    {
      std::cout << "LibArchive::symlinkWriteResolver: " << ec.message() << " "
                << symlink << std::endl;
      return result;
    }

  std::filesystem::file_status status
      = std::filesystem::symlink_status(resolved);
  switch(status.type())
    {
    case std::filesystem::file_type::regular:
      {
        result.push_back(std::make_tuple(relative, resolved));
        break;
      }
    case std::filesystem::file_type::symlink:
      {
        std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
            loc_res = symlinkWriteResolver(resolved.parent_path(), resolved);
        for(auto it = loc_res.begin(); it != loc_res.end(); it++)
          {
            result.push_back(std::make_tuple(relative / std::get<0>(*it),
                                             std::get<1>(*it)));
          }
        break;
      }
    case std::filesystem::file_type::directory:
      {
        std::filesystem::path l_base = resolved;
        for(auto &dir_it :
            std::filesystem::recursive_directory_iterator(resolved))
          {
            std::filesystem::path p = dir_it.path();
            status = std::filesystem::symlink_status(p);
            switch(status.type())
              {
              case std::filesystem::file_type::regular:
                {
                  std::filesystem::path l_relative
                      = p.lexically_relative(l_base);
                  result.push_back(std::make_tuple(relative / l_relative, p));
                  break;
                }
              case std::filesystem::file_type::symlink:
                {
                  std::vector<
                      std::tuple<std::filesystem::path, std::filesystem::path>>
                      loc_res = symlinkWriteResolver(p.parent_path(), p);
                  for(auto it = loc_res.begin(); it != loc_res.end(); it++)
                    {
                      result.push_back(std::make_tuple(
                          relative / std::get<0>(*it), std::get<1>(*it)));
                    }
                  break;
                }
              default:
                break;
              }
          }
        break;
      }
    default:
      break;
    }

  return result;
}

void
LibArchive::writeBufferToArchive(std::shared_ptr<archive> a,
                                 std::shared_ptr<archive_entry> e,
                                 const std::string &buf)
{
  int er = archive_write_header(a.get(), e.get());
  if(er != ARCHIVE_OK)
    {
      archiveError(a, "LibArchive::writeBufferToArchive:");
    }

  la_ssize_t wb;
  size_t pos;
  pos = 0;
  while(pos < buf.size())
    {
      wb = archive_write_data(
          a.get(), reinterpret_cast<const void *>(buf.c_str() + pos),
          buf.size() - pos);
      if(wb <= 0)
        {
          throw std::runtime_error("LibArchive::writeBufferToArchive: error "
                                   "on writing buffer to archive");
        }
      else
        {
          pos += static_cast<size_t>(wb);
        }
    }
}

std::filesystem::perms
LibArchive::getPermissionsFromEntry(const std::shared_ptr<archive_entry> &e)
{
  std::filesystem::perms result = std::filesystem::perms::none;

  if(archive_entry_perm_is_set(e.get()))
    {
      mode_t perms = archive_entry_perm(e.get());
      if(perms & S_IRUSR)
        {
          result |= std::filesystem::perms::owner_read;
        }
      if(perms & S_IWUSR)
        {
          result |= std::filesystem::perms::owner_write;
        }
      if(perms & S_IXUSR)
        {
          result |= std::filesystem::perms::owner_exec;
        }

      if(perms & S_IRGRP)
        {
          result |= std::filesystem::perms::group_read;
        }
      if(perms & S_IWGRP)
        {
          result |= std::filesystem::perms::group_write;
        }
      if(perms & S_IXGRP)
        {
          result |= std::filesystem::perms::group_exec;
        }

      if(perms & S_IROTH)
        {
          result |= std::filesystem::perms::others_read;
        }
      if(perms & S_IWOTH)
        {
          result |= std::filesystem::perms::others_write;
        }
      if(perms & S_IXOTH)
        {
          result |= std::filesystem::perms::others_exec;
        }
    }

  return result;
}
