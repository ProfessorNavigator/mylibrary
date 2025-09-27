cmake_minimum_required(VERSION 3.16)

project(MyLibrary VERSION 4.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_PLUGINS "Build with plugin support" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(IMAGEMAGICK REQUIRED IMPORTED_TARGET Magick++)

find_package(Intl REQUIRED)
find_package(Gettext)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_executable(mylibrary)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(mylibrary WIN32)
endif()

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(MLBookProc)

if(USE_PLUGINS)
    target_compile_options(mylibrary PRIVATE -DUSE_PLUGINS)
    add_subdirectory(MLPluginIfc)
    target_include_directories(mylibrary PRIVATE MLPluginIfc/include)
    target_link_libraries(mlpluginifc PRIVATE mlbookproc)
    target_link_libraries(mylibrary PRIVATE mlpluginifc)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(mylibrary PRIVATE ${CMAKE_DL_LIBS})
    endif()
endif()

if(BUILD_TESTING)
    include(CTest)
endif()

target_include_directories(mylibrary
    PRIVATE MLBookProc/include    
    PRIVATE include
)

if(GTKMM_VERSION VERSION_LESS "4.10")
    target_compile_options(mylibrary
        PRIVATE -DML_GTK_OLD
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mylibrary
        PRIVATE -DGNU_COMPILER
    )
endif()

target_link_libraries(mylibrary
    PRIVATE mlbookproc
    PRIVATE PkgConfig::GTKMM
    PRIVATE PkgConfig::IMAGEMAGICK
)

include(GNUInstallDirs)

install(TARGETS mylibrary
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(DIRECTORY MyLibrary DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(OMP_TEST)
        install(FILES
        ru.mail.bobilev_yury.MyLibrary.omp.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    else()
        install(FILES
        ru.mail.bobilev_yury.MyLibrary.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    endif()
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


