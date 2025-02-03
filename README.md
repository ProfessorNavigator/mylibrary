# MyLibrary

## Description

MyLibrary is a simple program for managing `.fb2`, `.epub`, `.pdf` and `.djvu` e-book file collections. It can also works with same formats packed in zip, 7z, jar, cpio, iso, tar, tar.gz, tar.bz2, tar.xz, rar (see notes) archives itself or packed in same types of  archives with `.fbd` files (epub, djvu and pdf books). MyLibrary creates own database and does not change files content, names or location.

## Installation

### Linux

`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`cmake -DCMAKE_BUILD_TYPE=release -G"Unix Makefiles" -B _build`\
`make -C _build install`

You may need to set install prefix by option CMAKE_INSTALL_PREFIX (default prefix is `/usr/local`).

If you need OpenMP support, set option USE_OPENMP to `ON` (see also `Dependencies` section).

If you get linker errors during build process, try to set option USE_TBB to `ON` (see also `Dependencies` section).

You may need superuser privileges to execute last command.

### Windows

You can build MyLibrary from source by MSYS2 project [https://www.msys2.org/](https://www.msys2.org/). Follow installation instructions from projects site. Install dependencies from `Dependencies` section and git (mingw-... packages). Than create directory you want to download source code to (path must not include spaces or non ASCII symbols). Open mingw console and execute following commands (in example we download code to C:\MyLibrary).

`cd /c/MyLibrary`\
`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`cmake -DCMAKE_BUILD_TYPE=release -DCMAKE_CXX_FLAGS="-mwindows" -G"MinGW Makefiles" -B _build`\
`mingw32-make -C _build install`

You also need to set CMAKE_INSTALL_PREFIX (depends from your choice it can be for example `/ucrt64` or `/mingw64`)

If you need OpenMP support, set option USE_OPENMP to `ON` (see also `Dependencies` section).

If you get linker errors during build process, try to set option USE_TBB to `ON` (see also `Dependencies` section).

If everything was correct, you can find mylibrary.exe file in `msys_dir/mingw_dir/bin`. Icon to create desktop shortcut can be found in `msys_dir/mingw_dir/share/icons/hicolor/256x256/apps/mylibrary.ico` path.

Experimental installer is available now (see releases). 

## Dependencies

MyLibrary uses cmake building system, so to build it from source you need cmake and for example make or ninja. Also you need [gtkmm-4.0](http://www.gtkmm.org/), [poppler](https://poppler.freedesktop.org/), [DjVuLibre](https://djvu.sourceforge.net/), [libarchive](https://libarchive.org/), [icu](https://icu.unicode.org/) (version >= 69), [libgcrypt](https://www.gnupg.org/software/libgcrypt/) and [libgpg-error](https://www.gnupg.org/software/libgpg-error/). All libraries must have headers (for building), so if you use for example Debian Linux, you need ...-dev versions of packages.

If you plan to use OpenMP or tbb, you also may need to install proper libraries (libgomp and oneTBB for example).

## Usage

It is simple. Just create collection (see proper menu item), search book and open it (right mouse click on book). Book will be opened by default system file reader proper to file type. Also you can create book-marks (right mouse click on book) and read book later. Book can be removed from collection, added to collection or copied to any path you want. Also you can add to collection directories and archives with books. Books and directories can be packed to archives on adding. You can refresh collection, remove it, export or import collection database. Also you can manually edit database entries and move book from one collection to another.

## Notes about archives usage
1. rar archives are supported partially. This means, that only archive reading is supported. Any operation, which need archive writing, will be rejected with proper warning.
2. In some cases MyLibrary can crash on rar archives processing (due to libacrhive bug, in MyLibrary 3.1 this error was partially corrected - program does not crash, but archives in some cases are not processed). Users are recommended to repack books to any other supported types of archives manually. 

## License

GPLv3 (see `COPYING`).

## Donation

If you want to help to develop this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573)

## Contacts

You can contact author by email \
bobilev_yury@mail.ru