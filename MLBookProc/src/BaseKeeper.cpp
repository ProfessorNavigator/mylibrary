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

#include <Algorithm.h>
#include <BaseKeeper.h>
#include <ByteOrder.h>
#include <CreateCollection.h>
#include <InpxLoader.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>

BaseKeeper::BaseKeeper(const std::shared_ptr<MLBookProc> &mlbp)
{
  this->mlbp = mlbp;
}

BaseKeeper::~BaseKeeper()
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
}

void
BaseKeeper::loadCollection(const std::filesystem::path &base_path)
{
  std::lock_guard<std::shared_mutex> lglock(base_mtx);
  current_base_path = base_path;
  base.clear();
  std::fstream f;
  f.open(base_path, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::u8string str(u8"BaseKeeper::loadCollection: cannot open file ");
      str += base_path.u8string();
      throw std::runtime_error(reinterpret_cast<const char *>(str.c_str()));
    }
  std::vector<char> buf;
  f.seekg(0, std::ios_base::end);
  buf.resize(static_cast<size_t>(f.tellg()));
  f.seekg(0, std::ios_base::beg);
  f.read(buf.data(), buf.size());
  f.close();

  try
    {
      uint64_t btr;
      size_t sz_64 = sizeof(btr);
      size_t rb = 0;
      size_t buf_sz = buf.size();
      if(buf_sz < sz_64 + 4)
        {
          throw std::runtime_error("incorrect base(1)");
        }
      {
        std::string ch_str;
        std::copy(buf.begin() + sz_64 + 1, buf.begin() + sz_64 + 4,
                  std::back_inserter(ch_str));
        if(ch_str != "UDB")
          {
            throw std::runtime_error("incorrect base(2)");
          }
      }
      ByteOrder bo;
#pragma omp parallel
#pragma omp masked
      {
        while(rb < buf_sz)
          {
            if(buf_sz < sz_64 + rb)
              {
                throw std::runtime_error("incorrect base(3)");
              }
            std::memcpy(&btr, &buf[rb], sz_64);
            rb += sz_64;

            bo.setLittle(btr);
            btr = bo;

            size_t bz_sz = static_cast<size_t>(btr);
            if(bz_sz == 0)
              {
                throw std::runtime_error("incorrect base(4)");
              }
            size_t lrb = rb;
            rb += bz_sz;

#pragma omp task
            {
              UDBase b;
              try
                {
                  b.readFromBuffer(buf, lrb, bz_sz);
                }
              catch(std::exception &er)
                {
#pragma omp critical
                  {
                    std::cout << "BaseKeeper::loadCollection: \"" << er.what()
                              << "\"" << std::endl;
                  }
                }

              std::vector<UDBElement> *b_ptr = b.getRawBase();
#pragma omp critical
              {
                std::copy(b_ptr->begin(), b_ptr->end(),
                          std::back_inserter(base));
              }
            }
          }
      }
    }
  catch(std::exception &er)
    {
      std::cout << "BaseKeeper::loadCollection: \"" << er.what() << "\""
                << std::endl;
      loadCollectionLegacy(buf);
    }

  Algorithm alg;
  auto result = alg.parallelFindIf(
      base.begin(), base.end(),
      [this](const UDBElement &el)
        {
          auto it_sub = std::find_if(
              el.subelements.begin(), el.subelements.end(),
              [this](const UDBElement &el)
                {
                  return bid.getId(el) == BaseID::CollectionType;
                });
          if(it_sub != el.subelements.end())
            {
              if(it_sub->content == "inpx")
                {
                  return true;
                }
            }
          return false;
        });
  if(result != base.end())
    {
      std::filesystem::path books_dir;
      std::vector<UDBElement>::iterator it_sub = std::find_if(
          result->subelements.begin(), result->subelements.end(),
          [this](const UDBElement &el)
            {
              return bid.getId(el) == BaseID::BooksDirectory;
            });
      if(it_sub != result->subelements.end())
        {
          books_dir
              = std::u8string(it_sub->content.begin(), it_sub->content.end());
        }
      std::filesystem::path inpx_path;
      it_sub = std::find_if(result->subelements.begin(),
                            result->subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::InpxPath;
                              });
      if(it_sub != result->subelements.end())
        {
          inpx_path
              = std::u8string(it_sub->content.begin(), it_sub->content.end());
        }
      std::unique_ptr<InpxLoader> loader(new InpxLoader(mlbp));
      UDBase result_base;
      try
        {
          result_base = loader->loadInpxCollection(books_dir, inpx_path);
        }
      catch(std::exception &er)
        {
          std::cout << "BaseKeeper::loadCollection: \"" << er.what() << "\""
                    << std::endl;
        }
      std::vector<UDBElement> *raw_base = result_base.getRawBase();
      std::copy(raw_base->begin(), raw_base->end(), std::back_inserter(base));
    }
  this->shrinkToFit();
}

size_t
BaseKeeper::getBooksQuantity()
{
  std::shared_lock shlock(base_mtx);
  size_t books_in_base = booksQuantity(base);

  return books_in_base;
}

size_t
BaseKeeper::getFilesQuantity()
{
  std::shared_lock shlock(base_mtx);
  size_t result = filesQuantity(base);

  return result;
}

UDBase
BaseKeeper::searchBook(const std::vector<UDBElement> &requests,
                       const double &coef_coincedence)
{
#pragma omp atomic write
  cancel_search = false;

  std::shared_lock shlock(base_mtx);
  UDBase result;

  double l_cc;
  if(coef_coincedence >= 1.0 || coef_coincedence < 0.0)
    {
      l_cc = 0.99;
    }
  else
    {
      l_cc = coef_coincedence;
    }

  std::vector<
      std::tuple<UDBElement, std::function<bool(const UDBElement &book_el,
                                                const UDBElement &to_search)>>>
      l_request;
#pragma omp parallel for
  for(auto it = requests.begin(); it != requests.end(); it++)
    {
      UDBElement el = *it;
      BaseID::ID id = bid.getId(el);

      switch(id)
        {
        case BaseID::BookTitle:
          {
            el.content = mlbp->stringToLower(el.content);
            normalizeString(el.content, Normalization::Other);
#pragma omp critical
            {
              l_request.push_back(
                  std::make_tuple(el, std::bind(&BaseKeeper::bookSearch, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2, l_cc)));
            }
            break;
          }
        case BaseID::Author:
          {
            if(el.content.empty())
              {
                for(auto it_sub = el.subelements.begin();
                    it_sub != el.subelements.end(); it_sub++)
                  {
                    it_sub->content = mlbp->stringToLower(it_sub->content);
                    normalizeString(it_sub->content, Normalization::Other);
                  }
                auto it_el = std::find_if(
                    el.subelements.begin(), el.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::LastName;
                      });
                if(it_el != el.subelements.end())
                  {
                    el.content = it_el->content;
                  }

                it_el = std::find_if(
                    el.subelements.begin(), el.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::FirstName;
                      });
                if(it_el != el.subelements.end())
                  {
                    if(!el.content.empty())
                      {
                        el.content += " ";
                      }
                    el.content += it_el->content;
                  }

                it_el = std::find_if(
                    el.subelements.begin(), el.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::MiddleName;
                      });
                if(it_el != el.subelements.end())
                  {
                    if(!el.content.empty())
                      {
                        el.content += " ";
                      }
                    el.content += it_el->content;
                  }

                it_el = std::find_if(
                    el.subelements.begin(), el.subelements.end(),
                    [this](const UDBElement &el)
                      {
                        return bid.getId(el) == BaseID::Nickname;
                      });
                if(it_el != el.subelements.end())
                  {
                    if(!it_el->content.empty())
                      {
                        if(!el.content.empty())
                          {
                            el.content += " aka ";
                          }
                        else
                          {
                            el.content += "aka ";
                          }
                        el.content += it_el->content;
                      }
                  }
              }
            else
              {
                el.content = mlbp->stringToLower(el.content);
                normalizeString(el.content, Normalization::Author);
              }
#pragma omp critical
            {
              l_request.push_back(
                  std::make_tuple(el, std::bind(&BaseKeeper::authorSearch,
                                                this, std::placeholders::_1,
                                                std::placeholders::_2, l_cc)));
            }
            break;
          }
        case BaseID::Sequence:
          {
            std::string name;
            std::string number;
            for(auto it_sub = el.subelements.begin();
                it_sub != el.subelements.end(); it_sub++)
              {
                it_sub->content = mlbp->stringToLower(it_sub->content);
                normalizeString(it_sub->content, Normalization::Other);
                switch(bid.getId(*it_sub))
                  {
                  case BaseID::SequenceName:
                    {
                      name = it_sub->content;
                      break;
                    }
                  case BaseID::SequenceNumber:
                    {
                      number = it_sub->content;
                      break;
                    }
                  default:
                    break;
                  }
              }
            std::string content;
            if(!name.empty())
              {
                if(number.empty())
                  {
                    content = name;
                  }
                else
                  {
                    content = name + " " + number;
                  }
              }
#pragma omp critical
            {
              l_request.push_back(std::make_tuple(
                  el, std::bind(&BaseKeeper::sequenceSearch, this,
                                std::placeholders::_1, std::placeholders::_2,
                                name, number, content, l_cc)));
            }
            break;
          }
        case BaseID::Genre:
          {
            el.content = mlbp->stringToLower(el.content);
            normalizeString(el.content, Normalization::Other);
#pragma omp critical
            {
              l_request.push_back(
                  std::make_tuple(el, std::bind(&BaseKeeper::genreSearch, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2, l_cc)));
            }
            break;
          }
        default:
          break;
        }
    }

  result = std::move(searchElement(
      [l_request, this](const UDBElement &el, UDBase &result)
        {
          if(bid.getId(el) == BaseID::CollectionInfo)
            {
#pragma omp critical
              {
                result.addElement(el);
              }
            }
          else
            {
              searchFunc(el, &result, l_request);
            }
        }));

  bool cncl;
#pragma omp atomic read
  cncl = cancel_search;
  if(cncl)
    {
      result.clearBase();
    }
  result.shrinkToFit();

  return result;
}

void
BaseKeeper::cancelSearch()
{
#pragma omp atomic write
  cancel_search = true;
}

std::string
BaseKeeper::getCollectionType()
{
  std::shared_lock shlock(base_mtx);
  std::string result;

  std::vector<UDBElement>::iterator it
      = std::find_if(base.begin(), base.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::CollectionInfo;
                       });

  if(it == base.end())
    {
      throw std::runtime_error(
          "BaseKeeper::getCollectionType: cannot find collection info");
    }

  std::vector<UDBElement>::iterator it_type
      = std::find_if(it->subelements.begin(), it->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::CollectionType;
                       });

  if(it_type == it->subelements.end())
    {
      throw std::runtime_error(
          "BaseKeeper::getCollectionType: cannot find collection type");
    }

  result = it_type->content;

  return result;
}

std::filesystem::path
BaseKeeper::getBooksDirectory()
{
  std::shared_lock shlock(base_mtx);
  std::filesystem::path result;

  std::vector<UDBElement>::iterator it
      = std::find_if(base.begin(), base.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::CollectionInfo;
                       });

  if(it == base.end())
    {
      throw std::runtime_error(
          "BaseKeeper::getBooksDirectory: cannot find collection info");
    }

  std::vector<UDBElement>::iterator it_type
      = std::find_if(it->subelements.begin(), it->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::BooksDirectory;
                       });

  if(it_type == it->subelements.end())
    {
      throw std::runtime_error(
          "BaseKeeper::getBooksDirectory: cannot find books directory");
    }

  result = std::u8string(it_type->content.begin(), it_type->content.end());

  return result;
}

void
BaseKeeper::editBookEntry(const UDBElement &book_search_result)
{
  auto it_fl = std::find_if(book_search_result.subelements.begin(),
                            book_search_result.subelements.end(),
                            [this](const UDBElement &el)
                              {
                                return bid.getId(el) == BaseID::File;
                              });
  if(it_fl == book_search_result.subelements.end())
    {
      return void();
    }

  auto it_book = std::find_if(book_search_result.subelements.begin(),
                              book_search_result.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::Book;
                                });
  if(it_book == book_search_result.subelements.end())
    {
      return void();
    }

  std::shared_lock shlock(base_mtx);

  std::vector<UDBElement *> files
      = std::move(searchFile(base, it_fl->content));
  if(files.size() == 0)
    {
      return void();
    }

  auto it_path
      = std::find_if(it_book->subelements.begin(), it_book->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::PathInFile;
                       });
  if(it_path == it_book->subelements.end())
    {
      for(size_t i = 0; i < files.size(); i++)
        {
          auto it = std::find_if(files[i]->subelements.begin(),
                                 files[i]->subelements.end(),
                                 [this](const UDBElement &el)
                                   {
                                     return bid.getId(el) == BaseID::Book;
                                   });
          if(it != files[i]->subelements.end())
            {
              *it = *it_book;
            }
        }
    }
  else
    {
      for(size_t i = 0; i < files.size(); i++)
        {
          auto it = std::find_if(
              files[i]->subelements.begin(), files[i]->subelements.end(),
              [this, it_path](const UDBElement &el)
                {
                  if(bid.getId(el) == BaseID::Book)
                    {
                      auto it = std::find_if(
                          el.subelements.begin(), el.subelements.end(),
                          [this, it_path](const UDBElement &el)
                            {
                              if(bid.getId(el) == BaseID::PathInFile)
                                {
                                  if(*it_path == el)
                                    {
                                      return true;
                                    }
                                }
                              return false;
                            });
                      if(it != el.subelements.end())
                        {
                          return true;
                        }
                    }
                  return false;
                });
          if(it != files[i]->subelements.end())
            {
              *it = *it_book;
            }
        }
    }

  std::filesystem::path p
      = current_base_path.parent_path() / mlbp->randomFileName();
  CreateCollection::saveBase(p, *this);
  if(std::filesystem::exists(p))
    {
      std::filesystem::remove_all(current_base_path);
      std::filesystem::rename(p, current_base_path);
    }
}

UDBase
BaseKeeper::searchBooksWithNotes(const std::shared_ptr<NotesKeeper> &notes)
{
#pragma omp atomic write
  cancel_search = false;

  std::shared_lock shlock(base_mtx);
  UDBase result;

  result = std::move(searchInNotes(base, notes));

  bool cncl;
#pragma omp atomic read
  cncl = cancel_search;

  if(cncl)
    {
      result.clearBase();
    }

  auto it = std::find_if(base.begin(), base.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::CollectionInfo;
                           });
  if(it != base.end())
    {
      std::vector<UDBElement> *raw_base = result.getRawBase();
      raw_base->insert(raw_base->begin(), *it);
    }
  else
    {
      throw std::runtime_error(
          "BaseKeeper::searchBooksWithNotes: cannot find CollectionInfo");
    }

  result.shrinkToFit();

  return result;
}

UDBase
BaseKeeper::getAllFiles()
{
#pragma omp atomic write
  cancel_search = false;

  std::shared_lock shlock(base_mtx);
  UDBase result = std::move(getFiles(base));

  bool cncl;
#pragma omp atomic read
  cncl = cancel_search;
  if(cncl)
    {
      result.clearBase();
    }

  auto it = std::find_if(base.begin(), base.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::CollectionInfo;
                           });
  if(it != base.end())
    {
      std::vector<UDBElement> *raw_base = result.getRawBase();
      raw_base->insert(raw_base->begin(), *it);
    }
  else
    {
      throw std::runtime_error(
          "BaseKeeper::getAllFiles: cannot find CollectionInfo");
    }

  result.shrinkToFit();

  return result;
}

UDBase
BaseKeeper::listAllAuthors()
{
#pragma omp atomic write
  cancel_search = false;

  std::shared_lock shlock(base_mtx);

  std::vector<UDBElement> all_auth;
  listAuthors(base, &all_auth);

  UDBase result;

  auto it = std::find_if(base.begin(), base.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::CollectionInfo;
                           });
  if(it != base.end())
    {
      result.addElement(*it);
    }
  shlock.unlock();

  Algorithm alg;
  alg.parallelSort(all_auth.begin(), all_auth.end(),
                   [this](const UDBElement &el1, const UDBElement &el2)
                     {
                       return mlbp->stringToLower(el1.content)
                              <= mlbp->stringToLower(el2.content);
                     });

  std::string search_res;
  for(auto it = all_auth.begin(); it != all_auth.end(); it++)
    {
      if(mlbp->stringToLower(it->content) != search_res)
        {
          result.addElement(*it);
          search_res = mlbp->stringToLower(it->content);
        }
    }

  result.shrinkToFit();

  return result;
}

void
BaseKeeper::exportBase(const std::filesystem::path &result_path)
{
  std::shared_lock shlock(base_mtx);
  if(base.empty())
    {
      return void();
    }

  auto it = std::find_if(base.begin(), base.end(),
                         [this](const UDBElement &el)
                           {
                             return bid.getId(el) == BaseID::CollectionInfo;
                           });
  if(it == base.end())
    {
      throw std::runtime_error("BaseKeeper::exportBase: incorrect base(1)");
    }

  auto it_type
      = std::find_if(it->subelements.begin(), it->subelements.end(),
                     [this](const UDBElement &el)
                       {
                         return bid.getId(el) == BaseID::CollectionType;
                       });
  if(it_type == it->subelements.end())
    {
      throw std::runtime_error(
          "BaseKeeper::exportBase: cannot find collection type");
    }

  if(it_type->content != "native")
    {
      throw std::runtime_error(
          "BaseKeeper::exportBase: incorrect collection type");
    }

  UDBase result(*this);
  shlock.unlock();

  std::vector<UDBElement> *raw_base = result.getRawBase();

  it = std::find_if(raw_base->begin(), raw_base->end(),
                    [this](const UDBElement &el)
                      {
                        switch(bid.getId(el))
                          {
                          case BaseID::File:
                          case BaseID::Dir:
                          case BaseID::Symlink:
                            {
                              return true;
                            }
                          default:
                            return false;
                          }
                      });
  if(it == raw_base->end())
    {
      throw std::runtime_error("BaseKeeper::exportBase: incorrect base(2)");
    }

  if(it->content.empty())
    {
      throw std::runtime_error("BaseKeeper::exportBase: incorrect base(3)");
    }

  std::filesystem::path base_path
      = std::u8string(it->content.begin(), it->content.end());

  UDBElement el;
  bid.setId(el, BaseID::AnchorBasePath);
  std::u8string u8str = base_path.filename().u8string();
  el.content = std::string(u8str.begin(), u8str.end());

  UDBElement type;
  type.id = it->id;
  el.subelements.emplace_back(type);

  base_path = base_path.parent_path();
  setRelativePath(*raw_base, base_path);

  raw_base->emplace_back(el);

  result.writeToFile(result_path);
}

std::filesystem::path
BaseKeeper::getCurrentCollectionBasePath()
{
  std::shared_lock shlock(base_mtx);
  return current_base_path;
}

UDBElement
BaseKeeper::getBaseAnchorFileName(const std::filesystem::path &base_path)
{
  UDBElement result;
  BaseID bid;
  bid.setId(result, BaseID::Error);

  UDBase base;

  base.readFromFile(base_path);

  std::vector<UDBElement> *raw_base = base.getRawBase();

  Algorithm alg;

  auto it
      = alg.parallelFindIf(raw_base->begin(), raw_base->end(),
                           [bid](const UDBElement &el)
                             {
                               return bid.getId(el) == BaseID::AnchorBasePath;
                             });
  if(it != raw_base->end())
    {
      result = *it;
    }

  return result;
}

void
BaseKeeper::importCollectionBase(const std::filesystem::path &source_base_path,
                                 const std::filesystem::path &coll_base_path,
                                 const std::filesystem::path &anchor_file)
{
  UDBase base;
  base.readFromFile(source_base_path);

  std::vector<UDBElement> *raw_base = base.getRawBase();

  BaseID bid;
  Algorithm alg;
  std::u8string u8str = anchor_file.filename().u8string();
  std::string str(u8str.begin(), u8str.end());
  auto it = alg.parallelFindIf(raw_base->begin(), raw_base->end(),
                               [bid, str](const UDBElement &el)
                                 {
                                   switch(bid.getId(el))
                                     {
                                     case BaseID::Dir:
                                     case BaseID::File:
                                     case BaseID::Symlink:
                                       {
                                         return el.content == str;
                                       }
                                     default:
                                       return false;
                                     }
                                 });

  if(it == raw_base->end())
    {
      throw std::runtime_error(
          "BaseKeeper::importCollectionBase: cannot find anchor object");
    }

  raw_base->erase(alg.parallelRemoveIf(raw_base->begin(), raw_base->end(),
                                       [bid](const UDBElement &el)
                                         {
                                           return bid.getId(el)
                                                  == BaseID::AnchorBasePath;
                                         }),
                  raw_base->end());

  std::filesystem::path base_path = anchor_file.parent_path();

  BaseKeeper::setAbsolutePath(*raw_base, base_path);

  std::filesystem::create_directories(coll_base_path.parent_path());
  std::filesystem::remove_all(coll_base_path);

  CreateCollection::saveBase(coll_base_path, base);
}

void
BaseKeeper::loadCollectionLegacy(const std::vector<char> &buf)
{
  uint16_t val16;
  size_t sz_16 = sizeof(val16);
  size_t rb = 0;

  if(rb + sz_16 > buf.size())
    {
      throw std::runtime_error(
          "BaseKeeper::loadCollectionLegacy: incorrect base(1)");
    }
  std::memcpy(&val16, &buf[rb], sz_16);
  rb += sz_16;

  ByteOrder bo;
  bo.setLittle(val16);
  val16 = bo;

  size_t sz = static_cast<size_t>(val16);
  if(rb + sz > buf.size())
    {
      throw std::runtime_error(
          "BaseKeeper::loadCollectionLegacy: incorrect base(2)");
    }
  UDBElement coll_info;
  bid.setId(coll_info, BaseID::CollectionInfo);

  UDBElement el;
  bid.setId(el, BaseID::CollectionType);
  el.content = "legacy";
  coll_info.subelements.emplace_back(el);

  std::string books_path;
  el = UDBElement();
  bid.setId(el, BaseID::BooksDirectory);
  el.content.reserve(sz);
  std::copy(buf.begin() + rb, buf.begin() + rb + sz,
            std::back_inserter(el.content));
  books_path = el.content;
  rb += el.content.size();
  coll_info.subelements.emplace_back(el);

  base.emplace_back(coll_info);

  uint64_t val64;
  size_t sz_64 = sizeof(val64);
#pragma omp parallel
#pragma omp masked
  {
    while(rb < buf.size())
      {
        if(rb + sz_64 > buf.size())
          {
            base.clear();
            throw std::runtime_error(
                "BaseKeeper::loadCollectionLegacy: incorrect base(3)");
          }
        std::memcpy(&val64, &buf[rb], sz_64);
        rb += sz_64;

        bo.setLittle(val64);
        val64 = bo;
        sz = static_cast<size_t>(val64);
        if(rb + sz > buf.size())
          {
            base.clear();
            throw std::runtime_error(
                "BaseKeeper::loadCollectionLegacy: incorrect base(4)");
          }
        std::string str;
        str.reserve(sz);
        std::copy(buf.begin() + rb, buf.begin() + rb + sz,
                  std::back_inserter(str));
        rb += str.size();

#pragma omp task
        {
          parseFileEntryLegacy(str, books_path);
        }
      }
  }
}

void
BaseKeeper::parseFileEntryLegacy(const std::string &buf,
                                 const std::string &books_path)
{
  size_t rb = 0;
  uint16_t val16;
  size_t sz_16 = sizeof(val16);
  if(rb + sz_16 > buf.size())
    {
      base.clear();
      throw std::runtime_error(
          "BaseKeeper::parseFileEntryLegacy: incorrect buffer(1)");
    }
  std::memcpy(&val16, &buf[rb], sz_16);
  rb += sz_16;

  ByteOrder bo;
  bo.setLittle(val16);
  val16 = bo;
  size_t sz = static_cast<size_t>(val16);
  if(rb + sz > buf.size())
    {
      base.clear();
      throw std::runtime_error(
          "BaseKeeper::parseFileEntryLegacy: incorrect buffer(2)");
    }
  UDBElement fl;
  bid.setId(fl, BaseID::File);
  fl.content.reserve(sz);
  std::copy(buf.begin() + rb, buf.begin() + rb + sz,
            std::back_inserter(fl.content));
  rb += sz;

  std::filesystem::path fl_path
      = std::filesystem::path(
            std::u8string(books_path.begin(), books_path.end()))
        / std::u8string(fl.content.begin(), fl.content.end());
  std::u8string u8str = fl_path.u8string();
  fl.content = std::string(u8str.begin(), u8str.end());

  if(rb + sz_16 > buf.size())
    {
      base.clear();
      throw std::runtime_error(
          "BaseKeeper::parseFileEntryLegacy: incorrect buffer(3)");
    }
  std::memcpy(&val16, &buf[rb], sz_16);
  rb += sz_16;

  bo.setLittle(val16);
  val16 = bo;
  sz = static_cast<size_t>(val16);
  if(rb + sz > buf.size())
    {
      base.clear();
      throw std::runtime_error(
          "BaseKeeper::parseFileEntryLegacy: incorrect buffer(4)");
    }
  UDBElement hash;
  bid.setId(hash, BaseID::FileHash);
  hash.content.reserve(sz);
  std::copy(buf.begin() + rb, buf.begin() + rb + sz,
            std::back_inserter(hash.content));
  rb += sz;

  fl.subelements.emplace_back(hash);

  uint64_t val64;
  size_t sz_64 = sizeof(val64);
  while(rb < buf.size())
    {
      if(rb + sz_64 > buf.size())
        {
          base.clear();
          throw std::runtime_error(
              "BaseKeeper::parseFileEntryLegacy: incorrect buffer(5)");
        }
      std::memcpy(&val64, &buf[rb], sz_64);
      rb += sz_64;

      bo.setLittle(val64);
      val64 = bo;
      sz = static_cast<size_t>(val64);
      if(rb + sz > buf.size())
        {
          base.clear();
          throw std::runtime_error(
              "BaseKeeper::parseFileEntryLegacy: incorrect buffer(6)");
        }
      std::string book_buf;
      book_buf.reserve(sz);
      std::copy(buf.begin() + rb, buf.begin() + rb + sz,
                std::back_inserter(book_buf));
      rb += sz;
      parseBookEntryLegacy(book_buf, fl.subelements);
    }

  fl.subelements.shrink_to_fit();
#pragma omp critical
  {
    base.emplace_back(fl);
  }
}

void
BaseKeeper::parseBookEntryLegacy(const std::string &buf,
                                 std::vector<UDBElement> &result)
{
  size_t rb = 0;
  uint16_t val16;
  size_t sz_16 = sizeof(val16);

  if(rb + sz_16 > buf.size())
    {
      base.clear();
      throw std::runtime_error(
          "BaseKeeper::parseBookEntryLegacy: incorrect buffer (1)");
    }
  std::memcpy(&val16, &buf[rb], sz_16);
  rb += sz_16;

  ByteOrder bo;
  bo.setLittle(val16);
  val16 = bo;
  size_t sz = static_cast<size_t>(val16);
  if(rb + sz > buf.size())
    {
      base.clear();
      throw std::runtime_error(
          "BaseKeeper::parseBookEntryLegacy: incorrect buffer (2)");
    }
  std::string str;
  str.reserve(sz);
  std::copy(buf.begin() + rb, buf.begin() + rb + sz, std::back_inserter(str));
  rb += sz;

  UDBElement book;
  bid.setId(book, BaseID::Book);

  if(str.size() > 0)
    {
      UDBElement path;
      bid.setId(path, BaseID::PathInFile);
      UDBElement *prev = &path;
      std::string::size_type n = 0;
      std::string find_str("\n");
      while(n != std::string::npos)
        {
          n = str.find(find_str);
          if(n != std::string::npos)
            {
              prev->content = str.substr(0, n);
              str.erase(0, n + find_str.size());
              UDBElement el;
              bid.setId(el, BaseID::PathInFile);
              prev->subelements.emplace_back(el);
              prev = &prev->subelements[prev->subelements.size() - 1];
            }
          else if(str.size() > 0)
            {
              prev->content = str;
            }
        }
      book.subelements.emplace_back(path);
    }

  for(int i = 1; i <= 5; i++)
    {
      if(rb + sz_16 > buf.size())
        {
          base.clear();
          throw std::runtime_error(
              "BaseKeeper::parseBookEntryLegacy: incorrect buffer (3)");
        }
      std::memcpy(&val16, &buf[rb], sz_16);
      rb += sz_16;

      bo.setLittle(val16);
      val16 = bo;
      sz = static_cast<size_t>(val16);
      if(rb + sz > buf.size())
        {
          base.clear();
          throw std::runtime_error(
              "BaseKeeper::parseBookEntryLegacy: incorrect buffer (4)");
        }
      str.clear();
      str.reserve(sz);
      std::copy(buf.begin() + rb, buf.begin() + rb + sz,
                std::back_inserter(str));
      rb += sz;

      if(str.size() > 0)
        {
          BaseID::ID id;
          switch(i)
            {
            case 1:
              {
                id = BaseID::Author;
                break;
              }
            case 2:
              {
                id = BaseID::BookTitle;
                break;
              }
            case 3:
              {
                id = BaseID::Sequence;
                break;
              }
            case 4:
              {
                id = BaseID::Genre;
                break;
              }
            case 5:
              {
                id = BaseID::Date;
                break;
              }
            }
          std::string find_str(",");
          std::string::size_type n = str.find(find_str);
          if(n == std::string::npos)
            {
              UDBElement el;
              bid.setId(el, id);
              el.content = str;
              for(auto it = el.content.begin(); it != el.content.end();)
                {
                  if(*it >= 0 && *it <= ' ')
                    {
                      el.content.erase(it);
                    }
                  else
                    {
                      break;
                    }
                }
              while(el.content.size() > 0)
                {
                  char v = *el.content.rbegin();
                  if(v >= 0 && v <= ' ')
                    {
                      el.content.pop_back();
                    }
                  else
                    {
                      break;
                    }
                }
              if(!el.content.empty())
                {
                  book.subelements.emplace_back(el);
                }
            }
          else
            {
              while(n != std::string::npos)
                {
                  UDBElement el;
                  bid.setId(el, id);
                  if(id == BaseID::BookTitle)
                    {
                      el.content = str;
                    }
                  else
                    {
                      el.content = str.substr(0, n);
                    }
                  for(auto it = el.content.begin(); it != el.content.end();)
                    {
                      if(*it >= 0 && *it <= ' ')
                        {
                          el.content.erase(it);
                        }
                      else
                        {
                          break;
                        }
                    }
                  while(el.content.size() > 0)
                    {
                      char v = *el.content.rbegin();
                      if(v >= 0 && v <= ' ')
                        {
                          el.content.pop_back();
                        }
                      else
                        {
                          break;
                        }
                    }
                  if(!el.content.empty())
                    {
                      el.content.shrink_to_fit();
                      book.subelements.emplace_back(el);
                    }
                  str.erase(0, n + find_str.size());
                  n = str.find(find_str);
                }
            }
        }
    }
  result.emplace_back(book);
}

UDBase
BaseKeeper::searchElement(
    std::function<void(const UDBElement &, UDBase &)> search_function)
{
  UDBase result;
  if(!search_function.operator bool())
    {
      return result;
    }

#pragma omp parallel
#pragma omp masked
  for(auto it = base.begin(); it != base.end(); it++)
    {
      bool cncl;
#pragma omp atomic read
      cncl = cancel_search;
      if(cncl)
        {
          break;
        }
#pragma omp task
      {
        search_function(*it, result);
      }
    }

  return result;
}

void
BaseKeeper::searchFunc(
    const UDBElement &el, UDBase *result,
    const std::vector<std::tuple<
        UDBElement,
        std::function<bool(const UDBElement &, const UDBElement &)>>>
        &l_request)
{
  if(bid.getId(el) == BaseID::File)
    {
#pragma omp task
      {
        if(l_request.size() > 0)
          {
            for(auto it_sub = el.subelements.begin();
                it_sub != el.subelements.end(); it_sub++)
              {
                if(bid.getId(*it_sub) != BaseID::Book)
                  {
                    continue;
                  }
                bool res = true;
                for(auto it_r = l_request.begin(); it_r != l_request.end();
                    it_r++)
                  {
                    if(!std::get<1>(*it_r)(*it_sub, std::get<0>(*it_r)))
                      {
                        res = false;
                        break;
                      }
                  }
                if(res)
                  {
                    UDBElement s_res;
                    bid.setId(s_res, BaseID::BookSearchResult);
                    s_res.subelements.reserve(2);

                    UDBElement fl;
                    fl.id = el.id;
                    fl.content = el.content;
                    s_res.subelements.emplace_back(fl);

                    s_res.subelements.push_back(*it_sub);
#pragma omp critical
                    {
                      result->addElement(s_res);
                    }
                  }
              }
          }
        else
          {
            for(auto it_sub = el.subelements.begin();
                it_sub != el.subelements.end(); it_sub++)
              {
                if(bid.getId(*it_sub) != BaseID::Book)
                  {
                    continue;
                  }
                UDBElement s_res;
                bid.setId(s_res, BaseID::BookSearchResult);
                s_res.subelements.reserve(2);

                UDBElement fl;
                fl.id = el.id;
                fl.content = el.content;
                s_res.subelements.emplace_back(fl);

                s_res.subelements.push_back(*it_sub);
#pragma omp critical
                {
                  result->addElement(s_res);
                }
              }
          }
      }
    }
  else
    {
      for(auto it = el.subelements.begin(); it != el.subelements.end(); it++)
        {
          bool cncl;
#pragma omp atomic read
          cncl = cancel_search;
          if(cncl)
            {
              break;
            }
          searchFunc(*it, result, l_request);
        }
    }
}

bool
BaseKeeper::authorSearch(const UDBElement &book_el,
                         const UDBElement &to_search,
                         const double &coef_coincidence)
{

  if(to_search.subelements.size() == 0)
    {
      for(auto it = book_el.subelements.begin();
          it != book_el.subelements.end(); it++)
        {
          if(it->id != to_search.id)
            {
              continue;
            }
          if(it->content.empty())
            {
              std::string str;
              auto it_auth = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::LastName;
                    });
              if(it_auth != it->subelements.end())
                {
                  str += it_auth->content;
                }

              it_auth = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::FirstName;
                    });
              if(it_auth != it->subelements.end())
                {
                  if(!str.empty() && !it_auth->content.empty())
                    {
                      str += " ";
                    }
                  str += it_auth->content;
                }

              it_auth = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::MiddleName;
                    });
              if(it_auth != it->subelements.end())
                {
                  if(!str.empty() && !it_auth->content.empty())
                    {
                      str += " ";
                    }
                  str += it_auth->content;
                }

              it_auth = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::Nickname;
                    });
              if(it_auth != it->subelements.end())
                {
                  if(!str.empty() && !it_auth->content.empty())
                    {
                      str += " aka ";
                    }
                  str += it_auth->content;
                }

              str = mlbp->stringToLower(str);
              normalizeString(str, Normalization::Author);
              if(str == to_search.content)
                {
                  return true;
                }
            }
          else
            {
              std::string str = mlbp->stringToLower(it->content);
              normalizeString(str, Normalization::Author);
              if(str == to_search.content)
                {
                  return true;
                }
            }
        }
    }
  else
    {
      for(auto it = book_el.subelements.begin();
          it != book_el.subelements.end(); it++)
        {
          if(it->id != to_search.id)
            {
              continue;
            }
          if(it->content.empty())
            {
              for(auto it_sub = to_search.subelements.begin();
                  it_sub != to_search.subelements.end(); it_sub++)
                {
                  auto it_book = std::find_if(it->subelements.begin(),
                                              it->subelements.end(),
                                              [it_sub](const UDBElement &el)
                                                {
                                                  return it_sub->id == el.id;
                                                });
                  if(it_book != it->subelements.end())
                    {
                      if(!searchLineFunc(it_sub->content, it_book->content,
                                         coef_coincidence))
                        {
                          return false;
                        }
                    }
                  else
                    {
                      return false;
                    }
                }
              return true;
            }
          else
            {
              if(searchLineFunc(to_search.content, it->content,
                                coef_coincidence, Normalization::Author))
                {
                  return true;
                }
            }
        }
    }
  return false;
}

bool
BaseKeeper::bookSearch(const UDBElement &book_el, const UDBElement &to_search,
                       const double &coef_coincidence)
{
  if(to_search.content.empty())
    {
      return false;
    }

  for(auto it = book_el.subelements.begin(); it != book_el.subelements.end();
      it++)
    {
      if(it->id != to_search.id)
        {
          continue;
        }
      if(searchLineFunc(to_search.content, it->content, coef_coincidence))
        {
          return true;
        }
    }

  return false;
}

bool
BaseKeeper::sequenceSearch(const UDBElement &book_el,
                           const UDBElement &to_search,
                           const std::string &name, const std::string &number,
                           const std::string &content,
                           const double &coef_coincidence)
{
  if(to_search.subelements.size() == 0)
    {
      return false;
    }

  for(auto it = book_el.subelements.begin(); it != book_el.subelements.end();
      it++)
    {
      if(it->id != to_search.id)
        {
          continue;
        }
      if(it->content.empty())
        {
          auto it_sub = std::find_if(
              it->subelements.begin(), it->subelements.end(),
              [name, coef_coincidence, this](const UDBElement &el)
                {
                  if(bid.getId(el) == BaseID::SequenceName)
                    {
                      return searchLineFunc(name, el.content,
                                            coef_coincidence);
                    }
                  return false;
                });
          if(it_sub != it->subelements.end())
            {
              if(number.empty())
                {
                  return true;
                }
              it_sub = std::find_if(
                  it->subelements.begin(), it->subelements.end(),
                  [number, this](const UDBElement &el)
                    {
                      if(bid.getId(el) == BaseID::SequenceNumber)
                        {
                          return el.content == number;
                        }
                      return false;
                    });
              if(it_sub != it->subelements.end())
                {
                  return true;
                }
            }
        }
      else
        {
          if(searchLineFunc(content, it->content, coef_coincidence))
            {
              return true;
            }
        }
    }

  return false;
}

bool
BaseKeeper::genreSearch(const UDBElement &book_el, const UDBElement &to_search,
                        const double &coef_coincidence)
{
  if(to_search.content.empty())
    {
      return false;
    }

  for(auto it = book_el.subelements.begin(); it != book_el.subelements.end();
      it++)
    {
      if(it->id != to_search.id)
        {
          continue;
        }
      if(searchLineFunc(to_search.content, it->content, coef_coincidence))
        {
          return true;
        }
    }

  return false;
}

std::vector<UDBElement *>
BaseKeeper::searchFile(const std::vector<UDBElement> &src,
                       const std::string &file_path)
{
  std::vector<UDBElement *> result;

#pragma omp parallel
#pragma omp masked
  {
    for(const UDBElement *i = src.data(); i < src.data() + src.size(); i++)
      {
        if(bid.getId(*i) == BaseID::File)
          {
            if(i->content == file_path)
              {
                result.push_back(const_cast<UDBElement *>(i));
              }
          }
        else
          {
#pragma omp task
            {
              std::vector<UDBElement *> res
                  = std::move(searchFile(i->subelements, file_path));
#pragma omp critical
              {
                std::copy(res.begin(), res.end(), std::back_inserter(result));
              }
            }
          }
      }
  }

  return result;
}

bool
BaseKeeper::searchLineFunc(const std::string &to_search,
                           const std::string &source,
                           const double &coef_coincidence,
                           const Normalization &variant)
{
  std::string loc_source = mlbp->stringToLower(source);
  normalizeString(loc_source, variant);

  if(loc_source.size() == 0 || to_search.size() == 0
     || to_search.size() > loc_source.size())
    {
      return false;
    }

  double weight;
  double incr = 1.0 / static_cast<double>(to_search.size());
  for(auto it = loc_source.begin();
      it
      != loc_source.begin() + loc_source.size()
             - (to_search.size() * (1 - coef_coincidence));
      it++)
    {
      if(it == loc_source.begin())
        {
          weight = 0.0;
          for(size_t i = 0; i < to_search.size() && it + i != loc_source.end();
              i++)
            {
              if(*(it + i) == to_search[i])
                {
                  weight += incr;
                }
              else
                {
                  break;
                }
              if(weight >= coef_coincidence)
                {
                  return true;
                }
            }
        }
      else if(*it == ' ')
        {
          if(it + 1 != loc_source.end())
            {
              weight = 0.0;
              for(size_t i = 0;
                  i < to_search.size() && it + i + 1 != loc_source.end(); i++)
                {
                  if(*(it + i + 1) == to_search[i])
                    {
                      weight += incr;
                    }
                  else
                    {
                      break;
                    }
                  if(weight >= coef_coincidence)
                    {
                      return true;
                    }
                }
            }
        }
    }

  return false;
}

void
BaseKeeper::normalizeString(std::string &str, const Normalization &variant)
{
  for(auto it = str.begin(); it != str.end();)
    {
      char el = *it;
      if(el >= 0 && el <= 32)
        {
          str.erase(it);
        }
      else
        {
          break;
        }
    }

  while(str.size() > 0)
    {
      char el = *str.rbegin();
      if(el >= 0 && el <= 32)
        {
          str.pop_back();
        }
      else
        {
          break;
        }
    }

  std::string find_str = "  ";
  std::string::size_type n = 0;
  for(;;)
    {
      n = str.find(find_str, n);
      if(n == std::string::npos)
        {
          break;
        }
      str.erase(str.begin() + n);
    }

  if(variant == Normalization::Author)
    {
      find_str = ". ";
      n = 0;
      for(;;)
        {
          n = str.find(find_str, n);
          if(n == std::string::npos)
            {
              break;
            }
          str.erase(str.begin() + n + 1);
        }
    }
  str.shrink_to_fit();
}

size_t
BaseKeeper::booksQuantity(const std::vector<UDBElement> &items)
{
  size_t result = 0;

#pragma omp parallel for
  for(auto it = items.begin(); it != items.end(); it++)
    {
      if(bid.getId(*it) == BaseID::File)
        {
          for(auto it_sub = it->subelements.begin();
              it_sub != it->subelements.end(); it_sub++)
            {
              if(bid.getId(*it_sub) == BaseID::Book)
                {
#pragma omp atomic update
                  result++;
                }
            }
        }
      else
        {
          size_t res = booksQuantity(it->subelements);
#pragma omp atomic update
          result += res;
        }
    }

  return result;
}

size_t
BaseKeeper::filesQuantity(const std::vector<UDBElement> &items)
{
  size_t result = 0;

#pragma omp parallel for
  for(auto it = items.begin(); it != items.end(); it++)
    {
      if(bid.getId(*it) == BaseID::File)
        {
#pragma omp atomic update
          result++;
        }
      else
        {
          size_t quan = filesQuantity(it->subelements);
#pragma omp atomic update
          result += quan;
        }
    }

  return result;
}

UDBase
BaseKeeper::searchInNotes(const std::vector<UDBElement> &src,
                          const std::shared_ptr<NotesKeeper> &notes)
{
  UDBase result;

  const std::vector<UDBElement> *raw_base = notes->getRawBase();
#pragma omp parallel
#pragma omp for
  for(auto it = src.begin(); it != src.end(); it++)
    {
      bool cncl;
#pragma omp atomic read
      cncl = cancel_search;
      if(cncl)
        {
#pragma omp cancel for
          continue;
        }
      if(bid.getId(*it) == BaseID::File)
        {
          std::filesystem::path fp
              = std::u8string(it->content.begin(), it->content.end());
          for(auto it_b = it->subelements.begin();
              it_b != it->subelements.end(); it_b++)
            {
              if(bid.getId(*it_b) != BaseID::Book)
                {
                  continue;
                }
              auto it_path = std::find_if(
                  it_b->subelements.begin(), it_b->subelements.end(),
                  [this](const UDBElement &el)
                    {
                      return bid.getId(el) == BaseID::PathInFile;
                    });
              if(it_path == it_b->subelements.end())
                {
                  auto it_note = std::find_if(
                      raw_base->begin(), raw_base->end(),
                      [this, fp](const UDBElement &el)
                        {
                          auto it = std::find_if(
                              el.subelements.begin(), el.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::File;
                                });
                          if(it == el.subelements.end())
                            {
                              return false;
                            }
                          std::filesystem::path lfp = std::u8string(
                              it->content.begin(), it->content.end());
                          return fp == lfp;
                        });
                  if(it_note == raw_base->end())
                    {
                      continue;
                    }
                  UDBElement bsr;
                  bid.setId(bsr, BaseID::BookSearchResult);

                  UDBElement el;
                  bid.setId(el, BaseID::File);
                  el.content = it->content;
                  bsr.subelements.emplace_back(el);

                  bsr.subelements.push_back(*it_b);
#pragma omp critical
                  {
                    result.addElement(bsr);
                  }
                }
              else
                {
                  auto it_note = std::find_if(
                      raw_base->begin(), raw_base->end(),
                      [this, it_path, fp](const UDBElement &el)
                        {
                          auto it = std::find_if(
                              el.subelements.begin(), el.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::File;
                                });
                          if(it == el.subelements.end())
                            {
                              return false;
                            }
                          std::filesystem::path lfp = std::u8string(
                              it->content.begin(), it->content.end());
                          if(lfp != fp)
                            {
                              return false;
                            }

                          it = std::find_if(
                              el.subelements.begin(), el.subelements.end(),
                              [this](const UDBElement &el)
                                {
                                  return bid.getId(el) == BaseID::PathInFile;
                                });
                          if(it == el.subelements.end())
                            {
                              return false;
                            }
                          return *it_path == *it;
                        });
                  if(it_note == raw_base->end())
                    {
                      continue;
                    }
                  UDBElement bsr;
                  bid.setId(bsr, BaseID::BookSearchResult);

                  UDBElement el;
                  bid.setId(el, BaseID::File);
                  el.content = it->content;
                  bsr.subelements.emplace_back(el);

                  bsr.subelements.push_back(*it_b);
#pragma omp critical
                  {
                    result.addElement(bsr);
                  }
                }
            }
        }
      else
        {
          UDBase res = std::move(searchInNotes(it->subelements, notes));
#pragma omp critical
          {
            result += res;
          }
        }
    }

  return result;
}

UDBase
BaseKeeper::getFiles(const std::vector<UDBElement> &src)
{
  UDBase result;

#pragma omp parallel
#pragma omp for
  for(auto it = src.begin(); it != src.end(); it++)
    {
      bool cncl;
#pragma omp atomic read
      cncl = cancel_search;

      if(cncl)
        {
#pragma omp cancel for
          continue;
        }

      if(bid.getId(*it) == BaseID::File)
        {
#pragma omp critical
          {
            result.addElement(*it);
          }
        }
      else
        {
          UDBase res = std::move(getFiles(it->subelements));
#pragma omp critical
          {
            result += res;
          }
        }
    }

  return result;
}

void
BaseKeeper::listAuthors(const std::vector<UDBElement> &src,
                        std::vector<UDBElement> *result)
{
  for(auto it = src.begin(); it != src.end(); it++)
    {
      bool cncl;
#pragma omp atomic read
      cncl = cancel_search;
      if(cncl)
        {
          break;
        }
      if(bid.getId(*it) == BaseID::File)
        {
          for(auto it_fl = it->subelements.begin();
              it_fl != it->subelements.end(); it_fl++)
            {
#pragma omp atomic read
              cncl = cancel_search;
              if(cncl)
                {
                  break;
                }
              if(bid.getId(*it_fl) != BaseID::Book)
                {
                  continue;
                }
              for(auto it_book = it_fl->subelements.begin();
                  it_book != it_fl->subelements.end(); it_book++)
                {
                  if(bid.getId(*it_book) != BaseID::Author)
                    {
                      continue;
                    }

                  std::string add_str;
                  if(it_book->content.empty())
                    {
                      auto it_auth = std::find_if(
                          it_book->subelements.begin(),
                          it_book->subelements.end(),
                          [this](const UDBElement &el)
                            {
                              return bid.getId(el) == BaseID::LastName;
                            });
                      if(it_auth != it_book->subelements.end())
                        {
                          add_str += it_auth->content;
                        }

                      it_auth = std::find_if(it_book->subelements.begin(),
                                             it_book->subelements.end(),
                                             [this](const UDBElement &el)
                                               {
                                                 return bid.getId(el)
                                                        == BaseID::FirstName;
                                               });
                      if(it_auth != it_book->subelements.end())
                        {
                          if(!add_str.empty() && !it_auth->content.empty())
                            {
                              add_str += " ";
                            }
                          add_str += it_auth->content;
                        }

                      it_auth = std::find_if(it_book->subelements.begin(),
                                             it_book->subelements.end(),
                                             [this](const UDBElement &el)
                                               {
                                                 return bid.getId(el)
                                                        == BaseID::MiddleName;
                                               });
                      if(it_auth != it_book->subelements.end())
                        {
                          if(!add_str.empty() && !it_auth->content.empty())
                            {
                              add_str += " ";
                            }
                          add_str += it_auth->content;
                        }

                      it_auth = std::find_if(it_book->subelements.begin(),
                                             it_book->subelements.end(),
                                             [this](const UDBElement &el)
                                               {
                                                 return bid.getId(el)
                                                        == BaseID::Nickname;
                                               });
                      if(it_auth != it_book->subelements.end())
                        {
                          if(!add_str.empty() && !it_auth->content.empty())
                            {
                              add_str += " aka ";
                            }
                          add_str += it_auth->content;
                        }
                    }
                  else
                    {
                      add_str = it_book->content;
                    }
                  normalizeString(add_str, Normalization::Author);
                  UDBElement el;
                  bid.setId(el, BaseID::AuthorSearchResult);
                  el.content = add_str;
                  result->emplace_back(el);
                }
            }
        }
      else
        {
          listAuthors(it->subelements, result);
        }
    }
}

void
BaseKeeper::setRelativePath(std::vector<UDBElement> &src,
                            const std::filesystem::path &base_path)
{
#pragma omp parallel for
  for(auto it = src.begin(); it != src.end(); it++)
    {
      switch(bid.getId(*it))
        {
        case BaseID::File:
          {
            std::filesystem::path p
                = std::u8string(it->content.begin(), it->content.end());
            std::filesystem::path rel = p.lexically_relative(base_path);
            std::u8string u8str = rel.u8string();
            it->content = std::string(u8str.begin(), u8str.end());
#ifdef _WIN32
            std::replace(it->content.begin(), it->content.end(), '\\', '/');
#endif
            break;
          }
        case BaseID::Dir:
        case BaseID::Symlink:
          {
            std::filesystem::path p
                = std::u8string(it->content.begin(), it->content.end());
            std::filesystem::path rel = p.lexically_relative(base_path);
            std::u8string u8str = rel.u8string();
            it->content = std::string(u8str.begin(), u8str.end());
#ifdef _WIN32
            std::replace(it->content.begin(), it->content.end(), '\\', '/');
#endif
            setRelativePath(it->subelements, base_path);
            break;
          }
        default:
          {
            setRelativePath(it->subelements, base_path);
            break;
          }
        }
    }
}

void
BaseKeeper::setAbsolutePath(std::vector<UDBElement> &src,
                            const std::filesystem::path &base_path)
{
  BaseID bid;
#pragma omp parallel for
  for(auto it = src.begin(); it != src.end(); it++)
    {
      switch(bid.getId(*it))
        {
        case BaseID::File:
          {
            std::filesystem::path p
                = std::u8string(it->content.begin(), it->content.end());
            p = base_path / p;
            std::filesystem::path res = p.lexically_normal();
            std::u8string u8str = res.u8string();
#ifdef _WIN32
            if(*u8str.begin() == '\\')
              {
                u8str.insert(u8str.begin(), '\\');
              }
#endif
            it->content = std::string(u8str.begin(), u8str.end());
            break;
          }
        case BaseID::Dir:
        case BaseID::Symlink:
          {
            std::filesystem::path p
                = std::u8string(it->content.begin(), it->content.end());
            p = base_path / p;
            std::filesystem::path res = p.lexically_normal();
            std::u8string u8str = res.u8string();
#ifdef _WIN32
            if(*u8str.begin() == '\\')
              {
                u8str.insert(u8str.begin(), '\\');
              }
#endif
            it->content = std::string(u8str.begin(), u8str.end());
            BaseKeeper::setAbsolutePath(it->subelements, base_path);
            break;
          }
        default:
          {
            BaseKeeper::setAbsolutePath(it->subelements, base_path);
            break;
          }
        }
    }
}
