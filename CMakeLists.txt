cmake_minimum_required(VERSION 3.13)

project(MyLibrary VERSION 3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(USE_OPENMP "Build with openmp support" OFF)
option(USE_TBB "Build with tbb support" OFF)

find_package(PkgConfig REQUIRED)

find_package(Intl REQUIRED)
find_package(Gettext)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET poppler-cpp)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)
pkg_check_modules(Gcrypt REQUIRED IMPORTED_TARGET libgcrypt)
pkg_check_modules(GPG-ERROR REQUIRED IMPORTED_TARGET gpg-error)

find_library(DJVU djvulibre REQUIRED)

if(POPPLER_VERSION VERSION_LESS "22.05")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OLDPOPPLER")
endif()

if(USE_OPENMP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP -fopenmp")
  add_link_options("-fopenmp")
endif()

if(USE_TBB)
  find_package(TBB REQUIRED)
endif()

add_executable(mylibrary)

add_subdirectory(src)

target_include_directories(mylibrary PRIVATE include)

target_link_libraries(mylibrary PUBLIC
  PkgConfig::GTKMM
  PkgConfig::POPPLER
  PkgConfig::ICU
  PkgConfig::LIBARCHIVE
  ${DJVU}
  PkgConfig::Gcrypt
  PkgConfig::GPG-ERROR
  atomic)

if(USE_TBB)
  target_link_libraries(mylibrary PUBLIC
  TBB::tbb)
endif()

include(GNUInstallDirs)

install(TARGETS mylibrary
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(DIRECTORY MyLibrary DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")

if(CMAKE_SYSTEM_NAME MATCHES "Linux")

    install(FILES
    ru.mail.bobilev_yury.MyLibrary.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

    install(FILES
    mylibrary.png DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps")

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")

    install(FILES
    mylibrary.ico DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps")

endif()

if(Gettext_FOUND)
   GETTEXT_PROCESS_PO_FILES("ru" ALL
   INSTALL_DESTINATION "${CMAKE_INSTALL_LOCALEDIR}"
   PO_FILES po/MyLibrary.po
)
endif()


