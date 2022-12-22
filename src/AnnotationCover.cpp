/*
 Copyright 2022 Yury Bobylev <bobilev_yury@mail.ru>

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

#include "AnnotationCover.h"

AnnotationCover::AnnotationCover(std::string filename)
{
  this->rcvd_filename = filename;
  fileRead();
}

AnnotationCover::~AnnotationCover()
{
  // TODO Auto-generated destructor stub
}

void
AnnotationCover::fileRead()
{
  std::string::size_type n;
  n = rcvd_filename.find("<zip>");
  if(n == std::string::npos)
    {
      std::filesystem::path filepath = std::filesystem::u8path(rcvd_filename);
      std::string ext = filepath.extension().u8string();
      AuxFunc af;
      af.stringToLower(ext);
      if(std::filesystem::file_size(filepath) <= 104857600 && ext != ".epub")
	{
	  std::fstream f;
	  f.open(filepath, std::ios_base::in | std::ios_base::binary);
	  if(!f.is_open())
	    {
	      std::cerr << "AnnotationCover: fb2 file not opened" << std::endl;
	    }
	  else
	    {
	      file.resize(std::filesystem::file_size(filepath));
	      f.read(&file[0], file.size());
	      f.close();
	    }
	}
      else
	{
	  if(std::filesystem::file_size(filepath) <= 104857600
	      && ext == ".epub")
	    {
	      epub_ch_f = true;
	      std::vector<std::tuple<int, int, std::string>> list;
	      AuxFunc af;
	      af.fileNames(filepath.u8string(), list);
	      std::string filename;
#ifdef __linux
	      filename = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
	      filename = std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	      filename = filename + "/" + af.randomFileName();
	      std::filesystem::path outfolder = std::filesystem::u8path(
		  filename);
	      if(std::filesystem::exists(outfolder))
		{
		  std::filesystem::remove_all(outfolder);
		}
	      auto itl =
		  std::find_if(
		      list.begin(),
		      list.end(),
		      []
		      (auto &el)
			{
			  std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
			  std::string ext = p.extension().u8string();
			  AuxFunc af;
			  af.stringToLower(ext);
			  return ext == ".opf";
			});
	      if(itl != list.end())
		{
		  af.unpackByIndex(filepath.u8string(), outfolder.u8string(),
				   std::get<0>(*itl));
		  if(std::filesystem::exists(outfolder))
		    {
		      for(auto &dirit : std::filesystem::directory_iterator(
			  outfolder))
			{
			  std::filesystem::path p = dirit.path();
			  ext = p.extension().u8string();
			  af.stringToLower(ext);
			  if(ext == ".opf")
			    {
			      std::fstream f;
			      f.open(p,
				     std::ios_base::in | std::ios_base::binary);
			      if(f.is_open())
				{
				  file.resize(std::filesystem::file_size(p));
				  f.read(&file[0], file.size());
				  f.close();
				}
			    }
			}
		      std::filesystem::remove_all(outfolder);
		    }
		}
	    }
	}
    }
  else
    {
      std::string index = rcvd_filename;
      index.erase(0, index.find("<index>") + std::string("<index>").size());
      index = index.substr(0, index.find("</index>"));
      std::stringstream strm;
      std::locale loc("C");
      strm.imbue(loc);
      strm << index;
      int ind;
      strm >> ind;
      std::string archaddr = rcvd_filename;
      archaddr.erase(
	  0, archaddr.find("<archpath>") + std::string("<archpath>").size());
      archaddr = archaddr.substr(0, archaddr.find("</archpath>"));
      std::string outfolder;
#ifdef __linux
      outfolder = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
      outfolder = std::filesystem::temp_directory_path().parent_path().u8string();
#endif
      AuxFunc af;
      outfolder = outfolder + "/" + af.randomFileName();
      std::filesystem::path filepath = std::filesystem::u8path(outfolder);
      if(std::filesystem::exists(filepath))
	{
	  std::filesystem::remove_all(filepath);
	}

      af.unpackByIndex(archaddr, outfolder, ind);
      if(std::filesystem::exists(filepath))
	{
	  for(auto &dirit : std::filesystem::directory_iterator(filepath))
	    {
	      std::filesystem::path p = dirit.path();
	      if(!std::filesystem::is_directory(p)
		  && p.extension().u8string() == ".fb2"
		  && std::filesystem::file_size(p) <= 104857600)
		{
		  std::fstream f;
		  f.open(p, std::ios_base::in | std::ios_base::binary);
		  if(!f.is_open())
		    {
		      std::cerr << "AnnotationCover: unpacked file not opened"
			  << std::endl;
		    }
		  else
		    {
		      file.resize(std::filesystem::file_size(p));
		      f.read(&file[0], file.size());
		      f.close();
		    }
		  std::filesystem::remove_all(filepath);
		  break;
		}
	      if(!std::filesystem::is_directory(p)
		  && p.extension().u8string() == ".epub"
		  && std::filesystem::file_size(p) <= 104857600)
		{
		  epub_ch_f = true;
		  epub_path = filepath;
		  std::vector<std::tuple<int, int, std::string>> list;
		  AuxFunc af;
		  std::string ext;
		  af.fileNames(p.u8string(), list);
		  std::string filename;
#ifdef __linux
		  filename = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
		  filename = std::filesystem::temp_directory_path().parent_path().u8string();
#endif
		  filename = filename + "/" + af.randomFileName();
		  std::filesystem::path outfolder = std::filesystem::u8path(
		      filename);
		  if(std::filesystem::exists(outfolder))
		    {
		      std::filesystem::remove_all(outfolder);
		    }
		  auto itl =
		      std::find_if(
			  list.begin(),
			  list.end(),
			  []
			  (auto &el)
			    {
			      std::filesystem::path p = std::filesystem::u8path(std::get<2>(el));
			      std::string ext = p.extension().u8string();
			      AuxFunc af;
			      af.stringToLower(ext);
			      return ext == ".opf";
			    });
		  if(itl != list.end())
		    {
		      af.unpackByIndex(p.u8string(), outfolder.u8string(),
				       std::get<0>(*itl));
		      if(std::filesystem::exists(outfolder))
			{
			  for(auto &dirit : std::filesystem::directory_iterator(
			      outfolder))
			    {
			      std::filesystem::path p_th = dirit.path();
			      ext = p_th.extension().u8string();
			      af.stringToLower(ext);
			      if(ext == ".opf")
				{
				  std::fstream f;
				  f.open(
				      p_th,
				      std::ios_base::in
					  | std::ios_base::binary);
				  if(f.is_open())
				    {
				      file.resize(
					  std::filesystem::file_size(p_th));
				      f.read(&file[0], file.size());
				      f.close();
				    }
				}
			    }
			  std::filesystem::remove_all(outfolder);
			}
		    }
		  break;
		}
	    }
	}
    }
}

std::string
AnnotationCover::annotationRet()
{
  std::string result;
  if(epub_ch_f)
    {
      result = annotationEpub();
    }
  else
    {
      std::string::size_type n = 0;
      std::string conv_name = file;
      n = conv_name.find("<?xml");
      if(n != std::string::npos)
	{
	  conv_name.erase(0, n);
	  conv_name = conv_name.substr(
	      0, conv_name.find("?>") + std::string("?>").size());
	  n = conv_name.find("encoding=");
	  if(n != std::string::npos)
	    {
	      conv_name.erase(0, n + std::string("encoding=").size());
	      n = conv_name.find("\"");
	      if(n != std::string::npos)
		{
		  conv_name.erase(
		      0, conv_name.find("\"") + std::string("\"").size());
		  conv_name = conv_name.substr(0, conv_name.find("\""));
		}
	      else
		{
		  n = conv_name.find("\'");
		  if(n != std::string::npos)
		    {
		      conv_name.erase(
			  0, conv_name.find("\'") + std::string("\'").size());
		      conv_name = conv_name.substr(0, conv_name.find("\'"));
		    }
		}
	    }
	  else
	    {
	      conv_name.clear();
	    }
	}
      else
	{
	  conv_name.clear();
	}
      result = file;
      if(!conv_name.empty())
	{
	  AuxFunc af;
	  af.toutf8(result, conv_name);
	}

      n = result.find("<annotation>");
      if(n != std::string::npos)
	{
	  result.erase(0, n + std::string("<annotation>").size());
	  result = result.substr(0, result.find("</annotation>"));
	}
      else
	{
	  result.clear();
	}
      n = result.find("<p>");
      if(n != std::string::npos)
	{
	  std::string line = result;
	  result.clear();
	  while(n != std::string::npos)
	    {
	      n = line.find("<p>");
	      if(n != std::string::npos)
		{
		  std::string tmp = line.substr(
		      n, line.find("</p>") + std::string("</p>").size());
		  line.erase(0, line.find("</p>") + std::string("</p>").size());
		  tmp.erase(0, std::string("<p>").size());
		  tmp = tmp.substr(0, tmp.find("</p>"));
		  tmp = tmp + "\n\n";
		  result = result + tmp;
		}
	    }
	}
    }

  return result;
}

std::string
AnnotationCover::coverRet()
{
  std::string result;
  if(epub_ch_f)
    {
      result = "<epub>" + coverEpub();
    }
  else
    {
      std::string href = file;
      href.erase(0, href.find("<coverpage>"));
      if(!href.empty())
	{
	  href = href.substr(
	      0,
	      href.find("</coverpage>") + std::string("</coverpage>").size());
	  href.erase(0, href.find("href=") + std::string("href=").size());

	  std::string::size_type n;
	  n = href.find("\"");
	  if(n != std::string::npos)
	    {
	      href.erase(0, n + std::string("\"").size());
	      n = href.find("#");
	      if(n != std::string::npos)
		{
		  href.erase(0, n + std::string("#").size());
		}
	      href = href.substr(0, href.find("\""));
	    }
	  std::string image = file;
	  for(;;)
	    {
	      image.erase(0, image.find("<binary"));
	      if(!image.empty())
		{
		  std::string ch_str = image;
		  ch_str = ch_str.substr(0, ch_str.find(">"));
		  n = ch_str.find(href);
		  if(n != std::string::npos)
		    {
		      image.erase(0, image.find(">") + std::string(">").size());
		      image = image.substr(0, image.find("</binary>"));
		      image.erase(std::remove_if(image.begin(), image.end(), []
		      (auto &el)
			{
			  return el == '\n';
			}),
				  image.end());
		      result = image;
		      break;
		    }
		  else
		    {
		      image.erase(
			  0,
			  image.find("</binary>")
			      + std::string("</binary>").size());
		      if(image.empty())
			{
			  break;
			}
		    }
		}
	      else
		{
		  break;
		}
	    }
	}
    }

  return result;
}
std::string
AnnotationCover::annotationEpub()
{
  std::string result;
  result = file;
  std::string::size_type n = 0;
  std::string conv_name = file;
  n = conv_name.find("<?xml");
  if(n != std::string::npos)
    {
      conv_name.erase(0, n);
      conv_name = conv_name.substr(
	  0, conv_name.find("?>") + std::string("?>").size());
      n = conv_name.find("encoding=");
      if(n != std::string::npos)
	{
	  conv_name.erase(0, n + std::string("encoding=").size());
	  n = conv_name.find("\"");
	  if(n != std::string::npos)
	    {
	      conv_name.erase(0,
			      conv_name.find("\"") + std::string("\"").size());
	      conv_name = conv_name.substr(0, conv_name.find("\""));
	    }
	  else
	    {
	      n = conv_name.find("\'");
	      if(n != std::string::npos)
		{
		  conv_name.erase(
		      0, conv_name.find("\'") + std::string("\'").size());
		  conv_name = conv_name.substr(0, conv_name.find("\'"));
		}
	    }
	}
      else
	{
	  conv_name.clear();
	}
    }
  else
    {
      conv_name.clear();
    }
  result = file;
  if(!conv_name.empty())
    {
      AuxFunc af;
      af.toutf8(result, conv_name);
    }
  n = result.find("<dc:description>");
  if(n != std::string::npos)
    {
      result.erase(0, n + std::string("<dc:description>").size());
      result = result.substr(0, result.find("</dc:description>"));
    }
  else
    {
      result.clear();
    }

  return result;
}

std::string
AnnotationCover::coverEpub()
{
  std::string result;
  result = file;
  std::string::size_type n = 0;
  std::string conv_name = file;
  n = conv_name.find("<?xml");
  if(n != std::string::npos)
    {
      conv_name.erase(0, n);
      conv_name = conv_name.substr(
	  0, conv_name.find("?>") + std::string("?>").size());
      n = conv_name.find("encoding=");
      if(n != std::string::npos)
	{
	  conv_name.erase(0, n + std::string("encoding=").size());
	  n = conv_name.find("\"");
	  if(n != std::string::npos)
	    {
	      conv_name.erase(0,
			      conv_name.find("\"") + std::string("\"").size());
	      conv_name = conv_name.substr(0, conv_name.find("\""));
	    }
	  else
	    {
	      n = conv_name.find("\'");
	      if(n != std::string::npos)
		{
		  conv_name.erase(
		      0, conv_name.find("\'") + std::string("\'").size());
		  conv_name = conv_name.substr(0, conv_name.find("\'"));
		}
	    }
	}
      else
	{
	  conv_name.clear();
	}
    }
  else
    {
      conv_name.clear();
    }
  result = file;
  if(!conv_name.empty())
    {
      AuxFunc af;
      af.toutf8(result, conv_name);
    }
  std::string line = result;
  result.clear();
  n = 0;
  std::string cover;
  while(n != std::string::npos)
    {
      n = line.find("<meta");
      if(n != std::string::npos)
	{
	  std::string meta = line;
	  meta.erase(0, n);
	  meta = meta.substr(0, meta.find("/>") + std::string("/>").size());
	  line.erase(line.find(meta), meta.size());
	  std::string::size_type meta_n = meta.find("name=\"cover\"");
	  if(meta_n != std::string::npos)
	    {
	      meta_n = meta.find("content=\"cover\"");
	      if(meta_n != std::string::npos)
		{
		  cover = file;
		  break;
		}
	    }
	}
    }

  if(!cover.empty())
    {
      n = 0;
      while(n != std::string::npos)
	{
	  n = cover.find("<item");
	  if(n != std::string::npos)
	    {
	      std::string c_p = cover;
	      c_p.erase(0, n);
	      c_p = c_p.substr(0, c_p.find("/>") + std::string("/>").size());
	      cover.erase(cover.find(c_p), c_p.size());
	      std::string::size_type c_n;
	      c_n = c_p.find("id=\"cover\"");
	      if(c_n != std::string::npos)
		{
		  c_p.erase(
		      0, c_p.find("href=\"") + std::string("href=\"").size());
		  c_p = c_p.substr(0, c_p.find("\""));
		  result = c_p;
		  break;
		}
	    }
	}
    }

  std::vector<std::tuple<int, int, std::string>> list;
  AuxFunc af;
  std::filesystem::path ch_p = std::filesystem::u8path(rcvd_filename);
  if(ch_p.extension().u8string() == ".epub")
    {
      af.fileNames(rcvd_filename, list);
      ch_p = std::filesystem::u8path(rcvd_filename);
    }
  else
    {
      if(std::filesystem::exists(epub_path))
	{
	  for(auto &dirit : std::filesystem::directory_iterator(epub_path))
	    {
	      std::filesystem::path p = dirit.path();
	      if(!std::filesystem::is_directory(p)
		  && p.extension().u8string() == ".epub")
		{
		  af.fileNames(p.u8string(), list);
		  ch_p = p;
		  break;
		}
	    }
	}
    }
  cover = result;
  result.clear();
  if(!cover.empty() && list.size() > 0)
    {
      auto itl = std::find_if(list.begin(), list.end(), [cover]
      (auto &el)
	{
	  std::string line = std::get<2>(el);
	  std::string::size_type n;
	  n = line.find(cover);
	  if(n != std::string::npos)
	    {
	      return true;
	    }
	  else
	    {
	      return false;
	    }
	});
      if(itl != list.end())
	{
	  std::string p_str;
#ifdef __linux
	  p_str = std::filesystem::temp_directory_path().u8string();
#endif
#ifdef _WIN32
	  p_str = std::filesystem::temp_directory_path().parent_path().u8string();
#endif
	  p_str = p_str + "/" + af.randomFileName();
	  std::filesystem::path filepath = std::filesystem::u8path(p_str);
	  if(std::filesystem::exists(filepath))
	    {
	      std::filesystem::remove_all(filepath);
	    }
	  af.unpackByIndex(ch_p.u8string(), filepath.u8string(),
			   std::get<0>(*itl));
	  if(std::filesystem::exists(filepath))
	    {
	      for(auto &dirit : std::filesystem::directory_iterator(filepath))
		{
		  std::filesystem::path p = dirit.path();
		  if(!std::filesystem::is_directory(p))
		    {
		      result = p.u8string();
		      break;
		    }
		}
	    }
	}
    }
  if(std::filesystem::exists(epub_path))
    {
      std::filesystem::remove_all(epub_path);
    }

  return result;
}
