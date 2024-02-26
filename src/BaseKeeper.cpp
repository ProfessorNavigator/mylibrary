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

#include <BaseKeeper.h>
#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <MLException.h>
#include <stddef.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>

BaseKeeper::BaseKeeper(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  cancel_search.store(false);
}

BaseKeeper::~BaseKeeper()
{

}

void
BaseKeeper::loadCollection(const std::string &col_name)
{
  std::lock_guard<std::mutex> lock_base(basemtx);
  base.clear();
  collection_path.clear();
  collection_name = col_name;
  std::filesystem::path base_path = af->homePath();
  base_path /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
  base_path /= std::filesystem::u8path(col_name);
  base_path /= std::filesystem::u8path("base");

  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string base_str;
      f.seekg(0, std::ios_base::end);
      base_str.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(base_str.data(), base_str.size());
      f.close();

      uint16_t val16;
      if(base_str.size() >= sizeof(val16))
	{
	  std::memcpy(&val16, &base_str[0], sizeof(val16));
	  base_str.erase(0, sizeof(val16));
	}
      else
	{
	  collection_path.clear();
	  collection_name.clear();
	  throw MLException(
	      "BaseKeeper::loadCollection: collection path size reading error");
	}

      ByteOrder bo;
      bo.set_little(val16);
      val16 = bo;
      if(static_cast<size_t>(val16) > base_str.size())
	{
	  collection_path.clear();
	  collection_name.clear();
	  throw MLException(
	      "BaseKeeper::loadCollection: collection path reading error");
	}
      std::string ent(base_str.begin(), base_str.begin() + val16);
      base_str.erase(0, ent.size());
      collection_path = std::filesystem::u8path(ent);
      if(!std::filesystem::exists(collection_path))
	{
	  collection_path.clear();
	  collection_name.clear();
	  throw MLException(
	      "BaseKeeper::loadCollection: collection path error");
	}
      while(!base_str.empty())
	{
	  try
	    {
	      base.push_back(readFileEntry(base_str));
	    }
	  catch(MLException &er)
	    {
	      collection_path.clear();
	      collection_name.clear();
	      base.clear();
	      std::cout << er.what() << std::endl;
	      return void();
	    }
	}
      base.shrink_to_fit();
    }
}

FileParseEntry
BaseKeeper::readFileEntry(std::string &base_str)
{
  uint64_t val64;
  size_t sz = sizeof(val64);
  if(base_str.size() < sz)
    {
      throw MLException("BaseKeeper::readFileEntry: incorrect base size");
    }
  std::memcpy(&val64, &base_str[0], sz);
  base_str.erase(0, sz);

  ByteOrder bo;
  bo.set_little(val64);
  val64 = bo;

  if(val64 == 0 || base_str.size() < static_cast<size_t>(val64))
    {
      throw MLException("BaseKeeper::readFileEntry: incorrect entry size");
    }

  std::string entry(base_str.begin(), base_str.begin() + val64);
  base_str.erase(0, entry.size());

  uint16_t val16;
  sz = sizeof(val16);
  if(entry.size() < sz)
    {
      throw MLException(
	  "BaseKeeper::readFileEntry: incorrect file address entry size");
    }
  std::memcpy(&val16, &entry[0], sz);
  entry.erase(0, sz);
  bo.set_little(val16);
  val16 = bo;

  if(val16 == 0 || entry.size() < static_cast<size_t>(val16))
    {
      throw MLException(
	  "BaseKeeper::readFileEntry: incorrect file address size");
    }

  FileParseEntry fpe;

  std::string e(entry.begin(), entry.begin() + val16);
  entry.erase(0, e.size());
  fpe.file_rel_path = e;

  sz = sizeof(val16);
  if(entry.size() < sz)
    {
      throw MLException(
	  "BaseKeeper::readFileEntry: incorrect file hash entry size");
    }
  std::memcpy(&val16, &entry[0], sz);
  entry.erase(0, sz);
  bo.set_little(val16);
  val16 = bo;

  if(val16 == 0 || entry.size() < static_cast<size_t>(val16))
    {
      throw MLException("BaseKeeper::readFileEntry: incorrect file hash size");
    }
  e = std::string(entry.begin(), entry.begin() + val16);
  entry.erase(0, val16);
  fpe.file_hash = e;

  fpe.books = readBookEntry(entry);

  return fpe;
}

std::vector<BookParseEntry>
BaseKeeper::readBookEntry(std::string &entry)
{
  std::vector<BookParseEntry> result;
  uint64_t val64;
  ByteOrder bo;
  std::string book_e;
  size_t sz;
  while(entry.size() > 0)
    {
      book_e.clear();
      sz = sizeof(val64);
      if(entry.size() < sz)
	{
	  throw MLException(
	      "BaseKeeper::readBookEntry: incorrect book entry size");
	}
      std::memcpy(&val64, &entry[0], sz);
      entry.erase(0, sz);
      bo.set_little(val64);
      val64 = bo;
      if(val64 == 0 || entry.size() < static_cast<size_t>(val64))
	{
	  throw MLException(
	      "BaseKeeper::readBookEntry: incorrect book entry size(2)");
	}
      book_e = std::string(entry.begin(), entry.begin() + val64);
      entry.erase(0, book_e.size());

      BookParseEntry bpe;
      for(int i = 1; i <= 6; i++)
	{
	  switch(i)
	    {
	    case 1:
	      {
		parseBookEntry(book_e, bpe.book_path);
		break;
	      }
	    case 2:
	      {
		parseBookEntry(book_e, bpe.book_author);
		break;
	      }
	    case 3:
	      {
		parseBookEntry(book_e, bpe.book_name);
		break;
	      }
	    case 4:
	      {
		parseBookEntry(book_e, bpe.book_series);
		break;
	      }
	    case 5:
	      {
		parseBookEntry(book_e, bpe.book_genre);
		break;
	      }
	    case 6:
	      {
		parseBookEntry(book_e, bpe.book_date);
		break;
	      }
	    default:
	      break;
	    }
	}
      result.push_back(bpe);
    }

  return result;
}

void
BaseKeeper::parseBookEntry(std::string &e, std::string &read_val)
{
  uint16_t val16;
  size_t sz = sizeof(val16);
  if(e.size() < sz)
    {
      throw MLException(
	  "BaseKeeper::parseBookEntry: incorrect book entry size");
    }
  std::memcpy(&val16, &e[0], sz);
  e.erase(0, sz);

  ByteOrder bo;
  bo.set_little(val16);
  val16 = bo;
  if(val16 > 0)
    {
      read_val = std::string(e.begin(), e.begin() + val16);
      e.erase(0, read_val.size());
    }
}

std::vector<BookBaseEntry>
BaseKeeper::searchBook(const BookBaseEntry &search)
{
  std::lock_guard<std::mutex> lock_base(basemtx);
  cancel_search.store(false);
  std::vector<BookBaseEntry> result;
  bool stop_search = false;
  bool all_empty = true;
  for(int i = 1; i <= 6; i++)
    {
      switch(i)
	{
	case 1:
	  {
	    if(!searchSurname(search, result))
	      {
		if(result.size() == 0)
		  {
		    stop_search = true;
		  }
		all_empty = false;
	      }
	    break;
	  }
	case 2:
	  {
	    if(!searchFirstName(search, result))
	      {
		if(result.size() == 0)
		  {
		    stop_search = true;
		  }
		all_empty = false;
	      }
	    break;
	  }
	case 3:
	  {
	    if(!searchLastName(search, result))
	      {
		if(result.size() == 0)
		  {
		    stop_search = true;
		  }
		all_empty = false;
	      }
	    break;
	  }
	case 4:
	  {
	    if(!search.bpe.book_name.empty())
	      {
		searchBook(search, result);
		if(result.size() == 0)
		  {
		    stop_search = true;
		  }
		all_empty = false;
	      }
	    break;
	  }
	case 5:
	  {
	    if(!search.bpe.book_series.empty())
	      {
		searchSeries(search, result);
		if(result.size() == 0)
		  {
		    stop_search = true;
		  }
		all_empty = false;
	      }
	    break;
	  }
	case 6:
	  {
	    if(!search.bpe.book_genre.empty())
	      {
		searchGenre(search, result);
		if(result.size() == 0)
		  {
		    stop_search = true;
		  }
		all_empty = false;
	      }
	    break;
	  }
	default:
	  break;
	}
      if(stop_search)
	{
	  break;
	}
    }
  if(all_empty)
    {
      for(auto it = base.begin(); it != base.end(); it++)
	{
	  if(cancel_search.load())
	    {
	      break;
	    }
	  std::filesystem::path book_file_path = collection_path;
	  book_file_path /= std::filesystem::u8path(it->file_rel_path);
	  for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
	    {
	      result.push_back(BookBaseEntry(*itb, book_file_path));
	    }
	}
    }
  if(cancel_search.load())
    {
      result.clear();
    }
  result.shrink_to_fit();

  return result;
}

bool
BaseKeeper::searchLineFunc(const std::string &to_search,
			   const std::string &source)
{
  std::string loc_search = af->stringToLower(to_search);
  std::string loc_source = af->stringToLower(source);
  std::string::size_type n;
  n = loc_source.find(loc_search);
  if(n != std::string::npos)
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool
BaseKeeper::searchSurname(const BookBaseEntry &search,
			  std::vector<BookBaseEntry> &result)
{
  bool all_empty = true;
  std::string surname = search.bpe.book_author;
  std::string::size_type n = surname.find("\n");
  if(n != std::string::npos)
    {
      surname = surname.substr(0, n);
      if(!surname.empty())
	{
	  all_empty = false;
	  for(auto it = base.begin(); it != base.end(); it++)
	    {
	      if(cancel_search.load())
		{
		  break;
		}

	      std::filesystem::path book_file_path = collection_path;
	      book_file_path /= std::filesystem::u8path(it->file_rel_path);
	      for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
		{
		  if(searchLineFunc(surname, itb->book_author))
		    {
		      result.push_back(BookBaseEntry(*itb, book_file_path));
		    }
		}
	    }
	}
    }
  return all_empty;
}

void
BaseKeeper::searchBook(const BookBaseEntry &search,
		       std::vector<BookBaseEntry> &result)
{
  if(result.size() == 0)
    {
      for(auto it = base.begin(); it != base.end(); it++)
	{
	  if(cancel_search.load())
	    {
	      break;
	    }
	  std::filesystem::path book_file_path = collection_path;
	  book_file_path /= std::filesystem::u8path(it->file_rel_path);
	  for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
	    {
	      if(searchLineFunc(search.bpe.book_name, itb->book_name))
		{
		  result.push_back(BookBaseEntry(*itb, book_file_path));
		}
	    }
	}
    }
  else
    {
      result.erase(std::remove_if(result.begin(), result.end(), [search, this]
      (auto &el)
	{
	  if(this->searchLineFunc(search.bpe.book_name, el.bpe.book_name))
	    {
	      return false;
	    }
	  else
	    {
	      return true;
	    }
	}),
		   result.end());
    }
}

void
BaseKeeper::searchSeries(const BookBaseEntry &search,
			 std::vector<BookBaseEntry> &result)
{
  if(result.size() == 0)
    {
      for(auto it = base.begin(); it != base.end(); it++)
	{
	  if(cancel_search.load())
	    {
	      break;
	    }
	  std::filesystem::path book_file_path = collection_path;
	  book_file_path /= std::filesystem::u8path(it->file_rel_path);
	  for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
	    {
	      if(searchLineFunc(search.bpe.book_series, itb->book_series))
		{
		  result.push_back(BookBaseEntry(*itb, book_file_path));
		}
	    }
	}
    }
  else
    {
      result.erase(std::remove_if(result.begin(), result.end(), [search, this]
      (auto &el)
	{
	  if(this->searchLineFunc(search.bpe.book_series, el.bpe.book_series))
	    {
	      return false;
	    }
	  else
	    {
	      return true;
	    }
	}),
		   result.end());
    }
}

void
BaseKeeper::clearBase()
{
  std::lock_guard<std::mutex> lock_base(basemtx);
  collection_path.clear();
  collection_name.clear();
  base.clear();
  base.shrink_to_fit();
}

std::vector<FileParseEntry>
BaseKeeper::get_base_vector()
{
  std::lock_guard<std::mutex> lock_base(basemtx);
  return base;
}

std::filesystem::path
BaseKeeper::get_books_path(const std::string &collection_name,
			   const std::shared_ptr<AuxFunc> &af)
{
  std::filesystem::path result;

  std::filesystem::path base_path = af->homePath();
  base_path /= std::filesystem::u8path(".local/share/MyLibrary/Collections");
  base_path /= std::filesystem::u8path(collection_name);
  base_path /= std::filesystem::u8path("base");

  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t fsz;
      f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);
      uint16_t val16;
      size_t sz = sizeof(val16);
      if(fsz < sz)
	{
	  f.close();
	  throw MLException("BaseKeeper::get_books_path: base file error");
	}
      else
	{
	  f.read(reinterpret_cast<char*>(&val16), sz);
	  ByteOrder bo;
	  bo.set_little(val16);
	  val16 = bo;
	}

      if(static_cast<size_t>(val16) + sz > fsz)
	{
	  f.close();
	  throw MLException(
	      "BaseKeeper::get_books_path: wrong books path size");
	}
      else
	{
	  std::string buf;
	  buf.resize(val16);
	  f.read(buf.data(), buf.size());
	  result = std::filesystem::u8path(buf);
	}

      f.close();
    }

  return result;
}

void
BaseKeeper::searchGenre(const BookBaseEntry &search,
			std::vector<BookBaseEntry> &result)
{
  if(result.size() == 0)
    {
      for(auto it = base.begin(); it != base.end(); it++)
	{
	  if(cancel_search.load())
	    {
	      break;
	    }
	  std::filesystem::path book_file_path = collection_path;
	  book_file_path /= std::filesystem::u8path(it->file_rel_path);
	  for(auto itb = it->books.begin(); itb != it->books.end(); itb++)
	    {
	      if(searchLineFunc(search.bpe.book_genre, itb->book_genre))
		{
		  result.push_back(BookBaseEntry(*itb, book_file_path));
		}
	    }
	}
    }
  else
    {
      result.erase(std::remove_if(result.begin(), result.end(), [search, this]
      (auto &el)
	{
	  if(this->searchLineFunc(search.bpe.book_genre, el.bpe.book_genre))
	    {
	      return false;
	    }
	  else
	    {
	      return true;
	    }
	}),
		   result.end());
    }
}

void
BaseKeeper::stopSearch()
{
  cancel_search.store(true);
}

bool
BaseKeeper::searchLastName(const BookBaseEntry &search,
			   std::vector<BookBaseEntry> &result)
{
  bool all_empty = true;
  std::string last_name = search.bpe.book_author;
  std::string sstr = "\n";
  std::string::size_type n = last_name.find(sstr);
  if(n != std::string::npos)
    {
      last_name.erase(0, n + sstr.size());
      n = last_name.find(sstr);
      if(n != std::string::npos)
	{
	  last_name.erase(0, n + sstr.size());
	  if(!last_name.empty())
	    {
	      all_empty = false;
	      if(result.size() == 0)
		{
		  for(auto it = base.begin(); it != base.end(); it++)
		    {
		      if(cancel_search.load())
			{
			  break;
			}
		      std::filesystem::path book_file_path = collection_path;
		      book_file_path /= std::filesystem::u8path(
			  it->file_rel_path);
		      for(auto itb = it->books.begin(); itb != it->books.end();
			  itb++)
			{
			  if(searchLineFunc(last_name, itb->book_author))
			    {
			      result.push_back(
				  BookBaseEntry(*itb, book_file_path));
			    }
			}
		    }
		}
	      else
		{
		  result.erase(
		      std::remove_if(
			  result.begin(),
			  result.end(),
			  [last_name, this]
			  (auto &el)
			    {
			      if(this->searchLineFunc(last_name, el.bpe.book_author))
				{
				  return false;
				}
			      else
				{
				  return true;
				}
			    }),
		      result.end());
		}
	    }
	}
    }
  return all_empty;
}

bool
BaseKeeper::searchFirstName(const BookBaseEntry &search,
			    std::vector<BookBaseEntry> &result)
{
  bool all_empty = true;
  std::string first_name = search.bpe.book_author;
  std::string sstr = "\n";
  std::string::size_type n = first_name.find(sstr);
  if(n != std::string::npos)
    {
      first_name.erase(0, n + sstr.size());
      n = first_name.find(sstr);
      if(n != std::string::npos)
	{
	  first_name = first_name.substr(0, n);
	  if(!first_name.empty())
	    {
	      all_empty = false;
	      if(result.size() == 0)
		{
		  for(auto it = base.begin(); it != base.end(); it++)
		    {
		      if(cancel_search.load())
			{
			  break;
			}
		      std::filesystem::path book_file_path = collection_path;
		      book_file_path /= std::filesystem::u8path(
			  it->file_rel_path);
		      for(auto itb = it->books.begin(); itb != it->books.end();
			  itb++)
			{
			  if(searchLineFunc(first_name, itb->book_author))
			    {
			      result.push_back(
				  BookBaseEntry(*itb, book_file_path));
			    }
			}
		    }
		}
	      else
		{
		  result.erase(
		      std::remove_if(
			  result.begin(),
			  result.end(),
			  [first_name, this]
			  (auto &el)
			    {
			      if(this->searchLineFunc(first_name, el.bpe.book_author))
				{
				  return false;
				}
			      else
				{
				  return true;
				}
			    }),
		      result.end());
		}
	    }
	}
    }
  return all_empty;
}
