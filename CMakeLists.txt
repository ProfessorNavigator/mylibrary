cmake_minimum_required(VERSION 3.16)

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
add_subdirectory(MLBookProc)

if(USE_OPENMP)
    try_compile(OMP_TEST "${CMAKE_BINARY_DIR}/MLBookProc/omp_test" "${PROJECT_SOURCE_DIR}/MLBookProc/omp_test" OmpTest)
    if(OMP_TEST)
        find_package(OpenMP REQUIRED)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP ${OpenMP_CXX_FLAGS}")
        add_link_options("${OpenMP_CXX_FLAGS}")
        message(STATUS "OpenMP features are supported, ${PROJECT_NAME} will be built with OpenMP support")
    else()
        set(USE_OPENMP OFF)
        message(WARNING "Compiler does not support some OpenMP features. ${PROJECT_NAME} will be built without OpenMP support.")
        find_package(TBB QUIET)
        if(TBB_FOUND)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_TBB")
        endif()
    endif()
else()
    find_package(TBB QUIET)
    if(TBB_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_TBB")
    endif()
endif()

if(GTKMM_VERSION VERSION_LESS "4.10")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DML_GTK_OLD")
endif()

if(USE_PLUGINS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PLUGINS")
    add_subdirectory(MLPluginIfc)
    target_include_directories(mylibrary PRIVATE MLPluginIfc/include)
    target_link_libraries(mlpluginifc PRIVATE mlbookproc)
    target_link_libraries(mylibrary PRIVATE mlpluginifc)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(mylibrary PRIVATE ${CMAKE_DL_LIBS})
    endif()
endif()

target_include_directories(mylibrary
    PRIVATE MLBookProc/include
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

    if(USE_OPENMP)
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


