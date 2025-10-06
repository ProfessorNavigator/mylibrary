/*
 * Copyright (C) 2022-2025 Yury Bobylev <bobilev_yury@mail.ru>
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

#include <AuxFunc.h>
#include <MyLibraryApplication.h>
#include <iostream>
#include <libintl.h>

int
main(int argc, char *argv[])
{
  std::shared_ptr<AuxFunc> af = AuxFunc::create();

  if(af->get_activated())
    {
      std::filesystem::path p = af->share_path();
      p /= std::filesystem::u8path("locale");
      char *report = bindtextdomain("MyLibrary", p.u8string().c_str());
      if(report)
        {
          std::cout << "MyLibrary text domain path: " << report << std::endl;
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

      int result;
      auto app = MyLibraryApplication::create(af);
      result = app->run(argc, argv);
      app.reset();
      af.reset();
      return result;
    }
  else
    {
      std::cout << "MyLibrary: MLBookProc has not been initialized, "
                   "finishing the application"
                << std::endl;
      af.reset();
      return 1;
    }
}
