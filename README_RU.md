# MyLibrary

## Описание
MyLibrary - это простая программа для создания и поддержания  коллекций книг в форматах  `.fb2`, `.epub`, `.pdf`, `.djvu` и `.fbd` (для книг в форматах epub, djvu и pdf). Возможна также работа с книгами, упакованными в различные типы архивов. Поддерживаются архивы zip, 7z, jar, cpio, iso, tar, tar.gz, tar.bz2, tar.xz, rar (см. замечания). В процессе формирования коллекции создаётся собственная база данных, исходные файлы книг, их расположение и названия при этом не изменяются.

## Установка

### Linux

`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`cmake -DCMAKE_BUILD_TYPE=release -G"Unix Makefiles" -B _build`\
`make -C _build install`

Дополнительно может понадобиться установить префикс с помощью опции CMAKE_INSTALL_PREFIX (префикс по умолчанию `/usr/local`).

Если вам необходима поддержка OpenMP, установите опцию USE_OPENMP в `ON` (см. также секцию `Зависимости`).

Если в процессе компиляции появляются ошибки линковщика, попробуйте установить опцию USE_TBB в `ON` (см. также секцию `Зависимости`).

Для выполнения последней команды могут потребоваться привилегии суперпользователя.

### Windows

В данный момент установка возможна с помощью проекта [MSYS2](https://www.msys2.org/). Выполните инструкции по установке с сайта проекта, а также установите зависимости, указанные в секции `Зависимости`. Кроме того вам может понадобиться git. Сборка тестировалась с пакетами версий mingw-... и может быть осуществлена в терминале MinGW с помощью следующих команд (в данном случае предполагается, что код скачивается в C:\MyLibrary, путь не должен содержать пробелы или символы, не входящие в кодировку ASCII) : 

`cd /c/MyLibrary`\
`git clone https://github.com/ProfessorNavigator/mylibrary.git`\
`cd mylibrary`\
`cmake -DCMAKE_BUILD_TYPE=release -DCMAKE_CXX_FLAGS="-mwindows" -G"MinGW Makefiles" -B _build`\
`mingw32-make -C _build install`

Вам также обязательно нужно установить префикс с помощью опции CMAKE_INSTALL_PREFIX (в зависимости от выбранного в MSYS2 типа пакетов, он может быть например `/ucrt` или `/mingw64`).

Если вам необходима поддержка OpenMP, установите опцию USE_OPENMP в `ON` (см. также секцию `Зависимости`).

Если в процессе компиляции появляются ошибки линковщика, попробуйте установить опцию USE_TBB в `ON` (см. также секцию `Зависимости`).

Если всё прошло корректно, то исполняемый файл mylibrary.exe будет находиться в `msys_dir/mingw_dir/bin`.  Иконка для создания ярлыка на рабочем столе  -  в `msys_dir/mingw_dir/share/icons/hicolor/256x256/apps/mylibrary.ico`.

Также теперь доступен экспериментальный инсталлятор (см. релизы).

## Зависимости

В MyLibrary используются следующие бибилиотеки:  [gtkmm-4.0](http://www.gtkmm.org/), [poppler](https://poppler.freedesktop.org/), [DjVuLibre](https://djvu.sourceforge.net/), [libarchive](https://libarchive.org/), [icu](https://icu.unicode.org/) (версия >= 69), [libgcrypt](https://www.gnupg.org/software/libgcrypt/) и [libgpg-error](https://www.gnupg.org/software/libgpg-error/). Все библиотеки для сборки должны иметь заголовочные файлы, т.е. если вы используете например Debian Linux, то вам потребуются также ...-dev версии пакетов. Кроме того для сборки понадобятся cmake и, например, make или ninja.

Если вы планируете использовать OpenMP или tbb, то может потребоваться установить соответствующие библиотеки (например libgomp и oneTBB).

## Использование

Всё просто: создайте коллекцию с помощью соответствующего пункта меню, выполните поиск и откройте книгу (щелчок правой кнопкой мыши на строке с книгой в таблице с результатами поиска). Книга откроется в программе, используемой в системе для обработки соответствующего типа файлов по умолчанию. Кроме того вы можете удалить книгу из коллекции, добавить книгу в коллекцию, добавить в коллекцию папку с книгами, произвести операции, перечисленные выше, со сжатием в архив, скопировать книгу коллекции в нужную вам папку, добавить в коллекцию архив с книгами. Также возможно обновление коллекции, её удаление, возможен экспорт и импорт базы данных коллекции и ручное редактирование записей в базе данных, перемещение книги из одной коллекции в другую. Доступен механизм закладок для быстрого доступа к книгам.

## Замечания по использованию архивов
1. rar архивы поддерживаются частично - доступны лишь операции, связанные с чтением архивов. Операции, требующие осуществления записи в архив, будут отклоняться с выводом соответствующего предупреждения.
2. При работе с rar архивами может произойти нештатное завершение программы (ошибка в libarchive, в MyLibrary 3.1 данное поведение частично исправлено: аварийного завершения программы не происходит, однако некоторые rar архивы по-прежнему могут не обрабатываться). В связи с этим пользователям рекомендуется перепаковать книги в любой другой поддерживаемый тип архивов вручную.

## Лицензия

GPLv3 (см. `COPYING`).

## Поддержка

Если есть желание поддержать проект, то можно пройти по следующей ссылке: [поддержка](https://yoomoney.ru/to/4100117795409573)

## Контакты автора

Вопросы, пожелания, предложения и отзывы можно направлять на следующий адрес: \
bobilev_yury@mail.ru