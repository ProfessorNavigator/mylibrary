/*
 * Copyright (C) 2022-2023 Yury Bobylev <bobilev_yury@mail.ru>
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

#include "SearchBook.h"

SearchBook::SearchBook(const std::string &collnm, const std::string &surnm,
		       const std::string &name, const std::string &secname,
		       const std::string &book, const std::string &series,
		       const std::string &genre, std::string *prev_search_nm,
		       std::vector<book_item> *base_v,
		       std::vector<book_item> *search_result_v,
		       std::atomic<int> *cancel)
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
	  if(cancel->load() == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/zipbase";
	  filepath = std::filesystem::u8path(filename);
	  readZipBase(filepath);
	  if(cancel->load() == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/epubbase";
	  filepath = std::filesystem::u8path(filename);
	  readBase(filepath);
	  if(cancel->load() == 1)
	    {
	      prev_search_nm->clear();
	      base_v->clear();
	      return void();
	    }
	  filename = filepath.parent_path().u8string();
	  filename = filename + "/pdfbase";
	  filepath = std::filesystem::u8path(filename);
	  readBase(filepath);
	  if(cancel->load() == 1)
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
      base_v->shrink_to_fit();
    }
}

void
SearchBook::cleanSearchV()
{
  search_result_v->clear();
  bool all_collection = true;
  if(cancel->load() != 1)
    {
      AuxFunc af;
      for(int i = 0; i < 6; i++)
	{
	  if(cancel->load() == 1)
	    {
	      break;
	    }
	  std::string searchstr;
	  switch(i)
	    {
	    case 0:
	      {
		searchstr = surnm;
		break;
	      }
	    case 1:
	      {
		searchstr = name;
		break;
	      }
	    case 2:
	      {
		searchstr = secname;
		break;
	      }
	    case 3:
	      {
		searchstr = book;
		break;
	      }
	    case 4:
	      {
		searchstr = series;
		break;
	      }
	    case 5:
	      {
		searchstr = genre;
		break;
	      }
	    default:
	      break;
	    }

	  af.stringToLower(searchstr);
	  if(!searchstr.empty())
	    {
	      all_collection = false;
	      std::vector<book_item> *opv;
	      if(search_result_v->size() == 0)
		{
		  opv = base_v;
		}
	      else
		{
		  opv = search_result_v;
		}

	      for(auto it = opv->begin(); it != opv->end();)
		{
		  book_item item = *it;
		  std::string line;
		  switch(i)
		    {
		    case 0:
		    case 1:
		    case 2:
		      {
			line = item.authors;
			break;
		      }
		    case 3:
		      {
			line = item.book;
			break;
		      }
		    case 4:
		      {
			line = item.series;
			break;
		      }
		    case 5:
		      {
			line = item.genre;
			break;
		      }
		    default:
		      break;
		    }

		  af.stringToLower(line);
		  std::string::size_type n = line.find(searchstr);

		  if(n != std::string::npos)
		    {
		      if(opv == base_v)
			{
			  search_result_v->push_back(item);
			}
		      it++;
		    }
		  else
		    {
		      if(opv == search_result_v)
			{
			  opv->erase(it);
			}
		      else
			{
			  it++;
			}
		    }
		}
	    }
	}
      if(all_collection)
	{
	  *search_result_v = *base_v;
	}

      std::sort(search_result_v->begin(), search_result_v->end(), [&af]
      (auto &el1, auto &el2)
	{
	  std::string line1, line2;
	  line1 = el1.authors;
	  line2 = el2.authors;
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
	  std::string genrestr = el.genre;
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
	  el.genre = genrestr;
	});
    }
  search_result_v->shrink_to_fit();
}

void
SearchBook::readBase(std::filesystem::path filepath)
{
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(!f.is_open())
    {
      std::cerr
	  << "SearchBook::readBase: " + filepath.stem().u8string()
	      + " file not opened" << std::endl;
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
	  if(cancel->load() == 1)
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

	      std::string p_in_col = line;
	      p_in_col = p_in_col.substr(0, p_in_col.find("<?>"));
	      p_in_col = bookpath + p_in_col;
	      book_item bi;
	      bi.authors = author;
	      bi.book = bookstr;
	      bi.series = seriesstr;
	      bi.genre = genrestr;
	      bi.date = datestr;
	      bi.path_to_book = p_in_col;
	      base_v->push_back(bi);
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
      std::cerr
	  << "SearchBook::readZipBase: " + filepath.stem().u8string()
	      + " file not opened" << std::endl;
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

		  book_item bi;
		  bi.authors = author;
		  bi.book = bookstr;
		  bi.series = seriesstr;
		  bi.genre = genrestr;
		  bi.date = datestr;
		  bi.path_to_book = f_path;
		  base_v->push_back(bi);
		}
	    }
	}
      f.close();
    }
}
