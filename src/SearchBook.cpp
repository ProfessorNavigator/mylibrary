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

#include "SearchBook.h"

SearchBook::SearchBook(
    std::string collnm,
    std::string surnm,
    std::string name,
    std::string secname,
    std::string book,
    std::string series,
    std::string genre,
    std::string *prev_search_nm,
    std::vector<
	std::tuple<std::string, std::string, std::string, std::string,
	    std::string, std::string>> *base_v,
    std::vector<
	std::tuple<std::string, std::string, std::string, std::string,
	    std::string, std::string>> *search_result_v,
    std::shared_ptr<int> cancel)
{
  this->collnm = collnm;
  this->surnm = surnm;
  this->name = name;
  this->secname = secname;
  this->book = book;
  this->series = series;
  this->genre = genre;
  this->prev_search_nm = prev_search_nm;
  this->base_v = base_v;
  this->search_result_v = search_result_v;
  if(this->genre == "nill")
    {
      this->genre.clear();
    }
  this->cancel = cancel;
}

SearchBook::~SearchBook()
{
  // TODO Auto-generated destructor stub
}

void
SearchBook::searchBook()
{
  if(collnm != *prev_search_nm)
    {
      *prev_search_nm = collnm;
      base_v->clear();
      AuxFunc af;
      std::string filename;
      af.homePath(&filename);
      filename = filename + "/.MyLibrary/Collections/" + collnm;
      std::filesystem::path filepath = std::filesystem::u8path(filename);
      if(std::filesystem::exists(filepath))
	{
	  filename = filename + "/fb2base";
	  filepath = std::filesystem::u8path(filename);
	  readBase(filepath);
	  if(*cancel == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/zipbase";
	  filepath = std::filesystem::u8path(filename);
	  readZipBase(filepath);
	  if(*cancel == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/epubbase";
	  filepath = std::filesystem::u8path(filename);
	  readBase(filepath);
	  if(*cancel == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/pdfbase";
	  filepath = std::filesystem::u8path(filename);
	  readBase(filepath);
	  if(*cancel == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/djvubase";
	  filepath = std::filesystem::u8path(filename);
	  readBase(filepath);
	}
      else
	{
	  std::cerr << "Search error: collection not found" << std::endl;
	}
    }
}

void
SearchBook::cleanSearchV()
{
  if(*cancel != 1)
    {
      *search_result_v = *base_v;
      std::string searchstr = surnm;
      AuxFunc af;
      af.stringToLower(searchstr);

      if(!surnm.empty() && *cancel == 0)
	{
	  search_result_v->erase(
	      std::remove_if(search_result_v->begin(), search_result_v->end(),
			     [searchstr]
			     (auto &el)
			       {
				 std::string line = std::get<0>(el);
				 AuxFunc af;
				 af.stringToLower(line);
				 std::string::size_type n;
				 n = line.find(searchstr);
				 if(n != std::string::npos)
				   {
				     return false;
				   }
				 else
				   {
				     return true;
				   }
			       }),
	      search_result_v->end());
	}

      searchstr = name;
      af.stringToLower(searchstr);
      if(!name.empty() && *cancel == 0)
	{
	  search_result_v->erase(
	      std::remove_if(search_result_v->begin(), search_result_v->end(),
			     [searchstr]
			     (auto &el)
			       {
				 std::string line = std::get<0>(el);
				 AuxFunc af;
				 af.stringToLower(line);
				 std::string::size_type n;
				 n = line.find(searchstr);
				 if(n != std::string::npos)
				   {
				     return false;
				   }
				 else
				   {
				     return true;
				   }
			       }),
	      search_result_v->end());
	}

      searchstr = secname;
      af.stringToLower(searchstr);
      if(!secname.empty() && *cancel == 0)
	{
	  search_result_v->erase(
	      std::remove_if(search_result_v->begin(), search_result_v->end(),
			     [searchstr]
			     (auto &el)
			       {
				 std::string line = std::get<0>(el);
				 AuxFunc af;
				 af.stringToLower(line);
				 std::string::size_type n;
				 n = line.find(searchstr);
				 if(n != std::string::npos)
				   {
				     return false;
				   }
				 else
				   {
				     return true;
				   }
			       }),
	      search_result_v->end());
	}

      searchstr = book;
      af.stringToLower(searchstr);
      if(!book.empty() && *cancel == 0)
	{
	  search_result_v->erase(
	      std::remove_if(search_result_v->begin(), search_result_v->end(),
			     [searchstr]
			     (auto &el)
			       {
				 std::string line = std::get<1>(el);
				 AuxFunc af;
				 af.stringToLower(line);
				 std::string::size_type n;
				 n = line.find(searchstr);
				 if(n != std::string::npos)
				   {
				     return false;
				   }
				 else
				   {
				     return true;
				   }
			       }),
	      search_result_v->end());
	}

      searchstr = series;
      af.stringToLower(searchstr);
      if(!series.empty() && *cancel == 0)
	{
	  search_result_v->erase(
	      std::remove_if(search_result_v->begin(), search_result_v->end(),
			     [searchstr]
			     (auto &el)
			       {
				 std::string line = std::get<2>(el);
				 AuxFunc af;
				 af.stringToLower(line);
				 std::string::size_type n;
				 n = line.find(searchstr);
				 if(n != std::string::npos)
				   {
				     return false;
				   }
				 else
				   {
				     return true;
				   }
			       }),
	      search_result_v->end());
	}

      searchstr = genre;
      if(!genre.empty() && *cancel == 0)
	{
	  search_result_v->erase(
	      std::remove_if(
		  search_result_v->begin(), search_result_v->end(), [searchstr]
		  (auto &el)
		    {
		      std::string genrestr = std::get<3>(el);
		      std::vector<std::string> genre_v;
		      std::string::size_type n = 0;
		      while (n != std::string::npos)
			{
			  std::string s_s = genrestr;
			  n = s_s.find(", ");
			  if(n != std::string::npos)
			    {
			      s_s = s_s.substr(0, n);
			      genre_v.push_back(s_s);
			      s_s = s_s + ", ";
			      genrestr.erase(0,
				  genrestr.find(s_s) + s_s.size());
			    }
			  else
			    {
			      if(!genrestr.empty())
				{
				  genre_v.push_back(genrestr);
				}
			    }
			}

		      auto itgv = std::find(genre_v.begin(), genre_v.end(),
			  searchstr);
		      if(itgv != genre_v.end())
			{
			  return false;
			}
		      else
			{
			  return true;
			}
		    }),
	      search_result_v->end());
	}
      std::sort(search_result_v->begin(), search_result_v->end(), [&af]
      (auto &el1, auto &el2)
	{
	  std::string line1, line2;
	  line1 = std::get<0>(el1);
	  line2 = std::get<0>(el2);
	  af.stringToLower(line1);
	  af.stringToLower(line2);
	  if(line1 < line2)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
      std::for_each(search_result_v->begin(), search_result_v->end(), []
      (auto &el)
	{
	  std::string genrestr = std::get<3>(el);
	  std::vector<std::string> genre_v;
	  std::string::size_type n = 0;
	  while (n != std::string::npos)
	    {
	      std::string s_s = genrestr;
	      n = s_s.find(", ");
	      if(n != std::string::npos)
		{
		  s_s = s_s.substr(0, n);
		  genre_v.push_back(s_s);
		  s_s = s_s + ", ";
		  genrestr.erase(0,
		      genrestr.find(s_s) + s_s.size());
		}
	      else
		{
		  if(!genrestr.empty())
		    {
		      genre_v.push_back(genrestr);
		    }
		}
	    }
	  std::sort(genre_v.begin(), genre_v.end());
	  for(size_t i = 0; i < genre_v.size(); i++)
	    {
	      if(i == 0)
		{
		  genrestr = genre_v[i];
		}
	      else
		{
		  genrestr = genrestr + ", " + genre_v[i];
		}
	    }
	  std::get<3>(el) = genrestr;
	});
    }
}

void
SearchBook::readBase(std::filesystem::path filepath)
{
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "Search: " + filepath.stem().u8string() + " file not opened"
	  << std::endl;
    }
  else
    {
      std::string bookpath;
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      bookpath = file_str.substr(
	  0, file_str.find("</bp>") + std::string("</bp>").size());
      file_str.erase(0, bookpath.size());
      bookpath.erase(0, std::string("<bp>").size());
      bookpath = bookpath.substr(0, bookpath.find("</bp>"));
      while(!file_str.empty())
	{
	  if(*cancel == 1)
	    {
	      break;
	    }
	  std::string line = file_str.substr(
	      0, file_str.find("<?L>") + std::string("<?L>").size());
	  file_str.erase(0, line.size());
	  std::string::size_type n;
	  n = line.find("<?L>");
	  line = line.substr(0, n);
	  if(n != std::string::npos)
	    {
	      std::string author = line;
	      author.erase(0, author.find("<?>") + std::string("<?>").size());
	      author = author.substr(0, author.find("<?>"));

	      std::string bookstr = line;
	      bookstr.erase(0, bookstr.find("<?>") + std::string("<?>").size());
	      bookstr.erase(0, bookstr.find("<?>") + std::string("<?>").size());
	      bookstr = bookstr.substr(0, bookstr.find("<?>"));

	      std::string seriesstr = line;
	      seriesstr.erase(
		  0, seriesstr.find("<?>") + std::string("<?>").size());
	      seriesstr.erase(
		  0, seriesstr.find("<?>") + std::string("<?>").size());
	      seriesstr.erase(
		  0, seriesstr.find("<?>") + std::string("<?>").size());
	      seriesstr = seriesstr.substr(0, seriesstr.find("<?>"));

	      std::string genrestr = line;
	      genrestr.erase(0,
			     genrestr.find("<?>") + std::string("<?>").size());
	      genrestr.erase(0,
			     genrestr.find("<?>") + std::string("<?>").size());
	      genrestr.erase(0,
			     genrestr.find("<?>") + std::string("<?>").size());
	      genrestr.erase(0,
			     genrestr.find("<?>") + std::string("<?>").size());
	      genrestr = genrestr.substr(0, genrestr.find("<?>"));

	      std::string datestr = line;
	      datestr.erase(0, datestr.find("<?>") + std::string("<?>").size());
	      datestr.erase(0, datestr.find("<?>") + std::string("<?>").size());
	      datestr.erase(0, datestr.find("<?>") + std::string("<?>").size());
	      datestr.erase(0, datestr.find("<?>") + std::string("<?>").size());
	      datestr.erase(0, datestr.find("<?>") + std::string("<?>").size());

	      std::tuple<std::string, std::string, std::string, std::string,
		  std::string, std::string> ttup;
	      std::string p_in_col = line;
	      p_in_col = p_in_col.substr(0, p_in_col.find("<?>"));
	      p_in_col = bookpath + p_in_col;
	      std::get<0>(ttup) = author;
	      std::get<1>(ttup) = bookstr;
	      std::get<2>(ttup) = seriesstr;
	      std::get<3>(ttup) = genrestr;
	      std::get<4>(ttup) = datestr;
	      std::get<5>(ttup) = p_in_col;
	      base_v->push_back(ttup);
	    }
	}
    }
}

void
SearchBook::readZipBase(std::filesystem::path filepath)
{
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr << "Search: " + filepath.stem().u8string() + " file not opened"
	  << std::endl;
    }
  else
    {
      std::string file_str;
      file_str.resize(std::filesystem::file_size(filepath));
      f.read(&file_str[0], file_str.size());
      f.close();
      std::string bookpath = file_str.substr(
	  0, file_str.find("</bp>") + std::string("</bp>").size());
      file_str.erase(0, bookpath.size());
      bookpath.erase(0, std::string("<bp>").size());
      bookpath = bookpath.substr(0, bookpath.find("</bp>"));
      while(!file_str.empty())
	{
	  std::string archpath = file_str.substr(
	      0, file_str.find("<?e>") + std::string("<?e>").size());
	  file_str.erase(0, archpath.size());
	  archpath.erase(0, std::string("<?a>").size());
	  archpath = archpath.substr(0, archpath.find("<?e>"));
	  std::string archgr = file_str.substr(0, file_str.find("<?a>"));
	  file_str.erase(0, archgr.size());
	  while(!archgr.empty())
	    {
	      std::string line = archgr.substr(
		  0, archgr.find("<?L>") + std::string("<?L>").size());
	      archgr.erase(0, line.size());
	      std::string::size_type n;
	      n = line.find("<?L>");
	      line = line.substr(0, n);
	      if(n != std::string::npos)
		{
		  std::string ind_in_arch = line;
		  ind_in_arch.erase(0, std::string("<?>").size());
		  ind_in_arch = ind_in_arch.substr(0, ind_in_arch.find("<?>"));
		  std::string f_path = "<zip><archpath>" + bookpath + archpath
		      + "</archpath>" + "<index>" + ind_in_arch + "</index>";

		  std::string author = line;
		  author.erase(0,
			       author.find("<?>") + std::string("<?>").size());
		  author.erase(0,
			       author.find("<?>") + std::string("<?>").size());
		  author = author.substr(0, author.find("<?>"));

		  std::string bookstr = line;
		  bookstr.erase(
		      0, bookstr.find("<?>") + std::string("<?>").size());
		  bookstr.erase(
		      0, bookstr.find("<?>") + std::string("<?>").size());
		  bookstr.erase(
		      0, bookstr.find("<?>") + std::string("<?>").size());
		  bookstr = bookstr.substr(0, bookstr.find("<?>"));

		  std::string seriesstr = line;
		  seriesstr.erase(
		      0, seriesstr.find("<?>") + std::string("<?>").size());
		  seriesstr.erase(
		      0, seriesstr.find("<?>") + std::string("<?>").size());
		  seriesstr.erase(
		      0, seriesstr.find("<?>") + std::string("<?>").size());
		  seriesstr.erase(
		      0, seriesstr.find("<?>") + std::string("<?>").size());
		  seriesstr = seriesstr.substr(0, seriesstr.find("<?>"));

		  std::string genrestr = line;
		  genrestr.erase(
		      0, genrestr.find("<?>") + std::string("<?>").size());
		  genrestr.erase(
		      0, genrestr.find("<?>") + std::string("<?>").size());
		  genrestr.erase(
		      0, genrestr.find("<?>") + std::string("<?>").size());
		  genrestr.erase(
		      0, genrestr.find("<?>") + std::string("<?>").size());
		  genrestr.erase(
		      0, genrestr.find("<?>") + std::string("<?>").size());
		  genrestr = genrestr.substr(0, genrestr.find("<?>"));

		  std::string datestr = line;
		  datestr.erase(
		      0, datestr.find("<?>") + std::string("<?>").size());
		  datestr.erase(
		      0, datestr.find("<?>") + std::string("<?>").size());
		  datestr.erase(
		      0, datestr.find("<?>") + std::string("<?>").size());
		  datestr.erase(
		      0, datestr.find("<?>") + std::string("<?>").size());
		  datestr.erase(
		      0, datestr.find("<?>") + std::string("<?>").size());
		  datestr.erase(
		      0, datestr.find("<?>") + std::string("<?>").size());

		  std::tuple<std::string, std::string, std::string, std::string,
		      std::string, std::string> ttup;

		  std::get<0>(ttup) = author;
		  std::get<1>(ttup) = bookstr;
		  std::get<2>(ttup) = seriesstr;
		  std::get<3>(ttup) = genrestr;
		  std::get<4>(ttup) = datestr;
		  std::get<5>(ttup) = f_path;
		  base_v->push_back(ttup);
		}
	    }
	}
      f.close();
    }
}
