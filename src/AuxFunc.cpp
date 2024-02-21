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

#include <AuxFunc.h>
#include <gpg-error.h>
#include <MLException.h>
#include <stddef.h>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <unicode/urename.h>
#include <unicode/utypes.h>
#include <unicode/uversion.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <memory>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#endif

AuxFunc::AuxFunc()
{

}

AuxFunc::~AuxFunc()
{

}

std::string
AuxFunc::to_utf_8(const std::string &input, const char *conv_name)
{
  std::string result;
  int32_t srclen = static_cast<int32_t>(input.size());
  UChar *buf = new UChar[srclen];
  std::shared_ptr<UChar> resbuf(buf, []
  (UChar *arr)
    {
      delete[] arr;
    });
  UErrorCode status = U_ZERO_ERROR;
  std::shared_ptr<UConverter> conv(ucnv_open(conv_name, &status), []
  (UConverter *c)
    {
      ucnv_close(c);
    });
  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::to_utf_8 converter " << conv_name
	  << " open error: " << u_errorName(status) << std::endl;
      result = input;
      return result;
    }

  uint64_t len;
  for(;;)
    {
      status = U_ZERO_ERROR;
      len = ucnv_toUChars(conv.get(), resbuf.get(), srclen, input.c_str(),
			  static_cast<int32_t>(input.size()), &status);
      if(status != U_BUFFER_OVERFLOW_ERROR)
	{
	  break;
	}
      else
	{
	  int32_t newsrclen = srclen * 1.2;
	  buf = new UChar[newsrclen];
	  resbuf = std::shared_ptr<UChar>(buf, []
	  (UChar *arr)
	    {
	      delete[] arr;
	    });
	  srclen = newsrclen;
	}
    }
  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::to_utf_8 error: " << u_errorName(status)
	  << std::endl;
      result = input;
      return result;
    }
  else
    {
      icu::UnicodeString ustr(resbuf.get(), len);
      ustr.toUTF8String(result);
    }

  return result;
}

std::filesystem::path
AuxFunc::homePath()
{
  std::string result;
  char *fnm = getenv("USERPROFILE");
  if(fnm)
    {
      result = fnm;
    }
  else
    {
      fnm = getenv("HOMEDRIVE");
      if(fnm)
	{
	  result = fnm;
	}
      else
	{
	  fnm = getenv("HOMEPATH");
	  if(fnm)
	    {
	      result = fnm;
	    }
	  else
	    {
	      fnm = getenv("HOME");
	      if(fnm)
		{
		  result = fnm;
		}
	      else
		{
		  fnm = getenv("SystemDrive");
		  if(fnm)
		    {
		      result = fnm;
		    }
		  else
		    {
		      std::cout << "MyLibrary: cannot find user home directory"
			  << std::endl;
		      exit(1);
		    }
		}
	    }
	}
    }
  std::string path = to_utf_8(result, nullptr);
  std::string::size_type n = 0;
  for(;;)
    {
      n = path.find("\\", n);
      if(n != std::string::npos)
	{
	  path.erase(path.begin() + n, path.begin() + n + 1);
	  path.insert(n, "/");
	}
      else
	{
	  break;
	}
    }
  return std::filesystem::u8path(path);
}

std::filesystem::path
AuxFunc::get_selfpath()
{
  std::filesystem::path p;
#ifdef __linux
  p = std::filesystem::u8path("/proc/self/exe");
  return std::filesystem::read_symlink(p);
#endif
#ifdef __WIN32
  char pth [MAX_PATH];
  GetModuleFileNameA(NULL, pth, MAX_PATH);
  p = std::filesystem::path(pth);
  return p;
#endif
}

std::filesystem::path
AuxFunc::temp_path()
{
#ifdef __linux
  return std::filesystem::temp_directory_path();
#endif
#ifdef _WIN32
  return std::filesystem::temp_directory_path().parent_path();
#endif
}

std::filesystem::path
AuxFunc::share_path()
{
  std::filesystem::path result = get_selfpath();
  result = result.parent_path();
  result += std::filesystem::u8path("/../share/MyLibrary");
  std::error_code ec;
  result = std::filesystem::canonical(result, ec);
  if(ec)
    {
      std::cout << "AuxFunc::share_path error: " << ec.message() << std::endl;
    }

  return result;
}

std::vector<GenreGroup>
AuxFunc::get_genre_list()
{
  std::vector<GenreGroup> result;
  std::string locname;
  bool wrong_loc = false;
  try
    {
      locname = setlocale(LC_CTYPE, nullptr);
    }
  catch(std::exception &e)
    {
      std::cout << "AuxFunc::get_genre_list get locale: " << e.what()
	  << std::endl;
      wrong_loc = true;
    }
  std::string::size_type n;
  n = locname.find(".");
  if(n != std::string::npos)
    {
      locname = locname.substr(0, n);
    }

  result = read_genre_groups(wrong_loc, locname);

  std::vector<std::tuple<std::string, Genre>> gv = read_genres(wrong_loc,
							       locname);

  for(auto it = gv.begin(); it != gv.end(); it++)
    {
      std::string g_code = std::get<0>(*it);
      auto itr = std::find_if(result.begin(), result.end(), [g_code]
      (auto &el)
	{
	  return el.group_code == g_code;
	});
      if(itr != result.end())
	{
	  itr->genres.push_back(std::get<1>(*it));
	}
    }

  return result;
}

std::vector<std::tuple<std::string, Genre>>
AuxFunc::read_genres(const bool &wrong_loc, const std::string &locname)
{
  std::vector<std::tuple<std::string, Genre>> result;
  std::filesystem::path genre_path = share_path();
  genre_path /= std::filesystem::u8path("genres.csv");
  std::fstream f;
  f.open(genre_path, std::ios_base::in);
  if(f.is_open())
    {
      std::string line;
      std::string::size_type n;
      getline(f, line);
      int count = 0;
      if(!line.empty())
	{
	  if(!wrong_loc)
	    {
	      for(;;)
		{
		  n = line.find(";");
		  if(n != std::string::npos)
		    {
		      std::string locl = line.substr(0, n);
		      line.erase(0, n + std::string(";").size());
		      n = locl.find(locname);
		      if(n != std::string::npos)
			{
			  break;
			}
		    }
		  else
		    {
		      n = line.find(locname);
		      if(n == std::string::npos)
			{
			  count = 2;
			}
		      break;
		    }
		  count++;
		}
	    }
	  else
	    {
	      count = 2;
	    }
	  while(!f.eof())
	    {
	      line.clear();
	      getline(f, line);
	      if(!line.empty())
		{
		  int fcount = 0;
		  std::tuple<std::string, Genre> gen;
		  for(;;)
		    {
		      n = line.find(";");
		      if(n != std::string::npos)
			{
			  std::string locl = line.substr(0, n);
			  line.erase(0, n + std::string(";").size());
			  if(fcount == 0)
			    {
			      std::get<1>(gen).genre_code = locl;
			    }
			  else
			    {
			      if(fcount == 1)
				{
				  std::get<0>(gen) = locl;
				}
			      else if(fcount == count)
				{
				  std::get<1>(gen).genre_name = locl;
				  break;
				}
			    }
			}
		      else
			{
			  if(fcount == count)
			    {
			      std::get<1>(gen).genre_name = line;
			    }
			  break;
			}
		      fcount++;
		    }
		  if(!std::get<1>(gen).genre_code.empty())
		    {
		      result.push_back(gen);
		    }
		}
	    }
	}
      f.close();
    }

  return result;
}

std::string
AuxFunc::time_t_to_date(const time_t &tt)
{
  std::tm *ltm = localtime(&tt);
  std::string result;
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  int val = ltm->tm_mday;
  strm << val;
  if(val < 10)
    {
      result = "0" + strm.str();
    }
  else
    {
      result = strm.str();
    }

  strm.clear();
  strm.str("");
  val = ltm->tm_mon + 1;
  strm << val;
  if(val < 10)
    {
      result = result + ".0" + strm.str();
    }
  else
    {
      result = result + "." + strm.str();
    }

  strm.clear();
  strm.str("");
  val = ltm->tm_year + 1900;
  strm << val;
  result = result + "." + strm.str();

  return result;
}

bool
AuxFunc::if_supported_type(const std::filesystem::path &ch_p)
{
  bool result = false;

  std::string ext = get_extension(ch_p);

  ext = stringToLower(ext);

  for(auto it = ext.begin(); it != ext.end();)
    {
      if(*it == '.')
	{
	  ext.erase(it);
	}
      else
	{
	  break;
	}
    }

  std::vector<std::string> types = get_supported_types();

  auto it = std::find(types.begin(), types.end(), ext);
  if(it != types.end())
    {
      result = true;
    }

  return result;
}

std::string
AuxFunc::detect_encoding(const std::string &buf)
{
  std::string result;

  UErrorCode status = U_ZERO_ERROR;

  UCharsetDetector *det = ucsdet_open(&status);
  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detect_encoding detector initialization error: "
	  << u_errorName(status) << std::endl;
      return result;
    }

  std::shared_ptr<UCharsetDetector> detector(det, []
  (UCharsetDetector *det)
    {
      ucsdet_close(det);
    });

  ucsdet_setText(det, buf.c_str(), static_cast<int32_t>(buf.size()), &status);

  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detect_encoding set text error: "
	  << u_errorName(status) << std::endl;
      return result;
    }

  const UCharsetMatch *match = ucsdet_detect(detector.get(), &status);

  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detect_encoding detecting error: "
	  << u_errorName(status) << std::endl;
      return result;
    }
  const char *nm = ucsdet_getName(match, &status);

  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detect_encoding get name error: "
	  << u_errorName(status) << std::endl;
      return result;
    }
  else
    {
      if(nm)
	{
	  result = nm;
	}
    }

  return result;
}

void
AuxFunc::html_to_utf8(std::string &result)
{
  icu::UnicodeString ustr;
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  std::string::size_type n = 0;
  std::string::size_type n_end;
  for(;;)
    {
      n = result.find("&#", n);
      if(n != std::string::npos)
	{
	  n_end = result.find(";", n);
	  if(n_end != std::string::npos)
	    {
	      std::string val(result.begin() + n, result.begin() + n_end);
	      result.erase(result.begin() + n,
			   result.begin() + n_end + std::string(";").size());
	      n_end = val.find("&#");
	      val.erase(0, n_end + std::string("&#").size());
	      strm.clear();
	      strm.str(val);
	      int32_t ch;
	      strm >> ch;
	      ustr = ch;
	      val.clear();
	      ustr.toUTF8String(val);
	      result.insert(n, val);
	    }
	  else
	    {
	      break;
	    }
	}
      else
	{
	  break;
	}
    }
}

void
AuxFunc::open_book_callback(const std::filesystem::path &path)
{
  std::string command;
  command = path.u8string();
#ifdef __linux
  for(auto it = command.begin(); it != command.end();)
    {
      if(*it == '\"' || *it == '$')
	{
	  it = command.insert(it, '\\');
	  it++;
	  it++;
	}
      else
	{
	  it++;
	}
    }
  command = "xdg-open \"" + command + "\"";
  command = utf8_to_system(command);
  int check = std::system(command.c_str());
  std::cout << "Book open command result code: " << check << std::endl;
#endif
#ifdef _WIN32
  HINSTANCE hin = ShellExecuteA (0, utf8_to_system("open").c_str (),
				  utf8_to_system(command).c_str (),
				  0, 0, 0);
     intptr_t err = reinterpret_cast<intptr_t> (hin);
   if (err <= 32)
     {
       std::cout << "Book open command error code: " << err << std::endl;
     }
#endif
}

int32_t
AuxFunc::get_charset_conv_quntity()
{
  return ucnv_countAvailable();
}

std::string
AuxFunc::utf_8_to(const std::string &input, const char *conv_name)
{
  std::string result;
  UErrorCode status = U_ZERO_ERROR;
  UConverter *c = ucnv_open(conv_name, &status);
  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::utf_8_to converter open error: "
	  << u_errorName(status) << std::endl;
      return result;
    }

  std::shared_ptr<UConverter> conv(c, []
  (UConverter *c)
    {
      ucnv_close(c);
    });

  status = U_ZERO_ERROR;
  std::vector<char> target;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(input.c_str());
  target.resize(ustr.length());
  char16_t data[ustr.length()];
  for(int i = 0; i < ustr.length(); i++)
    {
      data[i] = ustr.charAt(i);
    }
  size_t cb = ucnv_fromUChars(conv.get(), target.data(), ustr.length(), data,
			      ustr.length(), &status);
  if(!U_SUCCESS(status))
    {
      if(status == U_BUFFER_OVERFLOW_ERROR)
	{
	  status = U_ZERO_ERROR;
	  target.clear();
	  target.resize(cb);
	  ucnv_fromUChars(conv.get(), target.data(), cb, data, ustr.length(),
			  &status);
	  if(!U_SUCCESS(status))
	    {
	      std::cout << "AuxFunc::utf_8_to conversion error: "
		  << u_errorName(status) << std::endl;
	      return result;
	    }
	}
      else
	{
	  std::cout << "AuxFunc::utf_8_to conversion error: "
	      << u_errorName(status) << std::endl;
	  return result;
	}
    }

  result = std::string(target.begin(), target.end());

  return result;
}

const char*
AuxFunc::get_converter_by_number(const int32_t &num)
{
  return ucnv_getAvailableName(num);
}

std::vector<GenreGroup>
AuxFunc::read_genre_groups(const bool &wrong_loc, const std::string &locname)
{
  std::vector<GenreGroup> gg;
  std::filesystem::path genre_path = share_path();
  genre_path /= std::filesystem::u8path("genre_groups.csv");
  int genre_group_trans = 0;
  std::fstream f;
  f.open(genre_path, std::ios_base::in);
  if(f.is_open())
    {
      std::string line;
      getline(f, line);
      if(!line.empty())
	{
	  if(!wrong_loc)
	    {
	      std::string::size_type n;
	      for(;;)
		{
		  n = line.find(";");
		  if(n != std::string::npos)
		    {
		      std::string locl = line.substr(0, n);
		      line.erase(0, n + std::string(";").size());
		      n = locl.find(locname);
		      if(n != std::string::npos)
			{
			  break;
			}
		    }
		  else
		    {
		      n = line.find(locname);
		      if(n == std::string::npos)
			{
			  genre_group_trans = 0;
			}
		      break;
		    }
		  genre_group_trans++;
		}
	    }
	  else
	    {
	      genre_group_trans = 0;
	    }
	}

      while(!f.eof())
	{
	  line.clear();
	  getline(f, line);
	  if(!line.empty())
	    {
	      GenreGroup g;
	      std::string::size_type n;
	      int trans_count = 0;
	      for(;;)
		{
		  n = line.find(";");
		  if(n != std::string::npos)
		    {
		      if(trans_count == 0)
			{
			  g.group_code = line.substr(0, n);
			}
		      else if(trans_count == genre_group_trans)
			{
			  g.group_name = line.substr(0, n);
			}
		      line.erase(0, n + std::string(";").size());
		    }
		  else
		    {
		      if(!line.empty() && trans_count == genre_group_trans)
			{
			  g.group_name = line.substr(0, n);
			}
		      break;
		    }
		  trans_count++;
		}
	      gg.push_back(g);
	    }
	}
      f.close();
    }

  return gg;
}

std::string
AuxFunc::libgcrypt_error_handling(const gcry_error_t &err)
{
  std::string error;
  error.resize(1024);
  gpg_strerror_r(err, error.data(), error.size());
  for(;;)
    {
      if(error.rbegin() != error.rend())
	{
	  if(!*(error.rbegin()))
	    {
	      error.pop_back();
	    }
	  else
	    {
	      break;
	    }
	}
      else
	{
	  break;
	}
    }
  return error;
}

std::string
AuxFunc::to_hex(std::string *source)
{
  std::string result;
  result.resize(source->size() * 2);
  size_t count = 0;
  std::locale loc("C");
  std::for_each(source->begin(), source->end(), [&result, &count, loc]
  (auto &el)
    {
      uint8_t val8;
      std::memcpy(&val8, &el, sizeof(el));
      std::stringstream strm;
      strm.imbue(loc);
      strm << std::hex << static_cast<int>(val8);
      if(val8 <= 15)
	{
	  result[count] = '0';
	  result[count + 1] = strm.str()[0];
	}
      else
	{
	  result[count] = strm.str()[0];
	  result[count + 1] = strm.str()[1];
	}
      count += 2;
    });
  return result;
}

std::string
AuxFunc::stringToLower(const std::string &line)
{
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(line.c_str());
  ustr.toLower();
  std::string result;
  ustr.toUTF8String(result);
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

std::string
AuxFunc::utf8_to_system(const std::string &input)
{
  return utf_8_to(input, nullptr);
}

void
AuxFunc::copy_book_callback(const std::filesystem::path &source,
			    const std::filesystem::path &out)
{
  std::error_code ec;
  std::filesystem::copy(source, out,
			std::filesystem::copy_options::overwrite_existing, ec);
  if(ec)
    {
      throw MLException("AuxFunc::copy_book_callback error: " + ec.message());
    }
}

std::vector<std::string>
AuxFunc::get_supported_types()
{
  std::vector<std::string> types;

  types.push_back("fb2");
  types.push_back("epub");
  types.push_back("pdf");
  types.push_back("djvu");
  types.push_back("zip");
  types.push_back("7z");
  types.push_back("tar.gz");
  types.push_back("tar.xz");
  types.push_back("tar.bz2");
  types.push_back("tar");
  types.push_back("iso");
  types.push_back("cpio");
  types.push_back("jar");
  types.push_back("rar");

  return types;
}

std::vector<std::string>
AuxFunc::get_supported_archive_types_packing()
{
  std::vector<std::string> types;

  types.push_back("zip");
  types.push_back("7z");
  types.push_back("tar.gz");
  types.push_back("tar.xz");
  types.push_back("tar.bz2");
  types.push_back("tar");
  types.push_back("iso");
  types.push_back("cpio");
  types.push_back("jar");

  return types;
}

std::vector<std::string>
AuxFunc::get_supported_archive_types_unpacking()
{
  std::vector<std::string> types;

  types.push_back("zip");
  types.push_back("7z");
  types.push_back("tar.gz");
  types.push_back("tar.xz");
  types.push_back("tar.bz2");
  types.push_back("tar");
  types.push_back("iso");
  types.push_back("cpio");
  types.push_back("jar");
  types.push_back("rar");

  return types;
}

std::string
AuxFunc::get_extension(const std::filesystem::path &p)
{
  std::string result;

  std::filesystem::path lp = p;
  if(!std::filesystem::is_directory(p))
    {
      std::string ext;
      for(;;)
	{
	  if(lp.has_extension())
	    {
	      if(result.empty())
		{
		  result = lp.extension().u8string();
		  lp.replace_extension("");
		}
	      else
		{
		  ext = lp.extension().u8string();
		  if(stringToLower(ext) == ".tar")
		    {
		      result = ext + result;
		    }
		  break;
		}
	    }
	  else
	    {
	      break;
	    }
	}
    }
  return result;
}
