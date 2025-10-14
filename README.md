# MyLibrary

## Description

MyLibrary is a simple program for managing `.fb2`, `.epub`, `.pdf`, `.djvu`,  `.odt`, `.txt` and `.md` e-book file collections. It can also work with same formats packed in zip, 7z, jar, cpio, iso, tar, tar.gz, tar.bz2, tar.xz, rar (see `Notes`) archives itself or packed in same types of  archives with `.fbd` files (any types of files, not only books). MyLibrary creates own database and does not change files content, names or locations. See `Usage` for details.

## Installation

### Linux

`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`cmake -DCMAKE_BUILD_TYPE=release -B _build`\
`cmake --build _build --target test`\
`cmake --build _build`\
`cmake --install _build`

You may need superuser privileges to execute last command.

You may need to set install prefix by option CMAKE_INSTALL_PREFIX (default prefix is `/usr/local`).

MyLibrary includes MLBookProc library (see `Notes`). To build html documentation for MLBookProc, set option CREATE_HTML_DOCS_MLBOOKPROC to `ON` (also see `Dependencies`). To build pdf documentation for MLBookProc set option CREATE_PDF_DOCS_MLBOOKPROC to `ON` (also see `Dependencies`).

Program will be built with OpenMP support by default. If you do not need OpenMP support set USE_OPENMP option to `OFF` (see also `Dependencies` and `Notes` section). In case of building with OpenMP support, public build variable USE_OPENMP will be set in MLBookProc cmake package configuration file. This variable can be used in your project compiler ifdef directives in case of any need. 

There is optional plugins support in MyLibrary. To use plugins, set USE_PLUGINS option to `ON`. If you need html documentation for plugins creation, set CREATE_HTML_DOCS_PLUGINIFC option to `ON` (also see `Dependencies`). If you need pdf documentation for plugins creation, set CREATE_PDF_DOCS_PLUGINIFC option to `ON` (also see `Dependencies`). If USE_PLUGINS variable is `ON`, in case of gtkmm version lower than 4.10 public build variable ML_GTK_OLD will be set. You can use this variable in ifdef compiler directives in your plugins.

Tests were added in version 4.2 ([ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) needs to be installed). It is recommended to run tests (`cmake --build _build --target test`) after configure stage, just before program building. Special attention should be paid to OpenMPTest. If this test fails, you should NOT build program with OpenMP support.

### Windows

You can build MyLibrary from source by MSYS2 project [https://www.msys2.org/](https://www.msys2.org/). Follow installation instructions from projects site, install dependencies from `Dependencies` section and git, then create directory you want to download source code to (path must not include spaces or non ASCII symbols). Open MinGW console and execute following commands (in example we download code to C:\MyLibrary).

`cd /c/MyLibrary`\
`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`cmake -DCMAKE_BUILD_TYPE=Release -B ../_build`\
`cmake --build ../_build --target test`\
`cmake --build ../_build`\
`cmake --install ../_build`

Also you need to set CMAKE_INSTALL_PREFIX (depends from your choice it can be for example `/ucrt64` or `/mingw64`)

MyLibrary includes MLBookProc library (see `Notes`). To build html documentation for MLBookProc, set option CREATE_HTML_DOCS_MLBOOKPROC to `ON` (also see `Dependencies`). To build pdf documentation for MLBookProc set option CREATE_PDF_DOCS_MLBOOKPROC to `ON` (also see `Dependencies`).

Program will be built with OpenMP support by default. If you do not need OpenMP support set USE_OPENMP option to `OFF` (see also `Dependencies` and `Notes` sections). In case of building with OpenMP support, public build variable USE_OPENMP will be set in MLBookProc cmake package configuration file. This variable can be used in your project compiler ifdef directives in case of any need.

There is optional plugins support in MyLibrary. To use plugins, set USE_PLUGINS option to `ON`. If you need html documentation for plugins creation, set CREATE_HTML_DOCS_PLUGINIFC option to `ON` (also see `Dependencies`). If you need pdf documentation for plugins creation, set CREATE_PDF_DOCS_PLUGINIFC option to `ON` (also see `Dependencies`). If USE_PLUGINS variable is `ON`, in case of gtkmm version lower than 4.10 public build variable ML_GTK_OLD will be set. You can use this variable in ifdef compiler directives in your plugins.

If everything was correct, you can find mylibrary.exe file in `msys_dir/mingw_dir/bin`. Icon to create desktop shortcut can be found in `msys_dir/mingw_dir/share/icons/hicolor/512x512/apps/mylibrary.ico` path.

Tests were added in version 4.2 ([ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) needs to be installed). It is recommended to run tests (`cmake --build _build --target test`) after configure stage, just before program building. Special attention should be paid to OpenMPTest. If this test fails, you should NOT build program with OpenMP support.

Experimental installer is available now (see releases).

See `Notes` also. 

## Dependencies

MyLibrary uses cmake building system, so to build it from source you need cmake and for example make or ninja. Also you need [gtkmm-4.0](http://www.gtkmm.org/), [poppler](https://poppler.freedesktop.org/), [DjVuLibre](https://djvu.sourceforge.net/), [libarchive](https://libarchive.org/), [icu](https://icu.unicode.org/) (version >= 69), [libgcrypt](https://www.gnupg.org/software/libgcrypt/), [libgpg-error](https://www.gnupg.org/software/libgpg-error/) and [Magick++](https://imagemagick.org/Magick++/). All libraries must have headers (for building), so if you use for example Debian Linux, you need ...-dev versions of packages.

If you plan to use OpenMP, you also may need to install proper libraries (like libgomp).

To build documentation, you need doxygen. To build pdf documentation, you need LaTeX support (see doxygen [documentation](https://www.doxygen.nl/manual/output.html)). Also to build pdf documentation you need make. 

## Usage

It is simple. Just create collection (see proper menu item), search book and open it (right mouse click on book). Book will be opened by default system file reader proper to file type. Also you can create book-marks (right mouse click on book) and read book later. Book can be removed from collection, added to collection or copied to any place you want. Also you can add to collection directories and archives with books. Books and directories can be packed to archives on adding. You can refresh collection, remove it, export or import collection database. Also you can manually edit database entries and move book from one collection to another. You can create notes for books, list all collection files or list collection authors. Also you can use plugins (optionally). 

## Notes
### Notes about archives usage
1. rar archives are supported partially. This means, that only archive reading is supported. Any operation, which needs archive writing, will be rejected with proper warning.
2. In some cases MyLibrary can crash on rar archives processing (due to libacrhive bug, in MyLibrary 3.1 this error was partially corrected - program does not crash, but archives in some cases are not processed). Users are recommended to repack books to any other supported types of archives manually.

### MLBookProc library
Starting from version 4.0 of MyLibrary, methods to work with collections, bookmarks and notes have been moved to MLBookProc library. This library is part of MyLibrary, but it can be built and used independently (under the terms of GPLv3 license). To build MLBookProc independently you need to install dependencies from `Dependencies` section (all except gtkmm-4.0 and Magick++). Then proceed to MLBookProc directory and execute cmake commands (same as in `Installation` section). All options from `Installation` section are supported, except USE_PLUGINS, CREATE_HTML_DOCS_PLUGINIFC and CREATE_PDF_DOCS_PLUGINIFC. You can include MLBookProc in your project by following cmake commands:

`find_package(MLBookProc REQUIRED)`\
`target_link_libraries(<mytarget> PUBLIC MLBookProc::mlbookproc)`

### Notes about OpenMP usage
You may need to set system variable OMP_CANCELLATION to `true` to achieve maximum efficiency. However, if you launch MyLibrary by .desktop file installed from repository no further actions required (OMP_CANCELLATION variable will be set to proper value automatically). Otherwise start command may look like:

`OMP_CANCELLATION=true mylibrary`

If you set OMP_CANCELLATION to `false` or do not set it at all, MyLibrary will work without any errors, but some functions may be slightly slower than usual.

### Windows usage
1. Executable file and libraries from the installer are built without OpenMP support.
2. Documentation for MLBookProc and for plugins development is not included into the installer. Header files for MLBookProc and plugins development are also not included into the installer. If you need them, use MSYS2 project and/or build MyLibary from sources.

## Officially supported plugins
### MLInpxPlugin
Plugin for collections import from `inpx` files. \
[Link 1](https://github.com/ProfessorNavigator/mlinpxplugin) \
[Link 2](https://altlinux.space/professornavigator/mlinpxplugin)


### MLFBDPlugin
Plugin for fbd format books creation (book file will be packed into the archive along with the fbd file, containing `description` tag of fb2 format). It is possible to add files of any type, not only books. \
[Link 1](https://github.com/ProfessorNavigator/mlfbdplugin) \
[Link 2](https://altlinux.space/professornavigator/mlfbdplugin)
 

## License

GPLv3 (see `COPYING`).

## Donation

If you want to help to develop this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573)

## Contacts

You can contact author by email \
bobilev_yury@mail.ru
