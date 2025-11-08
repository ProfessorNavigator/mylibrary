/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <AuxFunc.h>
#include <OmpLockGuard.h>
#include <algorithm>
#include <archive.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <gpg-error.h>
#include <iostream>
#include <sstream>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <unicode/urename.h>
#include <unicode/utypes.h>
#include <unicode/uversion.h>

#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef __linux
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#endif

AuxFunc::AuxFunc()
{
  std::chrono::time_point<std::chrono::high_resolution_clock> ctm_p
      = std::chrono::high_resolution_clock::now();
  uint64_t ctm = std::chrono::duration_cast<std::chrono::nanoseconds>(
                     ctm_p.time_since_epoch())
                     .count();
  rng = new std::mt19937_64(ctm);
  std::numeric_limits<uint64_t> lim;
  dist = new std::uniform_int_distribution<uint64_t>(lim.min() + 1, lim.max());

  gcry_error_t err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P, 0);
  if(!err)
    {
      char *report = const_cast<char *>(gcry_check_version(nullptr));
      if(report)
        {
          gcry_error_t err = gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
          if(err != 0)
            {
              std::cout << "MLBookProc libgcrypt disabling secmem error: "
                        << libgcrypt_error_handling(err) << std::endl;
            }
          err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
          if(err != 0)
            {
              std::cout << "MLBookProc libgcrypt initialization error: "
                        << libgcrypt_error_handling(err) << std::endl;
            }
          std::cout << "MLBookProc: libgcrypt has been initialized, version: "
                    << report << std::endl;

          report = const_cast<char *>(archive_version_details());
          if(report)
            {
              std::cout << "MLBookProc: " << report << std::endl;
            }

          report = const_cast<char *>(ddjvu_get_version_string());
          if(report)
            {
              std::cout << "MLBookProc: " << report << std::endl;
            }

          report = setlocale(LC_CTYPE, "");
          if(report)
            {
              std::cout << "MLBookProc set locale: " << report << std::endl;
            }
          else
            {
              std::cout << "MLBookProc: cannot set locale" << std::endl;
            }
#ifdef USE_OPENMP
          std::cout << "MLBookProc OMP_CANCELLATION: "
                    << omp_get_cancellation() << std::endl;
          omp_init_lock(&djvu_context_mtx);
#endif
        }
      else
        {
          activated = false;
        }
    }
}

AuxFunc::~AuxFunc()
{
  delete rng;
  delete dist;
#ifdef USE_OPENMP
  omp_destroy_lock(&djvu_context_mtx);
#endif
}

std::string
AuxFunc::toUTF8(const std::string &input, const char *conv_name)
{
  std::string result;
  if(input.size() == 0)
    {
      return result;
    }

  UErrorCode status = U_ZERO_ERROR;
  std::shared_ptr<UConverter> conv(ucnv_open(conv_name, &status),
                                   [](UConverter *c)
                                     {
                                       ucnv_close(c);
                                     });
  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::toUTF8 converter " << conv_name
                << " ucnv_open: " << u_errorName(status) << std::endl;
      result = input;
      return result;
    }

  std::vector<UChar> target;
  target.resize(input.size());

  const char *lim = &input.data()[input.size()];
  const char *src = input.c_str();
  UChar *trgt = target.data();
  status = U_ZERO_ERROR;
  for(;;)
    {
      ucnv_toUnicode(conv.get(), &trgt, &target.data()[target.size()], &src,
                     lim, nullptr, true, &status);
      if(status == U_BUFFER_OVERFLOW_ERROR)
        {
          size_t buf_sz = target.size();
          target.resize(buf_sz + buf_sz * 0.3);
          trgt = &target[buf_sz];
          status = U_ZERO_ERROR;
        }
      else
        {
          break;
        }
    }
  if(status == U_ZERO_ERROR)
    {
      target.erase(std::vector<UChar>::iterator(trgt), target.end());
      icu::UnicodeString str(target.data(),
                             static_cast<int32_t>(target.size()),
                             static_cast<int32_t>(target.capacity()));
      str.toUTF8String(result);
    }

  return result;
}

std::string
AuxFunc::to_utf_8(const std::string &input, const char *conv_name)
{
  return toUTF8(input, conv_name);
}

std::filesystem::path
AuxFunc::homePath()
{
  std::string result;
  for(int i = 1; i <= 5; i++)
    {
      char *fnm = nullptr;
      switch(i)
        {
        case 1:
          {
            fnm = getenv("USERPROFILE");
            break;
          }
        case 2:
          {
            fnm = getenv("HOMEDRIVE");
            break;
          }
        case 3:
          {
            fnm = getenv("HOMEPATH");
            break;
          }
        case 4:
          {
            fnm = getenv("HOME");
            break;
          }
        case 5:
          {
            fnm = getenv("SystemDrive");
            break;
          }
        }

      if(fnm)
        {
          result = fnm;
          break;
        }
    }
  if(result.empty())
    {
      throw std::runtime_error("MLBookProc cannot find user home directory");
    }
  result = toUTF8(result, nullptr);
  return std::filesystem::u8path(result);
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
  char pth[MAX_PATH];
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
  result /= std::filesystem::u8path("..") / std::filesystem::u8path("share");
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
  if(locname.empty() || locname == "C")
    {
      locname = "en_EN";
      wrong_loc = true;
    }

  result = read_genre_groups(wrong_loc, locname);

  std::vector<std::tuple<std::string, Genre>> gv
      = read_genres(wrong_loc, locname);

  for(auto it = gv.begin(); it != gv.end(); it++)
    {
      std::string g_code = std::get<0>(*it);
      auto itr = std::find_if(result.begin(), result.end(),
                              [g_code](GenreGroup &el) {
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
  genre_path /= std::filesystem::u8path("MLBookProc");
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
              std::string sstr = ";";
              for(;;)
                {
                  n = line.find(sstr);
                  if(n != std::string::npos)
                    {
                      std::string locl = line.substr(0, n);
                      line.erase(0, n + sstr.size());
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
                  std::string sstr = ";";
                  for(;;)
                    {
                      n = line.find(sstr);
                      if(n != std::string::npos)
                        {
                          std::string locl = line.substr(0, n);
                          line.erase(0, n + sstr.size());
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
                      result.emplace_back(gen);
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
  std::string result;

  std::tm *ltm = gmtime(&tt);
  if(ltm)
    {
      result.resize(100);
      size_t sz = std::strftime(result.data(), result.size(), "%d.%m.%Y", ltm);
      result.resize(sz);
    }
  else
    {
      std::cout << "AuxFunc::time_t_to_date: " << std::strerror(errno)
                << std::endl;
    }

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

bool
AuxFunc::ifSupportedArchiveUnpackaingType(const std::filesystem::path &ch_p)
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

  std::vector<std::string> types = get_supported_archive_types_unpacking();

  auto it = std::find(types.begin(), types.end(), ext);
  if(it != types.end())
    {
      result = true;
    }

  return result;
}

bool
AuxFunc::ifSupportedArchivePackingType(const std::filesystem::path &ch_p)
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

  std::vector<std::string> types = get_supported_archive_types_packing();

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
  return detectEncoding(buf);
}

std::string
AuxFunc::detectEncoding(const std::string &buf)
{
  std::string result;

  UErrorCode status = U_ZERO_ERROR;

  UCharsetDetector *det = ucsdet_open(&status);
  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detectEncoding detector initialization error: "
                << u_errorName(status) << std::endl;
      return result;
    }

  std::shared_ptr<UCharsetDetector> detector(det,
                                             [](UCharsetDetector *det)
                                               {
                                                 ucsdet_close(det);
                                               });

  ucsdet_setText(det, buf.c_str(), static_cast<int32_t>(buf.size()), &status);

  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detectEncoding set text error: "
                << u_errorName(status) << std::endl;
      return result;
    }

  const UCharsetMatch *match = ucsdet_detect(detector.get(), &status);

  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detectEncoding detecting error: "
                << u_errorName(status) << std::endl;
      return result;
    }
  const char *nm = ucsdet_getName(match, &status);

  if(!U_SUCCESS(status))
    {
      std::cout << "AuxFunc::detectEncoding get name error: "
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
  std::string sstr1 = "&#";
  std::string sstr2 = ";";
  for(;;)
    {
      n = result.find(sstr1, n);
      if(n != std::string::npos)
        {
          n_end = result.find(sstr2, n);
          if(n_end != std::string::npos)
            {
              std::string val(result.begin() + n, result.begin() + n_end);
              result.erase(n, val.size() + sstr2.size());
              n_end = val.find(sstr1);
              val.erase(0, n_end + sstr1.size());
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
  HINSTANCE hin = ShellExecuteA(0, utf8_to_system("open").c_str(),
                                utf8_to_system(command).c_str(), 0, 0, 0);
  intptr_t err = reinterpret_cast<intptr_t>(hin);
  if(err <= 32)
    {
      std::cout << "Book open command error code: " << err << std::endl;
    }
#endif
}

int32_t
AuxFunc::get_charset_conv_quantity()
{
  return ucnv_countAvailable();
}

bool
AuxFunc::get_activated()
{
  return activated;
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

  std::shared_ptr<UConverter> conv(c, [](UConverter *c) {
    ucnv_close(c);
  });

  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(input.c_str());
  result.resize(input.size());
  char *trgt = result.data();
  const UChar *src = ustr.getBuffer();
  const UChar *limit = &src[ustr.length()];
  status = U_ZERO_ERROR;
  for(;;)
    {
      ucnv_fromUnicode(conv.get(), &trgt, &result.data()[result.size()], &src,
                       limit, nullptr, true, &status);
      if(status == U_BUFFER_OVERFLOW_ERROR)
        {
          size_t buf_sz = result.size();
          result.resize(buf_sz + 100);
          trgt = &result[buf_sz];
          status = U_ZERO_ERROR;
        }
      else
        {
          break;
        }
    }

  if(status == U_ZERO_ERROR)
    {
      result.erase(std::string::iterator(trgt), result.end());
    }
  else
    {
      std::cout << "AuxFunc::utf_8_to ucnv_fromUnicode: "
                << u_errorName(status) << std::endl;
      result.clear();
    }
  result.shrink_to_fit();

  return result;
}

const char *
AuxFunc::get_converter_by_number(const int32_t &num)
{
  return ucnv_getAvailableName(num);
}

std::vector<GenreGroup>
AuxFunc::read_genre_groups(const bool &wrong_loc, const std::string &locname)
{
  std::vector<GenreGroup> gg;
  std::filesystem::path genre_path = share_path();
  genre_path /= std::filesystem::u8path("MLBookProc");
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
              std::string sstr = ";";
              for(;;)
                {
                  n = line.find(sstr);
                  if(n != std::string::npos)
                    {
                      std::string locl = line.substr(0, n);
                      line.erase(0, n + sstr.size());
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
              std::string sstr = ";";
              for(;;)
                {
                  n = line.find(sstr);
                  if(n != std::string::npos)
                    {
                      if(trans_count == 0)
                        {
                          g.group_code = line.substr(0, n);
                          g.group_name = g.group_code;
                        }
                      else if(trans_count == genre_group_trans)
                        {
                          g.group_name = line.substr(0, n);
                        }
                      line.erase(0, n + sstr.size());
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
              gg.emplace_back(g);
            }
        }
      f.close();
    }

  return gg;
}

void
AuxFunc::djvuMessageCallback(ddjvu_context_t *context, void *closure)
{
#ifdef USE_OPENMP
  std::tuple<omp_lock_t *, std::vector<std::weak_ptr<int>> *> *djvu_tup
      = reinterpret_cast<
          std::tuple<omp_lock_t *, std::vector<std::weak_ptr<int>> *> *>(
          closure);
  OmpLockGuard olg(*std::get<0>(*djvu_tup));
#else
  std::tuple<std::mutex *, std::vector<std::weak_ptr<int>> *> *djvu_tup
      = reinterpret_cast<
          std::tuple<std::mutex *, std::vector<std::weak_ptr<int>> *> *>(
          closure);
  std::lock_guard<std::mutex> lglock(*std::get<0>(*djvu_tup));
#endif
  uint8_t signal = 1;
  for(auto it = std::get<1>(*djvu_tup)->begin();
      it != std::get<1>(*djvu_tup)->end();)
    {
      std::shared_ptr<int> pipe = it->lock();
      if(pipe)
        {
#if defined(__linux)
          pollfd fd;
          fd.fd = *(pipe.get() + 1);
          fd.events = POLLOUT;
          int respol = poll(&fd, 1, 1000);
          if(respol > 0)
            {
              if(fd.revents & POLLERR)
                {
                  throw std::runtime_error(
                      "AuxFunc::djvuMessageCallback: poll error");
                }
              if(fd.revents & POLLOUT)
                {
                  write(fd.fd, &signal, sizeof(signal));
                }
            }
          else
            {
              if(respol == 0)
                {
                  throw std::runtime_error(
                      "AuxFunc::djvuMessageCallback: poll timeout exceeded");
                }
              else
                {
                  std::string str = std::strerror(errno);
                  str = "AuxFunc::djvuMessageCallback: " + str;
                  throw std::runtime_error(str);
                }
            }
#elif defined(_WIN32)
          HANDLE *handles = reinterpret_cast<HANDLE *>(pipe.get());
          DWORD wb;
          WriteFile(*(handles + 1), &signal, sizeof(signal), &wb, nullptr);
#endif
          it++;
        }
      else
        {
          std::get<1>(*djvu_tup)->erase(it);
        }
    }
}

std::shared_ptr<AuxFunc>
AuxFunc::create()
{
  return std::shared_ptr<AuxFunc>(new AuxFunc);
}

std::tuple<std::shared_ptr<ddjvu_context_t>, std::shared_ptr<int>>
AuxFunc::getDJVUContext()
{
  std::tuple<std::shared_ptr<ddjvu_context_t>, std::shared_ptr<int>> result;
#ifndef USE_OPENMP
  std::lock_guard<std::mutex> lglock(djvu_context_mtx);
#else
  OmpLockGuard omp_lock(djvu_context_mtx);
#endif
  std::shared_ptr<ddjvu_context_t> context = djvu_context.lock();
  if(context.get() == nullptr)
    {
#ifdef USE_OPENMP
      std::tuple<omp_lock_t *, std::vector<std::weak_ptr<int>> *> *djvu_tup
          = new std::tuple<omp_lock_t *, std::vector<std::weak_ptr<int>> *>;
      std::get<0>(*djvu_tup) = &djvu_context_mtx;
      std::get<1>(*djvu_tup) = &djvu_pipes;
#else
      std::tuple<std::mutex *, std::vector<std::weak_ptr<int>> *> *djvu_tup
          = new std::tuple<std::mutex *, std::vector<std::weak_ptr<int>> *>;
      std::get<0>(*djvu_tup) = &djvu_context_mtx;
      std::get<1>(*djvu_tup) = &djvu_pipes;
#endif
      context = std::shared_ptr<ddjvu_context_t>(
          ddjvu_context_create("MLBookProc"),
          [djvu_tup, this](ddjvu_context_t *ctx) {
            ddjvu_context_release(ctx);
            djvu_pipes.clear();
            delete djvu_tup;
          });

      ddjvu_message_set_callback(context.get(), &AuxFunc::djvuMessageCallback,
                                 djvu_tup);
      djvu_context = context;
    }
  std::get<0>(result) = context;
  std::shared_ptr<int> pipe;
#if defined(__linux)
  pipe = std::shared_ptr<int>(new int[2], [](int *pipe) {
    for(size_t i = 0; i < 2; i++)
      {
        if(pipe[i] >= 0)
          {
            close(pipe[i]);
          }
      }
    delete[] pipe;
  });
  if(pipe2(pipe.get(), O_NONBLOCK) < 0)
    {
      std::string str = std::strerror(errno);
      str = "AuxFunc::getDJVUContext: " + str;
      throw std::runtime_error(str);
    }
#elif defined(_WIN32)
  HANDLE *handles = new HANDLE[2];
  pipe = std::shared_ptr<int>(reinterpret_cast<int *>(handles), [](int *ptr) {
    HANDLE *handles = reinterpret_cast<HANDLE *>(ptr);
    for(size_t i = 0; i < 2; i++)
      {
        if(handles[i])
          {
            CloseHandle(handles[i]);
          }
      }
    delete[] handles;
  });
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = nullptr;
  sa.bInheritHandle = true;

  if(!CreatePipe(handles, (handles + 1), &sa, 0))
    {
      throw std::runtime_error("AuxFunc::getDJVUContext: cannot create pipe");
    }
#endif
  djvu_pipes.push_back(pipe);
  std::get<1>(result) = pipe;
  return result;
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
AuxFunc::to_hex(const std::string &source)
{
  std::string result;
  result.resize(source.size() * 2);
  size_t count = 0;
  std::locale loc("C");
  std::for_each(source.begin(), source.end(),
                [&result, &count, loc](const char &el) {
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
  uint64_t rndm = (*dist)(*rng);
  std::string result;
  std::stringstream strm;
  std::locale loc("C");
  strm.imbue(loc);
  strm << std::hex << rndm;
  result = strm.str() + "MLBookProc";
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
  std::filesystem::remove_all(out);
  std::error_code ec;
  std::filesystem::copy(source, out, ec);
  if(ec)
    {
      throw std::runtime_error("AuxFunc::copy_book_callback error: "
                               + ec.message());
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
  types.push_back("odt");
  types.push_back("txt");
  types.push_back("md");
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
