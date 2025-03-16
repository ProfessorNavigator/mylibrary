/*
 * Copyright (C) 2024-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#ifndef AUXFUNC_H
#define AUXFUNC_H

#include <Genre.h>
#include <GenreGroup.h>
#include <filesystem>
#include <gcrypt.h>
#include <string>
#include <tuple>
#include <vector>

#ifdef USE_OPENMP
#include <omp.h>
#endif

class AuxFunc
{
public:
  static std::shared_ptr<AuxFunc>
  create();

  std::string
  to_utf_8(const std::string &input, const char *conv_name);

  std::string
  utf8_to_system(const std::string &input);

  std::string
  utf_8_to(const std::string &input, const char *conv_name);

  const char *
  get_converter_by_number(const int32_t &num);

  std::string
  detect_encoding(const std::string &buf);

  std::filesystem::path
  homePath();

  std::filesystem::path
  get_selfpath();

  std::filesystem::path
  temp_path();

  std::filesystem::path
  share_path();

  std::vector<GenreGroup>
  get_genre_list();

  std::string
  libgcrypt_error_handling(const gcry_error_t &err);

  std::string
  to_hex(std::string *source);

  std::string
  stringToLower(const std::string &line);

  std::string
  randomFileName();

  std::string
  time_t_to_date(const time_t &tt);

  bool
  if_supported_type(const std::filesystem::path &ch_p);

  void
  html_to_utf8(std::string &input);

  void
  open_book_callback(const std::filesystem::path &path);

  void
  copy_book_callback(const std::filesystem::path &source,
                     const std::filesystem::path &out);

  std::vector<std::string>
  get_supported_types();

  std::vector<std::string>
  get_supported_archive_types_packing();

  std::vector<std::string>
  get_supported_archive_types_unpacking();

  std::string
  get_extension(const std::filesystem::path &p);

  int32_t
  get_charset_conv_quantity();

  bool
  get_activated();

#ifdef USE_OPENMP
  template <class InputIt,
            class T = typename std::iterator_traits<InputIt>::value_type>
  static InputIt
  parallelFind(InputIt start, InputIt end, const T &val)
  {
    InputIt res = end;
#pragma omp parallel
    {
#pragma omp for
      for(InputIt i = start; i != end; i++)
        {
          if(res < i)
            {
              continue;
            }
          else if(*i == val)
            {
              res = i;
#pragma omp cancel for
            }
        }
    }
    return res;
  }

  template <class InputIt, class UnaryPred>
  static InputIt
  parallelFindIf(InputIt start, InputIt end, UnaryPred predicate)
  {
    InputIt res = end;
#pragma omp parallel
    {
#pragma omp for
      for(InputIt i = start; i != end; i++)
        {
          if(res < i)
            {
              continue;
            }
          else if(predicate(*i))
            {
              res = i;
#pragma omp cancel for
            }
        }
    }

    return res;
  }

  template <class InputIt,
            class T = typename std::iterator_traits<InputIt>::value_type>
  static InputIt
  parallelRemove(InputIt start, InputIt end, const T &val)
  {
    start = parallelFind(start, end, val);
    if(start != end)
      {
        for(InputIt i = start; ++i != end;)
          {
            if(!(*i == val))
              {
                *start++ = std::move(*i);
              }
          }
      }
    return start;
  }

  template <class InputIt, class UnaryPred>
  static InputIt
  parallelRemoveIf(InputIt start, InputIt end, UnaryPred predicate)
  {
    start = parallelFindIf(start, end, predicate);
    if(start != end)
      {
        for(InputIt i = start; ++i != end;)
          {
            if(!predicate(*i))
              {
                *start++ = std::move(*i);
              }
          }
      }
    return start;
  }
#endif

private:
  AuxFunc();

  std::vector<std::tuple<std::string, Genre>>
  read_genres(const bool &wrong_loc, const std::string &locname);

  std::vector<GenreGroup>
  read_genre_groups(const bool &wrong_loc, const std::string &locname);

  bool activated = true;
};

#endif // AUXFUNC_H
