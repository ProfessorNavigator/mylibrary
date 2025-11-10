/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <XMLTextEncoding.h>
#include <functional>
#include <iostream>
#include <memory>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>

std::string
XMLTextEncoding::detectDocumentEncoding(const std::string &document)
{
  std::string result;

  std::string find_str1("<?xml");
  std::string::size_type n1 = document.find(find_str1);
  if(n1 == std::string::npos)
    {
      return result;
    }
  std::string find_str2("?>");
  std::string::size_type n2 = document.find(find_str2, n1 + find_str1.size());
  if(n2 == std::string::npos)
    {
      return result;
    }

  bool s_quot = false;
  bool d_quot = false;
  bool stop = false;
  bool attr_name_finished = false;
  std::string attribute_name;
  for(size_t i = n1 + find_str1.size(); i < n2; i++)
    {
      switch(document[i])
        {
        case '\'':
          {
            if(s_quot)
              {
                if(attribute_name == "encoding")
                  {
                    stop = true;
                  }
                else
                  {
                    s_quot = false;
                    attribute_name.clear();
                    result.clear();
                  }
              }
            else if(d_quot)
              {
                result.push_back(document[i]);
              }
            else
              {
                s_quot = true;
              }
            break;
          }
        case '\"':
          {
            if(d_quot)
              {
                if(attribute_name == "encoding")
                  {
                    stop = true;
                  }
                else
                  {
                    d_quot = false;
                    attribute_name.clear();
                    result.clear();
                  }
              }
            else if(s_quot)
              {
                result.push_back(document[i]);
              }
            else
              {
                d_quot = true;
              }
            break;
          }
        case ' ':
          {
            if(s_quot || d_quot)
              {
                result.push_back(document[i]);
              }
            else
              {
                attr_name_finished = false;
              }
            break;
          }
        case '=':
          {
            if(!s_quot && !d_quot)
              {
                attr_name_finished = true;
              }
            else
              {
                result.push_back(document[i]);
              }
            break;
          }
        default:
          {
            if(attr_name_finished)
              {
                result.push_back(document[i]);
              }
            else
              {
                attribute_name.push_back(document[i]);
              }
            break;
          }
        }

      if(stop)
        {
          break;
        }
    }
  if(!stop)
    {
      std::cout
          << "XMLTextEncoding::detectDocumentEncoding: using fallback method"
          << std::endl;
      result.clear();
      std::vector<std::string> res
          = XMLTextEncoding::detectStringEncoding(document, true);
      if(res.size() > 0)
        {
          result = res[0];
        }
    }
  return result;
}

std::vector<std::string>
XMLTextEncoding::detectStringEncoding(const std::string &str,
                                      const bool &filter)
{
  std::vector<std::string> result;

  UErrorCode e_code = U_ZERO_ERROR;

  std::unique_ptr<UCharsetDetector, std::function<void(UCharsetDetector *)>>
      detector = std::unique_ptr<UCharsetDetector,
                                 std::function<void(UCharsetDetector *)>>(
          ucsdet_open(&e_code),
          [](UCharsetDetector *det)
            {
              ucsdet_close(det);
            });

  if(U_FAILURE(e_code))
    {
      std::string e_str("XMLTextEncoding::detectStringEncoding: ");
      e_str += u_errorName(e_code);
      std::cout << e_str << std::endl;
      return result;
    }

  ucsdet_enableInputFilter(detector.get(), static_cast<UBool>(filter));
  if(filter)
    {
      if(!ucsdet_isInputFilterEnabled(detector.get()))
        {
          std::string e_str("XMLTextEncoding::detectStringEncoding: filter "
                            "has not been set!");
          std::cout << e_str << std::endl;
        }
    }

  e_code = U_ZERO_ERROR;

  ucsdet_setText(detector.get(), str.c_str(), static_cast<int32_t>(str.size()),
                 &e_code);

  if(U_FAILURE(e_code))
    {
      std::string e_str("XMLTextEncoding::detectStringEncoding: ");
      e_str += u_errorName(e_code);
      std::cout << e_str << std::endl;
      return result;
    }

  e_code = U_ZERO_ERROR;
  int32_t sz = 0;
  const UCharsetMatch **match = ucsdet_detectAll(detector.get(), &sz, &e_code);
  if(U_FAILURE(e_code))
    {
      std::string e_str("XMLTextEncoding::detectStringEncoding: ");
      e_str += u_errorName(e_code);
      std::cout << e_str << std::endl;
      return result;
    }

  result.reserve(sz);

  for(int32_t i = 0; i < sz; i++)
    {
      e_code = U_ZERO_ERROR;
      const char *name = ucsdet_getName(match[i], &e_code);
      if(U_SUCCESS(e_code))
        {
          result.push_back(name);
        }
    }

  return result;
}

void
XMLTextEncoding::convertToEncoding(const std::string &source,
                                   std::string &result,
                                   const std::string &source_code_page,
                                   const std::string &result_code_page)
{
  result.clear();

  if(source.size() == 0)
    {
      return void();
    }

  UErrorCode e_code = U_ZERO_ERROR;
  std::unique_ptr<UConverter, std::function<void(UConverter * conv)>>
      source_converter;

  if(source_code_page.empty())
    {
      source_converter
          = std::unique_ptr<UConverter, std::function<void(UConverter *)>>(
              ucnv_open(nullptr, &e_code),
              [](UConverter *conv)
                {
                  ucnv_close(conv);
                });
    }
  else
    {
      source_converter
          = std::unique_ptr<UConverter, std::function<void(UConverter *)>>(
              ucnv_open(source_code_page.c_str(), &e_code),
              [](UConverter *conv)
                {
                  ucnv_close(conv);
                });
    }
  if(U_FAILURE(e_code))
    {
      std::string e_str(
          "XMLTextEncoding::convertToEncoding source converter: ");
      e_str += u_errorName(e_code);
      std::cout << e_str << std::endl;
      return void();
    }

  e_code = U_ZERO_ERROR;
  std::unique_ptr<UConverter, std::function<void(UConverter * conv)>>
      result_converter;

  if(result_code_page.empty())
    {
      result_converter
          = std::unique_ptr<UConverter, std::function<void(UConverter *)>>(
              ucnv_open(nullptr, &e_code),
              [](UConverter *conv)
                {
                  ucnv_close(conv);
                });
    }
  else
    {
      result_converter
          = std::unique_ptr<UConverter, std::function<void(UConverter *)>>(
              ucnv_open(result_code_page.c_str(), &e_code),
              [](UConverter *conv)
                {
                  ucnv_close(conv);
                });
    }
  if(U_FAILURE(e_code))
    {
      std::string e_str(
          "XMLTextEncoding::convertToEncoding result converter: ");
      e_str += u_errorName(e_code);
      std::cout << e_str << std::endl;
      return void();
    }

  std::vector<UChar> res;
  res.resize(source.size());
  UChar *start_res = res.data();
  const char *start_src = source.data();
  for(;;)
    {
      e_code = U_ZERO_ERROR;
      ucnv_toUnicode(source_converter.get(), &start_res, res.end().base(),
                     &start_src, source.end().base(), nullptr, true, &e_code);
      if(U_SUCCESS(e_code))
        {
          break;
        }
      else
        {
          if(e_code == U_BUFFER_OVERFLOW_ERROR)
            {
              size_t sz = res.size();
              res.resize(sz + sz);
              start_res = res.data() + sz;
            }
          else
            {
              std::string e_str(
                  "XMLTextEncoding::convertToEncoding conversion(1): ");
              e_str += u_errorName(e_code);
              std::cout << e_str << std::endl;
              return void();
            }
        }
    }

  res.erase(std::vector<UChar>::iterator(start_res), res.end());

  result.resize(res.size());
  char *start_result = result.data();
  const UChar *start_res_const = res.data();
  for(;;)
    {
      e_code = U_ZERO_ERROR;
      ucnv_fromUnicode(result_converter.get(), &start_result,
                       result.end().base(), &start_res_const, res.end().base(),
                       nullptr, true, &e_code);
      if(U_SUCCESS(e_code))
        {
          break;
        }
      else
        {
          if(e_code == U_BUFFER_OVERFLOW_ERROR)
            {
              size_t sz = result.size();
              result.resize(sz + sz);
              start_result = result.data() + sz;
            }
          else
            {
              std::string e_str(
                  "XMLTextEncoding::convertToEncoding conversion(1): ");
              e_str += u_errorName(e_code);
              std::cout << e_str << std::endl;
              result.clear();
              result.shrink_to_fit();
              return void();
            }
        }
    }
  result.erase(std::string::iterator(start_result), result.end());
  result.shrink_to_fit();
}
