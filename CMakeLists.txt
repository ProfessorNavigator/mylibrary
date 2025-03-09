cmake_minimum_required(VERSION 3.13)

project(MyLibrary VERSION 4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_PLUGINS "Build with plugin support" OFF)

find_package(PkgConfig REQUIRED)

find_package(Intl REQUIRED)
find_package(Gettext)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

add_executable(mylibrary)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(mlbookproc)

if(USE_OPENMP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP -fopenmp")
    add_link_options("-fopenmp")
endif()

if(GTKMM_VERSION VERSION_LESS "4.10")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DML_GTK_OLD")
endif()

if(USE_PLUGINS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PLUGINS")
    add_subdirectory(mlpluginifc)
    target_include_directories(mylibrary PRIVATE mlpluginifc/include)
    target_link_libraries(mlpluginifc PRIVATE mlbookproc)
    target_link_libraries(mylibrary PRIVATE mlpluginifc)
endif()

target_include_directories(mylibrary
    PRIVATE mlbookproc/include
    PRIVATE PkgConfig::GTKMM
    PRIVATE include        
)

target_link_libraries(mylibrary
    PRIVATE mlbookproc
    PRIVATE PkgConfig::GTKMM
    PRIVATE atomic
)

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


