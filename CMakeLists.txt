cmake_minimum_required(VERSION 3.19)
project(MyLibrary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(MyLibrary
    WIN32 MACOSX_BUNDLE     
)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(MLBookProc)
add_subdirectory(MLPlugin)

qt_add_translations(MyLibrary
    TS_FILES translations/MyLibrary_ru_RU.ts
)

qt_add_resources(MyLibrary "styles"
    PREFIX "/"
    FILES "styles/styles.css" "styles/checkbox.png"
)

qt_add_resources(MyLibrary "icon"
    PREFIX "/icons"
    FILES "mylibrary.svg"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(IMAGEMAGICK REQUIRED IMPORTED_TARGET Magick++)

find_package(Threads REQUIRED)

target_include_directories(MyLibrary
    PRIVATE
        include
        MLBookProc/include
        MLBookProc/XMLParserCPP/include
        MLPlugin/include
)

target_link_libraries(MyLibrary
    PRIVATE
        Qt::Core
        Qt::Widgets
        PkgConfig::IMAGEMAGICK
        Threads::Threads
        MLBookProc
        MLPlugin
)

target_include_directories(MLPlugin
    PRIVATE
        MLBookProc/include
)

target_link_libraries(MLPlugin
    PRIVATE
        MLBookProc
)

include(GNUInstallDirs)

install(TARGETS MyLibrary
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    install(FILES ru.mail.bobilev_yury.MyLibrary.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    install(FILES mylibrary.svg
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    install(FILES mylibrary.ico
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps")
endif()