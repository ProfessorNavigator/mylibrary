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

#include "AuxFunc.h"

AuxFunc::AuxFunc()
{
  // TODO Auto-generated constructor stub
}

AuxFunc::~AuxFunc()
{
  // TODO Auto-generated destructor stub
}

void
AuxFunc::homePath(std::string *filename)
{
  char *fnm = getenv("USERPROFILE");
  if(fnm)
    {
      *filename = std::string(getenv("USERPROFILE"));
    }
  else
    {
      fnm = getenv("HOMEDRIVE");
      if(fnm)
	{
	  *filename = std::string(getenv("HOMEDRIVE"));
	}
      else
	{
	  fnm = getenv("HOMEPATH");
	  if(fnm)
	    {
	      *filename = std::string(getenv("HOMEPATH"));
	    }
	  else
	    {
	      fnm = getenv("HOME");
	      if(fnm)
		{
		  *filename = std::string(getenv("HOME"));
		}
	      else
		{
		  fnm = getenv("SystemDrive");
		  if(fnm)
		    {
		      *filename = std::string(getenv("SystemDrive"));
		    }
		  else
		    {
		      std::cerr << "Cannot find user home folder" << std::endl;
		      exit(1);
		    }
		}
	    }
	}
    }
  toutf8(*filename);
}

void
AuxFunc::toutf8(std::string &line)
{
  const std::string::size_type srclen = line.size();
  std::vector<UChar> target(srclen);
  UErrorCode status = U_ZERO_ERROR;
  UConverter *conv = ucnv_open(NULL, &status);
  int32_t len = ucnv_toUChars(conv, target.data(), srclen, line.c_str(), srclen,
			      &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  icu::UnicodeString ustr(target.data(), len);
  line.clear();
  ustr.toUTF8String(line);
  ucnv_close(conv);
}

void
AuxFunc::toutf8(std::string &line, std::string conv_name)
{
  const std::string::size_type srclen = line.size();
  std::vector<UChar> target(srclen);
  UErrorCode status = U_ZERO_ERROR;
  UConverter *conv = ucnv_open(conv_name.c_str(), &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  status = U_ZERO_ERROR;
  int32_t len = ucnv_toUChars(conv, target.data(), srclen, line.c_str(), srclen,
			      &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  icu::UnicodeString ustr(target.data(), len);
  line.clear();
  ustr.toUTF8String(line);
  ucnv_close(conv);
}

std::string
AuxFunc::get_selfpath()
{
  std::filesystem::path p;
#ifdef __linux
  p = std::filesystem::u8path("/proc/self/exe");
  return std::filesystem::read_symlink(p).u8string();
#endif
#ifdef __WIN32
  char pth [MAX_PATH];
  GetModuleFileNameA(NULL, pth, MAX_PATH);
  p = std::filesystem::path(pth);
  return p.u8string();
#endif
}

int
AuxFunc::fileNames(std::string address,
		   std::vector<std::tuple<int, int, std::string>> &filenames)
{
  zip_t *z;

  std::string flname;
  int er = 0;
  int num;
  z = zip_open(address.c_str(), ZIP_RDONLY, &er);
  if(er < 1)
    {
      num = zip_get_num_files(z);

      for(int i = 0; i < num; i++)
	{
	  flname = zip_get_name(z, i, ZIP_FL_ENC_UTF_8);
	  struct zip_stat st;
	  zip_stat_index(z, i, ZIP_FL_ENC_GUESS, &st);
	  int sz = st.size;
	  std::tuple<int, int, std::string> tuple;
	  tuple = std::make_tuple(i, sz, flname); // @suppress("Invalid arguments")
	  filenames.push_back(tuple);
	}
      zip_close(z);
    }
  else
    {
      std::cerr << "Error on getting file names from archive: " << strerror(er)
	  << " " << address << std::endl;
    }

  return er;
}

int
AuxFunc::fileNamesNonZip(
    std::string address,
    std::vector<std::tuple<int, int, std::string>> &filenames)
{
  struct archive *a;
  struct archive_entry *entry;
  std::filesystem::path p = std::filesystem::u8path(address);
  int er = 0;
  a = archive_read_new();
  er = archive_read_support_filter_all(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "fileNamesNonZip: " << std::string(archive_error_string(a))
	  << " " << address << std::endl;
      archive_read_free(a);
      return er;
    }
  er = archive_read_support_format_all(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "fileNamesNonZip: " << std::string(archive_error_string(a))
	  << " " << address << std::endl;
      archive_read_free(a);
      return er;
    }

  er = archive_read_open_filename(a, p.string().c_str(), 1);
  if(er != ARCHIVE_OK)
    {
      std::cout << "fileNamesNonZip: " << std::string(archive_error_string(a))
	  << " " << address << std::endl;
      archive_read_free(a);
      return er;
    }
  else
    {
      int count = 0;
      for(;;)
	{
	  er = archive_read_next_header(a, &entry);
	  if(er == ARCHIVE_EOF)
	    {
	      break;
	    }
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "fileNamesNonZip: "
		  << std::string(archive_error_string(a)) << " " << address
		  << std::endl;
	    }
	  else
	    {
	      if(archive_entry_size(entry) > 0)
		{
		  std::tuple<int, int, std::string> restup;
		  std::get<0>(restup) = count;
		  std::get<1>(restup) = static_cast<int>(archive_entry_size(
		      entry));
		  std::get<2>(restup) = std::string(
		      archive_entry_pathname_utf8(entry));
		  filenames.push_back(restup);
		}
	    }
	  if(er == ARCHIVE_FATAL)
	    {
	      break;
	    }
	  count++;
	}
    }
  archive_read_free(a);
  er = 0;
  return er;
}

std::vector<std::tuple<std::string, std::string>>
AuxFunc::fileinfo(std::string address, int index)
{
  std::filesystem::path p = std::filesystem::u8path(address);
  std::vector<std::tuple<std::string, std::string>> result;
  if(p.extension().u8string() == ".zip")
    {
      zip_t *z;

      std::string flname;
      int er = 0;

      z = zip_open(address.c_str(), ZIP_RDONLY, &er);
      if(er < 1)
	{
	  struct zip_stat st;
	  zip_stat_index(z, index, ZIP_FL_ENC_GUESS, &st);
	  std::string key = "filename";
	  std::string value(st.name);
	  std::tuple<std::string, std::string> ttup;
	  std::get<0>(ttup) = key;
	  std::get<1>(ttup) = value;
	  result.push_back(ttup);

	  std::stringstream strm;
	  std::locale loc("C");
	  strm.imbue(loc);
	  strm << st.size;
	  key = "filesizeunc";
	  value = strm.str();
	  std::get<0>(ttup) = key;
	  std::get<1>(ttup) = value;
	  result.push_back(ttup);

	  strm.clear();
	  strm.str("");
	  strm.imbue(loc);
	  strm << st.comp_size;
	  key = "filesizec";
	  value = strm.str();
	  std::get<0>(ttup) = key;
	  std::get<1>(ttup) = value;
	  result.push_back(ttup);

	  zip_close(z);
	}
      else
	{
	  std::cerr << "Error on getting file info from archive: "
	      << strerror(er) << std::endl;
	}
    }
  else
    {
      struct archive *a;
      struct archive_entry *entry;
      int er;
      a = archive_read_new();
      er = archive_read_support_filter_all(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileinfo: " << std::string(archive_error_string(a))
	      << std::endl;
	  er = archive_read_free(a);
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "fileinfo: " << std::string(archive_error_string(a))
		  << std::endl;
	    }
	  return result;
	}
      er = archive_read_support_format_all(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileinfo: " << std::string(archive_error_string(a))
	      << std::endl;
	  er = archive_read_free(a);
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "fileinfo: " << std::string(archive_error_string(a))
		  << std::endl;
	    }
	  return result;
	}

      er = archive_read_open_filename(a, p.string().c_str(), 1);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileinfo: " << std::string(archive_error_string(a))
	      << std::endl;
	  er = archive_read_free(a);
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "fileinfo: " << std::string(archive_error_string(a))
		  << std::endl;
	    }
	  return result;
	}
      else
	{
	  int count = 0;
	  for(;;)
	    {
	      er = archive_read_next_header(a, &entry);
	      if(er == ARCHIVE_EOF)
		{
		  break;
		}
	      if(er != ARCHIVE_OK)
		{
		  std::cout << "fileinfo: "
		      << std::string(archive_error_string(a)) << std::endl;
		}
	      else
		{
		  if(count == index)
		    {
		      std::string key = "filename";
		      std::string value(archive_entry_pathname_utf8(entry));
		      std::tuple<std::string, std::string> ttup;
		      std::get<0>(ttup) = key;
		      std::get<1>(ttup) = value;
		      result.push_back(ttup);

		      std::stringstream strm;
		      std::locale loc("C");
		      strm.imbue(loc);
		      strm << archive_entry_size(entry);
		      key = "filesizeunc";
		      value = strm.str();
		      std::get<0>(ttup) = key;
		      std::get<1>(ttup) = value;
		      result.push_back(ttup);
		      break;
		    }
		}
	      if(er == ARCHIVE_FATAL)
		{
		  break;
		}
	      count++;
	    }
	}
      er = archive_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileinfo: " << std::string(archive_error_string(a))
	      << std::endl;
	  return result;
	}
    }

  return result;
}

int
AuxFunc::removeFmArch(std::string archpath, uint64_t index)
{
  std::filesystem::path p = std::filesystem::u8path(archpath);
  int result = -1;
  if(p.extension().u8string() == ".zip")
    {
      zip_t *z;
      int er = 0;
      z = zip_open(archpath.c_str(), 0, &er);
      if(er < 1)
	{
	  result = zip_delete(z, index);
	  zip_close(z);
	}
      else
	{
	  std::cerr << "Error on getting file names from archive: "
	      << strerror(er) << std::endl;
	}
    }
  else
    {
      std::string ext = p.u8string();
      std::string::size_type n;
      n = ext.find(".tar.gz");
      if(n != std::string::npos)
	{
	  ext = ".tar.gz";
	}
      else
	{
	  n = ext.find(".tar.bz2");
	  if(n != std::string::npos)
	    {
	      ext = ".tar.bz2";
	    }
	  else
	    {
	      n = ext.find(".tar.xz");
	      if(n != std::string::npos)
		{
		  ext = ".tar.xz";
		}
	      else
		{
		  ext = p.extension().u8string();
		}
	    }
	}
      archive *a_r = archive_read_new();
      archive *a_w = archive_write_new();
      archive_entry *entry;
      int er = archive_read_support_filter_all(a_r);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "removeFmArch: "
	      << std::string(archive_error_string(a_r)) << std::endl;
	  archive_read_free(a_r);
	  archive_write_free(a_w);
	  return result;
	}
      er = archive_write_set_format_filter_by_ext(a_w, ext.c_str());
      if(er != ARCHIVE_OK)
	{
	  std::cout << "removeFmArch: "
	      << std::string(archive_error_string(a_w)) << std::endl;
	  archive_read_free(a_r);
	  archive_write_free(a_w);
	  return result;
	}
      er = archive_read_support_format_all(a_r);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "removeFmArch: "
	      << std::string(archive_error_string(a_r)) << std::endl;
	  archive_read_free(a_r);
	  archive_write_free(a_w);
	  return result;
	}

      er = archive_read_open_filename(a_r, p.string().c_str(), 1);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "removeFmArch: "
	      << std::string(archive_error_string(a_r)) << std::endl;
	  archive_read_free(a_r);
	  archive_write_free(a_w);
	  return result;
	}
      else
	{
	  std::string fnm = p.parent_path().u8string();
	  fnm = fnm + "/" + randomFileName();
	  std::filesystem::path tmp_arch = std::filesystem::u8path(fnm);
	  bool writed = false;
	  er = archive_write_open_filename(a_w, tmp_arch.string().c_str());
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "removeFmArch: "
		  << std::string(archive_error_string(a_w)) << std::endl;
	      archive_read_free(a_r);
	      archive_write_free(a_w);
	      return result;
	    }
	  else
	    {
	      uint64_t count = 0;
	      for(;;)
		{
		  er = archive_read_next_header(a_r, &entry);
		  if(er == ARCHIVE_EOF)
		    {
		      break;
		    }
		  if(er != ARCHIVE_OK)
		    {
		      std::cout << "removeFmArch: "
			  << std::string(archive_error_string(a_r))
			  << std::endl;
		    }
		  else
		    {
		      if(er == ARCHIVE_FATAL)
			{
			  break;
			}
		    }
		  if(count != index)
		    {
		      er = archive_write_header(a_w, entry);
		      if(er != ARCHIVE_OK)
			{
			  if(er == ARCHIVE_RETRY)
			    {
			      er = archive_write_header(a_w, entry);
			    }
			  if(er == ARCHIVE_FATAL)
			    {
			      break;
			    }
			  if(er == ARCHIVE_WARN)
			    {
			      std::cout << "removeFmArch: "
				  << std::string(archive_error_string(a_w))
				  << std::endl;
			    }
			}
		      for(;;)
			{
			  std::vector<char> buf;
			  buf.resize(50);
			  size_t rb = archive_read_data(a_r, &buf[0],
							buf.size());
			  if(rb > 0)
			    {
			      buf.resize(rb);
			      ssize_t wb = archive_write_data(a_w, &buf[0],
							      buf.size());
			      if(wb > 0)
				{
				  writed = true;
				}
			      else if(wb < 0)
				{
				  archive_read_free(a_r);
				  archive_write_free(a_w);
				  return result;
				}
			      while(wb != static_cast<ssize_t>(buf.size()))
				{
				  buf.erase(buf.begin(), buf.begin() + wb);
				  wb = archive_write_data(a_w, &buf[0],
							  buf.size());
				}
			    }
			  else
			    {
			      break;
			    }
			}
		    }
		  count++;
		}
	    }
	  archive_read_free(a_r);
	  archive_write_free(a_w);
	  std::filesystem::remove_all(p);
	  if(writed)
	    {
	      std::filesystem::rename(tmp_arch, p);
	    }
	  else
	    {
	      std::filesystem::remove_all(tmp_arch);
	    }
	}
      result = 0;
    }

  return result;
}

std::string
AuxFunc::randomFileName()
{
  std::string result;
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  uint64_t t = duration.count();
  std::srand(t);
  int rnd = std::rand();
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << std::hex << rnd;
  result = strm.str() + "mylibrary";
  return result;
}

void
AuxFunc::unpackByIndex(std::string archaddress, std::string outfolder,
		       int index)
{
  zip_t *z;
  zip_file_t *file;
  zip_stat_t st;
  int er = 0;
  z = zip_open(archaddress.c_str(), ZIP_RDONLY, &er);
  if(er < 1)
    {

      std::filesystem::path path = std::filesystem::u8path(outfolder);
      if(!std::filesystem::exists(path))
	{
	  std::filesystem::create_directories(path);
	}
      file = zip_fopen_index(z, index, ZIP_FL_ENC_UTF_8);
      zip_stat_index(z, index, ZIP_STAT_NAME, &st);
      std::vector<char> content;
      content.resize(104857600);
      std::fstream f;
      std::string fname = path.u8string();
      fname = fname + "/";
      std::string auxstr = std::string(st.name);
      path = std::filesystem::u8path(auxstr);
      auxstr = path.filename().u8string();
      fname = fname + auxstr;
      path = std::filesystem::u8path(fname);
      f.open(path, std::ios_base::out | std::ios_base::binary);
      for(;;)
	{
	  zip_uint64_t ch = zip_fread(file, &content[0], content.size());
	  if(ch <= 0)
	    {
	      if(ch < 0)
		{
		  std::cerr << "unpackByIndex file  reading error" << std::endl;
		}
	      break;
	    }
	  else
	    {
	      if(ch < static_cast<uint64_t>(content.size()))
		{
		  content.resize(static_cast<size_t>(ch));
		}
	    }
	  f.write(&content[0], content.size());
	}
      f.close();
      zip_fclose(file);
      zip_close(z);
    }
  else
    {
      std::cout << "Unpack by index error: " << strerror(er) << std::endl;
    }
}

std::string
AuxFunc::unpackByIndex(std::string archaddress, int index, size_t filesz)
{
  zip_t *z;
  zip_file_t *file;
  zip_stat_t st;
  int er = 0;
  std::string result;
  result.resize(filesz);
  z = zip_open(archaddress.c_str(), ZIP_RDONLY, &er);
  if(er < 1)
    {
      file = zip_fopen_index(z, index, ZIP_FL_ENC_UTF_8);
      zip_stat_index(z, index, ZIP_STAT_NAME, &st);
      zip_uint64_t ch = zip_fread(file, &result[0], result.size());
      if(ch <= 0)
	{
	  if(ch < 0)
	    {
	      std::cerr << "unpackByIndex file  reading error" << std::endl;
	    }
	}
      zip_fclose(file);
      zip_close(z);
    }
  else
    {
      std::cout << "Unpack by index error: " << strerror(er) << std::endl;
    }
  return result;
}

void
AuxFunc::unpackByIndexNonZip(std::string archaddress, std::string outfolder,
			     int index)
{
  struct archive *a;
  struct archive_entry *entry;
  std::filesystem::path p = std::filesystem::u8path(archaddress);
  int er;
  a = archive_read_new();
  er = archive_read_support_filter_all(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "unpackByIndexNonZip(void): "
	  << std::string(archive_error_string(a)) << std::endl;
      er = archive_read_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileNamesNonZip: "
	      << std::string(archive_error_string(a)) << std::endl;
	}
      return void();
    }
  er = archive_read_support_format_all(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "unpackByIndexNonZip(void): "
	  << std::string(archive_error_string(a)) << std::endl;
      er = archive_read_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileNamesNonZip: "
	      << std::string(archive_error_string(a)) << std::endl;
	}
      return void();
    }

  er = archive_read_open_filename(a, p.string().c_str(), 1);
  if(er != ARCHIVE_OK)
    {
      std::cout << "unpackByIndexNonZip(void): "
	  << std::string(archive_error_string(a)) << std::endl;
      er = archive_read_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileNamesNonZip: "
	      << std::string(archive_error_string(a)) << std::endl;
	}
      return void();
    }
  else
    {
      int count = 0;
      for(;;)
	{
	  er = archive_read_next_header(a, &entry);
	  if(er == ARCHIVE_EOF)
	    {
	      break;
	    }
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "unpackByIndexNonZip(void): "
		  << std::string(archive_error_string(a)) << std::endl;
	    }
	  else
	    {
	      if(count == index)
		{
		  std::filesystem::path finarch = std::filesystem::u8path(
		      std::string(archive_entry_pathname_utf8(entry)));
		  std::filesystem::path outpath = std::filesystem::u8path(
		      outfolder + "/" + finarch.filename().u8string());
		  if(!std::filesystem::exists(outpath.parent_path()))
		    {
		      std::filesystem::create_directories(
			  outpath.parent_path());
		    }
		  std::fstream f;
		  f.open(
		      outpath,
		      std::ios_base::out | std::ios_base::app
			  | std::ios_base::binary);
		  if(f.is_open())
		    {
		      for(;;)
			{
			  std::vector<char> buf;
			  buf.resize(50);
			  size_t rb = archive_read_data(a, &buf[0], buf.size());
			  if(rb > 0)
			    {
			      buf.resize(rb);
			      f.write(buf.data(), buf.size());
			    }
			  else
			    {
			      break;
			    }
			}
		      f.close();
		    }
		  break;
		}
	    }
	  if(er == ARCHIVE_FATAL)
	    {
	      break;
	    }
	  count++;
	}
    }
  er = archive_read_free(a);
  if(er != ARCHIVE_OK)
    {
      std::cerr << "unpackByIndexNonZip(void): "
	  << std::string(archive_error_string(a)) << std::endl;
      return void();
    }
}

std::string
AuxFunc::unpackByIndexNonZipStr(std::string archaddress, int index)
{
  std::string result;
  struct archive *a;
  struct archive_entry *entry;
  std::filesystem::path p = std::filesystem::u8path(archaddress);
  int er;
  a = archive_read_new();
  er = archive_read_support_filter_all(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "unpackByIndexNonZipStr: "
	  << std::string(archive_error_string(a)) << std::endl;
      er = archive_read_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileNamesNonZip: "
	      << std::string(archive_error_string(a)) << std::endl;
	}
      return result;
    }
  er = archive_read_support_format_all(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "unpackByIndexNonZipStr: "
	  << std::string(archive_error_string(a)) << std::endl;
      er = archive_read_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileNamesNonZip: "
	      << std::string(archive_error_string(a)) << std::endl;
	}
      return result;
    }

  er = archive_read_open_filename(a, p.string().c_str(), 1);
  if(er != ARCHIVE_OK)
    {
      std::cout << "unpackByIndexNonZipStr: "
	  << std::string(archive_error_string(a)) << std::endl;
      er = archive_read_free(a);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "fileNamesNonZip: "
	      << std::string(archive_error_string(a)) << std::endl;
	}
      return result;
    }
  else
    {
      int count = 0;
      for(;;)
	{
	  er = archive_read_next_header(a, &entry);
	  if(er == ARCHIVE_EOF)
	    {
	      break;
	    }
	  if(er != ARCHIVE_OK)
	    {
	      std::cout << "unpackByIndexNonZipStr: "
		  << std::string(archive_error_string(a)) << std::endl;
	    }
	  else
	    {
	      if(count == index)
		{
		  for(;;)
		    {
		      std::vector<char> buf;
		      buf.resize(50);
		      size_t rb = archive_read_data(a, &buf[0], buf.size());
		      if(rb > 0)
			{
			  buf.resize(rb);
			  std::copy(buf.begin(), buf.end(),
				    std::back_inserter(result));
			}
		      else
			{
			  break;
			}
		    }
		  break;
		}
	    }
	  if(er == ARCHIVE_FATAL)
	    {
	      break;
	    }
	  count++;
	}
    }
  er = archive_read_free(a);
  if(er != ARCHIVE_OK)
    {
      std::cerr << "unpackByIndexNonZipStr: "
	  << std::string(archive_error_string(a)) << std::endl;
      return result;
    }
  return result;
}

int
AuxFunc::packing(std::string source, std::string out)
{
  int result = 0;
  int er = 0;
  std::filesystem::path dir;
  dir = std::filesystem::u8path(source);
  source = dir.generic_u8string();
  if(std::filesystem::exists(dir))
    {
      if(std::filesystem::is_directory(dir))
	{
	  zip_t *z;
	  zip_error_t err;
	  z = zip_open(out.c_str(), ZIP_TRUNCATE | ZIP_CREATE, &er);
	  if(er < 1)
	    {
	      std::vector<std::filesystem::path> listf;
	      std::vector<std::filesystem::path> listd;
	      std::string line;
	      if(!std::filesystem::is_empty(dir))
		{
		  for(auto &iter : std::filesystem::recursive_directory_iterator(
		      dir))
		    {
		      std::filesystem::path path = iter.path();
		      path = std::filesystem::u8path(path.generic_u8string());
		      if(std::filesystem::is_directory(path))
			{
			  listd.push_back(path);
			}
		      else
			{
			  listf.push_back(path);
			}
		    }
		  std::sort(listd.begin(), listd.end(), []
		  (auto &el1, auto &el2)
		    {
		      return el1.string().size() < el2.string().size();
		    });

		  std::string pardir = dir.filename().u8string();
		  zip_dir_add(z, pardir.c_str(), ZIP_FL_ENC_UTF_8);

		  for(size_t i = 0; i < listd.size(); i++)
		    {
		      line = listd[i].u8string();
		      std::string::size_type n;
		      n = line.find(source, 0);
		      line.erase(n, source.size());
		      line = pardir + line;
		      if(!std::filesystem::is_empty(listd[i]))
			{
			  zip_dir_add(z, line.c_str(), ZIP_FL_ENC_UTF_8);
			}
		    }
		  for(size_t i = 0; i < listf.size(); i++)
		    {
		      line = listf[i].u8string();
		      std::string::size_type n;
		      n = line.find(source, 0);
		      line.erase(n, source.size());
		      zip_source_t *zsource;
		      zsource = zip_source_file_create(
			  listf[i].u8string().c_str(), 0, 0, &err);
		      line = pardir + line;
		      zip_file_add(z, line.c_str(), zsource,
		      ZIP_FL_ENC_UTF_8);
		    }
		}

	      zip_close(z);
	      result = 1;
	    }
	  else
	    {
	      std::cerr << "Error on packaing: " << strerror(er) << std::endl;
	      result = -2;
	    }
	}
      else
	{
	  zip_t *z;
	  zip_error_t err;
	  std::string line = dir.filename().u8string();
	  z = zip_open(out.c_str(), ZIP_TRUNCATE | ZIP_CREATE, &er);
	  if(er >= 1)
	    {
	      std::cerr << "Packing (file) error: " << strerror(er)
		  << std::endl;
	      result = -3;
	    }
	  else
	    {
	      zip_source_t *zsource;
	      zsource = zip_source_file_create(source.c_str(), 0, 0, &err);
	      if(zsource == nullptr)
		{
		  std::cerr << "Error on open file while packing" << std::endl;
		  result = -4;
		}
	      else
		{
		  zip_file_add(z, line.c_str(), zsource, ZIP_FL_ENC_UTF_8);
		}
	      zip_close(z);
	      result = 1;
	    }
	}
    }
  else
    {
      std::cerr << "Source file for packing does not exists!" << std::endl;
      result = -1;
    }
  return result;
}

int
AuxFunc::packingNonZip(std::string source, std::string out,
		       std::string extension)
{
  std::filesystem::path source_p = std::filesystem::u8path(source);
  std::filesystem::path tmp_arch = std::filesystem::u8path(out);
  archive *a = archive_write_new();
  archive_entry *entry;
  std::string ext = utf8to(extension);
  int er;
  er = archive_write_set_format_filter_by_ext(a, ext.c_str());
  if(er != ARCHIVE_OK)
    {
      std::cout << "packingNonZip: " << std::string(archive_error_string(a))
	  << std::endl;
      archive_write_free(a);
      return er;
    }

  er = archive_write_open_filename(a, tmp_arch.string().c_str());
  if(er != ARCHIVE_OK)
    {
      std::cout << "packingNonZip: " << std::string(archive_error_string(a))
	  << std::endl;
      archive_write_free(a);
      return er;
    }
  else
    {
      entry = archive_entry_new();
      archive_entry_set_pathname_utf8(entry,
				      source_p.filename().u8string().c_str());
      int64_t sz = static_cast<int64_t>(std::filesystem::file_size(source_p));
      archive_entry_set_size(entry, sz);
      archive_entry_set_filetype(entry, AE_IFREG);
      er = archive_write_header(a, entry);
      if(er != ARCHIVE_OK)
	{
	  std::cout << "packingNonZip: " << std::string(archive_error_string(a))
	      << std::endl;
	  archive_entry_free(entry);
	  archive_write_free(a);
	  return er;
	}
      std::vector<char> buf;
      size_t byteread = 0;
      size_t fsz = static_cast<size_t>(sz);
      std::fstream f;
      f.open(source_p, std::ios_base::in | std::ios_base::binary);
      if(f.is_open())
	{
	  while(byteread < fsz)
	    {
	      size_t ch = fsz - byteread;
	      buf.clear();
	      if(ch > 1048576)
		{
		  buf.resize(1048576);
		}
	      else
		{
		  buf.resize(ch);
		}
	      ch = buf.size();
	      f.read(&buf[0], buf.size());
	      ssize_t wb = archive_write_data(a, &buf[0], buf.size());
	      if(wb < 0)
		{
		  f.close();
		  archive_entry_free(entry);
		  archive_write_free(a);
		  return -1;
		}
	      else
		{
		  while(wb != static_cast<ssize_t>(buf.size()))
		    {
		      buf.erase(buf.begin(), buf.begin() + wb);
		      wb = archive_write_data(a, &buf[0], buf.size());
		      if(wb < 0)
			{
			  f.close();
			  archive_entry_free(entry);
			  archive_write_free(a);
			  return -1;
			}
		    }
		}
	      byteread = byteread + ch;
	    }
	  f.close();
	}
      else
	{
	  std::cout << "packingNonZip: " << "source file not opened"
	      << std::endl;
	}
      archive_entry_free(entry);
    }
  er = archive_write_close(a);
  if(er != ARCHIVE_OK)
    {
      std::cout << "packingNonZip: " << std::string(archive_error_string(a))
	  << std::endl;
    }
  archive_write_free(a);
  return er;
}

void
AuxFunc::stringToLower(std::string &line)
{
  std::string innerline = line;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(innerline.c_str());
  ustr.toLower();
  line.clear();
  ustr.toUTF8String(line);
}

std::vector<char>
AuxFunc::filehash(std::filesystem::path filepath, int *cancel)
{
  std::vector<char> result;
  if(std::filesystem::exists(filepath))
    {
      uintmax_t fsz = std::filesystem::file_size(filepath);
      std::fstream f;
      std::vector<char> F;
      uintmax_t readb = 0;
      gcry_error_t err;
      gcry_md_hd_t hd;
      err = gcry_md_open(&hd, GCRY_MD_BLAKE2B_256, 0);
      if(err != 0)
	{
	  if(gcry_md_is_enabled(hd, GCRY_MD_BLAKE2B_256))
	    {
	      std::cout << "Enabled" << std::endl;
	    }
	  else
	    {
	      std::cout << "Not enabled" << std::endl;
	    }
	  std::string errstr;
	  errstr.resize(1024);
	  gpg_strerror_r(err, errstr.data(), errstr.size());
	  errstr.erase(std::remove_if(errstr.begin(), errstr.end(), []
	  (auto &el)
	    {
	      if(el)
		{
		  return false;
		}
	      else
		{
		  return true;
		}
	    }),
		       errstr.end());
	  if(!errstr.empty())
	    {
	      std::cerr << "Hash error1: " << err << "(" << errstr << ") on "
		  << filepath.u8string() << std::endl;
	    }
	  else
	    {

	      std::cerr << "Hash error1: " << err << " on "
		  << filepath.u8string() << std::endl;
	    }

	}
      else
	{
	  f.open(filepath, std::ios_base::in | std::ios_base::binary);
	  if(f.is_open())
	    {
	      for(;;)
		{
		  F.clear();
		  if(readb + 104857600 < fsz)
		    {
		      F.resize(104857600);
		      f.read(&F[0], 104857600);
		      readb = readb + 104857600;
		      gcry_md_write(hd, &F[0], F.size());
		    }
		  else
		    {
		      int left = fsz - readb;
		      F.resize(left);
		      f.read(&F[0], F.size());
		      readb = readb + left;
		      gcry_md_write(hd, &F[0], F.size());
		    }
		  if(readb >= fsz || *cancel == 1)
		    {
		      break;
		    }
		}
	      f.close();

	      size_t len = gcry_md_get_algo_dlen(GCRY_MD_BLAKE2B_256);
	      char *buf = reinterpret_cast<char*>(gcry_md_read(
		  hd, GCRY_MD_BLAKE2B_256));
	      result.insert(result.begin(), buf, buf + len);
	    }
	}

      gcry_md_close(hd);
      return result;
    }
  else
    {
      std::cerr << "Hash error1: File for hashing not exists" << std::endl;
      std::vector<char> result
	{ 'e', 'r', 'r', 'o', 'r' };
      return result;
    }
}

std::vector<char>
AuxFunc::filehash(std::filesystem::path filepath, std::function<void
(uint64_t)> progress,
		  int *cancel)
{
  std::vector<char> result;
  if(std::filesystem::exists(filepath))
    {
      uintmax_t fsz = std::filesystem::file_size(filepath);
      std::fstream f;
      std::vector<char> F;
      uintmax_t readb = 0;
      gcry_error_t err;
      gcry_md_hd_t hd;
      err = gcry_md_open(&hd, GCRY_MD_BLAKE2B_256, 0);
      if(err != 0)
	{
	  std::string errstr;
	  errstr.resize(1024);
	  gpg_strerror_r(err, errstr.data(), errstr.size());
	  errstr.erase(std::remove_if(errstr.begin(), errstr.end(), []
	  (auto &el)
	    {
	      if(el)
		{
		  return false;
		}
	      else
		{
		  return true;
		}
	    }),
		       errstr.end());
	  if(!errstr.empty())
	    {
	      std::cerr << "Hash error2: " << errstr << " on "
		  << filepath.u8string() << std::endl;
	    }
	  else
	    {
	      std::cerr << "Hash error2: " << err << " on "
		  << filepath.u8string() << std::endl;
	    }
	  if(progress)
	    {
	      progress(static_cast<uint64_t>(fsz));
	    }
	}
      else
	{
	  f.open(filepath, std::ios_base::in | std::ios_base::binary);
	  for(;;)
	    {
	      F.clear();
	      if(readb + 104857600 < fsz)
		{
		  F.resize(104857600);
		  f.read(&F[0], 104857600);
		  readb = readb + 104857600;
		  gcry_md_write(hd, &F[0], F.size());
		}
	      else
		{
		  int left = fsz - readb;
		  F.resize(left);
		  f.read(&F[0], left);
		  readb = readb + left;
		  gcry_md_write(hd, &F[0], F.size());
		}
	      if(progress)
		{
		  progress(static_cast<uint64_t>(F.size()));
		}
	      if(readb >= fsz || *cancel == 1)
		{
		  break;
		}
	    }
	  f.close();
	  size_t len = gcry_md_get_algo_dlen(GCRY_MD_BLAKE2B_256);
	  char *buf = reinterpret_cast<char*>(gcry_md_read(hd,
							   GCRY_MD_BLAKE2B_256));
	  result.insert(result.begin(), buf, buf + len);
	}

      gcry_md_close(hd);
      return result;
    }
  else
    {
      std::cerr << "Hash error2: File for hashing not exists" << std::endl;
      std::vector<char> result
	{ 'e', 'r', 'r', 'o', 'r' };
      return result;
    }
}

std::string
AuxFunc::to_hex(std::vector<char> *source)
{
  std::vector<char> locsource = *source;
  std::string result;
  size_t count = 0;
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  while(count < locsource.size())
    {
      uint64_t a;
      if(count + sizeof(a) <= locsource.size())
	{
	  std::memcpy(&a, &locsource[count], sizeof(a));
	}
      else
	{
	  std::vector<char> tmp;
	  tmp.resize(locsource.size() - count);
	  std::memcpy(&tmp[0], &locsource[count], locsource.size() - count);
	  while(tmp.size() < sizeof(a))
	    {
	      tmp.push_back('0');
	    }
	  std::memcpy(&a, &tmp[0], sizeof(a));
	}
      strm << std::hex << a;
      count = count + sizeof(a);
    }
  result = strm.str();
  return result;
}

std::string
AuxFunc::utf8to(std::string line)
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr;
  UConverter *c = ucnv_open(NULL, &status);
  if(!U_SUCCESS(status))
    {
      std::cerr << u_errorName(status) << std::endl;
    }
  status = U_ZERO_ERROR;
  std::vector<char> target2;
  ustr.remove();
  ustr = icu::UnicodeString::fromUTF8(line.c_str());
  target2.resize(ustr.length());
  char16_t data[ustr.length()];
  for(int i = 0; i < ustr.length(); i++)
    {
      data[i] = ustr.charAt(i);
    }
  size_t cb = ucnv_fromUChars(c, target2.data(), ustr.length(), data,
			      ustr.length(), &status);
  if(!U_SUCCESS(status))
    {
      if(status == U_BUFFER_OVERFLOW_ERROR)
	{
	  status = U_ZERO_ERROR;
	  target2.clear();
	  target2.resize(cb);
	  ucnv_fromUChars(c, target2.data(), cb, data, ustr.length(), &status);
	  if(!U_SUCCESS(status))
	    {
	      std::cerr << u_errorName(status) << std::endl;
	    }
	}
      else
	{
	  std::cerr << u_errorName(status) << std::endl;
	}
    }
  line.clear();
  line = std::string(target2.begin(), target2.end());
  ucnv_close(c);

  return line;
}
