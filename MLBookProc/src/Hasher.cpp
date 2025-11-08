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

#include <Hasher.h>
#include <fstream>
#include <gcrypt.h>
#include <iostream>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#endif

Hasher::Hasher(const std::shared_ptr<AuxFunc> &af)
{
  this->af = af;
#ifndef USE_OPENMP
  cancel.store(false);
#else
#pragma omp atomic write
  cancel = false;
#endif
}

std::string
Hasher::buf_hashing(const std::string &buf)
{
  std::string result;

  gcry_md_hd_t hd;
  gcry_error_t err = gcry_md_open(&hd, GCRY_MD_BLAKE2B_256, 0);
  if(err != 0)
    {
      std::string error
          = "Hasher::buf_hashing error: " + af->libgcrypt_error_handling(err);
      throw std::runtime_error(error);
    }
  gcry_md_write(hd, buf.c_str(), buf.size());
  result.resize(gcry_md_get_algo_dlen(GCRY_MD_BLAKE2B_256));
  char *result_buf
      = reinterpret_cast<char *>(gcry_md_read(hd, GCRY_MD_BLAKE2B_256));
  if(result_buf)
    {
      for(size_t i = 0; i < result.size(); i++)
        {
          result[i] = result_buf[i];
        }
    }
  else
    {
      std::cout << "Hasher::buf_hashing error: result buffer is null"
                << std::endl;
    }
  gcry_md_close(hd);

  return result;
}

std::string
Hasher::file_hashing(const std::filesystem::path &filepath)
{
  std::string result;

  gcry_md_hd_t hd;
  gcry_error_t err = gcry_md_open(&hd, GCRY_MD_BLAKE2B_256, 0);
  if(err != 0)
    {
      std::string error
          = "Hasher::buf_hashing error: " + af->libgcrypt_error_handling(err);
      throw std::runtime_error(error);
    }
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      uintmax_t read_b = 0;
      f.seekg(0, std::ios_base::end);
      uintmax_t fsz = f.tellg();
      f.seekg(0, std::ios_base::beg);
      uintmax_t dif;
#ifdef USE_OPENMP
      bool cncl = false;
#endif
      for(;;)
        {
#ifdef USE_OPENMP
#pragma omp atomic read
          cncl = cancel;
          if(cncl)
            {
              break;
            }
#else
          if(cancel.load(std::memory_order_relaxed))
            {
              break;
            }
#endif

          std::string buf;
          dif = fsz - read_b;
          if(dif > 10485760)
            {
              buf.resize(10485760);
            }
          else if(dif > 0)
            {
              buf.resize(dif);
            }
          else
            {
              break;
            }
          f.read(buf.data(), buf.size());
          read_b += static_cast<uintmax_t>(buf.size());
          gcry_md_write(hd, buf.c_str(), buf.size());
        }
      f.close();
    }
  else
    {
      gcry_md_close(hd);
      throw std::runtime_error(
          "Hasher::file_hashing: file for hashing cannot be opened");
    }

  result.resize(gcry_md_get_algo_dlen(GCRY_MD_BLAKE2B_256));
  char *result_buf
      = reinterpret_cast<char *>(gcry_md_read(hd, GCRY_MD_BLAKE2B_256));
  if(result_buf)
    {
      for(size_t i = 0; i < result.size(); i++)
        {
          result[i] = result_buf[i];
        }
    }
  else
    {
      std::cout << "Hasher::buf_hashing error: result buffer is null"
                << std::endl;
    }
  gcry_md_close(hd);

  return result;
}

void
Hasher::cancelAll()
{
#ifndef USE_OPENMP
  cancel.store(true, std::memory_order_relaxed);
#else
#pragma omp atomic write
  cancel = true;
#endif
  if(stop_all_signal)
    {
      stop_all_signal();
    }
}
