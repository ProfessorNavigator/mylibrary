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

#include <BookMarks.h>
#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <MLException.h>
#include <stddef.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

BookMarks::BookMarks(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
  bookmp = af->homePath();
  bookmp /= std::filesystem::u8path(
      ".local/share/MyLibrary/BookMarks/bookmarks");
  std::thread *thr = new std::thread([this]
  {
    try
      {
	loadBookMarks();
      }
    catch(MLException &e)
      {
	std::cout << e.what() << std::endl;
      };
  });
  thr->detach();
  delete thr;
}

BookMarks::~BookMarks()
{
  saveBookMarks();
}

void
BookMarks::loadBookMarks()
{
  std::unique_lock<std::mutex> un_l(bookmarksmtx);
  std::fstream f;
  f.open(bookmp, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      size_t fsz;

      f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(f.tellg());
      f.seekg(0, std::ios_base::beg);

      size_t bytesr = 0;
      uint64_t val64;
      std::string buf;
      ByteOrder bo;
      const size_t val64_sz = sizeof(val64);
      while(bytesr < fsz)
	{
	  if(val64_sz > fsz - bytesr)
	    {
	      f.close();
	      throw MLException(
		  "BookMarks::loadBookMarks: wrong entry");
	    }
	  else
	    {
	      f.read(reinterpret_cast<char*>(&val64), val64_sz);
	      bytesr += val64_sz;
	      bo.set_little(val64);
	      val64 = bo;
	    }

	  if(static_cast<size_t>(val64) > fsz - bytesr)
	    {
	      f.close();
	      throw MLException(
		  "BookMarks::loadBookMarks: wrong entry size");
	    }
	  else
	    {
	      buf.clear();
	      buf.resize(static_cast<std::string::size_type>(val64));
	      f.read(buf.data(), buf.size());
	      bytesr += buf.size();
	    }
	  try
	    {
	      BookBaseEntry bbe = parse_entry(buf);
	      bookmarks.emplace_back(bbe);
	    }
	  catch(MLException &e)
	    {
	      bookmarks.clear();
	      std::cout << e.what() << std::endl;
	      break;
	    }
	}
      f.close();
    }
}

BookBaseEntry
BookMarks::parse_entry(std::string &buf)
{
  BookBaseEntry result;

  uint16_t val16;
  ByteOrder bo;
  const size_t val16_sz = sizeof(val16);
  std::string readval;
  for(int i = 1; i <= 7; i++)
    {
      if(buf.size() < val16_sz)
	{
	  throw MLException("BookMarks::parse_entry: wrong size");
	}
      else
	{
	  std::memcpy(&val16, &buf[0], val16_sz);
	  buf.erase(0, val16_sz);
	  bo.set_little(val16);
	  val16 = bo;
	}
      if(buf.size() < static_cast<size_t>(val16))
	{
	  throw MLException("BookMarks::parse_entry: wrong size(2)");
	}
      else
	{
	  readval.clear();
	  readval = std::string(buf.begin(), buf.begin() + val16);
	  buf.erase(buf.begin(), buf.begin() + val16);
	}
      switch(i)
	{
	case 1:
	  {
	    result.file_path = std::filesystem::u8path(readval);
	    break;
	  }
	case 2:
	  {
	    result.bpe.book_path = readval;
	    break;
	  }
	case 3:
	  {
	    result.bpe.book_author = readval;
	    break;
	  }
	case 4:
	  {
	    result.bpe.book_name = readval;
	    break;
	  }
	case 5:
	  {
	    result.bpe.book_series = readval;
	    break;
	  }
	case 6:
	  {
	    result.bpe.book_genre = readval;
	    break;
	  }
	case 7:
	  {
	    result.bpe.book_date = readval;
	    break;
	  }
	default:
	  break;
	}
    }

  return result;
}

bool
BookMarks::saveBookMarks()
{
  bool result = false;

  bookmarksmtx.lock();
  if(bookmarks.size() > 0)
    {
      std::filesystem::create_directories(bookmp.parent_path());
      std::fstream f;
      f.open(bookmp, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
	{
	  result = true;
	  uint64_t val64;
	  const size_t val64_sz = sizeof(val64);
	  std::string entry;
	  ByteOrder bo;

	  for(auto it = bookmarks.begin(); it != bookmarks.end();
	      it++)
	    {
	      entry = form_entry(*it);
	      val64 = static_cast<uint64_t>(entry.size());
	      bo = val64;
	      bo.get_little(val64);
	      f.write(reinterpret_cast<char*>(&val64), val64_sz);
	      f.write(entry.c_str(), entry.size());
	    }
	  f.close();
	}
    }
  bookmarksmtx.unlock();

  return result;
}

int
BookMarks::createBookMark(const BookBaseEntry &bbe)
{
  int result = 0;

  bool save = false;

  bookmarksmtx.lock();
  auto itbm = std::find(bookmarks.begin(), bookmarks.end(), bbe);
  if(itbm == bookmarks.end())
    {
      bookmarks.push_back(bbe);
      save = true;
    }
  bookmarksmtx.unlock();

  if(save)
    {
      if(saveBookMarks())
	{
	  result = 1;
	}
      else
	{
	  result = -1;
	}
    }

  return result;
}

std::vector<BookBaseEntry>
BookMarks::getBookMarks()
{
  std::vector<BookBaseEntry> result;

  bookmarksmtx.lock();
  result = bookmarks;
  bookmarksmtx.unlock();

  return result;
}

void
BookMarks::removeBookMark(const BookBaseEntry &bbe)
{
  bool save = false;

  bookmarksmtx.lock();
  auto itbm = std::find(bookmarks.begin(), bookmarks.end(), bbe);
  if(itbm != bookmarks.end())
    {
      bookmarks.erase(itbm);
      save = true;
    }
  bookmarksmtx.unlock();

  if(save)
    {
      saveBookMarks();
    }
}

std::string
BookMarks::form_entry(const BookBaseEntry &bbe)
{
  std::string result;
  uint16_t val16;
  ByteOrder bo;
  size_t sz;
  std::string val;
  const size_t val16_sz = sizeof(val16);
  for(int i = 1; i <= 7; i++)
    {
      switch(i)
	{
	case 1:
	  {
	    val = bbe.file_path.u8string();
	    break;
	  }
	case 2:
	  {
	    val = bbe.bpe.book_path;
	    break;
	  }
	case 3:
	  {
	    val = bbe.bpe.book_author;
	    break;
	  }
	case 4:
	  {
	    val = bbe.bpe.book_name;
	    break;
	  }
	case 5:
	  {
	    val = bbe.bpe.book_series;
	    break;
	  }
	case 6:
	  {
	    val = bbe.bpe.book_genre;
	    break;
	  }
	case 7:
	  {
	    val = bbe.bpe.book_date;
	    break;
	  }
	default:
	  break;
	}

      val16 = static_cast<uint16_t>(val.size());
      bo = val16;
      bo.get_little(val16);
      sz = result.size();
      result.resize(sz + val16_sz);
      std::memcpy(&result[sz], &val16, val16_sz);

      std::copy(val.begin(), val.end(), std::back_inserter(result));
    }

  return result;
}
