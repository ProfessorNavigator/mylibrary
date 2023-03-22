/*
 Copyright 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>

 This file is part of MyLibrary.
 MyLibrary is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.
 MyLibrary is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with MyLibrary. If not,
 see <https://www.gnu.org/licenses/>.
 */

#include "RefreshCollection.h"

RefreshCollection::RefreshCollection(std::string collname, unsigned int thr_num,
				     bool fast_refresh, int *cancel)
{
  this->collname = collname;
  this->fast_refresh = fast_refresh;
  this->cancel = cancel;
  this->thr_num = thr_num;

}

RefreshCollection::~RefreshCollection()
{
  // TODO Auto-generated destructor stub
}

void
RefreshCollection::startRefreshing()
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(std::filesystem::exists(filepath))
    {
      if(bookpath.empty())
	{
	  for(auto &dirit : std::filesystem::directory_iterator(filepath))
	    {
	      std::filesystem::path p = dirit.path();
	      if(!std::filesystem::is_directory(p))
		{
		  std::string fnm_p = p.filename().u8string();
		  if(fnm_p.find("base") != std::string::npos
		      && bookpath.empty())
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in | std::ios_base::binary);
		      if(f.is_open())
			{
			  bookpath.resize(std::filesystem::file_size(p));
			  f.read(&bookpath[0], bookpath.size());
			  f.close();
			  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
			  bookpath.erase(0, std::string("<bp>").size());
			  if(!bookpath.empty())
			    {
			      break;
			    }
			}
		    }
		  else if(fnm_p.find("hash") != std::string::npos
		      && bookpath.empty())
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in);
		      if(f.is_open())
			{
			  getline(f, bookpath);
			  f.close();
			  if(!bookpath.empty())
			    {
			      break;
			    }
			}
		    }
		}
	    }
	}
    }

  std::filesystem::path book_path = std::filesystem::u8path(bookpath);
  if(std::filesystem::exists(book_path))
    {
      readList();
      readColl();
      for(;;)
	{
	  run_thrmtx.lock();
	  if(run_thr == 0)
	    {
	      break;
	    }
	  run_thrmtx.unlock();
	  usleep(100);
	}
      if(*cancel != 1)
	{
	  collRefresh();
	}
      else
	{
	  if(refresh_canceled)
	    {
	      refresh_canceled();
	    }
	}
    }
  else
    {
      if(collection_not_exists)
	{
	  collection_not_exists();
	}
    }
}

void
RefreshCollection::readList()
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(std::filesystem::exists(filepath))
    {
      filename = filename + "/fb2hash";
      filepath = std::filesystem::u8path(filename);
      readHash(filepath);

      filename = filepath.parent_path().u8string();
      filename = filename + "/ziphash";
      filepath = std::filesystem::u8path(filename);
      readHash(filepath);

      filename = filepath.parent_path().u8string();
      filename = filename + "/epubhash";
      filepath = std::filesystem::u8path(filename);
      readHash(filepath);

      filename = filepath.parent_path().u8string();
      filename = filename + "/pdfhash";
      filepath = std::filesystem::u8path(filename);
      readHash(filepath);

      filename = filepath.parent_path().u8string();
      filename = filename + "/djvuhash";
      filepath = std::filesystem::u8path(filename);
      readHash(filepath);
    }
}

void
RefreshCollection::readHash(std::filesystem::path filepath)
{
  std::fstream f;
  f.open(filepath, std::ios_base::in);
  if(!f.is_open())
    {
      std::cerr
	  << "RefreshCollecttion: " + filepath.stem().u8string()
	      + " file not opened" << std::endl;
    }
  else
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  std::tuple<std::filesystem::path, std::string> ttup;
		  std::string p_str = line;
		  p_str = p_str.substr(0, p_str.find("<?>"));
		  p_str = bookpath + p_str;
		  std::get<0>(ttup) = std::filesystem::u8path(p_str);
		  p_str = line;
		  p_str.erase(0, p_str.find("<?>") + std::string("<?>").size());
		  std::get<1>(ttup) = p_str;
		  saved_hashes.push_back(ttup);
		}
	    }
	  count++;
	}
      f.close();
    }
}

void
RefreshCollection::readColl()
{
  std::filesystem::path filepath = std::filesystem::u8path(bookpath);
  AuxFunc af;
  std::vector<std::filesystem::path> fb2;
  std::vector<std::filesystem::path> epub;
  std::vector<std::filesystem::path> pdf;
  std::vector<std::filesystem::path> djvu;
  std::vector<
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>>> zip;
  if(std::filesystem::exists(filepath))
    {
      for(auto &dirit : std::filesystem::recursive_directory_iterator(filepath))
	{
	  if(*cancel == 1)
	    {
	      return void();
	    }
	  std::filesystem::path p = dirit.path();
	  if(!std::filesystem::is_directory(p))
	    {
	      std::string ext_p = p.extension().u8string();
	      if(ext_p == ".fb2")
		{
		  fb2.push_back(p);
		}
	      else if(ext_p == ".epub")
		{
		  epub.push_back(p);
		}
	      else if(ext_p == ".pdf")
		{
		  pdf.push_back(p);
		}
	      else if(ext_p == ".djvu")
		{
		  djvu.push_back(p);
		}
	      else if(ext_p == ".zip")
		{
		  std::vector<std::tuple<int, int, std::string>> filenames;
		  af.fileNames(p.u8string(), filenames);
		  filenames.erase(
		      std::remove_if(
			  filenames.begin(),
			  filenames.end(),
			  []
			  (auto &el)
			    {
			      std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
			      std::string ext_p = p.extension().u8string();
			      if(ext_p != ".fb2" &&
				  ext_p != ".epub" &&
				  ext_p != ".pdf" &&
				  ext_p != ".djvu")
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    }),
		      filenames.end());
		  if(filenames.size() > 0)
		    {
		      zip.push_back(std::make_tuple(p, filenames));
		    }
		}
	      else if(ext_p == ".rar" || ext_p == ".7z" || ext_p == ".jar"
		  || ext_p == ".cpio" || ext_p == ".iso" || ext_p == ".a"
		  || ext_p == ".ar" || ext_p == ".tar" || ext_p == ".tgz"
		  || ext_p == ".gz" || ext_p == ".bz2" || ext_p == ".xz")
		{
		  std::vector<std::tuple<int, int, std::string>> archlist;
		  af.fileNamesNonZip(p.u8string(), archlist);
		  archlist.erase(
		      std::remove_if(archlist.begin(), archlist.end(), []
		      (auto &el)
			{
			  std::filesystem::path p = std::filesystem::u8path(
			      std::get<2>(el));
			  std::string ext = p.extension().u8string();
			  if (ext != ".fb2" &&
			      ext != ".epub" &&
			      ext != ".pdf" &&
			      ext != ".djvu")
			    {
			      return true;
			    }
			  else
			    {
			      return false;
			    }
			}),
		      archlist.end());
		  if(archlist.size() > 0)
		    {
		      std::tuple<std::filesystem::path,
			  std::vector<std::tuple<int, int, std::string>>> ttup;
		      std::get<0>(ttup) = p;
		      std::get<1>(ttup) = archlist;
		      zip.push_back(ttup);
		    }
		}
	    }
	}
    }
  if(total_hash && !fast_refresh)
    {
      long unsigned int sz = 0;
      for(size_t i = 0; i < fb2.size(); i++)
	{
	  sz = sz + std::filesystem::file_size(fb2[i]);
	}
      for(size_t i = 0; i < epub.size(); i++)
	{
	  sz = sz + std::filesystem::file_size(epub[i]);
	}
      for(size_t i = 0; i < zip.size(); i++)
	{
	  sz = sz + std::filesystem::file_size(std::get<0>(zip[i]));
	}
      for(size_t i = 0; i < pdf.size(); i++)
	{
	  sz = sz + std::filesystem::file_size(pdf[i]);
	}
      for(size_t i = 0; i < djvu.size(); i++)
	{
	  sz = sz + std::filesystem::file_size(djvu[i]);
	}
      total_hash(sz);
    }

  for(size_t i = 0; i < fb2.size(); i++)
    {
      if(*cancel == 1)
	{
	  return void();
	}
      std::filesystem::path p = fb2[i];
      cmtx.lock();
      std::thread *thr = new std::thread(
	  std::bind(&RefreshCollection::fb2ThrFunc, this, p));
      run_thrmtx.lock();
      run_thr++;
      thr->detach();
      delete thr;
      if(run_thr < thr_num)
	{
	  cmtx.try_lock();
	  cmtx.unlock();
	}
      run_thrmtx.unlock();
    }

  for(size_t i = 0; i < epub.size(); i++)
    {
      if(*cancel == 1)
	{
	  return void();
	}
      std::filesystem::path p = epub[i];
      cmtx.lock();
      std::thread *thr = new std::thread(
	  std::bind(&RefreshCollection::epubThrFunc, this, p));
      run_thrmtx.lock();
      run_thr++;
      thr->detach();
      delete thr;
      if(run_thr < thr_num)
	{
	  cmtx.try_lock();
	  cmtx.unlock();
	}
      run_thrmtx.unlock();
    }

  for(size_t i = 0; i < zip.size(); i++)
    {
      if(*cancel == 1)
	{
	  return void();
	}
      std::tuple<std::filesystem::path,
	  std::vector<std::tuple<int, int, std::string>>> ziptup;
      ziptup = zip[i];
      cmtx.lock();
      std::thread *thr = new std::thread(
	  std::bind(&RefreshCollection::zipThrFunc, this, ziptup));
      run_thrmtx.lock();
      run_thr++;
      thr->detach();
      delete thr;
      if(run_thr < thr_num)
	{
	  cmtx.try_lock();
	  cmtx.unlock();
	}
      run_thrmtx.unlock();
    }

  for(size_t i = 0; i < pdf.size(); i++)
    {
      if(*cancel == 1)
	{
	  return void();
	}
      std::filesystem::path p = pdf[i];
      cmtx.lock();
      std::thread *thr = new std::thread(
	  std::bind(&RefreshCollection::pdfThrFunc, this, p));
      run_thrmtx.lock();
      run_thr++;
      thr->detach();
      delete thr;
      if(run_thr < thr_num)
	{
	  cmtx.try_lock();
	  cmtx.unlock();
	}
      run_thrmtx.unlock();
    }

  for(size_t i = 0; i < djvu.size(); i++)
    {
      if(*cancel == 1)
	{
	  return void();
	}
      std::filesystem::path p = djvu[i];
      cmtx.lock();
      std::thread *thr = new std::thread(
	  std::bind(&RefreshCollection::djvuThrFunc, this, p));
      run_thrmtx.lock();
      run_thr++;
      thr->detach();
      delete thr;
      if(run_thr < thr_num)
	{
	  cmtx.try_lock();
	  cmtx.unlock();
	}
      run_thrmtx.unlock();
    }

  for(size_t i = 0; i < saved_hashes.size(); i++)
    {
      if(*cancel == 1)
	{
	  return void();
	}
      std::filesystem::path p = std::get<0>(saved_hashes[i]);
      std::string ext_p = p.extension().u8string();
      if(ext_p == ".fb2")
	{
	  auto itfb2 = std::find_if(fb2.begin(), fb2.end(), [p]
	  (auto &el)
	    {
	      return p == el;
	    });
	  if(itfb2 == fb2.end())
	    {
	      fb2remove.push_back(p);
	    }
	}
      else if(ext_p == ".epub")
	{
	  auto itepub = std::find_if(epub.begin(), epub.end(), [p]
	  (auto &el)
	    {
	      return p == el;
	    });
	  if(itepub == epub.end())
	    {
	      epubremove.push_back(p);
	    }
	}
      else if(ext_p == ".rar" || ext_p == ".7z" || ext_p == ".jar"
	  || ext_p == ".cpio" || ext_p == ".iso" || ext_p == ".a"
	  || ext_p == ".ar" || ext_p == ".tar" || ext_p == ".tgz"
	  || ext_p == ".gz" || ext_p == ".bz2" || ext_p == ".xz")
	{
	  auto itzip = std::find_if(zip.begin(), zip.end(), [p]
	  (auto &el)
	    {
	      return p == std::get<0>(el);
	    });
	  if(itzip == zip.end())
	    {
	      zipremove.push_back(p);
	    }
	}
      else if(ext_p == ".pdf")
	{
	  auto itpdf = std::find_if(pdf.begin(), pdf.end(), [p]
	  (auto &el)
	    {
	      return p == el;
	    });
	  if(itpdf == pdf.end())
	    {
	      pdfremove.push_back(p);
	    }
	}
      else if(ext_p == ".djvu")
	{
	  auto itdjvu = std::find_if(djvu.begin(), djvu.end(), [p]
	  (auto &el)
	    {
	      return p == el;
	    });
	  if(itdjvu == djvu.end())
	    {
	      djvuremove.push_back(p);
	    }
	}
    }
}

void
RefreshCollection::fb2ThrFunc(std::filesystem::path p)
{
  AuxFunc af;
  std::vector<char> hash;
  if(!fast_refresh)
    {
      if(byte_hashed)
	{
	  hash = af.filehash(p, byte_hashed, cancel);
	}
      else
	{
	  hash = af.filehash(p, cancel);
	}
      already_hashedmtx.lock();
      already_hashed.push_back(std::make_tuple(p, hash));
      already_hashedmtx.unlock();
    }
  std::string hex_hash = af.to_hex(&hash);
  auto itsh = std::find_if(
      saved_hashes.begin(), saved_hashes.end(), [p, hex_hash, this]
      (auto &el)
	{
	  if(!this->fast_refresh)
	    {
	      if(p == std::get<0>(el) && std::get<1>(el) == hex_hash)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	  else
	    {
	      if(p == std::get<0>(el))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	});
  if(itsh == saved_hashes.end())
    {
      fb2parsemtx.lock();
      fb2parse.push_back(p);
      fb2parsemtx.unlock();
    }
  run_thrmtx.lock();
  run_thr = run_thr - 1;
  if(run_thr < thr_num)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  run_thrmtx.unlock();
}

void
RefreshCollection::epubThrFunc(std::filesystem::path p)
{
  AuxFunc af;
  std::vector<char> hash;
  if(!fast_refresh)
    {
      if(byte_hashed)
	{
	  hash = af.filehash(p, byte_hashed, cancel);
	}
      else
	{
	  hash = af.filehash(p, cancel);
	}
      already_hashedmtx.lock();
      already_hashed.push_back(std::make_tuple(p, hash));
      already_hashedmtx.unlock();
    }
  std::string hex_hash = af.to_hex(&hash);
  auto itsh = std::find_if(
      saved_hashes.begin(), saved_hashes.end(), [p, hex_hash, this]
      (auto &el)
	{
	  if(!this->fast_refresh)
	    {
	      if(p == std::get<0>(el) && std::get<1>(el) == hex_hash)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	  else
	    {
	      if(p == std::get<0>(el))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	});
  if(itsh == saved_hashes.end())
    {
      epubparsemtx.lock();
      epubparse.push_back(p);
      epubparsemtx.unlock();
    }
  run_thrmtx.lock();
  run_thr = run_thr - 1;
  if(run_thr < thr_num)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  run_thrmtx.unlock();
}

void
RefreshCollection::pdfThrFunc(std::filesystem::path p)
{
  AuxFunc af;
  std::vector<char> hash;
  if(!fast_refresh)
    {
      if(byte_hashed)
	{
	  hash = af.filehash(p, byte_hashed, cancel);
	}
      else
	{
	  hash = af.filehash(p, cancel);
	}
      already_hashedmtx.lock();
      already_hashed.push_back(std::make_tuple(p, hash));
      already_hashedmtx.unlock();
    }
  std::string hex_hash = af.to_hex(&hash);
  auto itsh = std::find_if(
      saved_hashes.begin(), saved_hashes.end(), [p, hex_hash, this]
      (auto &el)
	{
	  if(!this->fast_refresh)
	    {
	      if(p == std::get<0>(el) && std::get<1>(el) == hex_hash)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	  else
	    {
	      if(p == std::get<0>(el))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	});
  if(itsh == saved_hashes.end())
    {
      pdfparsemtx.lock();
      pdfparse.push_back(p);
      pdfparsemtx.unlock();
    }
  run_thrmtx.lock();
  run_thr = run_thr - 1;
  if(run_thr < thr_num)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  run_thrmtx.unlock();
}

void
RefreshCollection::djvuThrFunc(std::filesystem::path p)
{
  AuxFunc af;
  std::vector<char> hash;
  if(!fast_refresh)
    {
      if(byte_hashed)
	{
	  hash = af.filehash(p, byte_hashed, cancel);
	}
      else
	{
	  hash = af.filehash(p, cancel);
	}
      already_hashedmtx.lock();
      already_hashed.push_back(std::make_tuple(p, hash));
      already_hashedmtx.unlock();
    }
  std::string hex_hash = af.to_hex(&hash);
  auto itsh = std::find_if(
      saved_hashes.begin(), saved_hashes.end(), [p, hex_hash, this]
      (auto &el)
	{
	  if(!this->fast_refresh)
	    {
	      if(p == std::get<0>(el) && std::get<1>(el) == hex_hash)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	  else
	    {
	      if(p == std::get<0>(el))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	});
  if(itsh == saved_hashes.end())
    {
      djvuparsemtx.lock();
      djvuparse.push_back(p);
      djvuparsemtx.unlock();
    }
  run_thrmtx.lock();
  run_thr = run_thr - 1;
  if(run_thr < thr_num)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  run_thrmtx.unlock();
}

void
RefreshCollection::zipThrFunc(
    std::tuple<std::filesystem::path,
	std::vector<std::tuple<int, int, std::string>>> ziptup)
{
  AuxFunc af;
  std::vector<char> hash;
  std::filesystem::path p = std::get<0>(ziptup);
  if(!fast_refresh)
    {
      if(byte_hashed)
	{
	  hash = af.filehash(p, byte_hashed, cancel);
	}
      else
	{
	  hash = af.filehash(p, cancel);
	}
      already_hashedmtx.lock();
      already_hashed.push_back(std::make_tuple(p, hash));
      already_hashedmtx.unlock();
    }
  std::string hex_hash = af.to_hex(&hash);
  auto itsh = std::find_if(
      saved_hashes.begin(), saved_hashes.end(), [p, hex_hash, this]
      (auto &el)
	{
	  if(!this->fast_refresh)
	    {
	      if(p == std::get<0>(el) && std::get<1>(el) == hex_hash)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	  else
	    {
	      if(p == std::get<0>(el))
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }
	});
  if(itsh == saved_hashes.end())
    {
      zipparsemtx.lock();
      zipparse.push_back(std::make_tuple(p, std::get<1>(ziptup)));
      zipparsemtx.unlock();
    }
  run_thrmtx.lock();
  run_thr = run_thr - 1;
  if(run_thr < thr_num)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  run_thrmtx.unlock();
}

void
RefreshCollection::removeBook(std::string book_str)
{
  if(bookpath.empty())
    {
      std::string filename;
      AuxFunc af;
      af.homePath(&filename);
      filename = filename + "/.MyLibrary/Collections/" + collname;
      std::filesystem::path filepath = std::filesystem::u8path(filename);
      bookpath.clear();
      if(std::filesystem::exists(filepath))
	{
	  for(auto &dirit : std::filesystem::directory_iterator(filepath))
	    {
	      std::filesystem::path p = dirit.path();
	      std::string ext_p = p.filename().u8string();
	      if(!std::filesystem::is_directory(p))
		{
		  if(ext_p.find("base") != std::string::npos)
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in | std::ios_base::binary);
		      if(f.is_open())
			{
			  if(bookpath.empty())
			    {
			      bookpath.resize(std::filesystem::file_size(p));
			      f.read(&bookpath[0], bookpath.size());
			      std::string::size_type n;
			      n = bookpath.find("<bp>");
			      if(n != std::string::npos)
				{
				  bookpath.erase(n, std::string("<bp>").size());
				  bookpath = bookpath.substr(
				      0, bookpath.find("</bp>"));
				}
			    }
			  f.close();
			}
		    }
		  else if(ext_p.find("hash") != std::string::npos)
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in);
		      if(f.is_open())
			{
			  getline(f, bookpath);
			  f.close();
			}
		    }
		}
	      if(bookpath.size() > 0)
		{
		  break;
		}
	    }
	}
    }
  std::string loc_p = book_str;
  std::string::size_type n;
  n = loc_p.find("<zip>");
  if(n == std::string::npos)
    {
      std::filesystem::path filepath = std::filesystem::u8path(loc_p);
      if(!std::filesystem::exists(filepath) && collection_not_exists)
	{
	  collection_not_exists();
	}
      std::filesystem::remove(filepath);
      std::string ext_filepath = filepath.extension().u8string();
      if(ext_filepath == ".fb2")
	{
	  fb2remove.push_back(std::filesystem::u8path(loc_p));
	}
      else if(ext_filepath == ".epub")
	{
	  epubremove.push_back(std::filesystem::u8path(loc_p));
	}
      else if(ext_filepath == ".pdf")
	{
	  pdfremove.push_back(std::filesystem::u8path(loc_p));
	}
      else if(ext_filepath == ".djvu")
	{
	  djvuremove.push_back(std::filesystem::u8path(loc_p));
	}
    }
  else
    {
      std::string archpath = loc_p;
      archpath.erase(
	  0, archpath.find("<archpath>") + std::string("<archpath>").size());
      archpath = archpath.substr(0, archpath.find("</archpath>"));
      std::string ind_str = loc_p;
      ind_str.erase(0, ind_str.find("<index>") + std::string("<index>").size());
      ind_str = ind_str.substr(0, ind_str.find("</index>"));
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << ind_str;
      uint64_t index;
      strm >> index;
      AuxFunc af;
      std::vector<std::tuple<std::string, std::string>> fileinfov;
      fileinfov = af.fileinfo(archpath, index);
      std::filesystem::path fnm;
      auto itinf = std::find_if(fileinfov.begin(), fileinfov.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "filename";
	});
      if(itinf != fileinfov.end())
	{
	  fnm = std::filesystem::u8path(std::get<1>(*itinf));
	}
      else
	{
	  if(collection_not_exists)
	    {
	      collection_not_exists();
	    }
	}
      af.removeFmArch(archpath, index);
      std::filesystem::path filepath = std::filesystem::u8path(archpath);
      if(std::filesystem::exists(filepath))
	{
	  std::vector<std::tuple<int, int, std::string>> tv;
	  if(filepath.extension().u8string() == ".zip")
	    {
	      af.fileNames(archpath, tv);
	    }
	  else
	    {
	      af.fileNamesNonZip(archpath, tv);
	    }
	  auto ittv =
	      std::find_if(
		  tv.begin(),
		  tv.end(),
		  [fnm]
		  (auto &el)
		    {
		      std::filesystem::path lp = std::filesystem::u8path(std::get<2>(el));
		      if(lp.stem() == fnm.stem() &&
			  lp.extension().u8string() == ".fbd")
			{
			  return true;
			}
		      else
			{
			  return false;
			}
		    });
	  if(ittv != tv.end())
	    {
	      af.removeFmArch(archpath, std::get<0>(*ittv));
	      tv.erase(ittv);
	    }
	  tv.erase(
	      std::remove_if(
		  tv.begin(),
		  tv.end(),
		  []
		  (auto &el)
		    {
		      std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
		      std::string ext_p = p.extension().u8string();
		      if(ext_p == ".fb2" ||
			  ext_p == ".epub" ||
			  ext_p == ".pdf" ||
			  ext_p == ".djvu")
			{
			  return false;
			}
		      else
			{
			  return true;
			}
		    }),
	      tv.end());
	  if(std::filesystem::exists(filepath))
	    {
	      zipparse.push_back(std::make_tuple(filepath, tv));
	    }
	  else
	    {
	      zipremove.push_back(filepath);
	    }
	}
      else
	{
	  zipremove.push_back(filepath);
	}
    }
  collRefresh();
}

void
RefreshCollection::addBook(std::string book_path, std::string book_name,
			   std::string arch_ext)
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  bookpath.clear();
  if(std::filesystem::exists(filepath))
    {
      for(auto &dirit : std::filesystem::directory_iterator(filepath))
	{
	  std::filesystem::path p = dirit.path();
	  std::string ext_p = p.filename().u8string();
	  if(!std::filesystem::is_directory(p))
	    {
	      if(ext_p.find("base") != std::string::npos)
		{
		  std::fstream f;
		  f.open(p, std::ios_base::in | std::ios_base::binary);
		  if(f.is_open())
		    {
		      if(bookpath.empty())
			{
			  bookpath.resize(std::filesystem::file_size(p));
			  f.read(&bookpath[0], bookpath.size());
			  std::string::size_type n;
			  n = bookpath.find("<bp>");
			  if(n != std::string::npos)
			    {
			      bookpath.erase(n, std::string("<bp>").size());
			      bookpath = bookpath.substr(
				  0, bookpath.find("</bp>"));
			    }
			}
		      f.close();
		    }
		}
	      else if(ext_p.find("hash") != std::string::npos)
		{
		  std::fstream f;
		  f.open(p, std::ios_base::in);
		  if(f.is_open())
		    {
		      getline(f, bookpath);
		      f.close();
		    }
		}
	    }
	  if(bookpath.size() > 0)
	    {
	      break;
	    }
	}
    }
  std::mutex addbmtx;
  int stopper = 1;
  addbmtx.lock();
  std::filesystem::path copy_fm = std::filesystem::u8path(book_path);
  if(std::filesystem::exists(copy_fm))
    {
      std::string chstr = book_name;
      chstr.erase(std::remove_if(chstr.begin(), chstr.end(), []
      (auto &el)
	{
	  return el == ' ';
	}),
		  chstr.end());
      if(chstr.empty())
	{
	  book_name = copy_fm.filename().u8string();
	}
      filename = bookpath + "/" + book_name;
      filepath = std::filesystem::u8path(filename);
      if(copy_fm.extension() != filepath.extension())
	{
	  filename = bookpath + "/" + filepath.stem().u8string()
	      + copy_fm.extension().u8string();
	  filepath = std::filesystem::u8path(filename);
	}
      if(!arch_ext.empty())
	{
	  std::string ch_ext = filepath.extension().u8string();
	  if(ch_ext != ".zip" && ch_ext != ".rar" && ch_ext != ".7z"
	      && ch_ext != ".jar" && ch_ext != ".cpio" && ch_ext != ".iso"
	      && ch_ext != ".a" && ch_ext != ".ar" && ch_ext != ".tar"
	      && ch_ext != ".tgz" && ch_ext != ".gz" && ch_ext != ".bz2"
	      && ch_ext != ".xz")
	    {
	      filename = filename = bookpath + "/" + filepath.stem().u8string()
		  + arch_ext;
	      filepath = std::filesystem::u8path(filename);
	      if(std::filesystem::exists(filepath))
		{
		  if(file_exists)
		    {
		      file_exists(&addbmtx, &stopper);
		      addbmtx.lock();
		      if(stopper == 1)
			{
			  addbmtx.try_lock();
			  addbmtx.unlock();
			  if(refresh_canceled)
			    {
			      refresh_canceled();
			    }
			  return void();
			}
		    }
		  else
		    {
		      return void();
		    }
		  std::filesystem::remove(filepath);
		}
	      if(filepath.extension().u8string() == ".zip")
		{
		  int pack_er = af.packing(copy_fm.u8string(),
					   filepath.u8string());
		  if(pack_er <= 0)
		    {
		      if(std::filesystem::exists(filepath))
			{
			  std::filesystem::remove_all(filepath);
			}
		      if(book_add_error)
			{
			  book_add_error();
			}
		      return void();
		    }
		}
	      else
		{
		  int pack_er = af.packingNonZip(copy_fm.u8string(),
						 filepath.u8string(), arch_ext);
		  if(pack_er != ARCHIVE_OK)
		    {
		      if(std::filesystem::exists(filepath))
			{
			  std::filesystem::remove_all(filepath);
			}
		      if(book_add_error)
			{
			  book_add_error();
			}
		      return void();
		    }
		}
	      stopper = 0;
	    }
	}
      if(!std::filesystem::exists(filepath)
	  && std::filesystem::exists(filepath.parent_path()))
	{
	  std::filesystem::copy(copy_fm, filepath);
	  stopper = 0;
	}
      else
	{
	  if(!std::filesystem::exists(filepath.parent_path())
	      && collection_not_exists)
	    {
	      collection_not_exists();
	      return void();
	    }
	}
      if(std::filesystem::exists(filepath) && stopper == 1)
	{
	  if(file_exists)
	    {
	      file_exists(&addbmtx, &stopper);
	      addbmtx.lock();
	      if(stopper == 1)
		{
		  addbmtx.try_lock();
		  addbmtx.unlock();
		  if(refresh_canceled)
		    {
		      refresh_canceled();
		    }
		  return void();
		}
	      std::filesystem::remove(filepath);
	      std::filesystem::copy(copy_fm, filepath);
	    }
	  else
	    {
	      return void();
	    }
	}
      std::string ext_filepath = filepath.extension().u8string();
      if(ext_filepath == ".fb2")
	{
	  fb2parse.push_back(filepath);
	}
      else if(ext_filepath == ".epub")
	{
	  epubparse.push_back(filepath);
	}
      else if(ext_filepath == ".pdf")
	{
	  pdfparse.push_back(filepath);
	}
      else if(ext_filepath == ".djvu")
	{
	  djvuparse.push_back(filepath);
	}
      else if(ext_filepath == ".zip")
	{
	  std::vector<std::tuple<int, int, std::string>> tv;
	  AuxFunc af;
	  af.fileNames(filepath.u8string(), tv);
	  tv.erase(
	      std::remove_if(
		  tv.begin(),
		  tv.end(),
		  []
		  (auto &el)
		    {
		      std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
		      std::string ext_p = p.extension().u8string();
		      if(ext_p == ".fb2" ||
			  ext_p == ".epub" ||
			  ext_p == ".pdf" ||
			  ext_p == ".djvu")
			{
			  return false;
			}
		      else
			{
			  return true;
			}
		    }),
	      tv.end());
	  zipparse.push_back(std::make_tuple(filepath, tv));
	}
      else if(ext_filepath == ".rar" || ext_filepath == ".7z"
	  || ext_filepath == ".jar" || ext_filepath == ".cpio"
	  || ext_filepath == ".iso" || ext_filepath == ".a"
	  || ext_filepath == ".ar" || ext_filepath == ".tar"
	  || ext_filepath == ".tgz" || ext_filepath == ".gz"
	  || ext_filepath == ".bz2" || ext_filepath == ".xz")
	{
	  std::vector<std::tuple<int, int, std::string>> tv;
	  AuxFunc af;
	  af.fileNamesNonZip(filepath.u8string(), tv);
	  tv.erase(
	      std::remove_if(
		  tv.begin(),
		  tv.end(),
		  []
		  (auto &el)
		    {
		      std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
		      std::string ext_p = p.extension().u8string();
		      if(ext_p == ".fb2" ||
			  ext_p == ".epub" ||
			  ext_p == ".pdf" ||
			  ext_p == ".djvu")
			{
			  return false;
			}
		      else
			{
			  return true;
			}
		    }),
	      tv.end());
	  zipparse.push_back(std::make_tuple(filepath, tv));
	}
    }
  addbmtx.try_lock();
  addbmtx.unlock();
  collRefresh();
}

void
RefreshCollection::collRefresh()
{
  std::string filename;
  AuxFunc af;
  std::string rand = af.randomFileName();
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + rand;
  std::filesystem::path t_c_p = std::filesystem::u8path(filename);
  if(std::filesystem::exists(t_c_p))
    {
      std::filesystem::remove_all(t_c_p);
    }
  std::filesystem::create_directories(t_c_p);
  CreateCollection cc(rand, std::filesystem::u8path(bookpath), thr_num,
		      &already_hashed, cancel);
  if(total_files)
    {
      cc.total_files = total_files;
    }
  if(files_added)
    {
      cc.files_added = files_added;
    }
  cc.createFileList(&fb2parse, &epubparse, &zipparse, &pdfparse, &djvuparse);

  collRefreshFb2(rand);
  collRefreshZip(rand);
  collRefreshEpub(rand);
  collRefreshPdf(rand);
  collRefreshDjvu(rand);

  std::filesystem::remove_all(t_c_p);

  if(refresh_finished)
    {
      refresh_finished();
    }
}

void
RefreshCollection::collRefreshFb2(std::string rand)
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname + "/fb2base";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/fb2hash";
  std::filesystem::path hashpath = std::filesystem::u8path(filename);
  std::vector<std::string> fb2base;
  std::vector<std::string> hash;
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "RefreshCollection: fb2base file not openned" << std::endl;
    }
  else
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  fb2base.push_back(line);
	}
    }

  std::filesystem::remove(filepath);

  f.open(hashpath, std::ios_base::in);
  if(!f.is_open())
    {
      std::cerr << "RefreshCollection: fb2hash file not openned" << std::endl;
    }
  else
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }
  std::filesystem::remove(hashpath);

  for(size_t i = 0; i < fb2remove.size(); i++)
    {
      std::filesystem::path p = fb2remove[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      fb2base.erase(std::remove_if(fb2base.begin(), fb2base.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		    fb2base.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  for(size_t i = 0; i < fb2parse.size(); i++)
    {
      std::filesystem::path p = fb2parse[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      fb2base.erase(std::remove_if(fb2base.begin(), fb2base.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		    fb2base.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  filename = filepath.parent_path().parent_path().u8string();
  filename = filename + "/" + rand + "/fb2base";
  std::filesystem::path t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(t_c_p));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  fb2base.push_back(line);
	}
    }

  filename = t_c_p.parent_path().u8string();
  filename = filename + "/fb2hash";
  t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count > 0)
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }

  f.open(filepath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = "<bp>" + bookpath + "</bp>";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < fb2base.size(); i++)
	{
	  line = fb2base[i] + "<?L>";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }

  f.open(hashpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = bookpath + "\n";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < hash.size(); i++)
	{
	  line = hash[i] + "\n";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }
  fb2base.clear();
  hash.clear();
}

void
RefreshCollection::collRefreshZip(std::string rand)
{
  std::vector<std::tuple<std::string, std::vector<std::string>>> zipbase;
  std::vector<std::string> hash;
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname + "/zipbase";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/ziphash";
  std::filesystem::path hashpath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string archpath = file_str.substr(
	      0, file_str.find("<?e>") + std::string("<?e>").size());
	  file_str.erase(0, archpath.size());
	  archpath.erase(0, std::string("<?a>").size());
	  archpath = archpath.substr(0, archpath.find("<?e>"));
	  std::string archgr = file_str.substr(0, file_str.find("<?a>"));
	  file_str.erase(0, archgr.size());
	  std::vector<std::string> tv;
	  while(!archgr.empty())
	    {
	      std::string line = archgr.substr(
		  0, archgr.find("<?L>") + std::string("<?L>").size());
	      archgr.erase(0, line.size());
	      line = line.substr(0, line.find("<?L>"));
	      tv.push_back(line);
	    }
	  std::tuple<std::string, std::vector<std::string>> ttup;
	  std::get<0>(ttup) = archpath;
	  std::get<1>(ttup) = tv;
	  zipbase.push_back(ttup);
	}
    }

  std::filesystem::remove(filepath);

  f.open(hashpath, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }
  std::filesystem::remove(hashpath);

  for(size_t i = 0; i < zipremove.size(); i++)
    {
      std::filesystem::path p = zipremove[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      zipbase.erase(std::remove_if(zipbase.begin(), zipbase.end(), [p_str]
      (auto &el)
	{
	  return std::get<0>(el) == p_str;
	}),
		    zipbase.end());
      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }

  for(size_t i = 0; i < zipparse.size(); i++)
    {
      std::filesystem::path p = std::get<0>(zipparse[i]);
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      zipbase.erase(std::remove_if(zipbase.begin(), zipbase.end(), [p_str]
      (auto &el)
	{
	  return std::get<0>(el) == p_str;
	}),
		    zipbase.end());
      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  filename = filepath.parent_path().parent_path().u8string();
  filename = filename + "/" + rand + "/zipbase";
  std::filesystem::path t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(t_c_p));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string archpath = file_str.substr(
	      0, file_str.find("<?e>") + std::string("<?e>").size());
	  file_str.erase(0, archpath.size());
	  archpath.erase(0, std::string("<?a>").size());
	  archpath = archpath.substr(0, archpath.find("<?e>"));
	  std::string archgr = file_str.substr(0, file_str.find("<?a>"));
	  file_str.erase(0, archgr.size());
	  std::vector<std::string> tv;
	  while(!archgr.empty())
	    {
	      std::string line = archgr.substr(
		  0, archgr.find("<?L>") + std::string("<?L>").size());
	      archgr.erase(0, line.size());
	      line = line.substr(0, line.find("<?L>"));
	      tv.push_back(line);
	    }
	  std::tuple<std::string, std::vector<std::string>> ttup;
	  std::get<0>(ttup) = archpath;
	  std::get<1>(ttup) = tv;
	  zipbase.push_back(ttup);
	}
    }

  filename = t_c_p.parent_path().u8string();
  filename = filename + "/ziphash";
  t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }

  f.open(filepath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = "<bp>" + bookpath + "</bp>";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < zipbase.size(); i++)
	{
	  line = std::get<0>(zipbase[i]);
	  line = "<?a>" + line + "<?e>";
	  f.write(line.c_str(), line.size());
	  std::vector<std::string> tv = std::get<1>(zipbase[i]);
	  for(size_t j = 0; j < tv.size(); j++)
	    {
	      line = tv[j] + "<?L>";
	      f.write(line.c_str(), line.size());
	    }
	}
      f.close();
    }
  f.open(hashpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = bookpath + "\n";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < hash.size(); i++)
	{
	  line = hash[i] + "\n";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }
  zipbase.clear();
  hash.clear();
}

void
RefreshCollection::collRefreshEpub(std::string rand)
{
  std::vector<std::string> epubbase;
  std::vector<std::string> hash;
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname + "/epubbase";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/epubhash";
  std::filesystem::path hashpath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  epubbase.push_back(line);
	}
    }

  std::filesystem::remove(filepath);

  f.open(hashpath, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }
  std::filesystem::remove(hashpath);

  for(size_t i = 0; i < epubremove.size(); i++)
    {
      std::filesystem::path p = epubremove[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      epubbase.erase(std::remove_if(epubbase.begin(), epubbase.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		     epubbase.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }

  for(size_t i = 0; i < epubparse.size(); i++)
    {
      std::filesystem::path p = epubparse[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      epubbase.erase(std::remove_if(epubbase.begin(), epubbase.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		     epubbase.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  filename = filepath.parent_path().parent_path().u8string();
  filename = filename + "/" + rand + "/epubbase";
  std::filesystem::path t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(t_c_p));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  epubbase.push_back(line);
	}
    }

  filename = t_c_p.parent_path().u8string();
  filename = filename + "/epubhash";
  t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }

  f.open(filepath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = "<bp>" + bookpath + "</bp>";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < epubbase.size(); i++)
	{
	  line = epubbase[i] + "<?L>";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }

  f.open(hashpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = bookpath + "\n";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < hash.size(); i++)
	{
	  line = hash[i] + "\n";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }
  epubbase.clear();
  hash.clear();
}

void
RefreshCollection::collRefreshPdf(std::string rand)
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname + "/pdfbase";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/pdfhash";
  std::filesystem::path hashpath = std::filesystem::u8path(filename);
  std::vector<std::string> pdfbase;
  std::vector<std::string> hash;
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "RefreshCollection: pdfbase file not openned" << std::endl;
    }
  else
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  pdfbase.push_back(line);
	}
    }

  std::filesystem::remove(filepath);

  f.open(hashpath, std::ios_base::in);
  if(!f.is_open())
    {
      std::cerr << "RefreshCollection: pdfhash file not openned" << std::endl;
    }
  else
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }
  std::filesystem::remove(hashpath);

  for(size_t i = 0; i < pdfremove.size(); i++)
    {
      std::filesystem::path p = pdfremove[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      pdfbase.erase(std::remove_if(pdfbase.begin(), pdfbase.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		    pdfbase.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  for(size_t i = 0; i < pdfparse.size(); i++)
    {
      std::filesystem::path p = pdfparse[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      pdfbase.erase(std::remove_if(pdfbase.begin(), pdfbase.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		    pdfbase.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  filename = filepath.parent_path().parent_path().u8string();
  filename = filename + "/" + rand + "/pdfbase";
  std::filesystem::path t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(t_c_p));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  pdfbase.push_back(line);
	}
    }

  filename = t_c_p.parent_path().u8string();
  filename = filename + "/pdfhash";
  t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count > 0)
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }

  f.open(filepath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = "<bp>" + bookpath + "</bp>";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < pdfbase.size(); i++)
	{
	  line = pdfbase[i] + "<?L>";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }

  f.open(hashpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = bookpath + "\n";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < hash.size(); i++)
	{
	  line = hash[i] + "\n";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }
  pdfbase.clear();
  hash.clear();
}

void
RefreshCollection::collRefreshDjvu(std::string rand)
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname + "/djvubase";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/djvuhash";
  std::filesystem::path hashpath = std::filesystem::u8path(filename);
  std::vector<std::string> djvubase;
  std::vector<std::string> hash;
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "RefreshCollection: djvubase file not openned" << std::endl;
    }
  else
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  djvubase.push_back(line);
	}
    }

  std::filesystem::remove(filepath);

  f.open(hashpath, std::ios_base::in);
  if(!f.is_open())
    {
      std::cerr << "RefreshCollection: djvuhash file not openned" << std::endl;
    }
  else
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count == 0)
		{
		  if(bookpath != line)
		    {
		      bookpath = line;
		    }
		}
	      else
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }
  std::filesystem::remove(hashpath);

  for(size_t i = 0; i < djvuremove.size(); i++)
    {
      std::filesystem::path p = djvuremove[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      djvubase.erase(std::remove_if(djvubase.begin(), djvubase.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		     djvubase.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  for(size_t i = 0; i < djvuparse.size(); i++)
    {
      std::filesystem::path p = djvuparse[i];
      std::string p_str = p.u8string();
      p_str.erase(0, bookpath.size());
      djvubase.erase(std::remove_if(djvubase.begin(), djvubase.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		     djvubase.end());

      hash.erase(std::remove_if(hash.begin(), hash.end(), [p_str]
      (auto &el)
	{
	  std::string::size_type n;
	  n = el.find(p_str);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	}),
		 hash.end());
    }
  filename = filepath.parent_path().parent_path().u8string();
  filename = filename + "/" + rand + "/djvubase";
  std::filesystem::path t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(t_c_p));
      f.read(&file_str[0], file_str.size());
      f.close();
      if(bookpath.empty())
	{
	  bookpath = file_str.substr(
	      0, file_str.find("</bp>") + std::string("</bp>").size());
	  file_str.erase(0, bookpath.size());
	  bookpath.erase(0, std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	}
      else
	{
	  file_str.erase(0,
			 file_str.find("</bp>") + std::string("</bp>").size());
	}
      while(!file_str.empty())
	{
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  line = line.substr(0, line.find("<?L>"));
	  djvubase.push_back(line);
	}
    }

  filename = t_c_p.parent_path().u8string();
  filename = filename + "/djvuhash";
  t_c_p = std::filesystem::u8path(filename);
  f.open(t_c_p, std::ios_base::in);
  if(f.is_open())
    {
      int count = 0;
      while(!f.eof())
	{
	  std::string line;
	  getline(f, line);
	  if(!line.empty())
	    {
	      if(count > 0)
		{
		  hash.push_back(line);
		}
	    }
	  count++;
	}
      f.close();
    }

  f.open(filepath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = "<bp>" + bookpath + "</bp>";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < djvubase.size(); i++)
	{
	  line = djvubase[i] + "<?L>";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }

  f.open(hashpath, std::ios_base::out | std::ios_base::binary);
  if(f.is_open())
    {
      std::string line = bookpath + "\n";
      f.write(line.c_str(), line.size());
      for(size_t i = 0; i < hash.size(); i++)
	{
	  line = hash[i] + "\n";
	  f.write(line.c_str(), line.size());
	}
      f.close();
    }
  djvubase.clear();
  hash.clear();
}

void
RefreshCollection::removeEmptyDirs()
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(std::filesystem::exists(filepath))
    {
      if(bookpath.empty())
	{
	  for(auto &dirit : std::filesystem::directory_iterator(filepath))
	    {
	      std::filesystem::path p = dirit.path();
	      if(!std::filesystem::is_directory(p))
		{
		  if(p.filename().u8string().find("base") != std::string::npos
		      && bookpath.empty())
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in | std::ios_base::binary);
		      if(f.is_open())
			{
			  bookpath.resize(std::filesystem::file_size(p));
			  f.read(&bookpath[0], bookpath.size());
			  f.close();
			  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
			  bookpath.erase(0, std::string("<bp>").size());
			  if(!bookpath.empty())
			    {
			      break;
			    }
			}
		    }
		  if(p.filename().u8string().find("hash") != std::string::npos
		      && bookpath.empty())
		    {
		      std::fstream f;
		      f.open(p, std::ios_base::in);
		      if(f.is_open())
			{
			  getline(f, bookpath);
			  f.close();
			  if(!bookpath.empty())
			    {
			      break;
			    }
			}
		    }
		}
	    }
	}
      if(!bookpath.empty())
	{
	  std::filesystem::path dir_to_ch = std::filesystem::u8path(bookpath);
	  if(std::filesystem::exists(dir_to_ch))
	    {
	      for(;;)
		{
		  std::vector<std::filesystem::path> dir_to_rem;
		  for(auto &dirit : std::filesystem::recursive_directory_iterator(
		      dir_to_ch))
		    {
		      std::filesystem::path p = dirit.path();
		      if(std::filesystem::is_directory(p)
			  && std::filesystem::is_empty(p))
			{
			  dir_to_rem.push_back(p);
			}
		    }
		  if(dir_to_rem.size() == 0)
		    {
		      break;
		    }
		  else
		    {
		      for(size_t i = 0; i < dir_to_rem.size(); i++)
			{
			  std::filesystem::remove_all(dir_to_rem[i]);
			}
		    }
		}
	    }
	}
    }
}

void
RefreshCollection::editBook(
    std::vector<std::tuple<std::string, std::string>> *newbasev,
    std::vector<std::tuple<std::string, std::string>> *oldbasev)
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(!std::filesystem::exists(filepath))
    {
      std::cerr << "Collection database not found" << std::endl;
    }
  else
    {
      std::string book_str;
      auto itob = std::find_if(oldbasev->begin(), oldbasev->end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "filepath";
	});
      if(itob != oldbasev->end())
	{
	  book_str = std::get<1>(*itob);
	}
      std::string::size_type n;
      n = book_str.find("<zip>");
      if(n != std::string::npos)
	{
	  filename = filename + "/zipbase";
	  editBookZip(book_str, filename, newbasev, oldbasev);
	}
      else
	{
	  filepath = std::filesystem::u8path(book_str);
	  std::string ext = filepath.extension().u8string();
	  if(ext == ".fb2")
	    {
	      filename = filename + "/fb2base";
	    }
	  else if(ext == ".epub")
	    {
	      filename = filename + "/epubbase";
	    }
	  else if(ext == ".pdf")
	    {
	      filename = filename + "/pdfbase";
	    }
	  else if(ext == ".djvu")
	    {
	      filename = filename + "/djvubase";
	    }
	  editBook(book_str, filename, newbasev);
	}
    }

  if(refresh_finished)
    {
      refresh_finished();
    }
}

void
RefreshCollection::editBookZip(
    std::string book_str, std::string filename,
    std::vector<std::tuple<std::string, std::string>> *newbasev,
    std::vector<std::tuple<std::string, std::string>> *oldbasev)
{
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string filestr;
      filestr.resize(std::filesystem::file_size(filepath));
      f.read(&filestr[0], filestr.size());
      f.close();
      std::vector<
	  std::tuple<std::string,
	      std::vector<
		  std::tuple<std::string, std::string, std::string, std::string,
		      std::string, std::string>>>> zipbase;
      std::string::size_type n;
      n = filestr.find("</bp>");
      if(n != std::string::npos)
	{
	  std::string bookpath = filestr.substr(
	      0, n + std::string("</bp>").size());
	  filestr.erase(0, bookpath.size());
	  bookpath.erase(0, bookpath.find("<bp>") + std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	  n = 0;
	  while(n != std::string::npos)
	    {
	      n = filestr.find("<?e>");
	      if(n != std::string::npos)
		{
		  std::string archpath = filestr.substr(
		      0, n + std::string("<?e>").size());
		  filestr.erase(0, archpath.size());
		  archpath.erase(
		      0, archpath.find("<?a>") + std::string("<?a>").size());
		  archpath = archpath.substr(0, archpath.find("<?e>"));
		  std::tuple<std::string,
		      std::vector<
			  std::tuple<std::string, std::string, std::string,
			      std::string, std::string, std::string>>> ttup;
		  std::get<0>(ttup) = archpath;
		  std::vector<
		      std::tuple<std::string, std::string, std::string,
			  std::string, std::string, std::string>> archv;
		  std::string archbase = filestr.substr(0,
							filestr.find("<?a>"));
		  filestr.erase(0, archbase.size());
		  std::string::size_type n_ab = 0;
		  std::tuple<std::string, std::string, std::string, std::string,
		      std::string, std::string> archvtup; //0-index, 1-author, 2-book name, 3-series, 4-genre, 5-date
		  while(n_ab != std::string::npos)
		    {
		      n_ab = archbase.find("<?L>");
		      if(n_ab != std::string::npos)
			{
			  std::string bookline = archbase.substr(
			      0, n_ab + std::string("<?L>").size());
			  archbase.erase(0, bookline.size());
			  bookline.erase(0, std::string("<?>").size());
			  std::get<0>(archvtup) = bookline.substr(
			      0, bookline.find("<?>"));

			  bookline.erase(
			      0,
			      bookline.find("<?>") + std::string("<?>").size());
			  std::get<1>(archvtup) = bookline.substr(
			      0, bookline.find("<?>"));

			  bookline.erase(
			      0,
			      bookline.find("<?>") + std::string("<?>").size());
			  std::get<2>(archvtup) = bookline.substr(
			      0, bookline.find("<?>"));

			  bookline.erase(
			      0,
			      bookline.find("<?>") + std::string("<?>").size());
			  std::get<3>(archvtup) = bookline.substr(
			      0, bookline.find("<?>"));

			  bookline.erase(
			      0,
			      bookline.find("<?>") + std::string("<?>").size());
			  std::get<4>(archvtup) = bookline.substr(
			      0, bookline.find("<?>"));

			  bookline.erase(
			      0,
			      bookline.find("<?>") + std::string("<?>").size());
			  std::get<5>(archvtup) = bookline.substr(
			      0, bookline.find("<?L>"));
			  archv.push_back(archvtup);
			}
		    }
		  std::get<1>(ttup) = archv;
		  zipbase.push_back(ttup);
		}
	    }

	  std::string authorold;
	  auto itob = std::find_if(oldbasev->begin(), oldbasev->end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Author";
	    });
	  if(itob != oldbasev->end())
	    {
	      authorold = std::get<1>(*itob);
	    }

	  std::string bookold;
	  itob = std::find_if(oldbasev->begin(), oldbasev->end(), []
	  (auto &el)
	    {
	      return std::get<0>(el) == "Book";
	    });
	  if(itob != oldbasev->end())
	    {
	      bookold = std::get<1>(*itob);
	    }

	  std::string archpathold = book_str;
	  archpathold.erase(0, archpathold.find(bookpath) + bookpath.size());
	  archpathold = archpathold.substr(0, archpathold.find("</archpath>"));

	  std::string indexold = book_str;
	  indexold.erase(
	      0, indexold.find("<index>") + std::string("<index>").size());
	  indexold = indexold.substr(0, indexold.find("</index>"));
	  auto itzipbase = std::find_if(
	      zipbase.begin(), zipbase.end(), [archpathold]
	      (auto &el)
		{
		  return std::get<0>(el) == archpathold;
		});
	  if(itzipbase != zipbase.end())
	    {
	      std::vector<
		  std::tuple<std::string, std::string, std::string, std::string,
		      std::string, std::string>> v = std::get<1>(*itzipbase);
	      auto itv = std::find_if(v.begin(), v.end(),
				      [indexold, authorold, bookold]
				      (auto &el)
					{
					  if(std::get<0>(el) == indexold &&
					      std::get<1>(el) == authorold &&
					      std::get<2>(el) == bookold)
					    {
					      return true;
					    }
					  else
					    {
					      return false;
					    }
					});
	      if(itv != v.end())
		{
		  auto itnewb = std::find_if(
		      newbasev->begin(), newbasev->end(), []
		      (auto &el)
			{
			  return std::get<0>(el) == "Author";
			});
		  if(itnewb != newbasev->end())
		    {
		      std::get<1>(*itv) = std::get<1>(*itnewb);
		    }

		  itnewb = std::find_if(newbasev->begin(), newbasev->end(), []
		  (auto &el)
		    {
		      return std::get<0>(el) == "Book";
		    });
		  if(itnewb != newbasev->end())
		    {
		      std::get<2>(*itv) = std::get<1>(*itnewb);
		    }

		  itnewb = std::find_if(newbasev->begin(), newbasev->end(), []
		  (auto &el)
		    {
		      return std::get<0>(el) == "Series";
		    });
		  if(itnewb != newbasev->end())
		    {
		      std::get<3>(*itv) = std::get<1>(*itnewb);
		    }

		  itnewb = std::find_if(newbasev->begin(), newbasev->end(), []
		  (auto &el)
		    {
		      return std::get<0>(el) == "Genre";
		    });
		  if(itnewb != newbasev->end())
		    {
		      std::get<4>(*itv) = std::get<1>(*itnewb);
		    }

		  itnewb = std::find_if(newbasev->begin(), newbasev->end(), []
		  (auto &el)
		    {
		      return std::get<0>(el) == "Date";
		    });
		  if(itnewb != newbasev->end())
		    {
		      std::get<5>(*itv) = std::get<1>(*itnewb);
		    }
		  std::get<1>(*itzipbase) = v;
		  f.open(filepath, std::ios_base::out | std::ios_base::binary);
		  if(f.is_open())
		    {
		      bookpath = "<bp>" + bookpath + "</bp>";
		      f.write(bookpath.c_str(), bookpath.size());
		      for(size_t i = 0; i < zipbase.size(); i++)
			{
			  std::string archpath = std::get<0>(zipbase[i]);
			  archpath = "<?a>" + archpath + "<?e>";
			  f.write(archpath.c_str(), archpath.size());
			  v = std::get<1>(zipbase[i]);
			  for(size_t j = 0; j < v.size(); j++)
			    {
			      std::tuple<std::string, std::string, std::string,
				  std::string, std::string, std::string> ttup;
			      ttup = v[j];
			      std::string line = std::get<0>(ttup);
			      line = "<?>" + line;
			      f.write(line.c_str(), line.size());

			      line.clear();
			      line = std::get<1>(ttup);
			      line = "<?>" + line;
			      f.write(line.c_str(), line.size());

			      line.clear();
			      line = std::get<2>(ttup);
			      line = "<?>" + line;
			      f.write(line.c_str(), line.size());

			      line.clear();
			      line = std::get<3>(ttup);
			      line = "<?>" + line;
			      f.write(line.c_str(), line.size());

			      line.clear();
			      line = std::get<4>(ttup);
			      line = "<?>" + line;
			      f.write(line.c_str(), line.size());

			      line.clear();
			      line = std::get<5>(ttup);
			      line = "<?>" + line + "<?L>";
			      f.write(line.c_str(), line.size());
			    }
			}
		      f.close();
		    }
		}
	    }
	}
    }
}

void
RefreshCollection::editBook(
    std::string book_str, std::string filename,
    std::vector<std::tuple<std::string, std::string>> *newbasev)
{
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string filestr;
      filestr.resize(std::filesystem::file_size(filepath));
      f.read(&filestr[0], filestr.size());
      f.close();
      std::vector<
	  std::tuple<std::string, std::string, std::string, std::string,
	      std::string, std::string>> filebase;
      std::string::size_type n;
      n = filestr.find("</bp>");
      if(n != std::string::npos)
	{
	  std::string bookpath = filestr.substr(
	      0, n + std::string("</bp>").size());
	  filestr.erase(0, bookpath.size());
	  bookpath.erase(0, bookpath.find("<bp>") + std::string("<bp>").size());
	  bookpath = bookpath.substr(0, bookpath.find("</bp>"));
	  n = 0;
	  while(n != std::string::npos)
	    {
	      n = filestr.find("<?L>");
	      if(n != std::string::npos)
		{
		  std::string bookent = filestr.substr(
		      0, n + std::string("<?L>").size());
		  filestr.erase(0, bookent.size());
		  std::tuple<std::string, std::string, std::string, std::string,
		      std::string, std::string> filetup; //0-path, 1-author, 2-book name, 3-series, 4-genre, 5-date
		  std::string bookline = bookent.substr(
		      0, bookent.find("<?>") + std::string("<?>").size());
		  bookent.erase(0, bookline.size());
		  bookline = bookline.substr(0, bookline.find("<?>"));
		  std::get<0>(filetup) = bookline;

		  bookline = bookent.substr(
		      0, bookent.find("<?>") + std::string("<?>").size());
		  bookent.erase(0, bookline.size());
		  bookline = bookline.substr(0, bookline.find("<?>"));
		  std::get<1>(filetup) = bookline;

		  bookline = bookent.substr(
		      0, bookent.find("<?>") + std::string("<?>").size());
		  bookent.erase(0, bookline.size());
		  bookline = bookline.substr(0, bookline.find("<?>"));
		  std::get<2>(filetup) = bookline;

		  bookline = bookent.substr(
		      0, bookent.find("<?>") + std::string("<?>").size());
		  bookent.erase(0, bookline.size());
		  bookline = bookline.substr(0, bookline.find("<?>"));
		  std::get<3>(filetup) = bookline;

		  bookline = bookent.substr(
		      0, bookent.find("<?>") + std::string("<?>").size());
		  bookent.erase(0, bookline.size());
		  bookline = bookline.substr(0, bookline.find("<?>"));
		  std::get<4>(filetup) = bookline;

		  bookline = bookent.substr(
		      0, bookent.find("<?L>") + std::string("<?L>").size());
		  bookent.erase(0, bookline.size());
		  bookline = bookline.substr(0, bookline.find("<?L>"));
		  std::get<5>(filetup) = bookline;
		  filebase.push_back(filetup);
		}
	    }
	  std::filesystem::path cvp = std::filesystem::u8path(bookpath);
	  std::string oldfp = book_str;
	  oldfp.erase(0, cvp.u8string().size());
	  auto itfb = std::find_if(filebase.begin(), filebase.end(), [oldfp]
	  (auto &el)
	    {
	      return std::get<0>(el) == oldfp;
	    });
	  if(itfb != filebase.end())
	    {
	      auto itnb = std::find_if(newbasev->begin(), newbasev->end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "Author";
		});
	      if(itnb != newbasev->end())
		{
		  std::get<1>(*itfb) = std::get<1>(*itnb);
		}

	      itnb = std::find_if(newbasev->begin(), newbasev->end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "Book";
		});
	      if(itnb != newbasev->end())
		{
		  std::get<2>(*itfb) = std::get<1>(*itnb);
		}

	      itnb = std::find_if(newbasev->begin(), newbasev->end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "Series";
		});
	      if(itnb != newbasev->end())
		{
		  std::get<3>(*itfb) = std::get<1>(*itnb);
		}

	      itnb = std::find_if(newbasev->begin(), newbasev->end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "Genre";
		});
	      if(itnb != newbasev->end())
		{
		  std::get<4>(*itfb) = std::get<1>(*itnb);
		}

	      itnb = std::find_if(newbasev->begin(), newbasev->end(), []
	      (auto &el)
		{
		  return std::get<0>(el) == "Date";
		});
	      if(itnb != newbasev->end())
		{
		  std::get<5>(*itfb) = std::get<1>(*itnb);
		}
	      f.open(filepath, std::ios_base::out | std::ios_base::binary);
	      if(f.is_open())
		{
		  bookpath = "<bp>" + bookpath + "</bp>";
		  f.write(bookpath.c_str(), bookpath.size());
		  for(size_t i = 0; i < filebase.size(); i++)
		    {
		      std::string line = std::get<0>(filebase[i]);
		      line = line + "<?>";
		      f.write(line.c_str(), line.size());

		      line = std::get<1>(filebase[i]);
		      line = line + "<?>";
		      f.write(line.c_str(), line.size());

		      line = std::get<2>(filebase[i]);
		      line = line + "<?>";
		      f.write(line.c_str(), line.size());

		      line = std::get<3>(filebase[i]);
		      line = line + "<?>";
		      f.write(line.c_str(), line.size());

		      line = std::get<4>(filebase[i]);
		      line = line + "<?>";
		      f.write(line.c_str(), line.size());

		      line = std::get<5>(filebase[i]);
		      line = line + "<?L>";
		      f.write(line.c_str(), line.size());
		    }
		  f.close();
		}
	    }
	}
    }
}

void
RefreshCollection::removeRar(std::string archaddr)
{
  AuxFunc af;
  std::string filename;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + collname + "/zipbase";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string fl;
      fl.resize(std::filesystem::file_size(filepath));
      f.read(&fl[0], fl.size());
      f.close();
      bookpath = fl.substr(0, fl.find("</bp>"));
      bookpath.erase(0, std::string("<bp>").size());
      std::string archline = archaddr.erase(0, bookpath.size());
      std::string to_rem = "<?a>" + archline;
      std::string result = fl;
      result.erase(0, result.rfind(to_rem));
      result = result.substr(0,
			     result.find("<?a>", std::string("<?a>").size()));
      fl.erase(fl.find(result), result.size());
      std::filesystem::remove_all(filepath);
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
	{
	  f.write(fl.c_str(), fl.size());
	  f.close();
	}
      filename = filepath.parent_path().u8string() + "/ziphash";
      filepath = std::filesystem::u8path(filename);
      std::vector<std::string> lv;
      f.open(filepath, std::ios_base::in);
      if(f.is_open())
	{
	  while(!f.eof())
	    {
	      std::string line;
	      getline(f, line);
	      if(!line.empty())
		{
		  lv.push_back(line);
		}
	    }
	  f.close();
	  lv.erase(std::remove_if(lv.begin(), lv.end(), [&archline]
	  (auto &el)
	    {
	      std::string::size_type n;
	      n = el.find(archline);
	      if(n != std::string::npos)
		{
		  return true;
		}
	      else
		{
		  return false;
		}
	    }),
		   lv.end());
	  std::filesystem::remove_all(filepath);
	  f.open(filepath, std::ios_base::out | std::ios_base::binary);
	  if(f.is_open())
	    {
	      for(size_t i = 0; i < lv.size(); i++)
		{
		  std::string line;
		  line = lv[i] + "\n";
		  f.write(line.c_str(), line.size());
		}
	      f.close();
	    }
	}
    }
  filepath = std::filesystem::u8path(bookpath + archaddr);
  if(std::filesystem::exists(filepath))
    {
      std::filesystem::remove_all(filepath);
    }
  else
    {
      if(collection_not_exists)
	{
	  collection_not_exists();
	}
    }
}
