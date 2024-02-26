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
#include <ArchiveFileEntry.h>
#include <ArchiveRemoveEntry.h>
#include <BookParseEntry.h>
#include <MLException.h>
#include <RefreshCollection.h>
#include <RemoveBook.h>
#include <iostream>
#include <system_error>

RemoveBook::RemoveBook(const std::shared_ptr<AuxFunc> &af,
		       const BookBaseEntry &bbe, const std::string &col_name,
		       const std::shared_ptr<BookMarks> &bookmarks)
{
  this->af = af;
  this->bbe = bbe;
  this->col_name = col_name;
  this->bookmarks = bookmarks;
}

RemoveBook::~RemoveBook()
{

}

void
RemoveBook::removeBook()
{
  std::string ext = bbe.file_path.extension().u8string();
  ext = af->stringToLower(ext);
  if(ext == ".fb2" || ext == ".epub" || ext == ".pdf" || ext == ".djvu"
      || ext == ".rar")
    {
      std::filesystem::remove_all(bbe.file_path);
    }
  else
    {
      std::filesystem::path tmp = bbe.file_path.parent_path();
      tmp /= std::filesystem::u8path(af->randomFileName());
      SelfRemovingPath srp(tmp);

      tmp = archive_remove(srp);
      if(std::filesystem::exists(tmp))
	{
	  std::filesystem::remove_all(bbe.file_path);
	  std::error_code ec;
	  std::filesystem::copy(tmp, bbe.file_path, ec);
	  if(ec)
	    {
	      throw MLException(
		  "RemoveBook::removeBook error: " + ec.message());
	    }
	}
      else
	{
	  std::filesystem::remove_all(bbe.file_path);
	}
    }
  std::atomic<bool> cancel;
  cancel.store(false);
  std::shared_ptr<RefreshCollection> rc = std::make_shared<RefreshCollection>(
      af, col_name, 1, &cancel, false, false, true, bookmarks);
  rc->refreshFile(bbe);
}

std::filesystem::path
RemoveBook::archive_remove(const SelfRemovingPath &out_dir)
{
  std::filesystem::path result;

  std::string path_in_arch;

  BookBaseEntry bber = bbe;
  std::string::size_type n;
  std::string sstr = "\n";
  n = bber.bpe.book_path.find(sstr);
  if(n != std::string::npos)
    {
      path_in_arch = bber.bpe.book_path.substr(0, n);
      bber.bpe.book_path.erase(0, n + sstr.size());
    }
  else
    {
      if(!bber.bpe.book_path.empty())
	{
	  path_in_arch = bber.bpe.book_path;
	}
    }

  result = out_dir.path;
  std::filesystem::create_directories(result);
  result /= bbe.file_path.filename();

  std::shared_ptr<ArchiveRemoveEntry> are =
      std::make_shared<ArchiveRemoveEntry>(
	  libarchive_remove_init(bbe.file_path, result));

  bool interrupt = false;
  std::shared_ptr<archive_entry> entry(archive_entry_new2(are->a_read.get()), []
  (archive_entry *e)
    {
      archive_entry_free(e);
    });
  int er;
  std::string ch_fnm;
  std::filesystem::path ch_fbd = std::filesystem::u8path(path_in_arch);
  ch_fbd.replace_extension(std::filesystem::u8path(".fbd"));
  unsigned long int file_count = 0;
  while(!interrupt)
    {
      archive_entry_clear(entry.get());
      er = archive_read_next_header2(are->a_read.get(), entry.get());
      switch(er)
	{
	case ARCHIVE_OK:
	  {
	    char *chnm = const_cast<char*>(archive_entry_pathname_utf8(
		entry.get()));
	    if(chnm)
	      {
		ch_fnm = chnm;
	      }
	    else
	      {
		chnm = const_cast<char*>(archive_entry_pathname(entry.get()));
		if(chnm)
		  {
		    ch_fnm = chnm;
		  }
		else
		  {
		    std::cout << "RemoveBook::archive_remove file name error"
			<< std::endl;
		  }
	      }
	    std::filesystem::path ch_p = std::filesystem::u8path(ch_fnm);
	    std::string ext = ch_p.extension().u8string();
	    ext = af->stringToLower(ext);
	    ch_p.replace_extension(std::filesystem::u8path(ext));
	    if(ch_fnm != path_in_arch && ch_p != ch_fbd)
	      {
		std::filesystem::path tmp = out_dir.path;
		tmp /= std::filesystem::u8path(af->randomFileName());
		SelfRemovingPath srp(tmp);
		tmp = libarchive_read_entry(are->a_read.get(), entry.get(),
					    srp.path);
		er = archive_write_header(are->a_write.get(), entry.get());
		if(er == ARCHIVE_OK || er == ARCHIVE_WARN)
		  {
		    if(er == ARCHIVE_WARN)
		      {
			libarchive_error(
			    are->a_write,
			    "RemoveBook::archive_remove writing warning", er);
			er = ARCHIVE_OK;
		      }
		    if(!std::filesystem::is_directory(tmp))
		      {
			er = libarchive_write_data_from_file(are->a_write.get(),
							     tmp);
		      }
		  }
		if(er != ARCHIVE_OK)
		  {
		    std::cout << "RemoveBook::archive_remove writing error: "
			<< er << std::endl;
		  }
		else
		  {
		    file_count++;
		  }
	      }
	    else
	      {
		ch_p = std::filesystem::u8path(ch_fnm);
		std::string ext = ch_p.extension().u8string();
		ext = af->stringToLower(ext);
		if(ext != ".fb2" && ext != ".fbd" && ext != ".epub"
		    && ext != ".pdf" && ext != ".djvu" && ext != ".rar")
		  {
		    std::filesystem::path tmp = out_dir.path;
		    tmp /= std::filesystem::u8path(af->randomFileName());
		    keep_path = tmp;
		    tmp = libarchive_read_entry(are->a_read.get(), entry.get(),
						tmp);
		    bber.file_path = tmp;

		    std::shared_ptr<RemoveBook> rb =
			std::make_shared<RemoveBook>(af, bber, col_name,
						     bookmarks);
		    tmp = out_dir.path;
		    tmp /= std::filesystem::u8path(af->randomFileName());
		    SelfRemovingPath srp(tmp);
		    tmp = rb->archive_remove(srp);

		    if(std::filesystem::exists(tmp))
		      {
			std::shared_ptr<archive_entry> e_write(
			    archive_entry_new2(are->a_write.get()), []
			    (archive_entry *e)
			      {
				archive_entry_free(e);
			      });
			er = libarchive_write_file(are->a_write.get(),
						   e_write.get(), ch_p, tmp);
			if(er != ARCHIVE_OK)
			  {
			    std::cout << "RemoveBook::archive_remove: "
				"error on writing" << std::endl;
			  }
			else
			  {
			    file_count++;
			  }
		      }
		  }
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
	    libarchive_error(
		are->a_read,
		"RemoveBook::archive_remove fatal read error: "
		    + are->fl->file_path.u8string(),
		er);
	    interrupt = true;
	    break;
	  }
	default:
	  {
	    libarchive_error(are->a_read,
			     "RemoveBook::archive_remove read error:", er);
	    break;
	  }
	}
    }
  are.reset();

  if(file_count == 0)
    {
      std::filesystem::remove_all(result);
    }

  return result;
}
