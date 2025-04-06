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

#ifdef USE_OPENMP
#include <omp.h>
#endif
#ifdef __linux
#include <cstring>
#include <stdlib.h>
#endif

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

#ifdef __linux
      {
        int res = setenv("GTK_THEME", "Adwaita", 1);
        if(res < 0)
          {
            std::cout << "MyLibrary main setenv error: "
                      << std::strerror(errno) << std::endl;
          }
      }
#endif

      int result;
#ifdef USE_OPENMP
      omp_set_dynamic(true);
      omp_set_max_active_levels(3);
#pragma omp parallel
      {
#pragma omp master
        {
          auto app = MyLibraryApplication::create(af);
          result = app->run(argc, argv);
          af.reset();
          app.reset();
        }
      }
#endif
#ifndef USE_OPENMP
      auto app = MyLibraryApplication::create(af);
      result = app->run(argc, argv);
      af.reset();
      app.reset();
#endif
      return result;
    }
  else
    {
      std::cout << "MyLibrary: libgcrypt has not been initialized, "
                   "finishing the application"
                << std::endl;
      af.reset();
      return 1;
    }
}
