/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

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

#include "CreateCollection.h"

CreateCollection::CreateCollection(std::string coll_nm,
				   std::filesystem::path book_p,
				   unsigned int nm_thr, int *cancel)
{
  this->coll_nm = coll_nm;
  this->book_p = book_p;
  this->cancel = cancel;
  threadnum = nm_thr;
}

CreateCollection::~CreateCollection()
{

}

void
CreateCollection::createCol()
{
  std::string filename;
  AuxFunc af;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + coll_nm;
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(std::filesystem::exists(filepath))
    {
      if(collection_exist)
	{
	  collection_exist();
	}
    }
  else
    {
      std::filesystem::create_directories(filepath);
      createFileList();
      createDatabase();
      if(creation_finished && *cancel != 1)
	{
	  creation_finished();
	}
      if(*cancel == 1)
	{
	  if(op_canceled)
	    {
	      op_canceled();
	    }
	}
    }
}

void
CreateCollection::createFileList()
{
  AuxFunc af;
  if(std::filesystem::exists(book_p) && std::filesystem::is_directory(book_p))
    {
      for(auto &dirit : std::filesystem::recursive_directory_iterator(book_p))
	{
	  std::filesystem::path p = dirit.path();
	  if(!std::filesystem::is_directory(p))
	    {
	      if(p.extension().u8string() == ".fb2")
		{
		  fb2.push_back(p);
		}
	      if(p.extension().u8string() == ".epub")
		{
		  epub.push_back(p);
		}
	      if(p.extension().u8string() == ".zip")
		{
		  std::vector<std::tuple<int, int, std::string>> archlist;
		  af.fileNames(p.u8string(), archlist);
		  archlist.erase(
		      std::remove_if(archlist.begin(), archlist.end(), []
		      (auto &el)
			{
			  std::filesystem::path p = std::filesystem::u8path(
			      std::get<2>(el));
			  if (p.extension().u8string() != ".fb2" &&
			      p.extension().u8string() != ".epub")
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
		      zipvect.push_back(ttup);
		    }
		}
	    }
	}
    }
}

void
CreateCollection::createFileList(
    std::vector<std::filesystem::path> *fb2in,
    std::vector<std::filesystem::path> *epubin,
    std::vector<
	std::tuple<std::filesystem::path,
	    std::vector<std::tuple<int, int, std::string>>>> *zipvectin)
{
  fb2 = *fb2in;
  zipvect = *zipvectin;
  epub = *epubin;
  createDatabase();
}

void
CreateCollection::createDatabase()
{
  AuxFunc af;
  std::string filename;
  af.homePath(&filename);
  filename = filename + "/.MyLibrary/Collections/" + coll_nm + "/fb2base";
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/fb2hash";
  std::filesystem::path fb2_hashp = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/ziphash";
  std::filesystem::path zip_hashp = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/epubhash";
  std::filesystem::path epub_hashp = std::filesystem::u8path(filename);
  std::fstream zip_hash_f;
  std::fstream epub_hash_f;
  if(total_files)
    {
      int zipf = 0;
      for(size_t i = 0; i < zipvect.size(); i++)
	{
	  zipf = zipf + static_cast<int>(std::get<1>(zipvect[i]).size());
	}
      total_files(
	  static_cast<int>(fb2.size()) + static_cast<int>(epub.size()) + zipf);
    }
  if(*cancel == 0)
    {
      std::fstream fb2_hash_f;
      std::fstream f;
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      fb2_hash_f.open(fb2_hashp, std::ios_base::out | std::ios_base::binary);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";
      fb2basemtx.lock();
      f.write(line.c_str(), line.size());
      fb2basemtx.unlock();
      line = book_p.u8string() + "\n";
      fb2hashmtx.lock();
      fb2_hash_f.write(line.c_str(), line.size());
      fb2hashmtx.unlock();
      f.close();
      fb2_hash_f.close();
      for(size_t i = 0; i < fb2.size(); i++)
	{
	  if(*cancel == 1)
	    {
	      break;
	    }
	  cmtx.lock();
	  std::filesystem::path fp = fb2[i];
	  std::thread *thr = new std::thread(
	      std::bind(&CreateCollection::fb2ThreadFunc, this, fp, filepath,
			fb2_hashp));
	  num_thr_runmtx.lock();
	  num_thr_run++;
	  thr->detach();
	  delete thr;
	  if(num_thr_run < threadnum)
	    {
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  std::fstream f;
  if(*cancel == 0)
    {
      filename = filepath.parent_path().u8string();
      filename = filename + "/zipbase";
      filepath = std::filesystem::u8path(filename);
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      zip_hash_f.open(zip_hashp, std::ios_base::out | std::ios_base::binary);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      line = book_p.u8string() + "\n";
      ziphashmtx.lock();
      zip_hash_f.write(line.c_str(), line.size());
      ziphashmtx.unlock();
      f.close();
      zip_hash_f.close();
      for(size_t i = 0; i < zipvect.size(); i++)
	{
	  if(*cancel == 1)
	    {
	      break;
	    }
	  cmtx.lock();
	  std::tuple<std::filesystem::path,
	      std::vector<std::tuple<int, int, std::string>>> arch_tup;
	  arch_tup = zipvect[i];
	  std::thread *thr = new std::thread(
	      std::bind(&CreateCollection::zipThreadFunc, this, arch_tup,
			filepath, zip_hashp));
	  num_thr_runmtx.lock();
	  num_thr_run++;
	  thr->detach();
	  delete thr;
	  if(num_thr_run < threadnum)
	    {
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  if(*cancel == 0)
    {
      filename = filepath.parent_path().u8string();
      filename = filename + "/epubbase";
      filepath = std::filesystem::u8path(filename);
      f.open(filepath, std::ios_base::out | std::ios_base::binary);
      epub_hash_f.open(epub_hashp, std::ios_base::out | std::ios_base::binary);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";
      std::string b_p = book_p.u8string();
      epubbasemtx.lock();
      f.write(line.c_str(), line.size());
      epubbasemtx.unlock();
      line = book_p.u8string() + "\n";
      epubhashmtx.lock();
      epub_hash_f.write(line.c_str(), line.size());
      epubhashmtx.unlock();
      f.close();
      epub_hash_f.close();
      for(size_t i = 0; i < epub.size(); i++)
	{
	  if(*cancel == 1)
	    {
	      break;
	    }
	  std::filesystem::path fp = epub[i];
	  std::thread *thr = new std::thread(
	      std::bind(&CreateCollection::epubThreadFunc, this, fp, filepath,
			epub_hashp));
	  num_thr_runmtx.lock();
	  num_thr_run++;
	  thr->detach();
	  delete thr;
	  if(num_thr_run < threadnum)
	    {
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  if(*cancel == 1)
    {
      std::filesystem::remove_all(filepath.parent_path());
    }
  for(;;)
    {
      if(num_thr_runmtx.try_lock())
	{
	  if(num_thr_run == 0)
	    {
	      num_thr_runmtx.unlock();
	      break;
	    }
	  else
	    {
	      num_thr_runmtx.unlock();
	    }
	}
      usleep(100);
    }
}

void
CreateCollection::fb2ThreadFunc(std::filesystem::path fp,
				std::filesystem::path filepath,
				std::filesystem::path fb2_hashp)
{
  AuxFunc af;
  std::vector<char> hash_v = af.filehash(fp);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::vector<std::tuple<std::string, std::string>> basevect;
  basevect = fb2Parser(fp);
  std::string f_p = fp.u8string();
  f_p.erase(f_p.find(book_p.u8string()), book_p.u8string().size());
  f_p = f_p + "<?>";
  std::fstream fb2_hash_f;
  std::fstream f;
  f.open(filepath,
	 std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  fb2_hash_f.open(
      fb2_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  fb2basemtx.lock();
  f.write(f_p.c_str(), f_p.size());
  fb2basemtx.unlock();
  fb2hashmtx.lock();
  fb2_hash_f.write(f_p.c_str(), f_p.size());
  fb2_hash_f.write(hash_str.c_str(), hash_str.size());
  fb2_hash_f.close();
  fb2hashmtx.unlock();
  std::string line;
  auto bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Author";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  fb2basemtx.lock();
  f.write(line.c_str(), line.size());
  fb2basemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Book";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  fb2basemtx.lock();
  f.write(line.c_str(), line.size());
  fb2basemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Series";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  fb2basemtx.lock();
  f.write(line.c_str(), line.size());
  fb2basemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Genre";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  fb2basemtx.lock();
  f.write(line.c_str(), line.size());
  fb2basemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Date";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?L>";
  fb2basemtx.lock();
  f.write(line.c_str(), line.size());
  fb2basemtx.unlock();
  line.clear();
  f.close();

  file_countmtx.lock();
  file_count = file_count + 1;
  if(files_added)
    {
      files_added(file_count);
    }
  file_countmtx.unlock();

  num_thr_runmtx.lock();
  num_thr_run = num_thr_run - 1;
  if(num_thr_run < threadnum)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  num_thr_runmtx.unlock();
}

void
CreateCollection::zipThreadFunc(
    std::tuple<std::filesystem::path,
	std::vector<std::tuple<int, int, std::string>>> arch_tup,
    std::filesystem::path filepath, std::filesystem::path zip_hashp)
{
  AuxFunc af;
  std::vector<char> hash_v;
  hash_v = af.filehash(std::get<0>(arch_tup));
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::string archadress = std::get<0>(arch_tup).u8string();
  std::string line = archadress;
  hash_str = line + "<?>" + hash_str;
  hash_str.erase(hash_str.find(book_p.u8string()), book_p.u8string().size());

  std::fstream f;
  std::fstream zip_hash_f;
  f.open(filepath,
	 std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  zip_hash_f.open(
      zip_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);

  ziphashmtx.lock();
  zip_hash_f.write(hash_str.c_str(), hash_str.size());
  ziphashmtx.unlock();
  zip_hash_f.close();

  line.erase(line.find(book_p.u8string()), book_p.u8string().size());
  line = "<?a>" + line + "<?e>";
  zipbasemtx.lock();
  f.write(line.c_str(), line.size());
  zipbasemtx.unlock();
  std::vector<std::tuple<int, int, std::string>> locv;
  locv = std::get<1>(arch_tup);
  for(size_t j = 0; j < locv.size(); j++)
    {
      if(*cancel == 1)
	{
	  break;
	}
      int index = std::get<0>(locv[j]);
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << index;
      line = "<?>" + strm.str() + "<?>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      size_t inpsz = static_cast<size_t>(std::get<1>(locv[j]));
      std::vector<std::tuple<std::string, std::string>> basevect;
      std::filesystem::path ch_p = std::filesystem::u8path(
	  std::get<2>(locv[j]));
      if(ch_p.extension().u8string() == ".fb2")
	{
	  std::string input = af.unpackByIndex(archadress, index, inpsz);
	  basevect = fb2Parser(input);
	}
      if(ch_p.extension().u8string() == ".epub")
	{
	  std::string outfolder;
#ifdef __linux
	  outfolder = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
	  outfolder =
	      std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	  outfolder = outfolder + "/" + af.randomFileName();
	  ch_p = std::filesystem::u8path(outfolder);
	  if(std::filesystem::exists(ch_p))
	    {
	      std::filesystem::remove_all(ch_p);
	    }
	  af.unpackByIndex(archadress, outfolder, index);
	  if(std::filesystem::exists(ch_p))
	    {
	      for(auto &dirit : std::filesystem::directory_iterator(ch_p))
		{
		  std::filesystem::path p = dirit.path();
		  if(!std::filesystem::is_directory(p)
		      && p.extension().u8string() == ".epub")
		    {
		      basevect = epubparser(p);
		      break;
		    }
		}
	      std::filesystem::remove_all(ch_p);
	    }
	}
      auto bvit = std::find_if(basevect.begin(), basevect.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Author";
	});
      if(bvit != basevect.end())
	{
	  line = std::get<1>(*bvit);
	}
      line = line + "<?>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      line.clear();

      bvit = std::find_if(basevect.begin(), basevect.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Book";
	});
      if(bvit != basevect.end())
	{
	  line = std::get<1>(*bvit);
	}
      line = line + "<?>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      line.clear();

      bvit = std::find_if(basevect.begin(), basevect.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Series";
	});
      if(bvit != basevect.end())
	{
	  line = std::get<1>(*bvit);
	}
      line = line + "<?>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      line.clear();

      bvit = std::find_if(basevect.begin(), basevect.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Genre";
	});
      if(bvit != basevect.end())
	{
	  line = std::get<1>(*bvit);
	}
      line = line + "<?>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      line.clear();

      bvit = std::find_if(basevect.begin(), basevect.end(), []
      (auto &el)
	{
	  return std::get<0>(el) == "Date";
	});
      if(bvit != basevect.end())
	{
	  line = std::get<1>(*bvit);
	}
      line = line + "<?L>";
      zipbasemtx.lock();
      f.write(line.c_str(), line.size());
      zipbasemtx.unlock();
      line.clear();

      file_countmtx.lock();
      file_count = file_count + 1;
      if(files_added)
	{
	  files_added(file_count);
	}
      file_countmtx.unlock();
    }
  f.close();
  num_thr_runmtx.lock();
  num_thr_run = num_thr_run - 1;
  if(num_thr_run < threadnum)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  num_thr_runmtx.unlock();
}

void
CreateCollection::epubThreadFunc(std::filesystem::path fp,
				 std::filesystem::path filepath,
				 std::filesystem::path epub_hashp)
{
  AuxFunc af;
  std::vector<char> hash_v = af.filehash(fp);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::vector<std::tuple<std::string, std::string>> basevect;
  basevect = epubparser(fp);
  std::string f_p = fp.u8string();
  f_p.erase(f_p.find(book_p.u8string()), book_p.u8string().size());
  f_p = f_p + "<?>";
  std::fstream f;
  std::fstream epub_hash_f;
  f.open(filepath,
	 std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  epub_hash_f.open(
      epub_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  epubbasemtx.lock();
  f.write(f_p.c_str(), f_p.size());
  epubbasemtx.unlock();

  epubhashmtx.lock();
  epub_hash_f.write(f_p.c_str(), f_p.size());
  epub_hash_f.write(hash_str.c_str(), hash_str.size());
  epubhashmtx.unlock();
  epub_hash_f.close();
  std::string line;
  auto bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Author";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  epubbasemtx.lock();
  f.write(line.c_str(), line.size());
  epubbasemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Book";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  epubbasemtx.lock();
  f.write(line.c_str(), line.size());
  epubbasemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Series";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  epubbasemtx.lock();
  f.write(line.c_str(), line.size());
  epubbasemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Genre";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?>";
  epubbasemtx.lock();
  f.write(line.c_str(), line.size());
  epubbasemtx.unlock();
  line.clear();

  bvit = std::find_if(basevect.begin(), basevect.end(), []
  (auto &el)
    {
      return std::get<0>(el) == "Date";
    });
  if(bvit != basevect.end())
    {
      line = std::get<1>(*bvit);
    }
  line = line + "<?L>";
  epubbasemtx.lock();
  f.write(line.c_str(), line.size());
  epubbasemtx.unlock();
  line.clear();
  f.close();

  file_countmtx.lock();
  file_count = file_count + 1;
  if(files_added)
    {
      files_added(file_count);
    }
  file_countmtx.unlock();
  num_thr_runmtx.lock();
  num_thr_run = num_thr_run - 1;
  if(num_thr_run < threadnum)
    {
      cmtx.try_lock();
      cmtx.unlock();
    }
  num_thr_runmtx.unlock();
}

std::vector<std::tuple<std::string, std::string>>
CreateCollection::fb2Parser(std::filesystem::path filepath)
{
  AuxFunc af;
  std::fstream f;
  std::string headstr;
  size_t fsz = std::filesystem::file_size(filepath);
  std::vector<std::tuple<std::string, std::string>> result;
  if(fsz <= 104857600)
    {
      headstr.resize(fsz);
      f.open(filepath, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
	{
	  f.read(&headstr[0], fsz);
	  f.close();
	  std::string::size_type n = 0;
	  std::string conv_name = "";
	  conv_name = headstr;
	  n = conv_name.find("<?xml");
	  if(n != std::string::npos)
	    {
	      conv_name.erase(0, n);
	      conv_name = conv_name.substr(
		  0, conv_name.find("?>") + std::string("?>").size());
	      n = conv_name.find("encoding=");
	      if(n != std::string::npos)
		{
		  conv_name.erase(0, n + std::string("encoding=").size());
		  n = conv_name.find("\"");
		  if(n != std::string::npos)
		    {
		      conv_name.erase(
			  0, conv_name.find("\"") + std::string("\"").size());
		      conv_name = conv_name.substr(0, conv_name.find("\""));
		    }
		  else
		    {
		      n = conv_name.find("\'");
		      if(n != std::string::npos)
			{
			  conv_name.erase(
			      0,
			      conv_name.find("\'") + std::string("\'").size());
			  conv_name = conv_name.substr(0, conv_name.find("\'"));
			}
		    }
		}
	      else
		{
		  conv_name = "";
		}
	    }
	  else
	    {
	      conv_name = "";
	    }
	  if(!conv_name.empty())
	    {
	      af.toutf8(headstr, conv_name);
	    }
	  headstr.erase(0, headstr.find("<description"));
	  headstr = headstr.substr(
	      0,
	      headstr.find("</description>")
		  + std::string("</description>").size());
	  std::string genre_str = "";
	  n = 0;
	  while(n != std::string::npos)
	    {
	      std::string line = headstr;
	      std::string genre;
	      n = line.find("<genre");
	      if(n != std::string::npos)
		{
		  line.erase(0, n);
		  genre = line.substr(
		      0,
		      line.find("</genre>") + std::string("</genre>").size());
		  headstr.erase(headstr.find(genre), genre.size());
		  genre.erase(0, genre.find(">") + std::string(">").size());
		  genre = genre.substr(0, genre.find("</genre>"));
		  if(!genre_str.empty())
		    {
		      genre_str = genre_str + ", " + genre;
		    }
		  else
		    {
		      genre_str = genre;
		    }
		}
	    }

	  std::string docinfo = headstr;
	  docinfo.erase(0, docinfo.find("<document-info"));
	  docinfo = docinfo.substr(
	      0,
	      docinfo.find("</document-info>")
		  + std::string("</document-info>").size());
	  headstr.erase(headstr.find(docinfo), docinfo.size());

	  std::string auth_str = "";
	  n = 0;
	  while(n != std::string::npos)
	    {
	      std::string line = headstr;
	      n = line.find("<author>");
	      std::string auth;
	      if(n != std::string::npos)
		{
		  line.erase(0, n);
		  auth = line.substr(
		      0,
		      line.find("</author>") + std::string("</author>").size());
		  headstr.erase(headstr.find(auth), auth.size());
		  auth.erase(
		      0,
		      auth.find("<author>") + std::string("<author>").size());
		  auth = auth.substr(0, auth.find("</author>"));
		  std::string::size_type n_tmp;
		  std::string lastnm = auth;
		  n_tmp = lastnm.find("<last-name>");
		  if(n_tmp != std::string::npos)
		    {
		      lastnm.erase(0,
				   n_tmp + std::string("<last-name>").size());
		      lastnm = lastnm.substr(0, lastnm.find("</last-name>"));
		      if(!auth_str.empty())
			{
			  auth_str = auth_str + ", " + lastnm;
			}
		      else
			{
			  auth_str = lastnm;
			}
		    }
		  std::string name = auth;
		  n_tmp = name.find("<first-name>");
		  if(n_tmp != std::string::npos)
		    {
		      name.erase(0, n_tmp + std::string("<first-name>").size());
		      name = name.substr(0, name.find("</first-name>"));
		      if(!auth_str.empty())
			{
			  auth_str = auth_str + " " + name;
			}
		      else
			{
			  auth_str = name;
			}
		    }

		  std::string midname = auth;
		  n_tmp = midname.find("<middle-name>");
		  if(n_tmp != std::string::npos)
		    {
		      midname.erase(
			  0, n_tmp + std::string("<middle-name>").size());
		      midname = midname.substr(0,
					       midname.find("</middle-name>"));
		      if(!auth_str.empty())
			{
			  auth_str = auth_str + " " + midname;
			}
		      else
			{
			  auth_str = midname;
			}
		    }

		  std::string nickname = auth;
		  n_tmp = nickname.find("<nickname>");
		  if(n_tmp != std::string::npos)
		    {
		      nickname.erase(0,
				     n_tmp + std::string("<nickname>").size());
		      nickname = nickname.substr(0,
						 nickname.find("</nickname>"));
		      if(!auth_str.empty())
			{
			  auth_str = auth_str + " " + nickname;
			}
		      else
			{
			  auth_str = nickname;
			}
		    }
		}
	    }
	  std::string booktitle = headstr;
	  n = booktitle.find("<book-title>");
	  if(n != std::string::npos)
	    {
	      booktitle.erase(0, n + std::string("<book-title>").size());
	      booktitle = booktitle.substr(0, booktitle.find("</book-title>"));
	    }
	  else
	    {
	      booktitle = "";
	    }
	  std::string sequence_str = "";
	  n = 0;
	  while(n != std::string::npos)
	    {
	      std::string sequence = headstr;
	      n = sequence.find("<sequence");
	      if(n != std::string::npos)
		{
		  sequence.erase(0, n);
		  sequence = sequence.substr(
		      0, sequence.find(">") + std::string(">").size());
		  headstr.erase(headstr.find(sequence), sequence.size());
		  std::string numb = sequence;
		  std::string::size_type n_tmp;
		  sequence.erase(
		      0, sequence.find("name=") + std::string("name=").size());
		  n_tmp = sequence.find("\"");
		  if(n_tmp != std::string::npos)
		    {
		      sequence.erase(0, n_tmp + std::string("\"").size());
		      sequence = sequence.substr(0, sequence.find("\""));
		    }
		  else
		    {
		      n_tmp = sequence.find("\'");
		      if(n_tmp != std::string::npos)
			{
			  sequence.erase(0, n_tmp + std::string("\'").size());
			  sequence = sequence.substr(0, sequence.find("\'"));
			}
		    }
		  n_tmp = numb.find("number=");
		  if(n_tmp != std::string::npos)
		    {
		      numb.erase(0, n_tmp + std::string("number=").size());
		      n_tmp = numb.find("\"");
		      if(n_tmp != std::string::npos)
			{
			  numb.erase(0, n_tmp + std::string("\"").size());
			  numb = numb.substr(0, numb.find("\""));
			}
		      else
			{
			  n_tmp = numb.find("\'");
			  if(n_tmp != std::string::npos)
			    {
			      numb.erase(0, n_tmp + std::string("\'").size());
			      numb = numb.substr(0, numb.find("\'"));
			    }
			}
		    }
		  else
		    {
		      numb = "";
		    }
		  if(sequence_str.empty())
		    {
		      sequence_str = sequence;
		      if(!numb.empty())
			{
			  sequence_str = sequence_str + " " + numb;
			}
		    }
		  else
		    {
		      sequence_str = sequence_str + ", " + sequence;
		      if(!numb.empty())
			{
			  sequence_str = sequence_str + " " + numb;
			}
		    }
		}
	    }
	  std::string date = headstr;
	  n = date.find("<date");
	  std::string::size_type n_tmp;
	  n_tmp = date.find("<date/");
	  if(n != std::string::npos && n_tmp == std::string::npos)
	    {
	      date.erase(0, n);
	      date = date.substr(
		  0, date.find("</date>") + std::string("</date>").size());
	      std::string value = date;
	      n = value.find("vlaue=");
	      if(n != std::string::npos)
		{
		  value.erase(0, n + std::string("value=").size());
		  n = value.find("\"");
		  if(n != std::string::npos)
		    {
		      value.erase(0, n + std::string("\"").size());
		      value = value.substr(0, value.find("\""));
		      date = value;
		    }
		  else
		    {
		      n = value.find("\'");
		      if(n != std::string::npos)
			{
			  value.erase(0, n + std::string("\'").size());
			  value = value.substr(0, value.find("\'"));
			  date = value;
			}
		    }
		}
	      else
		{
		  value.erase(0, value.find(">") + std::string(">").size());
		  value = value.substr(0, value.find("</date>"));
		  date = value;
		}
	    }
	  else
	    {
	      date = "";
	    }
	  std::tuple<std::string, std::string> restup;
	  std::get<0>(restup) = "Author";
	  std::get<1>(restup) = auth_str;
	  result.push_back(restup);
	  std::get<0>(restup) = "Book";
	  std::get<1>(restup) = booktitle;
	  result.push_back(restup);
	  std::get<0>(restup) = "Series";
	  std::get<1>(restup) = sequence_str;
	  result.push_back(restup);
	  std::get<0>(restup) = "Genre";
	  std::get<1>(restup) = genre_str;
	  result.push_back(restup);
	  std::get<0>(restup) = "Date";
	  std::get<1>(restup) = date;
	  result.push_back(restup);
	}
    }
  return result;
}

std::vector<std::tuple<std::string, std::string>>
CreateCollection::fb2Parser(std::string input)
{
  AuxFunc af;
  std::string headstr = input;
  std::vector<std::tuple<std::string, std::string>> result;
  std::string::size_type n = 0;
  std::string conv_name = "";
  conv_name = headstr;
  n = conv_name.find("<?xml");
  if(n != std::string::npos)
    {
      conv_name.erase(0, n);
      conv_name = conv_name.substr(
	  0, conv_name.find("?>") + std::string("?>").size());
      n = conv_name.find("encoding=");
      if(n != std::string::npos)
	{
	  conv_name.erase(0, n + std::string("encoding=").size());
	  n = conv_name.find("\"");
	  if(n != std::string::npos)
	    {
	      conv_name.erase(0,
			      conv_name.find("\"") + std::string("\"").size());
	      conv_name = conv_name.substr(0, conv_name.find("\""));
	    }
	  else
	    {
	      n = conv_name.find("\'");
	      if(n != std::string::npos)
		{
		  conv_name.erase(
		      0, conv_name.find("\'") + std::string("\'").size());
		  conv_name = conv_name.substr(0, conv_name.find("\'"));
		}
	    }
	}
      else
	{
	  conv_name = "";
	}
    }
  else
    {
      conv_name = "";
    }
  if(!conv_name.empty())
    {
      af.toutf8(headstr, conv_name);
    }
  headstr.erase(0, headstr.find("<description"));
  headstr = headstr.substr(
      0, headstr.find("</description>") + std::string("</description>").size());
  std::string genre_str = "";
  n = 0;
  while(n != std::string::npos)
    {
      std::string line = headstr;
      std::string genre;
      n = line.find("<genre");
      if(n != std::string::npos)
	{
	  line.erase(0, n);
	  genre = line.substr(
	      0, line.find("</genre>") + std::string("</genre>").size());
	  headstr.erase(headstr.find(genre), genre.size());
	  genre.erase(0, genre.find(">") + std::string(">").size());
	  genre = genre.substr(0, genre.find("</genre>"));
	  if(!genre_str.empty())
	    {
	      genre_str = genre_str + ", " + genre;
	    }
	  else
	    {
	      genre_str = genre;
	    }
	}
    }

  std::string docinfo = headstr;
  docinfo.erase(0, docinfo.find("<document-info"));
  docinfo = docinfo.substr(
      0,
      docinfo.find("</document-info>") + std::string("<document-info>").size());
  headstr.erase(headstr.find(docinfo), docinfo.size());

  std::string auth_str = "";
  n = 0;
  while(n != std::string::npos)
    {
      std::string line = headstr;
      n = line.find("<author>");
      std::string auth;
      if(n != std::string::npos)
	{
	  line.erase(0, n);
	  auth = line.substr(
	      0, line.find("</author>") + std::string("</author>").size());
	  headstr.erase(headstr.find(auth), auth.size());
	  auth.erase(0, auth.find("<author>") + std::string("<author>").size());
	  auth = auth.substr(0, auth.find("</author>"));
	  std::string::size_type n_tmp;
	  std::string lastnm = auth;
	  n_tmp = lastnm.find("<last-name>");
	  if(n_tmp != std::string::npos)
	    {
	      lastnm.erase(0, n_tmp + std::string("<last-name>").size());
	      lastnm = lastnm.substr(0, lastnm.find("</last-name>"));
	      if(!auth_str.empty())
		{
		  auth_str = auth_str + ", " + lastnm;
		}
	      else
		{
		  auth_str = lastnm;
		}
	    }
	  std::string name = auth;
	  n_tmp = name.find("<first-name>");
	  if(n_tmp != std::string::npos)
	    {
	      name.erase(0, n_tmp + std::string("<first-name>").size());
	      name = name.substr(0, name.find("</first-name>"));
	      if(!auth_str.empty())
		{
		  auth_str = auth_str + " " + name;
		}
	      else
		{
		  auth_str = name;
		}
	    }

	  std::string midname = auth;
	  n_tmp = midname.find("<middle-name>");
	  if(n_tmp != std::string::npos)
	    {
	      midname.erase(0, n_tmp + std::string("<middle-name>").size());
	      midname = midname.substr(0, midname.find("</middle-name>"));
	      if(!auth_str.empty())
		{
		  auth_str = auth_str + " " + midname;
		}
	      else
		{
		  auth_str = midname;
		}
	    }

	  std::string nickname = auth;
	  n_tmp = nickname.find("<nickname>");
	  if(n_tmp != std::string::npos)
	    {
	      nickname.erase(0, n_tmp + std::string("<nickname>").size());
	      nickname = nickname.substr(0, nickname.find("</nickname>"));
	      if(!auth_str.empty())
		{
		  auth_str = auth_str + " " + nickname;
		}
	      else
		{
		  auth_str = nickname;
		}
	    }
	}
    }
  std::string booktitle = headstr;
  n = booktitle.find("<book-title>");
  if(n != std::string::npos)
    {
      booktitle.erase(0, n + std::string("<book-title>").size());
      booktitle = booktitle.substr(0, booktitle.find("</book-title>"));
    }
  else
    {
      booktitle = "";
    }
  std::string sequence_str = "";
  n = 0;
  while(n != std::string::npos)
    {
      std::string sequence = headstr;
      n = sequence.find("<sequence");
      if(n != std::string::npos)
	{
	  sequence.erase(0, n);
	  sequence = sequence.substr(
	      0, sequence.find(">") + std::string(">").size());
	  headstr.erase(headstr.find(sequence), sequence.size());
	  std::string numb = sequence;
	  std::string::size_type n_tmp;
	  sequence.erase(0,
			 sequence.find("name=") + std::string("name=").size());
	  n_tmp = sequence.find("\"");
	  if(n_tmp != std::string::npos)
	    {
	      sequence.erase(0, n_tmp + std::string("\"").size());
	      sequence = sequence.substr(0, sequence.find("\""));
	    }
	  else
	    {
	      n_tmp = sequence.find("\'");
	      if(n_tmp != std::string::npos)
		{
		  sequence.erase(0, n_tmp + std::string("\'").size());
		  sequence = sequence.substr(0, sequence.find("\'"));
		}
	    }
	  n_tmp = numb.find("number=");
	  if(n_tmp != std::string::npos)
	    {
	      numb.erase(0, n_tmp + std::string("number=").size());
	      n_tmp = numb.find("\"");
	      if(n_tmp != std::string::npos)
		{
		  numb.erase(0, n_tmp + std::string("\"").size());
		  numb = numb.substr(0, numb.find("\""));
		}
	      else
		{
		  n_tmp = numb.find("\'");
		  if(n_tmp != std::string::npos)
		    {
		      numb.erase(0, n_tmp + std::string("\'").size());
		      numb = numb.substr(0, numb.find("\'"));
		    }
		}
	    }
	  else
	    {
	      numb = "";
	    }
	  if(sequence_str.empty())
	    {
	      sequence_str = sequence;
	      if(!numb.empty())
		{
		  sequence_str = sequence_str + " " + numb;
		}
	    }
	  else
	    {
	      sequence_str = sequence_str + ", " + sequence;
	      if(!numb.empty())
		{
		  sequence_str = sequence_str + " " + numb;
		}
	    }
	}
    }
  std::string date = headstr;
  n = date.find("<date");
  std::string::size_type n_tmp;
  n_tmp = date.find("<date/");
  if(n != std::string::npos && n_tmp == std::string::npos)
    {
      date.erase(0, n);
      date = date.substr(0,
			 date.find("</date>") + std::string("</date>").size());
      std::string value = date;
      n = value.find("vlaue=");
      if(n != std::string::npos)
	{
	  value.erase(0, n + std::string("value=").size());
	  n = value.find("\"");
	  if(n != std::string::npos)
	    {
	      value.erase(0, n + std::string("\"").size());
	      value = value.substr(0, value.find("\""));
	      date = value;
	    }
	  else
	    {
	      n = value.find("\'");
	      if(n != std::string::npos)
		{
		  value.erase(0, n + std::string("\'").size());
		  value = value.substr(0, value.find("\'"));
		  date = value;
		}
	    }
	}
      else
	{
	  value.erase(0, value.find(">") + std::string(">").size());
	  value = value.substr(0, value.find("</date>"));
	  date = value;
	}
    }
  else
    {
      date = "";
    }
  std::tuple<std::string, std::string> restup;
  std::string::size_type space_n = 0;
  while(space_n != std::string::npos)
    {
      space_n = auth_str.find("  ");
      if(space_n != std::string::npos)
	{
	  auth_str.erase(space_n, std::string(" ").size());
	}
    }
  std::get<0>(restup) = "Author";
  std::get<1>(restup) = auth_str;
  result.push_back(restup);
  std::get<0>(restup) = "Book";
  std::get<1>(restup) = booktitle;
  result.push_back(restup);
  std::get<0>(restup) = "Series";
  std::get<1>(restup) = sequence_str;
  result.push_back(restup);
  std::get<0>(restup) = "Genre";
  std::get<1>(restup) = genre_str;
  result.push_back(restup);
  std::get<0>(restup) = "Date";
  std::get<1>(restup) = date;
  result.push_back(restup);
  return result;
}

std::vector<std::tuple<std::string, std::string>>
CreateCollection::epubparser(std::filesystem::path input)
{
  std::vector<std::tuple<std::string, std::string>> result;

  std::string filename;
#ifdef __linux
  filename = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
  filename = std::filesystem::temp_directory_path().parent_path().u8string();
#endif
  AuxFunc af;
  filename = filename + "/" + af.randomFileName();
  std::filesystem::path filepath = std::filesystem::u8path(filename);
  if(std::filesystem::exists(filepath))
    {
      std::filesystem::remove_all(filepath);
    }
  std::vector<std::tuple<int, int, std::string>> list;
  af.fileNames(input.u8string(), list);
  auto itl = std::find_if(list.begin(), list.end(), []
  (auto &el)
    {
      std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
      AuxFunc af;
      std::string ext = p.extension().u8string();
      af.stringToLower(ext);
      return ext == ".opf";
    });
  if(itl != list.end())
    {
      af.unpackByIndex(input.u8string(), filepath.u8string(),
		       std::get<0>(*itl));
      for(auto &dirit : std::filesystem::recursive_directory_iterator(filepath))
	{
	  std::filesystem::path p = dirit.path();
	  if(!std::filesystem::is_directory(p))
	    {
	      std::string ext = p.extension().u8string();
	      af.stringToLower(ext);
	      if(ext == ".opf")
		{
		  std::fstream f;
		  f.open(p, std::ios_base::in | std::ios_base::binary);
		  if(f.is_open())
		    {
		      std::string file_str;
		      file_str.resize(std::filesystem::file_size(p));
		      f.read(&file_str[0], file_str.size());
		      f.close();

		      std::string::size_type n;
		      std::string conv_name = "";
		      conv_name = file_str;
		      n = conv_name.find("<?xml");
		      if(n != std::string::npos)
			{
			  conv_name.erase(0, n);
			  conv_name = conv_name.substr(
			      0,
			      conv_name.find("?>") + std::string("?>").size());
			  n = conv_name.find("encoding=");
			  if(n != std::string::npos)
			    {
			      conv_name.erase(
				  0, n + std::string("encoding=").size());
			      n = conv_name.find("\"");
			      if(n != std::string::npos)
				{
				  conv_name.erase(
				      0,
				      conv_name.find("\"")
					  + std::string("\"").size());
				  conv_name = conv_name.substr(
				      0, conv_name.find("\""));
				}
			      else
				{
				  n = conv_name.find("\'");
				  if(n != std::string::npos)
				    {
				      conv_name.erase(
					  0,
					  conv_name.find("\'")
					      + std::string("\'").size());
				      conv_name = conv_name.substr(
					  0, conv_name.find("\'"));
				    }
				}
			    }
			  else
			    {
			      conv_name = "";
			    }
			}
		      else
			{
			  conv_name = "";
			}
		      if(!conv_name.empty())
			{
			  af.toutf8(file_str, conv_name);
			}

		      n = 0;
		      std::string auth_str;
		      std::string line;
		      while(n != std::string::npos)
			{
			  n = file_str.find("</dc:creator>");
			  if(n != std::string::npos)
			    {
			      line = file_str.substr(
				  0, n + std::string("</dc:creator>").size());
			      line.erase(0, line.find("<dc:creator"));
			      file_str.erase(file_str.find(line), line.size());
			      std::string::size_type naut;
			      naut = line.find("opf:role=\"aut\"");
			      if(naut != std::string::npos)
				{
				  line.erase(
				      0,
				      line.find(">") + std::string(">").size());
				  line = line.substr(0, line.find("<"));
				  if(auth_str.empty())
				    {
				      auth_str = line;
				    }
				  else
				    {
				      auth_str = auth_str + ", " + line;
				    }
				}
			    }
			}
		      std::string booktitle;
		      n = 0;
		      while(n != std::string::npos)
			{
			  n = file_str.find("</dc:title>");
			  if(n != std::string::npos)
			    {
			      line = file_str.substr(
				  0, n + std::string("</dc:title>").size());
			      line.erase(0, line.find("<dc:title"));
			      file_str.erase(file_str.find(line), line.size());
			      line.erase(
				  0, line.find(">") + std::string(">").size());
			      line = line.substr(0, line.find("<"));
			      if(booktitle.empty())
				{
				  booktitle = line;
				}
			      else
				{
				  booktitle = booktitle + ", " + line;
				}
			    }
			}
		      std::string genre_str;
		      n = 0;
		      while(n != std::string::npos)
			{
			  n = file_str.find("</dc:subject>");
			  if(n != std::string::npos)
			    {
			      line = file_str.substr(
				  0, n + std::string("</dc:subject>").size());
			      line.erase(0, line.find("<dc:subject"));
			      file_str.erase(file_str.find(line), line.size());
			      line.erase(
				  0, line.find(">") + std::string(">").size());
			      line = line.substr(0, line.find("<"));
			      if(genre_str.empty())
				{
				  genre_str = line;
				}
			      else
				{
				  genre_str = genre_str + ", " + line;
				}
			    }
			}
		      std::string date;
		      n = 0;
		      while(n != std::string::npos)
			{
			  n = file_str.find("</dc:date>");
			  if(n != std::string::npos)
			    {
			      line = file_str.substr(
				  0, n + std::string("</dc:date>").size());
			      line.erase(0, line.find("<dc:date"));
			      file_str.erase(file_str.find(line), line.size());
			      line.erase(
				  0, line.find(">") + std::string(">").size());
			      line = line.substr(0, line.find("<"));
			      if(date.empty())
				{
				  date = line;
				}
			      else
				{
				  date = date + ", " + line;
				}
			    }
			}
		      std::tuple<std::string, std::string> restup;
		      std::string::size_type space_n;
		      while(space_n != std::string::npos)
			{
			  space_n = auth_str.find("  ");
			  if(space_n != std::string::npos)
			    {
			      auth_str.erase(space_n, std::string(" ").size());
			    }
			}
		      std::get<0>(restup) = "Author";
		      std::get<1>(restup) = auth_str;
		      result.push_back(restup);
		      std::get<0>(restup) = "Book";
		      std::get<1>(restup) = booktitle;
		      result.push_back(restup);
		      std::get<0>(restup) = "Series";
		      std::get<1>(restup) = "";
		      result.push_back(restup);
		      std::get<0>(restup) = "Genre";
		      std::get<1>(restup) = genre_str;
		      result.push_back(restup);
		      std::get<0>(restup) = "Date";
		      std::get<1>(restup) = date;
		      result.push_back(restup);
		    }
		  break;
		}
	    }
	}
      std::filesystem::remove_all(filepath);
    }
  return result;
}
