/*
 * Copyright (C) 2022-2024 Yury Bobylev <bobilev_yury@mail.ru>
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
#include <gcrypt.h>
#include <libdjvu/ddjvuapi.h>
#include <libintl.h>
#include <MyLibraryApplication.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

int
main(int argc, char *argv[])
{
  std::shared_ptr<AuxFunc> af = std::make_shared<AuxFunc>();
  std::filesystem::path p = af->share_path();
  p = p.parent_path();
  p /= std::filesystem::u8path("locale");
  char *report = bindtextdomain("MyLibrary", p.u8string().c_str());
  if(report)
    {
      std::cout << "MyLibrary text domain path: " << report
	  << std::endl;
    }
  report = bind_textdomain_codeset("MyLibrary", "UTF-8");
  if(report)
    {
      std::cout << "MyLibrary codeset: " << report << std::endl;
    }
  report = textdomain("MyLibrary");
  if(report)
    {
      std::cout << "MyLibrary text domain: " << report << std::endl;
    }

  report = const_cast<char*>(gcry_check_version(nullptr));
  if(report)
    {
      gcry_error_t err = gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
      if(err != 0)
	{
	  std::cout << "MyLibrary libgcrypt disabling secmem error: "
	      << af->libgcrypt_error_handling(err) << std::endl;
	  return err;
	}
      err = gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
      if(err != 0)
	{
	  std::cout << "MyLibrary libgcrypt initialization error: "
	      << af->libgcrypt_error_handling(err) << std::endl;
	  return err;
	}
      std::cout
	  << "MyLibrary: libgcrypt has been initialized, version: "
	  << report << std::endl;

      report = const_cast<char*>(archive_version_details());
      if(report)
	{
	  std::cout << "MyLibrary: " << report << std::endl;
	}

      report = const_cast<char*>(ddjvu_get_version_string());
      if(report)
	{
	  std::cout << "MyLibrary: " << report << std::endl;
	}
      auto app = MyLibraryApplication::create(af);
      int result = app->run(argc, argv);
      app.reset();
      af.reset();
      return result;
    }
  else
    {
      std::cout << "MyLibrary: libgcrypt has not been initialized, "
	  "finishing the application" << std::endl;
      af.reset();
      return 1;
    }

}
