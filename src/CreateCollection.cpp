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

#include <ARCHParser.h>
#include <BookParseEntry.h>
#include <ByteOrder.h>
#include <CreateCollection.h>
#include <DJVUParser.h>
#include <EPUBParser.h>
#include <FB2Parser.h>
#include <MLException.h>
#include <PDFParser.h>
#include <stddef.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <system_error>
#include <thread>

CreateCollection::CreateCollection(const std::shared_ptr<AuxFunc> &af,
				   const std::filesystem::path &collection_path,
				   const std::filesystem::path &books_path,
				   const bool &rar_support,
				   const int &num_threads,
				   std::atomic<bool> *cancel) : Hasher(af)
{
  this->af = af;
  base_path = collection_path;
  base_path /= std::filesystem::u8path("base");
  this->books_path = books_path;
  this->rar_support = rar_support;
  if(num_threads > 0)
    {
      this->num_threads = num_threads;
    }
  this->cancel = cancel;
}

CreateCollection::~CreateCollection()
{
  if(base_strm.is_open())
    {
      base_strm.close();
    }
}

CreateCollection::CreateCollection(const std::shared_ptr<AuxFunc> &af,
				   const int &num_threads,
				   std::atomic<bool> *cancel) : Hasher(af)
{
  this->af = af;
  if(num_threads > 0)
    {
      this->num_threads = num_threads;
    }
  this->cancel = cancel;
}

void
CreateCollection::createCollection()
{
  if(!std::filesystem::exists(books_path))
    {
      return void();
    }
  for(auto &dirit : std::filesystem::recursive_directory_iterator(
      books_path, std::filesystem::directory_options::follow_directory_symlink))
    {
      if(pulse)
	{
	  pulse();
	}
      std::filesystem::path p = dirit.path();
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
	      need_to_parse.push_back(p);
	    }
	}
    }
  if(!rar_support)
    {
      std::string ext;
      for(auto it = need_to_parse.begin(); it != need_to_parse.end();)
	{
	  ext = it->extension().u8string();
	  ext = af->stringToLower(ext);
	  if(ext == ".rar")
	    {
	      need_to_parse.erase(it);
	    }
	  else
	    {
	      it++;
	    }
	}
    }
  if(total_file_number)
    {
      total_file_number(static_cast<double>(need_to_parse.size()));
    }
  threadRegulator();
}

void
CreateCollection::threadRegulator()
{
  if(need_to_parse.size() > 0)
    {
      openBaseFile();
    }
  else
    {
      return void();
    }

  run_threads = 0;
  std::string ext;
  for(size_t i = 0; i < need_to_parse.size(); i++)
    {
      if(cancel->load() != 0)
	{
	  break;
	}
      std::filesystem::path p = need_to_parse[i];
      std::filesystem::path resolved;
      if(std::filesystem::symlink_status(p).type()
	  == std::filesystem::file_type::symlink)
	{
	  resolved = std::filesystem::read_symlink(p);
	  ext = resolved.extension().u8string();
	}
      else
	{
	  ext = p.extension().u8string();
	}
      ext = af->stringToLower(ext);
      std::unique_lock<std::mutex> lk(newthrmtx);
      std::thread *thr = nullptr;
      if(ext == ".fb2")
	{
	  run_threads++;
	  thr = new std::thread([this, p, resolved]
	  {
	    try
	      {
		this->fb2_thread(p, resolved);
	      }
	    catch(MLException &er)
	      {
		std::cout << er.what() << std::endl;
		std::lock_guard<std::mutex> lk(this->newthrmtx);
		this->run_threads--;
		this->add_thread.notify_one();
	      }
	  });
	}
      else if(ext == ".epub")
	{
	  run_threads++;
	  thr = new std::thread([this, p, resolved]
	  {
	    try
	      {
		this->epub_thread(p, resolved);
	      }
	    catch(MLException &er)
	      {
		std::cout << er.what() << std::endl;
		std::lock_guard<std::mutex> lk(this->newthrmtx);
		this->run_threads--;
		this->add_thread.notify_one();
	      }
	  });
	}
      else if(ext == ".pdf")
	{
	  run_threads++;
	  thr = new std::thread([this, p, resolved]
	  {
	    try
	      {
		this->pdf_thread(p, resolved);
	      }
	    catch(MLException &er)
	      {
		std::cout << er.what() << std::endl;
		std::lock_guard<std::mutex> lk(this->newthrmtx);
		this->run_threads--;
		this->add_thread.notify_one();
	      }
	  });
	}
      else if(ext == ".djvu")
	{
	  run_threads++;
	  thr = new std::thread([this, p, resolved]
	  {
	    try
	      {
		this->djvu_thread(p, resolved);
	      }
	    catch(MLException &er)
	      {
		std::cout << er.what() << std::endl;
		std::lock_guard<std::mutex> lk(this->newthrmtx);
		this->run_threads--;
		this->add_thread.notify_one();
	      }
	  });
	}
      else
	{
	  run_threads++;
	  thr = new std::thread([this, p, resolved]
	  {
	    try
	      {
		this->arch_thread(p, resolved);
	      }
	    catch(MLException &er)
	      {
		std::cout << er.what() << std::endl;
		std::lock_guard<std::mutex> lk(this->newthrmtx);
		this->run_threads--;
		this->add_thread.notify_one();
	      }
	  });
	}
      thr->detach();
      delete thr;
      add_thread.wait(lk, [this]
      {
	return this->run_threads < this->num_threads;
      });
      if(progress)
	{
	  progress(static_cast<double>(i + 1));
	}
    }

  std::unique_lock<std::mutex> lk(newthrmtx);
  add_thread.wait(lk, [this]
  {
    return this->run_threads <= 0;
  });

  if(base_strm.is_open())
    {
      base_strm.close();
    }
}

void
CreateCollection::fb2_thread(const std::filesystem::path &file_col_path,
			     const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  std::error_code ec;
  uintmax_t fsz = std::filesystem::file_size(filepath, ec);
  if(ec)
    {
      throw MLException("CreateCollection::fb2_thread: " + ec.message());
    }

  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string book;
      book.resize(static_cast<std::string::size_type>(fsz));
      f.read(book.data(), book.size());
      f.close();

      FB2Parser fb2(af);
      BookParseEntry be = fb2.fb2_parser(book);
      if(be.book_name.empty())
	{
	  be.book_name = filepath.stem().u8string();
	}

      FileParseEntry fe;
      fe.file_rel_path =
	  file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(already_hashed.begin(), already_hashed.end(),
				[file_col_path]
				(auto &el)
				  {
				    return std::get<0>(el) == file_col_path;
				  });
      if(ithsh != already_hashed.end())
	{
	  fe.file_hash = std::get<1>(*ithsh);
	}
      else
	{
	  fe.file_hash = buf_hashing(book);
	}
      fe.books.push_back(be);
      write_file_to_base(fe);
    }

  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  add_thread.notify_one();
}

void
CreateCollection::openBaseFile()
{
  std::filesystem::create_directories(base_path.parent_path());
  if(!base_strm.is_open())
    {
      base_strm.open(
	  base_path,
	  std::ios_base::out | std::ios_base::app | std::ios_base::binary);
    }
  if(!base_strm.is_open())
    {
      throw MLException(
	  "CreateCollection::openBaseFile: cannot open base file");
    }
  else
    {
      uintmax_t fsz;
      base_strm.seekg(0, std::ios_base::end);
      fsz = base_strm.tellg();
      if(fsz == 0)
	{
	  base_strm.seekg(0, std::ios_base::beg);
	  std::string col_path = books_path.u8string();
	  if(col_path.size() > 0)
	    {
	      std::string::size_type n = 0;
	      std::string sstr = "\\";
	      for(;;)
		{
		  n = col_path.find(sstr, n);
		  if(n != std::string::npos)
		    {
		      col_path.erase(n, sstr.size());
		      col_path.insert(n, "/");
		    }
		  else
		    {
		      break;
		    }
		}
	    }
	  uint16_t sz = static_cast<uint16_t>(col_path.size());
	  ByteOrder bo;
	  bo = sz;
	  bo.get_little(sz);
	  base_strm.write(reinterpret_cast<char*>(&sz), sizeof(sz));
	  base_strm.write(col_path.c_str(), col_path.size());
	}
    }
}

void
CreateCollection::closeBaseFile()
{
  if(base_strm.is_open())
    {
      base_strm.close();
    }
}

void
CreateCollection::book_entry_to_file_entry(std::string &file_entry,
					   const std::string &book_entry)
{
  uint16_t val16 = static_cast<uint16_t>(book_entry.size());
  ByteOrder bo(val16);
  bo.get_little(val16);
  size_t sz = file_entry.size();
  file_entry.resize(sz + sizeof(val16));
  std::memcpy(&file_entry[sz], &val16, sizeof(val16));

  std::copy(book_entry.begin(), book_entry.end(),
	    std::back_inserter(file_entry));
}

void
CreateCollection::epub_thread(const std::filesystem::path &file_col_path,
			      const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  FileParseEntry fe;

  auto ithsh = std::find_if(already_hashed.begin(), already_hashed.end(),
			    [file_col_path]
			    (auto &el)
			      {
				return std::get<0>(el) == file_col_path;
			      });
  if(ithsh != already_hashed.end())
    {
      fe.file_hash = std::get<1>(*ithsh);
    }
  else
    {
      fe.file_hash = file_hashing(filepath);
    }

  fe.file_rel_path = file_col_path.lexically_proximate(books_path).u8string();

  EPUBParser epub(af);
  BookParseEntry be = epub.epub_parser(filepath);
  if(be.book_name.empty())
    {
      be.book_name = filepath.stem().u8string();
    }
  fe.books.push_back(be);

  write_file_to_base(fe);

  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  add_thread.notify_one();
}

void
CreateCollection::pdf_thread(const std::filesystem::path &file_col_path,
			     const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  std::error_code ec;
  uintmax_t fsz = std::filesystem::file_size(filepath, ec);
  if(ec)
    {
      throw MLException("CreateCollection::pdf_thread: " + ec.message());
    }

  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string book;
      book.resize(fsz);
      f.read(book.data(), book.size());
      f.close();

      PDFParser pdf(af);
      BookParseEntry be = pdf.pdf_parser(book);
      if(be.book_name.empty())
	{
	  be.book_name = filepath.stem().u8string();
	}

      FileParseEntry fe;
      fe.file_rel_path =
	  file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(already_hashed.begin(), already_hashed.end(),
				[file_col_path]
				(auto &el)
				  {
				    return std::get<0>(el) == file_col_path;
				  });
      if(ithsh != already_hashed.end())
	{
	  fe.file_hash = std::get<1>(*ithsh);
	}
      else
	{
	  fe.file_hash = buf_hashing(book);
	}

      fe.books.push_back(be);
      write_file_to_base(fe);
    }

  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  add_thread.notify_one();
}

void
CreateCollection::arch_thread(const std::filesystem::path &file_col_path,
			      const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  ARCHParser arp(af, rar_support, cancel);
  FileParseEntry fe;
  fe.books = arp.arch_parser(filepath);
  if(fe.books.size() > 0)
    {
      fe.file_rel_path =
	  file_col_path.lexically_proximate(books_path).u8string();
      auto ithsh = std::find_if(already_hashed.begin(), already_hashed.end(),
				[file_col_path]
				(auto &el)
				  {
				    return std::get<0>(el) == file_col_path;
				  });
      if(ithsh != already_hashed.end())
	{
	  fe.file_hash = std::get<1>(*ithsh);
	}
      else
	{
	  fe.file_hash = file_hashing(filepath);
	}
      write_file_to_base(fe);
    }

  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  add_thread.notify_one();
}

void
CreateCollection::write_file_to_base(const FileParseEntry &fe)
{
  std::string file_entry;
  // File address entry
  std::string sep_cor = fe.file_rel_path;
  if(sep_cor.size() > 0)
    {

      std::string sstr = "\\";
      std::string::size_type n = 0;
      for(;;)
	{
	  n = sep_cor.find(sstr, n);
	  if(n != std::string::npos)
	    {
	      sep_cor.erase(n, sstr.size());
	      sep_cor.insert(n, "/");
	    }
	  else
	    {
	      break;
	    }
	}
    }
  uint16_t val16 = static_cast<uint16_t>(sep_cor.size());
  ByteOrder bo;
  bo = val16;
  bo.get_little(val16);
  file_entry.resize(sizeof(val16));
  std::memcpy(&file_entry[0], &val16, sizeof(val16));

  std::copy(sep_cor.begin(), sep_cor.end(), std::back_inserter(file_entry));

  // Book hash
  book_entry_to_file_entry(file_entry, fe.file_hash);
  uint64_t val64;
  size_t sz;
  for(auto it = fe.books.begin(); it != fe.books.end(); it++)
    {
      BookParseEntry be = *it;
      // Book entry size
      sz = be.book_author.size() + be.book_date.size() + be.book_genre.size()
	  + be.book_name.size() + be.book_path.size() + be.book_series.size();
      sz += 12; // size of size fields

      val64 = static_cast<uint64_t>(sz);
      bo = val64;
      bo.get_little(val64);
      sz = file_entry.size();
      file_entry.resize(sz + sizeof(val64));
      std::memcpy(&file_entry[sz], &val64, sizeof(val64));

      // Book entries
      for(int i = 1; i <= 6; i++)
	{
	  switch(i)
	    {
	    case 1:
	      {
		book_entry_to_file_entry(file_entry, be.book_path);
		break;
	      }
	    case 2:
	      {
		book_entry_to_file_entry(file_entry, be.book_author);
		break;
	      }
	    case 3:
	      {
		book_entry_to_file_entry(file_entry, be.book_name);
		break;
	      }
	    case 4:
	      {
		book_entry_to_file_entry(file_entry, be.book_series);
		break;
	      }
	    case 5:
	      {
		book_entry_to_file_entry(file_entry, be.book_genre);
		break;
	      }
	    case 6:
	      {
		book_entry_to_file_entry(file_entry, be.book_date);
		break;
	      }
	    default:
	      break;
	    }
	}
    }
  val64 = static_cast<uint64_t>(file_entry.size());
  bo = val64;
  bo.get_little(val64);

  base_strm_mtx.lock();
  base_strm.write(reinterpret_cast<char*>(&val64), sizeof(val64));
  base_strm.write(file_entry.c_str(), file_entry.size());
  base_strm_mtx.unlock();
}

void
CreateCollection::djvu_thread(const std::filesystem::path &file_col_path,
			      const std::filesystem::path &resolved)
{
  std::filesystem::path filepath;
  if(std::filesystem::exists(resolved))
    {
      filepath = resolved;
    }
  else
    {
      filepath = file_col_path;
    }

  DJVUParser djvu(af);
  BookParseEntry be = djvu.djvu_parser(filepath);

  FileParseEntry fe;
  fe.file_rel_path = file_col_path.lexically_proximate(books_path).u8string();
  auto ithsh = std::find_if(already_hashed.begin(), already_hashed.end(),
			    [file_col_path]
			    (auto &el)
			      {
				return std::get<0>(el) == file_col_path;
			      });
  if(ithsh != already_hashed.end())
    {
      fe.file_hash = std::get<1>(*ithsh);
    }
  else
    {
      fe.file_hash = file_hashing(filepath);
    }

  fe.books.push_back(be);
  write_file_to_base(fe);

  std::lock_guard<std::mutex> lk(newthrmtx);
  run_threads--;
  add_thread.notify_one();
}
