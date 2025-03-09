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

private:
  AuxFunc();

  std::vector<std::tuple<std::string, Genre>>
  read_genres(const bool &wrong_loc, const std::string &locname);

  std::vector<GenreGroup>
  read_genre_groups(const bool &wrong_loc, const std::string &locname);

  bool activated = true;
};

#endif // AUXFUNC_H
