cmake_minimum_required(VERSION 3.13)

project(MyLibrary VERSION 3.2 LANGUAGES CXX)

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

if(USE_TBB)
  find_package(TBB REQUIRED)
endif()

if(POPPLER_VERSION VERSION_LESS "22.05")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OLDPOPPLER")
endif()

if(GTKMM_VERSION VERSION_LESS "4.10")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DML_GTK_OLD")
endif()

if(USE_OPENMP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP -fopenmp")
  add_link_options("-fopenmp")
endif()

add_executable(mylibrary)

add_subdirectory(src)
add_subdirectory(include)

target_include_directories(mylibrary PRIVATE include
                                     PRIVATE PkgConfig::GTKMM
                                     PRIVATE PkgConfig::POPPLER
                                     PRIVATE PkgConfig::ICU
                                     PRIVATE PkgConfig::LIBARCHIVE
                                     PRIVATE PkgConfig::Gcrypt
                                     PRIVATE PkgConfig::GPG-ERROR)

target_link_libraries(mylibrary PRIVATE PkgConfig::GTKMM
                                PRIVATE PkgConfig::POPPLER
                                PRIVATE PkgConfig::ICU
                                PRIVATE PkgConfig::LIBARCHIVE
                                PRIVATE ${DJVU}
                                PRIVATE PkgConfig::Gcrypt
                                PRIVATE PkgConfig::GPG-ERROR
                                PRIVATE atomic)

if(USE_TBB)
  target_link_libraries(mylibrary PRIVATE TBB::tbb)
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
    mylibrary.svg DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps")

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


