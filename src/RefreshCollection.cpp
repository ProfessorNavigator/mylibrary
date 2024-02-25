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

#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <MLException.h>
#include <RefreshCollection.h>
#include <stddef.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <thread>
#include <tuple>

RefreshCollection::RefreshCollection(
    const std::shared_ptr<AuxFunc> &af, const std::string &collection_name,
    const int &num_threads, std::atomic<bool> *cancel, const bool &remove_empty,
    const bool &fast_refresh, const bool &refresh_bookmarks,
    const std::shared_ptr<BookMarks> &bookmarks) : CreateCollection(af,
								    num_threads,
								    cancel)
{
  this->af = af;
  if(num_threads > 0)
    {
      this->num_threads = num_threads;
    }
  this->cancel = cancel;
  base_path = get_base_path(collection_name);
  books_path = get_books_path();
  this->remove_empty = remove_empty;
  this->fast_refresh = fast_refresh;
  this->refresh_bookmarks = refresh_bookmarks;
  this->bookmarks = bookmarks;
  rar_support = true;
}

RefreshCollection::~RefreshCollection()
{

}

std::filesystem::path
RefreshCollection::get_base_path(const std::string &collection_name)
{
  std::filesystem::path result;

  result = af->homePath();
  result /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
  result /= std::filesystem::u8path(collection_name);
  result /= std::filesystem::u8path("base");

  return result;
}

std::filesystem::path
RefreshCollection::get_books_path()
{
  std::filesystem::path result;

  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t fsz = static_cast<size_t>(std::filesystem::file_size(base_path));
      uint16_t sz;
      if(fsz >= sizeof(sz))
	{
	  f.read(reinterpret_cast<char*>(&sz), sizeof(sz));
	}
      else
	{
	  f.close();
	  throw MLException(
	      "RefreshCollection::get_books_path: incorrect base file");
	}
      ByteOrder bo;
      bo.set_little(sz);
      sz = bo;

      if(fsz >= sizeof(sz) + static_cast<size_t>(sz))
	{
	  std::string buf;
	  buf.resize(sz);
	  f.read(buf.data(), buf.size());
	  result = std::filesystem::u8path(buf);
	}
      else
	{
	  f.close();
	  throw MLException(
	      "RefreshCollection::get_books_path: incorrect base file(2)");
	}

      f.close();
    }

  return result;
}

void
RefreshCollection::refreshCollection()
{
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();
  if(std::filesystem::exists(books_path))
    {
      std::vector<std::filesystem::path> books_files;
      std::vector<std::filesystem::path> empty_paths;
      for(auto &dirit : std::filesystem::recursive_directory_iterator(
	  books_path,
	  std::filesystem::directory_options::follow_directory_symlink))
	{
	  if(pulse)
	    {
	      pulse();
	    }
	  std::filesystem::path p = dirit.path();
	  if(remove_empty)
	    {
	      if(std::filesystem::is_empty(p))
		{
		  empty_paths.push_back(p);
		}
	      else
		{
		  if(!std::filesystem::is_directory(p))
		    {
		      std::filesystem::path check_type;
		      if(!(std::filesystem::symlink_status(p).type()
			  == std::filesystem::file_type::symlink))
			{
			  check_type = p;
			}
		      else
			{
			  check_type = std::filesystem::read_symlink(p);
			}
		      if(af->if_supported_type(check_type))
			{
			  books_files.push_back(p);
			}
		    }
		}
	    }
	  else
	    {
	      if(!std::filesystem::is_directory(p))
		{
		  std::filesystem::path check_type;
		  if(!(std::filesystem::symlink_status(p).type()
		      == std::filesystem::file_type::symlink))
		    {
		      check_type = p;
		    }
		  else
		    {
		      check_type = std::filesystem::read_symlink(p);
		    }
		  if(af->if_supported_type(check_type))
		    {
		      books_files.push_back(p);
		    }
		}
	    }
	}
      for(auto it = empty_paths.begin(); it != empty_paths.end(); it++)
	{
	  std::filesystem::remove_all(*it);
	}

      if(!rar_support)
	{
	  std::string ext;
	  for(auto it = books_files.begin(); it != books_files.end();)
	    {
	      ext = it->extension().u8string();
	      ext = af->stringToLower(ext);
	      if(ext == ".rar")
		{
		  books_files.erase(it);
		}
	      else
		{
		  it++;
		}
	    }
	}

      compaire_vectors(base, books_files);
      if(!fast_refresh)
	{
	  check_hashes(&base, &books_files);
	}

      std::filesystem::remove_all(base_path);
      openBaseFile();
      for(auto it = base.begin(); it != base.end(); it++)
	{
	  write_file_to_base(*it);
	}
      closeBaseFile();
      if(total_file_number)
	{
	  total_file_number(static_cast<double>(need_to_parse.size()));
	}
      threadRegulator();

      if(refresh_bookmarks)
	{
	  refreshBookMarks(bk);
	}
    }
  else
    {
      throw MLException(
	  "RefreshCollection::refreshCollection: books not found");
    }
}

void
RefreshCollection::compaire_vectors(
    std::vector<FileParseEntry> &base,
    std::vector<std::filesystem::path> &books_files)
{
  for(auto it = base.begin(); it != base.end();)
    {
      auto itbf = std::find_if(
	  books_files.begin(),
	  books_files.end(),
	  std::bind(&RefreshCollection::compare_function1, this,
		    std::placeholders::_1, *it));
      if(itbf == books_files.end())
	{
	  base.erase(it);
	}
      else
	{
	  it++;
	}
    }

  for(auto it = books_files.begin(); it != books_files.end(); it++)
    {
      auto itbase = std::find_if(
	  base.begin(),
	  base.end(),
	  std::bind(&RefreshCollection::compare_function2, this,
		    std::placeholders::_1, *it));
      if(itbase == base.end())
	{
	  need_to_parse.push_back(*it);
	}
    }
}

bool
RefreshCollection::compare_function1(const std::filesystem::path &book_path,
				     const FileParseEntry &ent)
{
  std::filesystem::path cur = std::filesystem::u8path(ent.file_rel_path);
  std::filesystem::path comp = book_path.lexically_proximate(books_path);
  if(cur == comp)
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool
RefreshCollection::compare_function2(const FileParseEntry &ent,
				     const std::filesystem::path &book_path)
{
  return compare_function1(book_path, ent);
}

void
RefreshCollection::refreshFile(const BookBaseEntry &bbe)
{
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();
  for(auto it = base.begin(); it != base.end();)
    {
      if(compare_function1(bbe.file_path, *it))
	{
	  base.erase(it);
	  break;
	}
      else
	{
	  it++;
	}
    }

  std::filesystem::remove_all(base_path);

  openBaseFile();
  for(auto it = base.begin(); it != base.end(); it++)
    {
      write_file_to_base(*it);
    }
  closeBaseFile();
  if(std::filesystem::exists(bbe.file_path))
    {
      need_to_parse.push_back(bbe.file_path);
      threadRegulator();
    }
  if(refresh_bookmarks)
    {
      refreshBookMarks(bk);
    }
}

void
RefreshCollection::check_hashes(std::vector<FileParseEntry> *base,
				std::vector<std::filesystem::path> *books_files)
{
  uintmax_t summ = 0;
  if(total_bytes_to_hash)
    {
      for(auto it = books_files->begin(); it != books_files->end(); it++)
	{
	  if(pulse)
	    {
	      pulse();
	    }
	  summ += std::filesystem::file_size(*it);
	}
      total_bytes_to_hash(static_cast<double>(summ));
    }
  bytes_summ.store(0);
  run_threads = 0;
  for(auto it = books_files->begin(); it != books_files->end(); it++)
    {
      if(cancel->load() != 0)
	{
	  break;
	}
      std::unique_lock<std::mutex> lk(newthrmtx);
      run_threads++;
      std::thread *thr = new std::thread(
	  std::bind(&RefreshCollection::hash_thread, this, *it, base));
      thr->detach();
      delete thr;
      continue_hashing.wait(lk, [this]
      {
	return this->run_threads < this->num_threads;
      });
    }
  std::unique_lock<std::mutex> lk(newthrmtx);
  continue_hashing.wait(lk, [this]
  {
    return this->run_threads <= 0;
  });
}

void
RefreshCollection::hash_thread(const std::filesystem::path &file_to_hash,
			       std::vector<FileParseEntry> *base)
{
  std::string hash;
  try
    {
      hash = file_hashing(file_to_hash);
    }
  catch(MLException &er)
    {
      std::cout << er.what() << std::endl;
      std::lock_guard<std::mutex> lk(newthrmtx);
      run_threads--;
      continue_hashing.notify_one();
    }
  already_hashedmtx.lock();
  already_hashed.push_back(std::make_tuple(file_to_hash, hash));
  basemtx.lock();
  auto itbase = std::find_if(
      base->begin(),
      base->end(),
      std::bind(&RefreshCollection::compare_function2, this,
		std::placeholders::_1, file_to_hash));
  if(itbase != base->end())
    {
      if(hash != itbase->file_hash)
	{
	  need_to_parsemtx.lock();
	  auto itntp = std::find(need_to_parse.begin(), need_to_parse.end(),
				 file_to_hash);
	  if(itntp == need_to_parse.end())
	    {
	      need_to_parse.push_back(file_to_hash);
	    }
	  need_to_parsemtx.unlock();
	  base->erase(itbase);
	}
    }
  else
    {
      need_to_parsemtx.lock();
      auto itntp = std::find(need_to_parse.begin(), need_to_parse.end(),
			     file_to_hash);
      if(itntp == need_to_parse.end())
	{
	  need_to_parse.push_back(file_to_hash);
	}
      need_to_parsemtx.unlock();
    }
  basemtx.unlock();
  already_hashedmtx.unlock();
  bytes_summ.store(
      bytes_summ.load() + std::filesystem::file_size(file_to_hash));
  if(bytes_hashed)
    {
      bytes_hashed(static_cast<double>(bytes_summ.load()));
    }

  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  continue_hashing.notify_one();
}

bool
RefreshCollection::editBook(const BookBaseEntry &bbe_old,
			    const BookBaseEntry &bbe_new)
{
  bool result = false;
  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();

  auto itbase = std::find_if(
      base.begin(),
      base.end(),
      std::bind(&RefreshCollection::compare_function2, this,
		std::placeholders::_1, bbe_old.file_path));
  if(itbase != base.end())
    {
      for(auto it = itbase->books.begin(); it != itbase->books.end(); it++)
	{
	  std::filesystem::path p = books_path;
	  p /= std::filesystem::u8path(itbase->file_rel_path);
	  BookBaseEntry bbe(*it, p);
	  if(bbe == bbe_old)
	    {
	      *it = bbe_new.bpe;
	      result = true;
	    }
	}
    }

  if(result)
    {
      std::filesystem::remove_all(base_path);
      openBaseFile();
      for(auto it = base.begin(); it != base.end(); it++)
	{
	  write_file_to_base(*it);
	}
      closeBaseFile();
      if(refresh_bookmarks)
	{
	  std::vector<BookBaseEntry> bmv = bookmarks->getBookMarks();
	  auto itbmv = std::find_if(bmv.begin(), bmv.end(), [bbe_old]
	  (auto &el)
	    {
	      return el == bbe_old;
	    });
	  if(itbmv != bmv.end())
	    {
	      bookmarks->removeBookMark(bbe_old);
	      bookmarks->createBookMark(bbe_new);
	    }
	}
    }

  return result;
}

bool
RefreshCollection::refreshBook(const BookBaseEntry &bbe)
{
  bool result = false;

  std::shared_ptr<BaseKeeper> bk = std::make_shared<BaseKeeper>(af);
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();

  auto itbase = std::find_if(
      base.begin(),
      base.end(),
      std::bind(&RefreshCollection::compare_function2, this,
		std::placeholders::_1, bbe.file_path));
  if(itbase != base.end())
    {
      auto itbpe = std::find_if(itbase->books.begin(), itbase->books.end(),
				[bbe]
				(auto &el)
				  {
				    return el == bbe.bpe;
				  });
      if(itbpe != itbase->books.end())
	{
	  *itbpe = bbe.bpe;
	  std::filesystem::remove_all(base_path);
	  openBaseFile();
	  for(auto it = base.begin(); it != base.end(); it++)
	    {
	      write_file_to_base(*it);
	    }
	  closeBaseFile();
	  result = true;
	}
    }

  return result;
}

void
RefreshCollection::set_rar_support(const bool &rar_support)
{
  this->rar_support = rar_support;
}

void
RefreshCollection::refreshBookMarks(const std::shared_ptr<BaseKeeper> &bk)
{
  bk->loadCollection(base_path.parent_path().filename().u8string());
  std::vector<FileParseEntry> base = bk->get_base_vector();

  std::vector<BookBaseEntry> bmv = bookmarks->getBookMarks();
  for(auto it = bmv.begin(); it != bmv.end(); it++)
    {
      auto itbase = std::find_if(
	  base.begin(),
	  base.end(),
	  std::bind(&RefreshCollection::compare_function2, this,
		    std::placeholders::_1, it->file_path));
      if(itbase != base.end())
	{
	  auto itbpe = std::find_if(itbase->books.begin(), itbase->books.end(),
				    [it]
				    (auto &el)
				      {
					return el == it->bpe;
				      });
	  if(itbpe == itbase->books.end())
	    {
	      bookmarks->removeBookMark(*it);
	    }
	}
      else
	{
	  bookmarks->removeBookMark(*it);
	}
    }
}
