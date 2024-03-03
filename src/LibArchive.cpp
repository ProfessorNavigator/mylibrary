/*
 * Copyright (C) 2024 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <archive.h>
#include <AuxFunc.h>
#include <ByteOrder.h>
#include <LibArchive.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <system_error>

LibArchive::LibArchive()
{

}

LibArchive::~LibArchive()
{

}

std::filesystem::path
LibArchive::unpackByPosition(const std::filesystem::path &archaddress,
			     const std::filesystem::path &outfolder,
			     const ZipFileEntry &entry)
{
  std::filesystem::path result;
  std::shared_ptr<ArchiveFileEntry> fl = createArchFile(archaddress,
							entry.position);
  std::shared_ptr<archive> a;
  if(fl)
    {
      a = libarchive_read_init(fl);
    }
  else
    {
      fl = createArchFile(archaddress, 0);
    }
  if(!a)
    {
      a = libarchive_read_init_fallback(fl);
    }
  if(a)
    {
      std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()), []
      (archive_entry *e)
	{
	  archive_entry_free(e);
	});
      int er = archive_read_next_header2(a.get(), entry.get());
      switch(er)
	{
	case ARCHIVE_OK:
	  {
	    result = libarchive_read_entry(a.get(), entry.get(), outfolder);
	    break;
	  }
	case ARCHIVE_EOF:
	  {
	    std::cout << "LibArchive::unpackByIndex(p) read error: EOF reached"
		<< std::endl;
	    break;
	  }
	case ARCHIVE_FATAL:
	  {
	    libarchive_error(
		a, "LibArchive::unpackByIndex(p) critical read error:", er);
	    break;
	  }
	default:
	  {
	    libarchive_error(a, "LibArchive::unpackByIndex(p) read error:", er);
	    break;
	  }
	}
    }

  return result;
}

std::string
LibArchive::unpackByPosition(const std::filesystem::path &archaddress,
			     const ZipFileEntry &entry)
{
  std::string result;

  std::shared_ptr<ArchiveFileEntry> fl = createArchFile(archaddress,
							entry.position);

  std::shared_ptr<archive> a;
  if(fl)
    {
      a = libarchive_read_init(fl);
    }
  else
    {
      fl = createArchFile(archaddress, 0);
    }
  if(!a)
    {
      a = libarchive_read_init_fallback(fl);
    }
  if(a)
    {
      std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()), []
      (archive_entry *e)
	{
	  archive_entry_free(e);
	});
      int er = archive_read_next_header2(a.get(), entry.get());
      switch(er)
	{
	case ARCHIVE_OK:
	  {
	    result = libarchive_read_entry_str(a.get(), entry.get());
	    break;
	  }
	case ARCHIVE_EOF:
	  {
	    std::cout << "LibArchive::unpackByIndex(s) read error: EOF reached"
		<< std::endl;
	    break;
	  }
	case ARCHIVE_FATAL:
	  {
	    libarchive_error(
		a, "LibArchive::unpackByIndex(s) critical read error:", er);
	    break;
	  }
	default:
	  {
	    libarchive_error(a, "LibArchive::unpackByIndex(s) read error:", er);
	    break;
	  }
	}
    }

  return result;
}

std::filesystem::path
LibArchive::libarchive_read_entry(archive *a, archive_entry *entry,
				  const std::filesystem::path &outfolder)
{
  std::filesystem::path result;
  switch(archive_entry_filetype(entry))
    {
    case AE_IFREG:
      {
	char *path = const_cast<char*>(archive_entry_pathname_utf8(entry));
	std::string pathstr;
	if(!path)
	  {
	    std::cout
		<< "LibArchive::libarchive_read_entry: path pointer is null!"
		<< std::endl;
	    path = const_cast<char*>(archive_entry_pathname(entry));
	    if(path)
	      {
		pathstr = std::string(path);
		std::string ext;
		std::string::size_type n;
		n = pathstr.rfind(".");
		if(n != std::string::npos)
		  {
		    ext = std::string(pathstr.begin() + n, pathstr.end());
		    pathstr.erase(pathstr.begin() + n, pathstr.end());
		    n = pathstr.rfind(".tar");
		    if(n != std::string::npos)
		      {
			pathstr.erase(0, n);
			ext = pathstr + ext;
		      }
		  }
		ext.erase(std::remove_if(ext.begin(), ext.end(), []
		(auto &el)
		  {
		    return el == 0;
		  }),
			  ext.end());
		AuxFunc af;
		pathstr = af.randomFileName() + ext;
	      }
	    else
	      {
		std::cout
		    << "LibArchive::libarchive_read_entry: path pointer is null (critical)!"
		    << std::endl;
		return result;
	      }
	  }
	else
	  {
	    pathstr = std::string(path);
	  }
	std::filesystem::path finarch = std::filesystem::u8path(pathstr);
	std::filesystem::path outpath = outfolder;
	outpath /= finarch;
	if(!std::filesystem::exists(outpath.parent_path()))
	  {
	    std::filesystem::create_directories(outpath.parent_path());
	  }
	std::fstream f;
	f.open(outpath, std::ios_base::out | std::ios_base::binary);
	if(f.is_open())
	  {
	    for(;;)
	      {
		void *buf = nullptr;
		size_t len;
		la_int64_t offset;
		int er = archive_read_data_block(a,
						 const_cast<const void**>(&buf),
						 &len, &offset);
		if(er == ARCHIVE_OK)
		  {
		    if(buf && len > 0)
		      {
			f.seekg(offset, std::ios_base::beg);
			f.write(reinterpret_cast<char*>(buf), len);
		      }
		  }
		else
		  {
		    break;
		  }
	      }
	    f.close();
	    result = outpath;
	  }
	break;
      }
    case AE_IFDIR:
      {
	char *path = const_cast<char*>(archive_entry_pathname_utf8(entry));
	if(path)
	  {
	    std::filesystem::path finarch = std::filesystem::u8path(path);
	    std::filesystem::path outpath = outfolder;
	    outpath /= finarch;
	    std::filesystem::create_directories(outpath);
	    result = outpath;
	  }
	break;
      }
    default:
      break;
    }

  return result;
}

std::shared_ptr<archive>
LibArchive::libarchive_read_init_fallback(std::shared_ptr<ArchiveFileEntry> fl)
{
  std::shared_ptr<archive> a;
  int er;
  a = std::shared_ptr<archive>(archive_read_new(), []
  (archive *a)
    {
      archive_free(a);
    });
  er = archive_read_support_filter_all(a.get());
  if(er != ARCHIVE_OK)
    {
      libarchive_error(
	  a, "LibArchive::libarchive_read_init_fallback filter set error:", er);
      a.reset();
      return a;
    }
  er = archive_read_support_format_all(a.get());
  if(er != ARCHIVE_OK)
    {
      libarchive_error(
	  a, "LibArchive::libarchive_read_init_fallback format set error:", er);
      a.reset();
      return a;
    }

  er = archive_read_set_seek_callback(a.get(),
				      &LibArchive::libarchive_seek_callback);

  if(fl)
    {
      fl->read_bytes = 0;
      er = archive_read_open2(a.get(), reinterpret_cast<void*>(fl.get()),
			      &LibArchive::libarchive_open_callback,
			      &LibArchive::libarchive_read_callback,
			      &LibArchive::libarchive_skip_callback,
			      &LibArchive::libarchive_close_callback);
      if(er != ARCHIVE_OK)
	{
	  libarchive_error(
	      a,
	      "LibArchive::libarchive_read_init_fallback archive open error:",
	      er);
	  a.reset();
	  return a;
	}
    }
  else
    {
      er = ARCHIVE_FATAL;
      archive_set_error(a.get(), er, "%s", "File struct pointer is null");
      libarchive_error(a, "LibArchive::libarchive_read_init_fallback error:",
		       er);
      a.reset();
      return a;
    }

  return a;
}

std::shared_ptr<ArchiveFileEntry>
LibArchive::createArchFile(const std::filesystem::path &archaddress,
			   const la_int64_t &position)
{
  std::shared_ptr<ArchiveFileEntry> fl = std::make_shared<ArchiveFileEntry>();

  fl->file_path = archaddress;
  std::error_code ec;
  fl->buf_sz = static_cast<la_ssize_t>(std::filesystem::file_size(fl->file_path,
								  ec)) / 10;
  if(ec)
    {
      fl.reset();
      std::cout << "LibArchive::createArchFile: " << ec.message() << " ("
	  << archaddress.u8string() << ")" << std::endl;
      return fl;
    }

  if(fl->buf_sz > 52428800)
    {
      fl->buf_sz = 52428800;
    }
  else if(fl->buf_sz == 0)
    {
      fl->buf_sz = 1;
    }
  fl->read_bytes = position;

  return fl;
}

la_int64_t
LibArchive::libarchive_seek_callback(archive *a, void *data, la_int64_t offset,
				     int whence)
{
  la_int64_t result = ARCHIVE_FATAL;

  ArchiveFileEntry *fl = reinterpret_cast<ArchiveFileEntry*>(data);
  if(fl)
    {
      switch(whence)
	{
	case SEEK_SET:
	  {
	    fl->file.seekg(offset, std::ios_base::beg);
	    if(!fl->file.fail())
	      {
		result = static_cast<la_int64_t>(fl->file.tellg());
	      }
	    break;
	  }
	case SEEK_CUR:
	  {
	    fl->file.seekg(offset, std::ios_base::beg);
	    if(!fl->file.fail())
	      {
		result = static_cast<la_int64_t>(fl->file.tellg());
	      }
	    break;
	  }
	case SEEK_END:
	  {
	    fl->file.seekg(offset, std::ios_base::end);
	    if(!fl->file.fail())
	      {
		result = static_cast<la_int64_t>(fl->file.tellg());
	      }
	    break;
	  }
	default:
	  break;
	}
      if(result == ARCHIVE_FATAL)
	{
	  archive_set_error(a, EINVAL, "%s", "File position set error");
	}
    }
  else
    {
      archive_set_error(a, EBADF, "%s", "File struct pointer is null");
    }

  return result;
}

std::string
LibArchive::libarchive_read_entry_str(archive *a, archive_entry *entry)
{
  std::string result;

  if(archive_entry_filetype(entry) == AE_IFREG)
    {
      for(;;)
	{
	  void *buf = nullptr;
	  size_t len;
	  la_int64_t offset;
	  int er = archive_read_data_block(a, const_cast<const void**>(&buf),
					   &len, &offset);
	  if(er == ARCHIVE_OK)
	    {
	      if(buf && len > 0)
		{
		  char *lbuf = reinterpret_cast<char*>(buf);
		  for(size_t i = 0; i < len; i++)
		    {
		      result.push_back(*(lbuf + i));
		    }
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

int
LibArchive::libarchive_open_callback(archive *a, void *data)
{
  int result = ARCHIVE_FATAL;

  ArchiveFileEntry *fl = reinterpret_cast<ArchiveFileEntry*>(data);
  if(fl)
    {
      fl->file.open(fl->file_path, std::ios_base::in | std::ios_base::binary);
      if(fl->file.is_open())
	{
	  fl->file_size = static_cast<la_ssize_t>(std::filesystem::file_size(
	      fl->file_path));
	  fl->read_buf = new char[fl->buf_sz];
	  fl->file.seekg(fl->read_bytes, std::ios_base::beg);
	  if(!fl->file.fail())
	    {
	      result = ARCHIVE_OK;
	    }
	  else
	    {
	      archive_set_error(a, EINVAL, "%s", "Wrong file position");
	    }
	}
      else
	{
	  archive_set_error(a, EBADF, "%s", "File is not opened");
	}
    }
  else
    {
      archive_set_error(a, EBADF, "%s", "File struct pointer is null");
    }

  return result;
}

la_ssize_t
LibArchive::libarchive_read_callback(archive *a, void *data,
				     const void **buffer)
{
  la_ssize_t result = -1;

  ArchiveFileEntry *f = reinterpret_cast<ArchiveFileEntry*>(data);
  if(f)
    {
      la_ssize_t chsz = f->file_size - static_cast<la_ssize_t>(f->file.tellp());

      if(chsz < f->buf_sz)
	{
	  f->file.read(f->read_buf, chsz);
	  result = chsz;
	}
      else
	{
	  f->file.read(f->read_buf, f->buf_sz);
	  result = f->buf_sz;
	}

      *buffer = f->read_buf;

      if(f->file.fail())
	{
	  result = -1;
	  archive_set_error(a, EINVAL, "%s", "File read error");

	}
    }
  else
    {
      archive_set_error(a, EBADF, "%s", "File struct pointer is null");
    }

  return result;
}

la_int64_t
LibArchive::libarchive_skip_callback(archive *a, void *data, la_int64_t request)
{
  la_int64_t result = 0;

  ArchiveFileEntry *f = reinterpret_cast<ArchiveFileEntry*>(data);
  if(f)
    {
      f->file.seekg(request, std::ios_base::cur);
      if(!f->file.fail())
	{
	  result = request;
	}
    }

  return result;
}

int
LibArchive::libarchive_close_callback(archive *a, void *data)
{
  int result = ARCHIVE_FATAL;

  ArchiveFileEntry *fl = reinterpret_cast<ArchiveFileEntry*>(data);
  if(fl)
    {
      if(fl->file.is_open())
	{
	  fl->file.close();
	}
      result = ARCHIVE_OK;
    }
  else
    {
      archive_set_error(a, EBADF, "%s", "File struct pointer is null");
    }

  return result;
}

int
LibArchive::fileNames(const std::filesystem::path &filepath,
		      std::vector<ZipFileEntry> &filenames)
{
  int result = 0;
  std::fstream f;
  f.open(filepath, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string eocd;
      uint32_t var = 0x06054b50;
      ByteOrder bo = var;
      bo.get_little(var);
      std::string findstr;
      findstr.resize(sizeof(var));
      std::memcpy(findstr.data(), &var, sizeof(var));
      std::string::size_type n;
      std::string read;
      int64_t offset;
      for(;;)
	{
	  read.clear();
	  read.resize(22);
	  offset = static_cast<int64_t>(eocd.size() + read.size());
	  offset = -offset;
	  f.seekg(offset, std::ios_base::end);
	  if(f.fail())
	    {
	      std::cout << "LibArchive::fileNames critical error" << std::endl;
	      f.close();
	      return -1;
	    }
	  f.read(read.data(), read.size());
	  eocd = read + eocd;
	  n = eocd.find(findstr);
	  if(n != std::string::npos)
	    {
	      eocd.erase(0, n);
	      break;
	    }
	}

      uint32_t socd = 0;
      std::memcpy(&socd, &eocd[12], sizeof(socd));
      bo.set_little(socd);
      socd = bo;
      uint32_t oscd = 0;
      std::memcpy(&oscd, &eocd[16], sizeof(oscd));
      bo.set_little(oscd);
      oscd = bo;
      uint64_t cdoffset = static_cast<uint64_t>(oscd);
      uint64_t cdsize = static_cast<uint64_t>(socd);
      if(socd == 0xffffffff || oscd == 0xffffffff)
	{
	  size_t locoffset = eocd.size();
	  eocd.clear();
	  f.seekg(-(locoffset), std::ios_base::end);
	  var = 0x06064b50;
	  bo = var;
	  bo.get_little(var);
	  findstr.clear();
	  findstr.resize(sizeof(var));
	  std::memcpy(findstr.data(), &var, sizeof(var));
	  std::string::size_type n;
	  for(;;)
	    {
	      read.clear();
	      read.resize(22);
	      f.seekg(-(eocd.size() + read.size() + locoffset),
		      std::ios_base::end);
	      f.read(read.data(), read.size());
	      eocd = read + eocd;
	      n = eocd.find(findstr);
	      if(n != std::string::npos)
		{
		  eocd.erase(0, n);
		  break;
		}
	      if(eocd.size() > 1024)
		{
		  std::cout << "LibArchive::fileNames: too big record" << std::endl;
		  f.close();
		  return -1;
		}

	    }
	  if(socd == 0xffffffff)
	    {
	      std::memcpy(&cdsize, &eocd[40], sizeof(cdsize));
	      bo.set_little(cdsize);
	      cdsize = bo;
	    }
	  if(oscd == 0xffffffff)
	    {
	      std::memcpy(&cdoffset, &eocd[48], sizeof(cdoffset));
	      bo.set_little(cdoffset);
	      cdoffset = bo;
	    }
	  if(eocd.size() >= 60)
	    {
	      uint32_t locator = 0;
	      std::memcpy(&locator, &eocd[56], sizeof(locator));
	      bo.set_little(locator);
	      locator = bo;
	      if(locator != 0x07064b50)
		{
		  std::cout << "LibArchive::fileNames: central directory is encrypted "
		      "or compressed, need fall back mode" << std::endl;
		  f.close();
		  return -2;
		}
	    }
	}
      if(cdoffset == 0 || cdsize == 0)
	{
	  f.close();
	  return -1;
	}
      std::string cd;
      cd.resize(static_cast<size_t>(cdsize));
      f.seekg(cdoffset, std::ios_base::beg);
      f.read(cd.data(), cd.size());
      f.close();
      var = 0x02014b50;
      bo = var;
      bo.get_little(var);
      findstr.clear();
      findstr.resize(sizeof(var));
      std::memcpy(findstr.data(), &var, sizeof(var));
      n = cd.find(findstr);
      if(n != std::string::npos)
	{
	  cd.erase(0, n);
	  size_t off = 0;
	  size_t cd_sz = cd.size();
	  size_t var_sz = sizeof(uint32_t) + 42;
	  while(off < cd.size())
	    {
	      if(off + var_sz > cd_sz)
		{
		  std::cout << "LibArchive::fileNames error: "
		      "incorrect central directory entry size (1)" << std::endl;
		  filenames.clear();
		  return -1;
		}

	      uint32_t c_sz = 0;
	      std::memcpy(&c_sz, &cd[off + 20], sizeof(c_sz));
	      bo.set_little(c_sz);
	      c_sz = bo;

	      uint32_t u_sz = 0;
	      std::memcpy(&u_sz, &cd[off + 24], sizeof(u_sz));
	      bo.set_little(u_sz);
	      u_sz = bo;

	      uint16_t fnml = 0;
	      std::memcpy(&fnml, &cd[off + 28], sizeof(fnml));
	      bo.set_little(fnml);
	      fnml = bo;
	      if(off + static_cast<size_t>(fnml) + 46 > cd_sz)
		{
		  std::cout << "LibArchive::fileNames error: "
		      "incorrect central directory entry size (2)" << std::endl;
		  filenames.clear();
		  return -1;
		}

	      uint16_t efl = 0;
	      std::memcpy(&efl, &cd[off + 30], sizeof(efl));
	      bo.set_little(efl);
	      efl = bo;

	      uint16_t fkl = 0;
	      std::memcpy(&fkl, &cd[off + 32], sizeof(fkl));
	      bo.set_little(fkl);
	      fkl = bo;

	      uint32_t offset = 0;
	      std::memcpy(&offset, &cd[off + 42], sizeof(offset));
	      bo.set_little(offset);
	      offset = bo;

	      std::string fnm;
	      fnm.resize(fnml);
	      std::memcpy(fnm.data(), &cd[off + 46], fnm.size());

	      if(c_sz == 0xffffffff || u_sz == 0xffffffff
		  || offset == 0xffffffff)
		{
		  if(off + static_cast<size_t>(fnml) + static_cast<size_t>(efl)
		      + 46 > cd_sz)
		    {
		      std::cout << "LibArchive::fileNames error: "
			  "incorrect central directory entry size (3)" << std::endl;
		      filenames.clear();
		      return -1;
		    }
		  std::string extra;
		  extra.resize(efl);
		  std::memcpy(&extra[0], &cd[off + 46 + fnml], extra.size());
		  std::string data;
		  for(;;)
		    {
		      if(extra.size() >= 4)
			{
			  uint16_t id = 0;
			  uint16_t sz = 0;
			  std::memcpy(&id, &extra[0], sizeof(id));
			  extra.erase(0, 2);
			  std::memcpy(&sz, &extra[0], sizeof(sz));
			  extra.erase(0, 2);
			  bo.set_little(id);
			  id = bo;
			  bo.set_little(sz);
			  sz = bo;
			  if(id == 0x0001)
			    {
			      if(extra.size() >= static_cast<size_t>(sz))
				{
				  std::copy(extra.begin(), extra.begin() + sz,
					    std::back_inserter(data));
				}
			      break;
			    }
			  else
			    {
			      if(extra.size() >= static_cast<size_t>(sz))
				{
				  extra.erase(0, sz);
				}
			      else
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
		  if(data.size() < 8)
		    {
		      return -1;
		    }
		  int variant = 1;
		  uint64_t c = 0;
		  uint64_t u = 0;
		  uint64_t o = 0;
		  while(data.size() >= 8)
		    {
		      switch(variant)
			{
			case 1:
			  {
			    if(u_sz == 0xffffffff)
			      {
				std::memcpy(&u, &data[0], sizeof(u));
				bo.set_little(u);
				u = bo;
				data.erase(0, sizeof(u));
			      }
			    else
			      {
				u = static_cast<uint64_t>(u_sz);
			      }
			    break;
			  }
			case 2:
			  {
			    if(c_sz == 0xffffffff)
			      {
				std::memcpy(&c, &data[0], sizeof(c));
				bo.set_little(c);
				c = bo;
				data.erase(0, sizeof(c));
			      }
			    else
			      {
				c = static_cast<uint64_t>(c_sz);
			      }
			    break;
			  }
			case 3:
			  {
			    if(offset == 0xffffffff)
			      {
				std::memcpy(&o, &data[0], sizeof(o));
				bo.set_little(o);
				o = bo;
				data.erase(0, sizeof(o));
			      }
			    else
			      {
				o = static_cast<uint64_t>(offset);
			      }
			    break;
			  }
			default:
			  {
			    data.clear();
			    break;
			  }
			}
		      variant++;
		    }
		  ZipFileEntry ent;
		  ent.size = u;
		  ent.compressed_size = c;
		  ent.position = static_cast<la_int64_t>(o);
		  ent.filename = fnm;
		  filenames.emplace_back(ent);
		}
	      else
		{
		  ZipFileEntry ent;
		  ent.size = static_cast<uint64_t>(u_sz);
		  ent.compressed_size = static_cast<uint64_t>(c_sz);
		  ent.position = static_cast<la_int64_t>(offset);
		  ent.filename = fnm;
		  filenames.emplace_back(ent);
		}
	      off += 46 + fnml + efl + fkl;
	    }
	  result = 1;
	}

    }
  return result;
}

ZipFileEntry
LibArchive::fileinfo(const std::filesystem::path &address,
		     const std::string &filename)
{
  std::filesystem::path p = address;
  ZipFileEntry result;
  std::shared_ptr<AuxFunc> af = std::make_shared<AuxFunc>();
  std::string ext = p.extension().u8string();
  ext = af->stringToLower(ext);
  if(ext == ".zip")
    {
      std::vector<ZipFileEntry> archv;
      fileNames(address, archv);
      auto itav = std::find_if(archv.begin(), archv.end(), [filename]
      (auto &el)
	{
	  return el.filename == filename;
	});
      if(itav != archv.end())
	{
	  result = *itav;
	}
    }
  else
    {
      std::shared_ptr<ArchiveFileEntry> fl = createArchFile(address, 0);
      std::shared_ptr<archive> a = libarchive_read_init_fallback(fl);
      if(a)
	{
	  int er;
	  std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()), []
	  (archive_entry *e)
	    {
	      archive_entry_free(e);
	    });
	  bool interrupt = false;
	  std::string ch_fnm;
	  while(!interrupt)
	    {
	      archive_entry_clear(entry.get());
	      er = archive_read_next_header2(a.get(), entry.get());
	      switch(er)
		{
		case ARCHIVE_OK:
		  {
		    char *chnm = const_cast<char*>(archive_entry_pathname_utf8(
			entry.get()));
		    if(chnm)
		      {
			ch_fnm = chnm;
		      }
		    else
		      {
			chnm = const_cast<char*>(archive_entry_pathname(
			    entry.get()));
			if(chnm)
			  {
			    ch_fnm = chnm;
			  }
			else
			  {
			    std::cout << "LibArchive::fileinfo file name error"
				<< std::endl;
			  }
		      }
		    if(ch_fnm == filename)
		      {
			result.filename = ch_fnm;
			result.size = static_cast<uint64_t>(archive_entry_size(
			    entry.get()));
			result.compressed_size = 0;
			interrupt = true;
		      }
		    break;
		  }
		case ARCHIVE_EOF:
		  {
		    interrupt = true;
		    break;
		  }
		case ARCHIVE_FATAL:
		  {
		    libarchive_error(a, "LibArchive::fileinfo critical error:",
				     er);
		    interrupt = true;
		    break;
		  }
		default:
		  {
		    libarchive_error(a, "LibArchive::fileinfo error:", er);
		    break;
		  }
		}
	    }
	}
    }

  return result;
}

std::filesystem::path
LibArchive::unpackByFileNameStream(const std::filesystem::path &archaddress,
				   const std::filesystem::path &outfolder,
				   const std::string &filename)
{
  std::filesystem::path result;

  std::shared_ptr<ArchiveFileEntry> fl = createArchFile(archaddress, 0);
  std::shared_ptr<archive> a = libarchive_read_init_fallback(fl);
  if(a)
    {
      std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()), []
      (archive_entry *e)
	{
	  archive_entry_free(e);
	});
      std::string ch_fnm;
      bool interrupt = false;
      int er;
      while(!interrupt)
	{
	  archive_entry_clear(entry.get());
	  er = archive_read_next_header2(a.get(), entry.get());
	  switch(er)
	    {
	    case ARCHIVE_OK:
	      {
		char *chnm = const_cast<char*>(archive_entry_pathname_utf8(
		    entry.get()));
		if(chnm)
		  {
		    ch_fnm = chnm;
		  }
		else
		  {
		    chnm =
			const_cast<char*>(archive_entry_pathname(entry.get()));
		    if(chnm)
		      {
			ch_fnm = chnm;
		      }
		    else
		      {
			std::cout
			    << "LibArchive::unpackByIndexStream file name error"
			    << std::endl;
		      }
		  }
		if(ch_fnm == filename)
		  {
		    result = libarchive_read_entry(a.get(), entry.get(),
						   outfolder);
		    interrupt = true;
		  }
		break;
	      }
	    case ARCHIVE_EOF:
	      {
		interrupt = true;
		break;
	      }
	    case ARCHIVE_FATAL:
	      {
		libarchive_error(
		    a, "LibArchive::unpackByIndexStream fatal read error:", er);
		interrupt = true;
		break;
	      }
	    default:
	      {
		libarchive_error(a,
				 "LibArchive::unpackByIndexStream read error:",
				 er);
		break;
	      }
	    }
	}
    }

  return result;
}

std::string
LibArchive::unpackByFileNameStreamStr(const std::filesystem::path &archaddress,
				      const std::string &filename)
{
  std::string result;

  std::shared_ptr<ArchiveFileEntry> fl = createArchFile(archaddress, 0);

  std::shared_ptr<archive> a = libarchive_read_init_fallback(fl);

  if(a)
    {
      std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()), []
      (archive_entry *e)
	{
	  archive_entry_free(e);
	});
      bool interrupt = false;
      int er;
      std::string ch_fnm;
      while(!interrupt)
	{
	  archive_entry_clear(entry.get());
	  er = archive_read_next_header2(a.get(), entry.get());
	  switch(er)
	    {
	    case ARCHIVE_OK:
	      {
		char *chnm = const_cast<char*>(archive_entry_pathname_utf8(
		    entry.get()));
		if(chnm)
		  {
		    ch_fnm = chnm;
		  }
		else
		  {
		    chnm =
			const_cast<char*>(archive_entry_pathname(entry.get()));
		    if(chnm)
		      {
			ch_fnm = chnm;
		      }
		    else
		      {
			std::cout
			    << "LibArchive::unpackByIndexStreamStr file name error"
			    << std::endl;
		      }
		  }
		if(ch_fnm == filename)
		  {
		    result = libarchive_read_entry_str(a.get(), entry.get());
		    interrupt = true;
		  }
		break;
	      }
	    case ARCHIVE_EOF:
	      {
		interrupt = true;
		break;
	      }
	    case ARCHIVE_FATAL:
	      {
		libarchive_error(
		    a, "LibArchive::unpackByIndexStream fatal read error:", er);
		interrupt = true;
		break;
	      }
	    default:
	      {
		libarchive_error(a,
				 "LibArchive::unpackByIndexStream read error:",
				 er);
		break;
	      }
	    }
	}
    }

  return result;
}

std::shared_ptr<archive>
LibArchive::libarchive_read_init(std::shared_ptr<ArchiveFileEntry> fl)
{
  std::shared_ptr<archive> a;
  int er;
  a = std::shared_ptr<archive>(archive_read_new(), []
  (archive *a)
    {
      archive_free(a);
    });
  er = archive_read_support_filter_all(a.get());
  if(er != ARCHIVE_OK)
    {
      libarchive_error(a, "LibArchive::libarchive_read_init filter set error:",
		       er);
      a.reset();
      return a;
    }
  er = archive_read_support_format_all(a.get());
  if(er != ARCHIVE_OK)
    {
      libarchive_error(a, "LibArchive::libarchive_read_init format set error:",
		       er);
      a.reset();
      return a;
    }

  if(fl)
    {
      er = archive_read_open2(a.get(), reinterpret_cast<void*>(fl.get()),
			      &LibArchive::libarchive_open_callback,
			      &LibArchive::libarchive_read_callback,
			      &LibArchive::libarchive_skip_callback,
			      &LibArchive::libarchive_close_callback);
      if(er != ARCHIVE_OK)
	{
	  libarchive_error(
	      a, "LibArchive::libarchive_read_init archive open error:", er);
	  a.reset();
	  return a;
	}
    }
  else
    {
      er = ARCHIVE_FATAL;
      archive_set_error(a.get(), er, "%s", "File struct pointer is null");
      libarchive_error(a,
		       "LibArchive::libarchive_read_init archive open error:",
		       er);
      a.reset();
      return a;
    }

  return a;
}

int
LibArchive::fileNamesStream(const std::filesystem::path &address,
			    std::vector<ZipFileEntry> &filenames)
{
  std::shared_ptr<ArchiveFileEntry> fl = createArchFile(address, 0);

  std::shared_ptr<archive> a = libarchive_read_init_fallback(fl);
  int er = ARCHIVE_OK;
  if(a)
    {
      std::shared_ptr<archive_entry> entry(archive_entry_new2(a.get()), []
      (archive_entry *e)
	{
	  archive_entry_free(e);
	});
      bool interrupt = false;
      while(!interrupt)
	{
	  archive_entry_clear(entry.get());
	  er = archive_read_next_header2(a.get(), entry.get());
	  switch(er)
	    {
	    case ARCHIVE_OK:
	      {
		ZipFileEntry ent;
		ent.size = static_cast<int>(archive_entry_size(entry.get()));
		char *path = const_cast<char*>(archive_entry_pathname_utf8(
		    entry.get()));
		std::string pathstr;
		if(!path)
		  {
		    std::cout
			<< "LibArchive::fileNamesStream: path pointer is null!"
			<< std::endl;
		    path =
			const_cast<char*>(archive_entry_pathname(entry.get()));
		    if(path)
		      {
			pathstr =
			    "Book"
				+ std::filesystem::u8path(path).extension().u8string();
		      }
		    else
		      {
			std::cout
			    << "LibArchive::fileNamesStream: path pointer is null (critical)!"
			    << std::endl;
		      }
		  }
		else
		  {
		    pathstr = std::string(path);
		  }
		ent.filename = pathstr;
		filenames.emplace_back(ent);
		break;
	      }
	    case ARCHIVE_EOF:
	      {
		interrupt = true;
		break;
	      }
	    case ARCHIVE_FATAL:
	      {
		libarchive_error(a,
				 "LibArchive::fileNamesStream critical error:",
				 er);
		interrupt = true;
		break;
	      }
	    default:
	      {
		libarchive_error(a, "LibArchive::fileNamesStream error:", er);
		break;
	      }
	    }
	}
    }

  if(er == ARCHIVE_EOF)
    {
      er = ARCHIVE_OK;
    }

  return er;
}

void
LibArchive::libarchive_error(const std::shared_ptr<archive> &a,
			     const std::string &message,
			     const int &error_number)
{
  const char *error = archive_error_string(a.get());
  if(error)
    {
      std::cout << message << " " << error << std::endl;
    }
  else
    {
      std::cout << message << " " << error_number << std::endl;
    }
}

int
LibArchive::libarchive_packing(const std::filesystem::path &sourcepath,
			       const std::filesystem::path &outpath)
{
  std::shared_ptr<archive> a = libarchive_write_init(outpath);
  return libarchive_packing(a, sourcepath, false, "");
}

int
LibArchive::write_func(archive *a, const std::filesystem::path &source,
		       const std::filesystem::path &path_in_arch)
{
  int er = ARCHIVE_FATAL;
  std::shared_ptr<archive_entry> entry(archive_entry_new2(a), []
  (archive_entry *e)
    {
      archive_entry_free(e);
    });

  switch(std::filesystem::symlink_status(source).type())
    {
    case std::filesystem::file_type::directory:
      {
	er = libarchive_write_directory(a, entry.get(), path_in_arch, source);
	break;
      }
    case std::filesystem::file_type::regular:
      {
	er = libarchive_write_file(a, entry.get(), path_in_arch, source);
	break;
      }
    case std::filesystem::file_type::symlink:
      {
	std::filesystem::path chp = std::filesystem::read_symlink(source);
	if(std::filesystem::exists(chp))
	  {
	    switch(std::filesystem::status(chp).type())
	      {
	      case std::filesystem::file_type::regular:
		{
		  er = libarchive_write_file(a, entry.get(), path_in_arch, chp);
		  break;
		}
	      case std::filesystem::file_type::directory:
		{
		  std::vector<
		      std::tuple<std::filesystem::path, std::filesystem::path>> res;
		  res = dir_symlink_resolver(chp, path_in_arch);
		  for(auto it = res.begin(); it != res.end(); it++)
		    {
		      std::shared_ptr<archive_entry> lent(
			  archive_entry_new2(a), []
			  (archive_entry *e)
			    {
			      archive_entry_free(e);
			    });
		      if(std::filesystem::is_directory(std::get<0>(*it)))
			{
			  er = libarchive_write_directory(a, lent.get(),
							  std::get<1>(*it),
							  std::get<0>(*it));
			}
		      else
			{
			  er = libarchive_write_file(a, lent.get(),
						     std::get<1>(*it),
						     std::get<0>(*it));
			}
		    }
		  break;
		}
	      default:
		break;
	      }
	  }
	break;
      }
    default:
      break;
    }

  return er;
}

std::vector<std::tuple<std::filesystem::path, std::filesystem::path>>
LibArchive::dir_symlink_resolver(const std::filesystem::path &source,
				 const std::filesystem::path &append_to)
{
  std::vector<std::tuple<std::filesystem::path, std::filesystem::path>> result;

  std::filesystem::path chp = source;
  for(auto &dirit : std::filesystem::recursive_directory_iterator(chp))
    {
      std::filesystem::path p = dirit.path();
      std::filesystem::path p_in_a = append_to;
      p_in_a /= p.lexically_proximate(chp);
      result.push_back(std::make_tuple(p, p_in_a));
    }

  std::vector<std::tuple<std::filesystem::path, std::filesystem::path>> add;
  for(auto it = result.begin(); it != result.end(); it++)
    {
      std::filesystem::file_type ft = std::filesystem::symlink_status(
	  std::get<0>(*it)).type();
      if(ft == std::filesystem::file_type::symlink)
	{
	  std::filesystem::path p = std::filesystem::read_symlink(
	      std::get<0>(*it));
	  switch(std::filesystem::status(p).type())
	    {
	    case std::filesystem::file_type::regular:
	      {
		std::filesystem::path p_in_a = std::get<1>(*it);
		p_in_a /= p.lexically_proximate(std::get<0>(*it));
		add.push_back(std::make_tuple(p, p_in_a));
		break;
	      }
	    case std::filesystem::file_type::directory:
	      {
		add = dir_symlink_resolver(p, std::get<1>(*it));
		break;
	      }
	    default:
	      break;
	    }
	}
    }
  std::copy(add.begin(), add.end(), std::back_inserter(result));

  result.erase(std::remove_if(result.begin(), result.end(), []
  (auto &el)
    {
      if(!std::filesystem::exists(std::get<0>(el)))
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }),
	       result.end());

  for(size_t i = 0;; i++)
    {
      auto it = result.begin() + i;
      if(it != result.end())
	{
	  if(it + 1 != result.end())
	    {
	      for(auto itr = it + 1; itr != result.end();)
		{
		  if(std::get<0>(*itr) == std::get<0>(*it))
		    {
		      result.erase(itr);
		    }
		  else
		    {
		      itr++;
		    }
		}
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

  return result;
}

int
LibArchive::libarchive_write_directory(
    archive *a, archive_entry *entry, const std::filesystem::path &path_in_arch,
    const std::filesystem::path &source)
{
  archive_entry_set_pathname_utf8(entry, path_in_arch.u8string().c_str());
  std::filesystem::file_status f_stat = std::filesystem::status(source);
  archive_entry_set_perm(entry, static_cast<__LA_MODE_T>(f_stat.permissions()));
  archive_entry_set_filetype(entry, AE_IFDIR);
  std::filesystem::file_time_type last_write = std::filesystem::last_write_time(
      source);
  auto sytem_clock_tp = std::chrono::time_point_cast<
      std::chrono::system_clock::duration>(
      last_write - std::filesystem::file_time_type::clock::now()
	  + std::chrono::system_clock::now());
  time_t l_w_t = std::chrono::system_clock::to_time_t(sytem_clock_tp);
  archive_entry_set_mtime(entry, l_w_t, 0);
  int er = archive_write_header(a, entry);
  if(er != ARCHIVE_OK)
    {
      std::shared_ptr<archive> aa(a, []
      (archive *a)
	{});
      libarchive_error(aa, "LibArchive::write_directory write header error:",
		       er);
    }
  return er;
}

int
LibArchive::libarchive_write_file(archive *a, archive_entry *entry,
				  const std::filesystem::path &path_in_arch,
				  const std::filesystem::path &source)
{
  archive_entry_set_pathname_utf8(entry, path_in_arch.u8string().c_str());
  std::filesystem::file_status f_stat = std::filesystem::status(source);
  archive_entry_set_perm(entry, static_cast<__LA_MODE_T>(f_stat.permissions()));
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_size(
      entry, static_cast<la_int64_t>(std::filesystem::file_size(source)));
  std::filesystem::file_time_type last_write = std::filesystem::last_write_time(
      source);
  auto sytem_clock_tp = std::chrono::time_point_cast<
      std::chrono::system_clock::duration>(
      last_write - std::filesystem::file_time_type::clock::now()
	  + std::chrono::system_clock::now());
  time_t l_w_t = std::chrono::system_clock::to_time_t(sytem_clock_tp);
  archive_entry_set_mtime(entry, l_w_t, 0);
  int er = archive_write_header(a, entry);
  if(er != ARCHIVE_OK)
    {
      std::shared_ptr<archive> aa(a, []
      (archive *a)
	{});
      libarchive_error(aa,
		       "LibArchive::libarchive_write_file write header error:",
		       er);
      return er;
    }

  er = libarchive_write_data_from_file(a, source);

  return er;
}

la_ssize_t
LibArchive::libarchive_write_callback(archive *a, void *data,
				      const void *buffer, size_t length)
{
  la_ssize_t result = -1;

  ArchiveFileEntry *fl = reinterpret_cast<ArchiveFileEntry*>(data);
  if(fl)
    {
      fl->file.write(reinterpret_cast<const char*>(buffer), length);
      fl->read_bytes += static_cast<la_ssize_t>(length);
      result = static_cast<la_ssize_t>(length);
    }
  else
    {
      archive_set_error(a, EBADF, "%s", "File struct pointer is null");
    }

  return result;
}

int
LibArchive::libarchive_free_callback(archive *a, void *data)
{
  ArchiveFileEntry *fl = reinterpret_cast<ArchiveFileEntry*>(data);
  delete fl;
  return ARCHIVE_OK;
}

int
LibArchive::libarchive_write_data_from_file(archive *a,
					    const std::filesystem::path &source)
{
  int er = ARCHIVE_OK;
  struct self_close
  {
    std::fstream f;
    ~self_close()
    {
      if(f.is_open())
	{
	  f.close();
	}
    }
  } self_d;

  std::string buf;
  size_t byteread = 0;
  size_t fsz;
  self_d.f.open(source, std::ios_base::in | std::ios_base::binary);
  if(self_d.f.is_open())
    {
      self_d.f.seekg(0, std::ios_base::end);
      fsz = static_cast<size_t>(self_d.f.tellg());
      self_d.f.seekg(0, std::ios_base::beg);
      size_t ch;
      while(byteread < fsz)
	{
	  ch = fsz - byteread;
	  buf.clear();
	  if(ch > 1048576)
	    {
	      buf.resize(1048576);
	    }
	  else
	    {
	      buf.resize(ch);
	    }
	  ch = buf.size();
	  self_d.f.read(&buf[0], buf.size());
	  er = libarchive_write_data(a, buf);
	  if(er != ARCHIVE_OK)
	    {
	      return er;
	    }
	  byteread = byteread + ch;
	}
    }
  else
    {
      std::cout << "LibArchive::libarchive_write_data_from_file: "
	  "source file not opened" << std::endl;
      er = ARCHIVE_FATAL;
    }

  return er;
}

int
LibArchive::libarchive_open_callback_write(archive *a, void *data)
{
  int result = ARCHIVE_FATAL;

  ArchiveFileEntry *fl = reinterpret_cast<ArchiveFileEntry*>(data);
  if(fl)
    {
      fl->file.open(fl->file_path, std::ios_base::out | std::ios_base::binary);
      if(fl->file.is_open())
	{
	  result = ARCHIVE_OK;
	}
      else
	{
	  archive_set_error(a, EBADF, "%s",
			    "File has not been opened for writing");
	}
    }
  else
    {
      archive_set_error(a, EBADF, "%s", "File struct pointer is null");
    }

  return result;
}

ArchiveRemoveEntry
LibArchive::libarchive_remove_init(const std::filesystem::path &sourcepath,
				   const std::filesystem::path &outpath)
{
  ArchiveRemoveEntry result;

  result.fl = createArchFile(sourcepath, 0);
  result.a_read = libarchive_read_init_fallback(result.fl);
  result.a_write = libarchive_write_init(outpath);

  return result;
}

int
LibArchive::libarchive_write_data(archive *a, const std::string &data)
{
  std::string buf = data;
  ssize_t wb = archive_write_data(a, &buf[0], buf.size());
  if(wb < 0)
    {
      archive_set_error(a, ECANCELED, "%s",
			"Data cannot be written to archive (1)");
      return ARCHIVE_FATAL;
    }
  else
    {
      while(wb != static_cast<ssize_t>(buf.size()))
	{
	  buf.erase(buf.begin(), buf.begin() + wb);
	  wb = archive_write_data(a, &buf[0], buf.size());
	  if(wb < 0)
	    {
	      archive_set_error(a, ECANCELED, "%s",
				"Data cannot be written to archive (2)");
	      return ARCHIVE_FATAL;
	    }
	}
    }
  return ARCHIVE_OK;
}

std::shared_ptr<archive>
LibArchive::libarchive_write_init(const std::filesystem::path &outpath)
{
  std::shared_ptr<archive> a;
  a = std::shared_ptr<archive>(archive_write_new(), []
  (archive *a)
    {
      archive_free(a);
    });
  int er;
  er = archive_write_set_format_filter_by_ext(a.get(),
					      outpath.string().c_str());
  if(er != ARCHIVE_OK)
    {
      libarchive_error(
	  a, "LibArchive::libarchive_write_init format setting error:", er);
      a.reset();
      return a;
    }

  er = archive_write_set_options(a.get(), "hdrcharset=UTF-8");
  if(er != ARCHIVE_OK)
    {
      libarchive_error(
	  a, "LibArchive::libarchive_write_init options setting error:", er);
      if(er == ARCHIVE_FATAL)
	{
	  a.reset();
	  return a;
	}
    }

  std::filesystem::create_directories(outpath.parent_path());

  ArchiveFileEntry *fl = new ArchiveFileEntry;
  fl->file_path = outpath;
  er = archive_write_open2(a.get(), reinterpret_cast<void*>(fl),
			   &LibArchive::libarchive_open_callback_write,
			   &LibArchive::libarchive_write_callback,
			   &LibArchive::libarchive_close_callback,
			   &LibArchive::libarchive_free_callback);
  if(er != ARCHIVE_OK)
    {
      libarchive_error(a,
		       "LibArchive::libarchive_write_init archive open error:",
		       er);
      a.reset();
      return a;
    }

  return a;
}

int
LibArchive::libarchive_packing(const std::shared_ptr<archive> &a,
			       const std::filesystem::path &sourcepath,
			       const bool &rename_source,
			       const std::string &new_source_name)
{
  if(!std::filesystem::exists(sourcepath))
    {
      return -100;
    }
  if(a)
    {
      //writev tuple: 0-source path, 1-path in archive
      std::vector<std::tuple<std::filesystem::path, std::filesystem::path>> writev;

      std::filesystem::path base = sourcepath.parent_path();

      std::filesystem::file_type ft =
	  std::filesystem::status(sourcepath).type();
      switch(ft)
	{
	case std::filesystem::file_type::directory:
	  {
	    std::tuple<std::filesystem::path, std::filesystem::path> ttup;
	    std::get<0>(ttup) = sourcepath;
	    std::get<1>(ttup) = std::filesystem::relative(sourcepath, base);
	    if(rename_source)
	      {
		std::string remove_str = sourcepath.filename().u8string();
		std::string path = std::get<1>(ttup).u8string();
		std::string::size_type n = path.find(remove_str);
		if(n != std::string::npos)
		  {
		    path.erase(n, remove_str.size());
		    path.insert(n, new_source_name);
		    std::get<1>(ttup) = std::filesystem::u8path(path);
		  }
	      }
	    writev.push_back(ttup);
	    for(auto &dirit : std::filesystem::recursive_directory_iterator(
		sourcepath))
	      {
		std::filesystem::path p = dirit.path();
		std::filesystem::file_type ft =
		    std::filesystem::status(p).type();
		switch(ft)
		  {
		  case std::filesystem::file_type::directory:
		  case std::filesystem::file_type::regular:
		  case std::filesystem::file_type::symlink:
		    {
		      std::tuple<std::filesystem::path, std::filesystem::path> ttup;
		      std::get<0>(ttup) = p;
		      std::get<1>(ttup) = p.lexically_proximate(base);
		      if(rename_source)
			{
			  std::string remove_str =
			      sourcepath.filename().u8string();
			  std::string path = std::get<1>(ttup).u8string();
			  std::string::size_type n = path.find(remove_str);
			  if(n != std::string::npos)
			    {
			      path.erase(n, remove_str.size());
			      path.insert(n, new_source_name);
			      std::get<1>(ttup) = std::filesystem::u8path(path);
			    }
			}
		      writev.push_back(ttup);
		      break;
		    }
		  default:
		    break;
		  }
	      }
	    break;
	  }
	case std::filesystem::file_type::regular:
	case std::filesystem::file_type::symlink:
	  {
	    std::tuple<std::filesystem::path, std::filesystem::path> ttup;
	    std::get<0>(ttup) = sourcepath;
	    std::get<1>(ttup) = std::filesystem::relative(sourcepath, base);
	    if(rename_source)
	      {
		std::string remove_str = sourcepath.filename().u8string();
		std::string path = std::get<1>(ttup).u8string();
		std::string::size_type n = path.find(remove_str);
		if(n != std::string::npos)
		  {
		    path.erase(n, remove_str.size());
		    path.insert(n, new_source_name);
		    std::get<1>(ttup) = std::filesystem::u8path(path);
		  }
	      }
	    writev.push_back(ttup);
	    break;
	  }
	default:
	  break;
	}
      int er;
      for(auto it = writev.begin(); it != writev.end(); it++)
	{
	  er = write_func(a.get(), std::get<0>(*it), std::get<1>(*it));
	  if(er != ARCHIVE_OK)
	    {
	      std::cout
		  << "LibArchive::libarchive_packing: archive writing error"
		  << std::endl;
	      return er;
	    }
	}
    }
  else
    {
      return -200;
    }

  return 0;
}
