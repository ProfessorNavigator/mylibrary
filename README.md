# MyLibrary

## Description

MyLibrary is a simple programe for catalogin `.fb2` and `.epub` files. It can also works with `.fb2` and `.epub` files, packed in zip archives.

## Installation

### Linux

`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`meson setup -Dbuildtype=release build`\
`ninja -C build install`

You may need superuser privileges to execute last command.

### Windows

You can build MyLibrary from source by MSYS2 project [https://www.msys2.org/](https://www.msys2.org/). Follow installation instructions from their site. Install dependencies from `Dependencies` section and git (mingw packages). Than create folder where you want to download source code (path must not include spaces or non Latin letters). Open mingw console and execute following commands (in example we download code to C:\MyLibrary)\

`cd /c/MyLibrary`\
`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`meson setup -Dbuildtype=release build`\
`ninja -C build install`

If everything was correct, you can find mylibrary.exe file in `msys_folder/mingw_folder/bin`. Icon to create desktop shortcut can be found in `msys_folder/mingw_folder/share/icons/hicolor/256x256/apps/mylibrary.ico` path. 

## Dependencies

MyLibrary uses meson building system, so to build it from source you need meson and ninja.\
Also you need [gtkmm-4.0](http://www.gtkmm.org/),  [libzip](https://libzip.org/), [icu](https://icu.unicode.org/) (version >= 69), and [libgcrypt](https://www.gnupg.org/software/libgcrypt/). All libraries must have headers (for building), so if you use for example Debian Linux, you need ...-dev versions of packages.

## Usage

It is simple. Just create collection (see proper menu item), search book and open it (right mouse click on book). Book will be opened in default system `.fb2`/`.epub` file reader. Also you can create book-marks (right mouse click on book) and read it later (proper menu item). Book can be removed from collection, added to collection or copied to any path you want.

## License

GPLv3 (see `COPYING`).

## Donation

If you want to help to develop this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573)

## Contacts

You can contact author by email \
bobilev_yury@mail.ru