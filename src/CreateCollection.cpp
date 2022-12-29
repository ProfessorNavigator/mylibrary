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
  // TODO Auto-generated destructor stub
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
	      std::string ext = p.extension().u8string();
	      if(ext == ".fb2")
		{
		  fb2.push_back(p);
		}
	      else if(ext == ".epub")
		{
		  epub.push_back(p);
		}
	      else if(ext == ".pdf")
		{
		  pdf.push_back(p);
		}
	      else if(ext == ".djvu")
		{
		  djvu.push_back(p);
		}
	      else if(ext == ".zip")
		{
		  std::vector<std::tuple<int, int, std::string>> archlist;
		  af.fileNames(p.u8string(), archlist);
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
	    std::vector<std::tuple<int, int, std::string>>>> *zipvectin,
    std::vector<std::filesystem::path> *pdfin,
    std::vector<std::filesystem::path> *djvuin)
{
  fb2 = *fb2in;
  zipvect = *zipvectin;
  epub = *epubin;
  pdf = *pdfin;
  djvu = *djvuin;
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
  filename = filepath.parent_path().u8string();
  filename = filename + "/pdfhash";
  std::filesystem::path pdf_hashp = std::filesystem::u8path(filename);
  filename = filepath.parent_path().u8string();
  filename = filename + "/djvuhash";
  std::filesystem::path djvu_hashp = std::filesystem::u8path(filename);

  if(total_files)
    {
      int zipf = 0;
      for(size_t i = 0; i < zipvect.size(); i++)
	{
	  zipf = zipf + static_cast<int>(std::get<1>(zipvect[i]).size());
	}
      total_files(
	  static_cast<int>(fb2.size()) + static_cast<int>(epub.size())
	      + static_cast<int>(pdf.size()) + static_cast<int>(djvu.size())
	      + zipf);
    }
  if(*cancel == 0)
    {
      std::fstream fb2_hash_f;
      std::fstream fb2_base_f;
      std::string line = "<bp>" + book_p.u8string() + "</bp>";

      fb2basemtx.lock();
      fb2_base_f.open(filepath, std::ios_base::out | std::ios_base::binary);
      fb2_base_f.write(line.c_str(), line.size());
      fb2_base_f.close();
      fb2basemtx.unlock();

      line = book_p.u8string() + "\n";

      fb2hashmtx.lock();
      fb2_hash_f.open(fb2_hashp, std::ios_base::out | std::ios_base::binary);
      fb2_hash_f.write(line.c_str(), line.size());
      fb2_hash_f.close();
      fb2hashmtx.unlock();

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
	      cmtx.try_lock();
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  if(*cancel == 0)
    {
      std::fstream zip_hash_f;
      std::fstream zip_base_f;
      filename = filepath.parent_path().u8string();
      filename = filename + "/zipbase";
      filepath = std::filesystem::u8path(filename);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";

      zipbasemtx.lock();
      zip_base_f.open(filepath, std::ios_base::out | std::ios_base::binary);
      zip_base_f.write(line.c_str(), line.size());
      zip_base_f.close();
      zipbasemtx.unlock();

      line = book_p.u8string() + "\n";

      ziphashmtx.lock();
      zip_hash_f.open(zip_hashp, std::ios_base::out | std::ios_base::binary);
      zip_hash_f.write(line.c_str(), line.size());
      zip_hash_f.close();
      ziphashmtx.unlock();

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
	      cmtx.try_lock();
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  if(*cancel == 0)
    {
      std::fstream epub_hash_f;
      std::fstream epub_base_f;
      filename = filepath.parent_path().u8string();
      filename = filename + "/epubbase";
      filepath = std::filesystem::u8path(filename);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";
      std::string b_p = book_p.u8string();

      epubbasemtx.lock();
      epub_base_f.open(filepath, std::ios_base::out | std::ios_base::binary);
      epub_base_f.write(line.c_str(), line.size());
      epub_base_f.close();
      epubbasemtx.unlock();

      line = book_p.u8string() + "\n";

      epubhashmtx.lock();
      epub_hash_f.open(epub_hashp, std::ios_base::out | std::ios_base::binary);
      epub_hash_f.write(line.c_str(), line.size());
      epub_hash_f.close();
      epubhashmtx.unlock();

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
	      cmtx.try_lock();
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  if(*cancel == 0)
    {
      std::fstream pdf_hash_f;
      std::fstream pdf_base_f;
      filename = filepath.parent_path().u8string();
      filename = filename + "/pdfbase";
      filepath = std::filesystem::u8path(filename);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";
      std::string b_p = book_p.u8string();

      pdfbasemtx.lock();
      pdf_base_f.open(filepath, std::ios_base::out | std::ios_base::binary);
      pdf_base_f.write(line.c_str(), line.size());
      pdf_base_f.close();
      pdfbasemtx.unlock();

      line = book_p.u8string() + "\n";

      pdfhashmtx.lock();
      pdf_hash_f.open(pdf_hashp, std::ios_base::out | std::ios_base::binary);
      pdf_hash_f.write(line.c_str(), line.size());
      pdf_hash_f.close();
      pdfhashmtx.unlock();

      for(size_t i = 0; i < pdf.size(); i++)
	{
	  if(*cancel == 1)
	    {
	      break;
	    }
	  std::filesystem::path fp = pdf[i];
	  std::thread *thr = new std::thread(
	      std::bind(&CreateCollection::pdfThreadFunc, this, fp, filepath,
			pdf_hashp));
	  num_thr_runmtx.lock();
	  num_thr_run++;
	  thr->detach();
	  delete thr;
	  if(num_thr_run < threadnum)
	    {
	      cmtx.try_lock();
	      cmtx.unlock();
	    }
	  num_thr_runmtx.unlock();
	}
    }
  if(*cancel == 0)
    {
      std::fstream djvu_hash_f;
      std::fstream djvu_base_f;
      filename = filepath.parent_path().u8string();
      filename = filename + "/djvubase";
      filepath = std::filesystem::u8path(filename);
      std::string line = "<bp>" + book_p.u8string() + "</bp>";
      std::string b_p = book_p.u8string();

      djvubasemtx.lock();
      djvu_base_f.open(filepath, std::ios_base::out | std::ios_base::binary);
      djvu_base_f.write(line.c_str(), line.size());
      djvu_base_f.close();
      djvubasemtx.unlock();

      line = book_p.u8string() + "\n";

      djvuhashmtx.lock();
      djvu_hash_f.open(djvu_hashp, std::ios_base::out | std::ios_base::binary);
      djvu_hash_f.write(line.c_str(), line.size());
      djvu_hash_f.close();
      djvuhashmtx.unlock();

      for(size_t i = 0; i < djvu.size(); i++)
	{
	  if(*cancel == 1)
	    {
	      break;
	    }
	  std::filesystem::path fp = djvu[i];
	  std::thread *thr = new std::thread(
	      std::bind(&CreateCollection::djvuThreadFunc, this, fp, filepath,
			djvu_hashp));
	  num_thr_runmtx.lock();
	  num_thr_run++;
	  thr->detach();
	  delete thr;
	  if(num_thr_run < threadnum)
	    {
	      cmtx.try_lock();
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
  std::vector<char> write_v;
  std::vector<char> hash_v = af.filehash(fp, cancel);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::vector<std::tuple<std::string, std::string>> basevect;
  basevect = fb2parser(fp);
  std::string f_p = fp.u8string();
  f_p.erase(f_p.find(book_p.u8string()), book_p.u8string().size());
  f_p = f_p + "<?>";
  std::copy(f_p.begin(), f_p.end(), std::back_inserter(write_v));
  std::fstream fb2_hash_f;
  fb2hashmtx.lock();
  fb2_hash_f.open(
      fb2_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
  line.clear();
  std::fstream fb2_base_f;
  fb2basemtx.lock();
  fb2_base_f.open(
      filepath,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  fb2_base_f.write(write_v.data(), write_v.size());
  fb2_base_f.close();
  fb2basemtx.unlock();

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
  std::vector<char> write_v;
  hash_v = af.filehash(std::get<0>(arch_tup), cancel);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::string archadress = std::get<0>(arch_tup).u8string();
  std::string line = archadress;
  hash_str = line + "<?>" + hash_str;
  hash_str.erase(hash_str.find(book_p.u8string()), book_p.u8string().size());

  std::fstream zip_hash_f;
  ziphashmtx.lock();
  zip_hash_f.open(
      zip_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  zip_hash_f.write(hash_str.c_str(), hash_str.size());
  zip_hash_f.close();
  ziphashmtx.unlock();

  line.erase(line.find(book_p.u8string()), book_p.u8string().size());
  line = "<?a>" + line + "<?e>";
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
  std::vector<std::tuple<int, int, std::string>> locv;
  locv = std::get<1>(arch_tup);
  for(size_t j = 0; j < locv.size(); j++)
    {
      if(*cancel == 1)
	{
	  break;
	}
      line.clear();
      int index = std::get<0>(locv[j]);
      std::filesystem::path ch_p = std::filesystem::u8path(
	  std::get<2>(locv[j]));
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << index;
      line = "<?>" + strm.str() + "<?>";
      std::copy(line.begin(), line.end(), std::back_inserter(write_v));
      size_t inpsz = static_cast<size_t>(std::get<1>(locv[j]));
      std::vector<std::tuple<std::string, std::string>> basevect;
      std::string extch_p = ch_p.extension().u8string();
      if(extch_p == ".fb2")
	{
	  std::string input = af.unpackByIndex(archadress, index, inpsz);
	  basevect = fb2parser(input);
	}
      else if(extch_p == ".epub" || extch_p == ".pdf" || extch_p == ".djvu")
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
		  if(!std::filesystem::is_directory(p))
		    {
		      std::string ext_p = p.extension().u8string();
		      if(ext_p == ".epub")
			{
			  std::vector<std::tuple<int, int, std::string>> tlv;
			  af.fileNames(std::get<0>(arch_tup).u8string(), tlv);
			  auto ittlv = std::find_if(tlv.begin(), tlv.end(), [p]
			  (auto &el)
			    {
			      std::filesystem::path lp =
			      std::filesystem::u8path(std::get<2>(el));
			      if(lp.stem() == p.stem() &&
				  lp.extension().u8string() == ".fbd")
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
			  if(ittlv != tlv.end())
			    {
			      std::string fbdf = af.unpackByIndex(
				  archadress, std::get<0>(*ittlv),
				  std::get<1>(*ittlv));
			      basevect = fb2parser(fbdf);
			    }
			  else
			    {
			      basevect = epubparser(p);
			    }
			  break;
			}
		      else if(ext_p == ".pdf")
			{
			  std::vector<std::tuple<int, int, std::string>> tlv;
			  af.fileNames(std::get<0>(arch_tup).u8string(), tlv);
			  auto ittlv = std::find_if(tlv.begin(), tlv.end(), [p]
			  (auto &el)
			    {
			      std::filesystem::path lp =
			      std::filesystem::u8path(std::get<2>(el));
			      if(lp.stem() == p.stem() &&
				  lp.extension().u8string() == ".fbd")
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
			  if(ittlv != tlv.end())
			    {
			      std::string fbdf = af.unpackByIndex(
				  archadress, std::get<0>(*ittlv),
				  std::get<1>(*ittlv));
			      basevect = fb2parser(fbdf);
			    }
			  else
			    {
			      basevect = pdfparser(p);
			    }
			  break;
			}
		      else if(ext_p == ".djvu")
			{
			  std::vector<std::tuple<int, int, std::string>> tlv;
			  af.fileNames(std::get<0>(arch_tup).u8string(), tlv);
			  auto ittlv = std::find_if(tlv.begin(), tlv.end(), [p]
			  (auto &el)
			    {
			      std::filesystem::path lp =
			      std::filesystem::u8path(std::get<2>(el));
			      if(lp.stem() == p.stem() &&
				  lp.extension().u8string() == ".fbd")
				{
				  return true;
				}
			      else
				{
				  return false;
				}
			    });
			  if(ittlv != tlv.end())
			    {
			      std::string fbdf = af.unpackByIndex(
				  archadress, std::get<0>(*ittlv),
				  std::get<1>(*ittlv));
			      basevect = fb2parser(fbdf);
			    }
			  else
			    {
			      basevect = djvuparser(p);
			    }
			  break;
			}
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
      std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
      std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
      std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
      std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
      std::copy(line.begin(), line.end(), std::back_inserter(write_v));
      line.clear();

      file_countmtx.lock();
      file_count = file_count + 1;
      if(files_added)
	{
	  files_added(file_count);
	}
      file_countmtx.unlock();
    }

  std::fstream zip_base_f;
  zipbasemtx.lock();
  zip_base_f.open(
      filepath,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  zip_base_f.write(write_v.data(), write_v.size());
  zip_base_f.close();
  zipbasemtx.unlock();

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
  std::vector<char> write_v;
  std::vector<char> hash_v = af.filehash(fp, cancel);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::vector<std::tuple<std::string, std::string>> basevect;
  basevect = epubparser(fp);
  std::string f_p = fp.u8string();
  f_p.erase(f_p.find(book_p.u8string()), book_p.u8string().size());
  f_p = f_p + "<?>";
  std::copy(f_p.begin(), f_p.end(), std::back_inserter(write_v));

  std::fstream epub_hash_f;
  epubhashmtx.lock();
  epub_hash_f.open(
      epub_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  epub_hash_f.write(f_p.c_str(), f_p.size());
  epub_hash_f.write(hash_str.c_str(), hash_str.size());
  epub_hash_f.close();
  epubhashmtx.unlock();

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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
  line.clear();

  epubbasemtx.lock();
  std::fstream epub_base_f;
  epub_base_f.open(
      filepath,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  epub_base_f.write(write_v.data(), write_v.size());
  epub_base_f.close();
  epubbasemtx.unlock();

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
CreateCollection::pdfThreadFunc(std::filesystem::path fp,
				std::filesystem::path filepath,
				std::filesystem::path pdf_hashp)
{
  AuxFunc af;
  std::vector<char> write_v;
  std::vector<char> hash_v = af.filehash(fp, cancel);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::vector<std::tuple<std::string, std::string>> basevect;
  basevect = pdfparser(fp);
  std::string f_p = fp.u8string();
  f_p.erase(f_p.find(book_p.u8string()), book_p.u8string().size());
  f_p = f_p + "<?>";
  std::copy(f_p.begin(), f_p.end(), std::back_inserter(write_v));

  std::fstream pdf_hash_f;
  pdfhashmtx.lock();
  pdf_hash_f.open(
      pdf_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  pdf_hash_f.write(f_p.c_str(), f_p.size());
  pdf_hash_f.write(hash_str.c_str(), hash_str.size());
  pdf_hash_f.close();
  pdfhashmtx.unlock();

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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
  line.clear();

  pdfbasemtx.lock();
  std::fstream pdf_base_f;
  pdf_base_f.open(
      filepath,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  pdf_base_f.write(write_v.data(), write_v.size());
  pdf_base_f.close();
  pdfbasemtx.unlock();

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
CreateCollection::djvuThreadFunc(std::filesystem::path fp,
				 std::filesystem::path filepath,
				 std::filesystem::path djvu_hashp)
{
  AuxFunc af;
  std::vector<char> write_v;
  std::vector<char> hash_v = af.filehash(fp, cancel);
  std::string hash_str = af.to_hex(&hash_v);
  hash_str = hash_str + "\n";
  std::vector<std::tuple<std::string, std::string>> basevect;
  basevect = djvuparser(fp);
  std::string f_p = fp.u8string();
  f_p.erase(f_p.find(book_p.u8string()), book_p.u8string().size());
  f_p = f_p + "<?>";
  std::copy(f_p.begin(), f_p.end(), std::back_inserter(write_v));

  std::fstream djvu_hash_f;
  djvuhashmtx.lock();
  djvu_hash_f.open(
      djvu_hashp,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  djvu_hash_f.write(f_p.c_str(), f_p.size());
  djvu_hash_f.write(hash_str.c_str(), hash_str.size());
  djvu_hash_f.close();
  djvuhashmtx.unlock();

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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
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
  std::copy(line.begin(), line.end(), std::back_inserter(write_v));
  line.clear();

  djvubasemtx.lock();
  std::fstream djvu_base_f;
  djvu_base_f.open(
      filepath,
      std::ios_base::out | std::ios_base::app | std::ios_base::binary);
  djvu_base_f.write(write_v.data(), write_v.size());
  djvu_base_f.close();
  djvubasemtx.unlock();

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
CreateCollection::fb2parser(std::filesystem::path filepath)
{
  AuxFunc af;
  std::fstream f;
  std::string headstr;
  size_t fsz = std::filesystem::file_size(filepath);
  std::vector<std::tuple<std::string, std::string>> result;
  headstr.resize(fsz);
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      f.read(&headstr[0], fsz);
      f.close();
      std::string::size_type n = 0;
      std::string conv_name;
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
			  0, conv_name.find("\'") + std::string("\'").size());
		      conv_name = conv_name.substr(0, conv_name.find("\'"));
		    }
		}
	    }
	  else
	    {
	      conv_name.clear();
	    }
	}
      else
	{
	  conv_name.clear();
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
      std::string genre_str;
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
	  docinfo.find("</document-info>")
	      + std::string("</document-info>").size());
      headstr.erase(headstr.find(docinfo), docinfo.size());

      std::string auth_str;
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
	      auth.erase(
		  0, auth.find("<author>") + std::string("<author>").size());
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
	  booktitle.clear();
	}
      std::string sequence_str;
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
		  numb.clear();
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
	  date.clear();
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
  return result;
}

std::vector<std::tuple<std::string, std::string>>
CreateCollection::fb2parser(std::string input)
{
  AuxFunc af;
  std::string headstr = input;
  std::vector<std::tuple<std::string, std::string>> result;
  std::string::size_type n = 0;
  std::string conv_name;
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
	  conv_name.clear();
	}
    }
  else
    {
      conv_name.clear();
    }
  if(!conv_name.empty())
    {
      af.toutf8(headstr, conv_name);
    }
  headstr.erase(0, headstr.find("<description"));
  headstr = headstr.substr(
      0, headstr.find("</description>") + std::string("</description>").size());
  std::string genre_str;
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

  std::string auth_str;
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
      booktitle.clear();
    }
  std::string sequence_str;
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
	      numb.clear();
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
      date.clear();
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
		      std::string conv_name;
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
			      conv_name.clear();
			    }
			}
		      else
			{
			  conv_name.clear();
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
			      line.erase(
				  0, line.find(">") + std::string(">").size());
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
		      std::get<1>(restup) = std::string();
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

std::vector<std::tuple<std::string, std::string>>
CreateCollection::pdfparser(std::filesystem::path input)
{
  std::vector<std::tuple<std::string, std::string>> result;
  poppler::document *doc = poppler::document::load_from_file(input.string());
  poppler::ustring inf = doc->get_author();
  std::tuple<std::string, std::string> restup;
  std::vector<char> buf = inf.to_utf8();
  std::string auth_str;
  std::copy(buf.begin(), buf.end(), std::back_inserter(auth_str));
  std::get<0>(restup) = "Author";
  std::get<1>(restup) = auth_str;
  result.push_back(restup);

  inf = doc->get_title();
  buf = inf.to_utf8();
  std::string booktitle;
  std::copy(buf.begin(), buf.end(), std::back_inserter(booktitle));
  if(booktitle.empty())
    {
      booktitle = input.stem().u8string();
    }
  std::get<0>(restup) = "Book";
  std::get<1>(restup) = booktitle;
  result.push_back(restup);

  std::get<0>(restup) = "Series";
  std::get<1>(restup) = std::string();
  result.push_back(restup);

  std::get<0>(restup) = "Genre";
  std::get<1>(restup) = std::string();
  result.push_back(restup);

#ifdef _OLDPOPPLER
  time_t fcr = doc->get_creation_date();
#endif
#ifndef _OLDPOPPLER
  time_t fcr = doc->get_creation_date_t();
#endif
  struct tm *mtm;
  mtm = gmtime(&fcr);
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << mtm->tm_mday;
  strm << "-";
  strm << mtm->tm_mon + 1;
  strm << "-";
  strm << 1900 + mtm->tm_year;
  std::get<0>(restup) = "Date";
  std::get<1>(restup) = strm.str();
  result.push_back(restup);

  delete doc;
  return result;
}

std::vector<std::tuple<std::string, std::string>>
CreateCollection::djvuparser(std::filesystem::path input)
{
  std::vector<std::tuple<std::string, std::string>> result;
  std::tuple<std::string, std::string> restup;
  std::get<0>(restup) = "Author";
  std::get<1>(restup) = std::string();
  result.push_back(restup);

  std::get<0>(restup) = "Book";
  std::get<1>(restup) = input.stem().u8string();
  result.push_back(restup);

  std::get<0>(restup) = "Series";
  std::get<1>(restup) = std::string();
  result.push_back(restup);

  std::get<0>(restup) = "Genre";
  std::get<1>(restup) = std::string();
  result.push_back(restup);

  std::get<0>(restup) = "Date";
  std::get<1>(restup) = std::string();
  result.push_back(restup);

  return result;
}
