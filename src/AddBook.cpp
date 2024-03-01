/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <archive.h>
#include <archive_entry.h>
#include <AddBook.h>
#include <ArchiveRemoveEntry.h>
#include <BookBaseEntry.h>
#include <LibArchive.h>
#include <MLException.h>
#include <RefreshCollection.h>
#include <SelfRemovingPath.h>
#include <ZipFileEntry.h>
#include <atomic>
#include <iostream>
#include <iterator>
#include <system_error>
#include <thread>
#include <type_traits>

AddBook::AddBook(const std::shared_ptr<AuxFunc> &af,
		 const std::string &collection_name, const bool &remove_sources,
		 const std::shared_ptr<BookMarks> &bookmarks)
{
  this->af = af;
  this->collection_name = collection_name;
  this->remove_sources = remove_sources;
  this->bookmarks = bookmarks;
}

AddBook::~AddBook()
{

}

void
AddBook::simple_add(
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path>> &books)
{
  for(auto it = books.begin(); it != books.end(); it++)
    {
      std::filesystem::path out = std::get<1>(*it);
      std::filesystem::create_directories(out.parent_path());
      std::filesystem::remove_all(out);
      std::error_code ec;
      std::filesystem::copy(std::get<0>(*it), out, ec);
      if(ec)
	{
	  std::cout << "AddBook::simple_add error: " << ec.message()
	      << " Source: " << std::get<0>(*it).u8string()
	      << " Collection path: " << out.u8string() << std::endl;
	}
    }
  std::atomic<bool> cancel;
  cancel.store(false);
  std::shared_ptr<RefreshCollection> rfr = std::make_shared<RefreshCollection>(
      af, collection_name, std::thread::hardware_concurrency(), &cancel, false,
      true, false, bookmarks);
  rfr->refreshCollection();

  remove_src(books);
}

void
AddBook::overwrite_archive(
    const std::filesystem::path &archive_path,
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path> > &books)
{
  std::filesystem::remove_all(archive_path);
  std::filesystem::create_directories(archive_path.parent_path());

  LibArchive la;

  std::shared_ptr<archive> a = la.libarchive_write_init(archive_path);
  if(a)
    {
      for(auto it = books.begin(); it != books.end(); it++)
	{
	  std::shared_ptr<archive_entry> e(archive_entry_new2(a.get()), []
	  (archive_entry *e)
	    {
	      archive_entry_free(e);
	    });
	  la.libarchive_write_file(a.get(), e.get(), std::get<1>(*it),
				   std::get<0>(*it));
	}
    }
  a.reset();

  std::atomic<bool> cancel;
  cancel.store(false);
  std::shared_ptr<RefreshCollection> rfr = std::make_shared<RefreshCollection>(
      af, collection_name, std::thread::hardware_concurrency(), &cancel, false,
      true, false, bookmarks);

  BookBaseEntry bbe;
  bbe.file_path = archive_path;
  rfr->refreshFile(bbe);

  remove_src(books);
}

std::vector<std::string>
AddBook::archive_filenames(const std::filesystem::path &archive_path)
{
  std::vector<std::string> result;

  std::vector<ZipFileEntry> filenames;

  LibArchive la;

  la.fileNamesStream(archive_path, filenames);

  std::string filename;
  std::string::size_type n;
  std::string sstr = "\\";
  for(auto it = filenames.begin(); it != filenames.end(); it++)
    {
      filename = it->filename;
      if(filename.size() > 0)
	{
	  n = 0;
	  for(;;)
	    {
	      n = filename.find(sstr, n);
	      if(n != std::string::npos)
		{
		  filename.erase(n, sstr.size());
		  filename.insert(n, "/");
		}
	      else
		{
		  break;
		}
	    }
	}
      result.push_back(filename);
    }

  return result;
}

void
AddBook::add_to_existing_archive(
    const std::filesystem::path &archive_path,
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path> > &books)
{
  LibArchive la;

  if(std::filesystem::exists(archive_path))
    {
      std::filesystem::path new_arch = archive_path.parent_path();
      new_arch /= std::filesystem::u8path(
	  af->randomFileName() + archive_path.filename().u8string());
      SelfRemovingPath main(new_arch);
      std::shared_ptr<ArchiveRemoveEntry> rment = std::make_shared<
	  ArchiveRemoveEntry>(
	  la.libarchive_remove_init(archive_path, new_arch));
      if(rment->a_read && rment->a_write && rment->fl)
	{
	  std::shared_ptr<archive_entry> entry(
	      archive_entry_new2(rment->a_read.get()), []
	      (archive_entry *e)
		{
		  archive_entry_free(e);
		});
	  bool interrupt = false;
	  int er, er_write;
	  er_write = ARCHIVE_OK;
	  std::filesystem::path loc = archive_path.parent_path();
	  while(!interrupt)
	    {
	      archive_entry_clear(entry.get());
	      er = archive_read_next_header2(rment->a_read.get(), entry.get());
	      switch(er)
		{
		case ARCHIVE_OK:
		  {
		    std::filesystem::path p = loc;
		    p /= std::filesystem::u8path(af->randomFileName());
		    SelfRemovingPath srp(p);

		    p = la.libarchive_read_entry(rment->a_read.get(),
						 entry.get(), srp.path);
		    if(std::filesystem::exists(p))
		      {
			er_write = archive_write_header(rment->a_write.get(),
							entry.get());
			if(er_write == ARCHIVE_OK || er_write == ARCHIVE_WARN)
			  {
			    if(er_write == ARCHIVE_WARN)
			      {
				la.libarchive_error(
				    rment->a_write,
				    "AddBook::add_to_existing_archive data writing warning",
				    er_write);
			      }
			    if(!std::filesystem::is_directory(p))
			      {
				er_write = la.libarchive_write_data_from_file(
				    rment->a_write.get(), p);
				if(er_write != ARCHIVE_OK)
				  {
				    throw MLException(
					"AddBook::add_to_existing_archive data writing error");
				  }
			      }
			  }
			else
			  {
			    throw MLException(
				"AddBook::add_to_existing_archive header writing error");
			  }
		      }
		    else
		      {
			throw MLException(
			    "AddBook::add_to_existing_archive incorrect filename in archive");
		      }
		    break;
		  }
		case ARCHIVE_EOF:
		  {
		    interrupt = true;
		    break;
		  }
		case ARCHIVE_FATAL:
		  {
		    la.libarchive_error(
			rment->a_read,
			"AddBook::add_to_existing_archive critical error:", er);
		    interrupt = true;
		    break;
		  }
		default:
		  {
		    la.libarchive_error(
			rment->a_read,
			"AddBook::add_to_existing_archive error:", er);
		    break;
		  }
		}
	    }

	  if(er == ARCHIVE_EOF && er_write == ARCHIVE_OK)
	    {
	      for(auto it = books.begin(); it != books.end(); it++)
		{
		  entry = std::shared_ptr<archive_entry>(
		      archive_entry_new2(rment->a_write.get()), []
		      (archive_entry *e)
			{
			  archive_entry_free(e);
			});
		  er_write = la.libarchive_write_file(rment->a_write.get(),
						      entry.get(),
						      std::get<1>(*it),
						      std::get<0>(*it));
		  if(er_write != ARCHIVE_OK)
		    {
		      throw MLException(
			  "AddBook::add_to_existing_archive new file writing error");
		    }
		}
	      if(er_write == ARCHIVE_OK && std::filesystem::exists(main.path))
		{
		  rment.reset();
		  std::filesystem::remove_all(archive_path);
		  std::filesystem::rename(main.path, archive_path);
		}
	      else
		{
		  throw MLException("AddBook::add_to_existing_archive error");
		}
	    }
	  else
	    {
	      throw MLException(
		  "AddBook::add_to_existing_archive archives error");
	    }

	  remove_src(books);
	}
      else
	{
	  throw MLException(
	      "AddBook::add_to_existing_archive error: archives have not been opened");
	}

      std::atomic<bool> cancel;
      cancel.store(false);
      std::shared_ptr<RefreshCollection> rfr = std::make_shared<
	  RefreshCollection>(af, collection_name,
			     std::thread::hardware_concurrency(), &cancel,
			     false, true, false, bookmarks);

      BookBaseEntry bbe;
      bbe.file_path = archive_path;
      rfr->refreshFile(bbe);
    }
}

void
AddBook::simple_add_dir(
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path> > &books)
{
  for(auto it = books.begin(); it != books.end(); it++)
    {
      std::filesystem::path out = std::get<1>(*it);
      std::filesystem::create_directories(out.parent_path());
      std::filesystem::remove_all(out);
      const auto c_o_group = std::filesystem::copy_options::recursive
	  | std::filesystem::copy_options::copy_symlinks;
      std::error_code ec;
      std::filesystem::copy(std::get<0>(*it), out, c_o_group, ec);
      if(ec)
	{
	  std::cout << "AddBook::simple_add_dir error: " << ec.message()
	      << " Source: " << std::get<0>(*it).u8string()
	      << " Collection path: " << out.u8string() << std::endl;
	}
    }
  std::atomic<bool> cancel;
  cancel.store(false);
  std::shared_ptr<RefreshCollection> rfr = std::make_shared<RefreshCollection>(
      af, collection_name, std::thread::hardware_concurrency(), &cancel, false,
      true, false, bookmarks);
  rfr->refreshCollection();

  remove_src(books);
}

void
AddBook::overwrite_archive_dir(
    const std::filesystem::path &archive_path,
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path> > &books)
{
  std::filesystem::remove_all(archive_path);
  std::filesystem::create_directories(archive_path.parent_path());

  LibArchive la;
  std::shared_ptr<archive> a = la.libarchive_write_init(archive_path);
  if(a)
    {
      for(auto it = books.begin(); it != books.end(); it++)
	{
	  std::shared_ptr<archive_entry> e(archive_entry_new2(a.get()), []
	  (archive_entry *e)
	    {
	      archive_entry_free(e);
	    });
	  la.libarchive_packing(a, std::get<0>(*it), true,
				std::get<1>(*it).u8string());
	}
    }
  a.reset();

  std::atomic<bool> cancel;
  cancel.store(false);
  std::shared_ptr<RefreshCollection> rfr = std::make_shared<RefreshCollection>(
      af, collection_name, std::thread::hardware_concurrency(), &cancel, false,
      true, false, bookmarks);

  BookBaseEntry bbe;
  bbe.file_path = archive_path;
  rfr->refreshFile(bbe);

  remove_src(books);
}

void
AddBook::add_to_existing_archive_dir(
    const std::filesystem::path &archive_path,
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path>> &books)
{
  LibArchive la;

  if(std::filesystem::exists(archive_path))
    {
      std::filesystem::path new_arch = archive_path.parent_path();
      new_arch /= std::filesystem::u8path(
	  af->randomFileName() + archive_path.filename().u8string());
      SelfRemovingPath main(new_arch);
      std::shared_ptr<ArchiveRemoveEntry> rment = std::make_shared<
	  ArchiveRemoveEntry>(
	  la.libarchive_remove_init(archive_path, new_arch));
      if(rment->a_read && rment->a_write && rment->fl)
	{
	  std::shared_ptr<archive_entry> entry(
	      archive_entry_new2(rment->a_read.get()), []
	      (archive_entry *e)
		{
		  archive_entry_free(e);
		});
	  bool interrupt = false;
	  int er, er_write;
	  er_write = ARCHIVE_OK;
	  std::filesystem::path loc = archive_path.parent_path();
	  while(!interrupt)
	    {
	      archive_entry_clear(entry.get());
	      er = archive_read_next_header2(rment->a_read.get(), entry.get());
	      switch(er)
		{
		case ARCHIVE_OK:
		  {
		    std::filesystem::path p = loc;
		    p /= std::filesystem::u8path(af->randomFileName());
		    SelfRemovingPath srp(p);

		    p = la.libarchive_read_entry(rment->a_read.get(),
						 entry.get(), srp.path);
		    if(std::filesystem::exists(p))
		      {
			er_write = archive_write_header(rment->a_write.get(),
							entry.get());
			if(er_write == ARCHIVE_OK || er_write == ARCHIVE_WARN)
			  {
			    if(er_write == ARCHIVE_WARN)
			      {
				la.libarchive_error(
				    rment->a_write,
				    "AddBook::add_to_existing_archive_dir write header warning:",
				    er);
			      }
			    if(!std::filesystem::is_directory(p))
			      {
				er_write = la.libarchive_write_data_from_file(
				    rment->a_write.get(), p);
				if(er_write != ARCHIVE_OK)
				  {
				    throw MLException(
					"AddBook::add_to_existing_archive_dir data writing error");
				  }
			      }
			  }
			else
			  {
			    throw MLException(
				"AddBook::add_to_existing_archive_dir header writing error");
			  }
		      }
		    else
		      {
			throw MLException(
			    "AddBook::add_to_existing_archive_dir incorrect filename in archive");
		      }
		    break;
		  }
		case ARCHIVE_EOF:
		  {
		    interrupt = true;
		    break;
		  }
		case ARCHIVE_FATAL:
		  {
		    la.libarchive_error(
			rment->a_read,
			"AddBook::add_to_existing_archive_dir critical error:",
			er);
		    interrupt = true;
		    break;
		  }
		default:
		  {
		    la.libarchive_error(
			rment->a_read,
			"AddBook::add_to_existing_archive_dir error:", er);
		    break;
		  }
		}
	    }

	  if(er == ARCHIVE_EOF && er_write == ARCHIVE_OK)
	    {
	      for(auto it = books.begin(); it != books.end(); it++)
		{
		  la.libarchive_packing(rment->a_write, std::get<0>(*it), true,
					std::get<1>(*it).u8string());
		}
	      if(er_write == ARCHIVE_OK && std::filesystem::exists(main.path))
		{
		  rment.reset();
		  std::filesystem::remove_all(archive_path);
		  std::filesystem::rename(main.path, archive_path);
		}
	      else
		{
		  throw MLException(
		      "AddBook::add_to_existing_archive_dir error");
		}
	    }
	  else
	    {
	      throw MLException(
		  "AddBook::add_to_existing_archive_dir archives error");
	    }

	  remove_src(books);
	}
      else
	{
	  throw MLException(
	      "AddBook::add_to_existing_archive_dir error: archives have not been opened");
	}

      std::atomic<bool> cancel;
      cancel.store(false);
      std::shared_ptr<RefreshCollection> rfr = std::make_shared<
	  RefreshCollection>(af, collection_name,
			     std::thread::hardware_concurrency(), &cancel,
			     false, true, false, bookmarks);

      BookBaseEntry bbe;
      bbe.file_path = archive_path;
      rfr->refreshFile(bbe);
    }
}

void
AddBook::remove_src(
    const std::vector<std::tuple<std::filesystem::path, std::filesystem::path> > &books)
{
  if(remove_sources)
    {
      for(auto it = books.begin(); it != books.end(); it++)
	{
	  std::filesystem::remove_all(std::get<0>(*it));
	}
    }
}
