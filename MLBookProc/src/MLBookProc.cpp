/*
 * Copyright (C) 2026 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <MLBookProc.h>
#include <algorithm>
#include <archive.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <omp.h>
#include <unicode/unistr.h>

MLBookProc::MLBookProc()
{
  std::chrono::time_point<std::chrono::high_resolution_clock> ctm_p
      = std::chrono::high_resolution_clock::now();
  uint64_t ctm = std::chrono::duration_cast<std::chrono::nanoseconds>(
                     ctm_p.time_since_epoch())
                     .count();
  rng = new std::mt19937_64(ctm);

  activation();
  supported_types.push_back("fb2");
  supported_types.push_back("epub");
  supported_types.push_back("pdf");
  supported_types.push_back("djvu");
  supported_types.push_back("odt");
  supported_types.push_back("txt");
  supported_types.push_back("md");
  supported_types.push_back("zip");
  supported_types.push_back("7z");
  supported_types.push_back("tar.gz");
  supported_types.push_back("tar.xz");
  supported_types.push_back("tar.bz2");
  supported_types.push_back("tar");
  supported_types.push_back("iso");
  supported_types.push_back("cpio");
  supported_types.push_back("jar");
  supported_types.push_back("rar");
  supported_types.shrink_to_fit();
}

MLBookProc::~MLBookProc()
{
  std::lock_guard<std::mutex> lglock(djvu_context_mtx);
  delete rng;
}

void
MLBookProc::activation()
{
  gcry_error_t err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P, 0);
  if(!err)
    {
      char *report = const_cast<char *>(gcry_check_version(nullptr));
      if(report)
        {
          err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
          if(err != 0)
            {
              libgcryptErrorHandling(err);
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
          std::cout << "MLBookProc OMP_CANCELLATION: "
                    << omp_get_cancellation() << std::endl;
        }
      else
        {
          throw std::runtime_error("MLBookProc: libgcrypt version is null");
        }
    }
}

std::shared_ptr<MLBookProc>
MLBookProc::create()
{
  return std::shared_ptr<MLBookProc>(new MLBookProc);
}

std::string
MLBookProc::stringToLower(const std::string &str)
{
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str.c_str());
  ustr.toLower();
  std::string result;
  ustr.toUTF8String(result);
  return result;
}

std::string
MLBookProc::getExtension(const std::filesystem::path &p)
{
  std::string result;

  if(std::filesystem::is_directory(p))
    {
      return result;
    }

  std::u8string u8str = p.filename().u8string();
  result = getExtension(std::string(u8str.begin(), u8str.end()));

  return result;
}

std::string
MLBookProc::getExtension(const std::string &file_name)
{
  std::string result;

  std::string::size_type n = file_name.find(".tar");
  if(n != std::string::npos)
    {
      result = std::string(file_name.begin() + n, file_name.end());
    }
  else
    {
      n = file_name.rfind(".");
      if(n != std::string::npos)
        {
          result = std::string(file_name.begin() + n, file_name.end());
        }
    }

  return result;
}

std::vector<std::string>
MLBookProc::getSupportedFileTypes()
{
  return supported_types;
}

std::vector<std::string>
MLBookProc::getSupportedArchivesTypesUnpacking()
{
  std::vector<std::string> result;

  auto it = std::find(supported_types.begin(), supported_types.end(), "zip");
  std::copy(it, supported_types.end(), std::back_inserter(result));
  result.shrink_to_fit();

  return result;
}

std::vector<std::string>
MLBookProc::getSupportedArchivesTypesPacking()
{
  std::vector<std::string> result;

  result = std::move(getSupportedArchivesTypesUnpacking());
  result.erase(std::remove(result.begin(), result.end(), "rar"), result.end());
  result.shrink_to_fit();

  return result;
}

bool
MLBookProc::ifSupportedFile(const std::string &filename)
{
  bool result = false;

  std::string lower = stringToLower(filename);

  lower = getExtension(lower);

  std::string find_str(".");
  std::string::size_type n = lower.find(find_str);
  if(n != std::string::npos)
    {
      lower.erase(n, find_str.size());
    }

  auto it = std::find(supported_types.begin(), supported_types.end(), lower);
  if(it != supported_types.end())
    {
      result = true;
    }

  return result;
}

bool
MLBookProc::ifSupportedFile(const std::filesystem::path &p)
{
  if(std::filesystem::is_directory(p))
    {
      return false;
    }
  std::u8string u8str = p.filename().u8string();
  return ifSupportedFile(std::string(u8str.begin(), u8str.end()));
}

void
MLBookProc::libgcryptErrorHandling(const gcry_error_t &err)
{
  std::string error;
  error.resize(1024);
  gpg_strerror_r(err, error.data(), error.size());
  error.erase(std::remove_if(error.begin(), error.end(),
                             [](const char &el)
                               {
                                 if(el)
                                   {
                                     return false;
                                   }
                                 return true;
                               }),
              error.end());
  throw std::runtime_error(error);
}

std::filesystem::path
MLBookProc::tempDirPath()
{
  std::filesystem::path result = std::filesystem::temp_directory_path();
#ifdef _WIN32
  return result.parent_path();
#else
  return result;
#endif
}

template int64_t
MLBookProc::randomNumber();
template int32_t
MLBookProc::randomNumber();
template int16_t
MLBookProc::randomNumber();
template int8_t
MLBookProc::randomNumber();
template uint64_t
MLBookProc::randomNumber();
template uint32_t
MLBookProc::randomNumber();
template uint16_t
MLBookProc::randomNumber();
template uint8_t
MLBookProc::randomNumber();
template <typename T>
T
MLBookProc::randomNumber()
{
  std::numeric_limits<T> lim;
  std::uniform_int_distribution<T> dist(lim.min() + 1, lim.max());
  return dist(*rng);
}

std::filesystem::path
MLBookProc::randomFileName()
{
  std::filesystem::path result;

  uint64_t num = randomNumber<uint64_t>();
  std::stringstream strm;
  strm.imbue(std::locale("C"));
  strm << std::hex << num;
  std::string str = strm.str();
  result = std::u8string(str.begin(), str.end());

  return result;
}

std::string
MLBookProc::timeToDate(const time_t &tt)
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
      std::cout << "MLBookProc::timeTodate: " << std::strerror(errno)
                << std::endl;
    }

  return result;
}

std::shared_ptr<DJVUContext>
MLBookProc::getDJVUContext()
{
  std::lock_guard<std::mutex> lglock(djvu_context_mtx);
  std::shared_ptr<DJVUContext> result = djvu_contex.lock();
  if(!result.operator bool())
    {
      result = std::make_shared<DJVUContext>();
      ddjvu_message_set_callback(result->context,
                                 &MLBookProc::djvuMessageCallback, this);
      djvu_contex = result;
    }

  return result;
}

void
MLBookProc::djvuMessageCallback(ddjvu_context_t *context, void *closure)
{
  MLBookProc *mlbp = reinterpret_cast<MLBookProc *>(closure);
  std::shared_ptr<DJVUContext> ctx = mlbp->getDJVUContext();
  ctx->context_var.notify_all();
}
