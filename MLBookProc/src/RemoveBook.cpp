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

#include <ArchiveFileEntry.h>
#include <ArchiveRemoveEntry.h>
#include <BookParseEntry.h>
#include <LibArchive.h>
#include <RefreshCollection.h>
#include <RemoveBook.h>
#include <XMLTextEncoding.h>
#include <algorithm>
#include <iostream>

RemoveBook::RemoveBook(const std::shared_ptr<AuxFunc> &af,
                       const BookBaseEntry &bbe, const std::string &col_name,
                       const std::shared_ptr<BookMarks> &bookmarks)
{
  this->af = af;
  this->bbe = bbe;
  this->col_name = col_name;
  this->bookmarks = bookmarks;
  supported_archives = af->get_supported_archive_types_packing();
}

void
RemoveBook::removeBook()
{
  std::string ext = af->get_extension(bbe.file_path);
  for(auto it = ext.begin(); it != ext.end();)
    {
      if(*it == '.')
        {
          ext.erase(it);
        }
      else
        {
          break;
        }
    }

  auto it_sup
      = std::find(supported_archives.begin(), supported_archives.end(), ext);

  if(it_sup == supported_archives.end())
    {
      std::filesystem::remove_all(bbe.file_path);
    }
  else
    {
      std::filesystem::path tmp = bbe.file_path.parent_path();
      tmp /= std::filesystem::u8path(af->randomFileName());
      SelfRemovingPath srp(tmp);
      archiveRemove(bbe.file_path, bbe.bpe.book_path, srp.path);
    }

  std::shared_ptr<RefreshCollection> rc = std::make_shared<RefreshCollection>(
      af, col_name, 1, false, false, true, bookmarks);
  rc->refreshFile(bbe);
}

void
RemoveBook::archiveRemove(const std::filesystem::path &archive_path,
                          const std::string &book_path,
                          const std::filesystem::path &out_d)
{
  if(!std::filesystem::exists(archive_path))
    {
      std::cout << "RemoveBook::archiveRemove: source archive " << archive_path
                << " does not exist" << std::endl;
      return void();
    }

  if(book_path.empty())
    {
      std::cout << "RemoveBook::archiveRemove: book path is empty"
                << std::endl;
      return void();
    }

  std::string l_book_path;
  bool final = false;
  std::string find_str("\n");
  std::string next_path = book_path;

  std::string::size_type n = next_path.find(find_str);
  if(n != std::string::npos)
    {
      l_book_path = next_path.substr(0, n);
      std::string ext
          = af->get_extension(std::filesystem::u8path(l_book_path));
      for(auto it = ext.begin(); it != ext.end();)
        {
          if(*it == '.')
            {
              ext.erase(it);
            }
          else
            {
              break;
            }
        }

      auto it = std::find(supported_archives.begin(), supported_archives.end(),
                          ext);
      if(it == supported_archives.end())
        {
          final = true;
        }
      else
        {
          next_path.erase(0, n + find_str.size());
        }
    }
  else
    {
      l_book_path = next_path;
      final = true;
    }

  if(l_book_path.empty())
    {
      std::cout << "RemoveBook::archiveRemove: file name is empty"
                << std::endl;
      return void();
    }

  std::string fbd_file_name;
  if(final)
    {
      find_str = ".";
      n = l_book_path.find(find_str);
      if(n != std::string::npos)
        {
          fbd_file_name = l_book_path.substr(0, n);
        }
      else
        {
          fbd_file_name = l_book_path;
        }
      fbd_file_name += ".fbd";
    }

  std::filesystem::path new_arch_path
      = out_d
        / std::filesystem::u8path(af->randomFileName()
                                  + af->get_extension(archive_path));

  std::unique_ptr<LibArchive> la(new LibArchive(af));
  ArchiveRemoveEntry rm_e
      = la->libarchive_remove_init(archive_path, new_arch_path);
  if(rm_e.a_read.get() == nullptr || rm_e.a_write.get() == nullptr)
    {
      std::cout
          << "RemoveBook::archiveRemove: error on creating libarchive objects"
          << std::endl;
      return void();
    }

  if(final)
    {
      int er = ARCHIVE_OK;
      unsigned long file_count = 0;
      std::shared_ptr<archive_entry> read_ent(
          archive_entry_new2(rm_e.a_read.get()),
          [](archive_entry *e)
            {
              archive_entry_free(e);
            });
      std::shared_ptr<archive_entry> write_ent(
          archive_entry_new2(rm_e.a_write.get()),
          [](archive_entry *e)
            {
              archive_entry_free(e);
            });

      std::filesystem::create_directories(out_d);

      while(er == ARCHIVE_OK || er == ARCHIVE_WARN)
        {
          archive_entry_clear(read_ent.get());
          er = archive_read_next_header2(rm_e.a_read.get(), read_ent.get());
          SelfRemovingPath srp_read;
          std::string path_in_arch;
          if(er == ARCHIVE_OK || er == ARCHIVE_WARN)
            {
              if(er == ARCHIVE_WARN)
                {
                  la->libarchive_error(
                      rm_e.a_read, "RemoveBook::archiveRemove reading:", er);
                }
              const char *chnm = archive_entry_pathname(read_ent.get());
              if(chnm)
                {
                  std::vector<std::string> cp
                      = XMLTextEncoding::detectStringEncoding(chnm);
                  if(cp.size() > 0)
                    {
                      XMLTextEncoding::convertToEncoding(chnm, path_in_arch,
                                                         cp[0], "UTF-8");
                    }
                }
              if(!path_in_arch.empty())
                {
                  if(path_in_arch != l_book_path
                     && path_in_arch != fbd_file_name)
                    {
                      srp_read = la->libarchive_read_entry(
                          rm_e.a_read.get(), read_ent.get(), out_d);
                    }
                }
            }
          else
            {
              if(er != ARCHIVE_EOF)
                {
                  la->libarchive_error(
                      rm_e.a_read, "RemoveBook::archiveRemove reading:", er);
                }
              break;
            }

          if(!srp_read.path.empty() && std::filesystem::exists(srp_read.path))
            {
              archive_entry_clear(write_ent.get());
              if(std::filesystem::is_directory(srp_read.path))
                {
                  er = la->libarchive_write_directory(
                      rm_e.a_write.get(), write_ent.get(),
                      std::filesystem::u8path(path_in_arch), srp_read.path);
                }
              else
                {
                  er = la->libarchive_write_file(
                      rm_e.a_write.get(), write_ent.get(),
                      std::filesystem::u8path(path_in_arch), srp_read.path);
                }
              if(er == ARCHIVE_OK || er == ARCHIVE_WARN)
                {
                  file_count++;
                }
              else
                {
                  la->libarchive_error(
                      rm_e.a_write, "RemoveBook::archiveRemove writing:", er);
                  break;
                }
            }
        }

      rm_e.reset();
      if(er == ARCHIVE_EOF)
        {
          std::filesystem::remove_all(archive_path);
          if(file_count > 0)
            {
              std::error_code ec;
              std::filesystem::rename(new_arch_path, archive_path, ec);
              if(ec)
                {
                  std::cout << "RemoveBook::archiveRemove "
                               "std::filesystem::rename error: "
                            << ec.message() << std::endl;
                }
            }
        }
    }
  else
    {
      int er = ARCHIVE_OK;
      unsigned long file_count = 0;
      std::shared_ptr<archive_entry> read_ent(
          archive_entry_new2(rm_e.a_read.get()),
          [](archive_entry *e)
            {
              archive_entry_free(e);
            });

      std::shared_ptr<archive_entry> write_ent(
          archive_entry_new2(rm_e.a_write.get()),
          [](archive_entry *e)
            {
              archive_entry_free(e);
            });

      std::filesystem::path unpack_dir
          = out_d / std::filesystem::u8path(af->randomFileName());
      std::filesystem::create_directories(unpack_dir);

      while(er == ARCHIVE_OK || er == ARCHIVE_WARN)
        {
          archive_entry_clear(read_ent.get());
          er = archive_read_next_header2(rm_e.a_read.get(), read_ent.get());
          SelfRemovingPath srp_read;
          std::string path_in_arch;
          if(er == ARCHIVE_OK || er == ARCHIVE_WARN)
            {
              if(er == ARCHIVE_WARN)
                {
                  la->libarchive_error(
                      rm_e.a_read, "RemoveBook::archiveRemove reading:", er);
                }
              const char *chnm = archive_entry_pathname(read_ent.get());
              if(chnm)
                {
                  std::vector<std::string> cp
                      = XMLTextEncoding::detectStringEncoding(chnm);
                  if(cp.size() > 0)
                    {
                      XMLTextEncoding::convertToEncoding(chnm, path_in_arch,
                                                         cp[0], "UTF-8");
                    }
                }
              srp_read = la->libarchive_read_entry(rm_e.a_read.get(),
                                                   read_ent.get(), unpack_dir);
            }
          else
            {
              if(er != ARCHIVE_EOF)
                {
                  la->libarchive_error(
                      rm_e.a_read, "RemoveBook::archiveRemove reading:", er);
                }
              break;
            }

          if(!srp_read.path.empty() && std::filesystem::exists(srp_read.path))
            {
              if(path_in_arch == l_book_path)
                {
                  SelfRemovingPath out_dir_l(
                      unpack_dir
                      / std::filesystem::u8path(af->randomFileName()));
                  archiveRemove(srp_read.path, next_path, out_d);
                  if(std::filesystem::exists(srp_read.path))
                    {
                      archive_entry_clear(write_ent.get());
                      er = la->libarchive_write_file(
                          rm_e.a_write.get(), write_ent.get(),
                          std::filesystem::u8path(path_in_arch),
                          srp_read.path);
                      if(er == ARCHIVE_OK || er == ARCHIVE_WARN)
                        {
                          if(er == ARCHIVE_WARN)
                            {
                              la->libarchive_error(
                                  rm_e.a_write,
                                  "RemoveBook::archiveRemove writing:", er);
                            }
                          file_count++;
                        }
                      else
                        {
                          la->libarchive_error(
                              rm_e.a_write,
                              "RemoveBook::archiveRemove writing:", er);
                          break;
                        }
                    }
                }
              else
                {
                  archive_entry_clear(write_ent.get());
                  if(std::filesystem::is_directory(srp_read.path))
                    {
                      er = la->libarchive_write_directory(
                          rm_e.a_write.get(), write_ent.get(),
                          std::filesystem::u8path(path_in_arch),
                          srp_read.path);
                    }
                  else
                    {
                      er = la->libarchive_write_file(
                          rm_e.a_write.get(), write_ent.get(),
                          std::filesystem::u8path(path_in_arch),
                          srp_read.path);
                    }
                  if(er == ARCHIVE_OK || er == ARCHIVE_WARN)
                    {
                      if(er == ARCHIVE_WARN)
                        {
                          la->libarchive_error(
                              rm_e.a_write,
                              "RemoveBook::archiveRemove writing:", er);
                        }
                      file_count++;
                    }
                  else
                    {
                      la->libarchive_error(
                          rm_e.a_write,
                          "RemoveBook::archiveRemove writing:", er);
                      break;
                    }
                }
            }
        }

      rm_e.reset();
      if(er == ARCHIVE_EOF)
        {
          std::filesystem::remove_all(archive_path);
          if(file_count > 0)
            {
              std::error_code ec;
              std::filesystem::rename(new_arch_path, archive_path, ec);
              if(ec)
                {
                  std::cout << "RemoveBook::archiveRemove "
                               "std::filesystem::rename error: "
                            << ec.message() << std::endl;
                }
            }
        }
    }
}
