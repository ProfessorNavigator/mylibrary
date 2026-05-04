# MyLibrary
## Description
MyLibrary is a simple program designed to manage e-book collections. It supports following types of books: `.fb2`, `.epub`, `.pdf`, `.djvu`, `.odt`, `.txt`, `.md` and `.fbd` (`fbd` can be used for any types of files, not just books). MyLibrary also supports same types of books, packed in archives. Supported archive types are: zip, 7z, jar, cpio, iso, tar, tar.gz, tar.bz2, tar.xz, rar (see `Notes`). Additionally MyLibrary supports inpx collections.  Program creates own databases, e-book files will not be moved or edited. See also `Usage` section.

## Installation
### Linux
`cmake -DCMAKE_BUILD_TYPE=Release -B _build`\
`cmake --build _build`\
`cmake --install _build`

You may need to set installation path by CMAKE_INSTALL_PREFIX (default path is `/usr/local`).

MLBookProc library is included in MyLibrary code base (see `Notes`). To create MLBookProc library documentation, set BUILD_MLBOOKPROC_DOCS option to `ON`.

XMLParserCPP library is included in MLBookProc (see `Notes`). To create XMLParserCPP documentation, set CREATE_DOCS_XMLPARSERCPP option to `ON`.

MyLibrary supports plugins. If you need documentation for plugins creation, set BUILD_MLPLUGIN_DOCS option to `ON` (see also `Dependencies`).

### Windows
You can build MyLibrary from sources by [MSYS2](https://www.msys2.org/) project assistance. Follow the installation instructions from [MSYS2](https://www.msys2.org/) website and install dependencies then (see `Dependencies`). You may also need git. Building can be carried out in MinGW console by the following commands:  

`cmake -DCMAKE_BUILD_TYPE=Release -B ../_build`\
`cmake --build ../_build`\
`cmake --install ../_build`

Also you must set CMAKE_PREFIX_PATH (it can be `/ucrt64` or `/mingw64` for example - choose by used MinGW console type).

MLBookProc library is included in MyLibrary code base (see `Notes`). To create MLBookProc library documentation, set BUILD_MLBOOKPROC_DOCS option to `ON`.

XMLParserCPP library is included in MLBookProc (see `Notes`). To create XMLParserCPP documentation, set CREATE_DOCS_XMLPARSERCPP option to `ON`.

MyLibrary supports plugins. If you need documentation for plugins creation, set BUILD_MLPLUGIN_DOCS option to `ON` (see also `Dependencies`).

If all operations has been carried out correctly, you can find MyLibrary.exe file in `<msys_dir>/<mingw_type_dir>/bin`. The icon for desktop shortcut creation can be found in `<msys_dir>/<mingw_type_dir>/share/icons/hicolor/512x512/apps/mylibrary.ico`.

Experimental installer is also available (see releases). 

## Dependencies
MyLibrary uses following libraries: [Qt6](https://www.qt.io/) (Core, Widgets, LinguistTools components), [poppler](https://poppler.freedesktop.org/), [DjVuLibre](https://djvu.sourceforge.net/), [libarchive](https://libarchive.org/), [icu](https://icu.unicode.org/) (version >= 69), [libgcrypt](https://www.gnupg.org/software/libgcrypt/), [libgpg-error](https://www.gnupg.org/software/libgpg-error/), [Magick++](https://imagemagick.org/magick++/#gsc.tab=0), [LibUDB](https://github.com/ProfessorNavigator/libudb). All libraries must have headers, so, if you use Debian Linux for example, you need to install ...-dev packages versions. Also you need [CMake](https://cmake.org/) and make, ninja or any other building program.

Your compiler must support OpenMP standard, you may need to install proper libraries so (libgomp for example).

You need doxygen, if you want to create documentation.

## Usage
### Base usage
Everything is simple: create collection by proper menu option, search book and open it (right mouse click on book line in search results table). Selected book will be opened in program set as default program by your OS for book file type. Also you can remove book from collection, add book to collection, add directory with books to collection, copy book to directory on your choice, add archive with books to collection. You can refresh collection, remove it, export or import collection database. You may edit database entries manually. You can create bookmarks for fast access to books and notes for selected book. You may enable or disable plugins on your choice. You can list all collection files and all collection authors. You may create collections from inpx files. See `Notes` also.

### Advanced usage.
`Surname` search line can be used as universal search line: you can enter not only surname, but surname, name, second name, nickname or all above at once.

You can set search results coefficient of coincidence in search settings.

Collection books can be located on external server. In that case you should set access to server by SMB protocol and mount books directories by gvfs, kio-fuse or any other similar program on local computer (keeping authorization). If everything has been set correctly, MyLibrary can use books located on external server by the same way as local without any extra settings. You can also use collection export/import functions to copy collection from your computer to any other machines in your network. 

## Notes
### Collections types
Starting from version 5.0 three types of collections are supported: legacy, inpx and native. Legacy collections are collections created by previous version of the program. Inpx collections are collections created from inpx files. Native collections are collections created by program of current version. Legacy and inpx collections have limited functionality: you cannot edit database entries manually, some other functions are also disabled. Refresh collection function in case of legacy or inpx collection will recreate collection as native. If you do not want to recreate inpx collection as native, you can just replace inpx file by new one manually and reload collection (see main menu). 

### Notes about archives usage
1. Rar archives are supported partially. This means, that only archive reading is supported. Any operation, which needs archive writing, will be rejected with proper warning.
2. In some cases MyLibrary can crash on rar archives processing (due to libacrhive bug). Users are recommended to repack books to any other supported types of archives manually.

### MLBookProc notes
Starting from version 4.0 most functions for books, collections, bookmarks and notes processing have been moved to MLBookProc library. This library is included in MyLibary but can also be built and used separately (on GPLv3 license conditions). To use MLBookProc independently you need to install dependencies from `Dependencies` section (all except Qt and Magick++). Then you should proceed to MLBookProc directory and carry out CMake commands, as explained in `Installation` section. MLBookProc library can be linked to your project as follows:

`find_package(MLBookProc REQUIRED)`\
`target_link_libraries(<mytarget> PUBLIC MLBookProc::MLBookProc)`

### XMLParserCPP notes
Starting from version 4.3 all xml processing functions have been moved to XMLParserCPP library (included in MLBookProc). To use it independently you should install icu (see `Dependencies`). Then proceed to MLBookProc/XMLParserCPP directory and carry out CMake commands, as explained in `Installation` section. For XMLParserCPP, the CREATE_DOCS_XMLPARSERCPP option is relevant. XMLParserCPP includes methods and classes for xml files parsing, for xml files writing and also methods for text encodings processing. See XMLParserCPP documentation for details.

You can link XMLParserCPP to your project as follows:

`find_package(XMLParserCPP REQUIRED)`\
`target_link_libraries(<mytarget> PUBLIC XMLParserCPP::XMLParserCPP)`

### OpenMP usage notes
You may need to set system variable OMP_CANCELLATION to `true` to achieve maximum efficiency. However, if you launch MyLibrary by .desktop file installed from repository no further actions required (OMP_CANCELLATION variable will be set to proper value automatically). Otherwise start command may be the following:

`OMP_CANCELLATION=true mylibrary`

If you set OMP_CANCELLATION to `false` or do not set it at all, MyLibrary will work without any errors, but some functions may be slightly slower than usual.

### Windows usage
Installer build does not have any documentation or header files. If you need to create plugin or need to use MLBookProc and XMLParserCPP libraries in your project, use MSYS2 installation instead.

## Officially supported plugins
### MLArchiverPlugin
Archives creating plugin.\
[Link 1](https://altlinux.space/professornavigator/mlarchiverplugin) \
[Link 2](https://github.com/ProfessorNavigator/mlarchiverplugin)

### MLFBDPlugin
Fbd files creating plugin (file will be packed to archive alongside with file with fbd extension, containing fb2 `description` tag). Any types of files can be packed. \
[Link 1](https://altlinux.space/professornavigator/mlfbdplugin) \
[Link 2](https://github.com/ProfessorNavigator/mlfbdplugin)

## License

GPLv3 (see `COPYING`).

## Donation

If you want to help to develop this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573)

## Contacts

You can contact author by email \
bobilev_yury@mail.ru
