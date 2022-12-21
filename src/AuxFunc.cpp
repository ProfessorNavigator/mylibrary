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

#include "AuxFunc.h"

AuxFunc::AuxFunc()
{
  //ctor
}

AuxFunc::~AuxFunc()
{
  //dtor
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
AuxFunc::fileNames(std::string adress,
		   std::vector<std::tuple<int, int, std::string>> &filenames)
{
  zip_t *z;

  std::string flname;
  int er = 0;
  int num;
  z = zip_open(adress.c_str(), ZIP_RDONLY, &er);
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
	  tuple = std::make_tuple(i, sz, flname);
	  filenames.push_back(tuple);
	}
      zip_close(z);
    }
  else
    {
      std::cerr << "Error on getting file names from archive: " << strerror(er)
	  << std::endl;
    }

  return er;
}

int
AuxFunc::removeFmArch(std::string archpath, uint64_t index)
{
  int result = -1;
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
      std::cerr << "Error on getting file names from archive: " << strerror(er)
	  << std::endl;
    }

  return result;
}

std::string
AuxFunc::randomFileName()
{
  std::string result;
  int rnd = std::rand();
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << std::hex << rnd;
  result = strm.str() + "mylibrary";
  return result;
}

void
AuxFunc::unpackByIndex(std::string archadress, std::string outfolder, int index)
{
  zip_t *z;
  zip_file_t *file;
  zip_stat_t st;
  int er = 0;
  z = zip_open(archadress.c_str(), ZIP_RDONLY, &er);
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
AuxFunc::unpackByIndex(std::string archadress, int index, size_t filesz)
{
  zip_t *z;
  zip_file_t *file;
  zip_stat_t st;
  int er = 0;
  std::string result;
  result.resize(filesz);
  z = zip_open(archadress.c_str(), ZIP_RDONLY, &er);
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
AuxFunc::stringToLower(std::string &line)
{
  std::string innerline = line;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(innerline.c_str());
  ustr.toLower();
  line.clear();
  ustr.toUTF8String(line);
}

std::vector<char>
AuxFunc::filehash(std::filesystem::path filepath)
{
  if(!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
    {
      gcry_check_version(NULL);
      gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    }
  if(std::filesystem::exists(filepath))
    {
      uintmax_t fsz = std::filesystem::file_size(filepath);
      std::fstream f;
      std::vector<char> F;
      uintmax_t readb = 0;
      gcry_error_t err;
      gcry_md_hd_t hd;
      err = gcry_md_open(&hd, GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE);
      if(err != 0)
	{
	  std::cerr << gcry_strerror(err) << std::endl;
	}
      f.open(filepath, std::ios_base::in | std::ios_base::binary);
      for(;;)
	{
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
	  if(readb >= fsz)
	    {
	      break;
	    }
	}
      f.close();
      size_t len = gcry_md_get_algo_dlen(GCRY_MD_SHA256);
      char *buf = reinterpret_cast<char*>(gcry_md_read(hd, GCRY_MD_SHA256));
      std::vector<char> result;
      result.insert(result.begin(), buf, buf + len);
      gcry_md_close(hd);
      return result;
    }
  else
    {
      std::cerr << "File for hashing not exists" << std::endl;
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
  while (count < locsource.size())
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
	  while (tmp.size() < sizeof(a))
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
