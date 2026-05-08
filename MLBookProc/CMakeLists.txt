cmake_minimum_required(VERSION 3.16)

project(MLBookProc VERSION 2.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_MLBOOKPROC_DOCS "Build documentation for MLBookProc library" OFF)

add_compile_options(-Wall)

if(BUILD_SHARED_LIBS)
    add_library(MLBookProc SHARED)
    set_target_properties(MLBookProc PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        )
else()
    add_library(MLBookProc STATIC)
endif()

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(XMLParserCPP)

find_package(LibUDB REQUIRED)
find_package(Threads REQUIRED)

if(BUILD_MLBOOKPROC_DOCS)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(ICUUC REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(GCRYPT REQUIRED IMPORTED_TARGET libgcrypt)
pkg_check_modules(GPG-ERROR REQUIRED IMPORTED_TARGET gpg-error)
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)
pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET poppler-cpp)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  pkg_check_modules(DJVU REQUIRED IMPORTED_TARGET ddjvuapi)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
  find_package(DJVU REQUIRED)
endif()

target_include_directories(MLBookProc
    PRIVATE include
    PRIVATE XMLParserCPP/include
)

target_link_libraries(MLBookProc
    PUBLIC LibUDB::udb
    PUBLIC PkgConfig::ICU
    PUBLIC PkgConfig::ICUUC
    PUBLIC PkgConfig::GCRYPT
    PUBLIC PkgConfig::GPG-ERROR
    PUBLIC PkgConfig::LIBARCHIVE
    PUBLIC PkgConfig::POPPLER
    PUBLIC Threads::Threads
    PRIVATE XMLParserCPP    
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(MLBookProc
      PUBLIC PkgConfig::DJVU
    )
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_include_directories(MLBookProc PUBLIC ${DJVU_INCLUDE_DIR})
    target_link_libraries(MLBookProc PUBLIC ${DJVU_DLL})
endif()

if(BUILD_MLBOOKPROC_DOCS)
  set(DOXYGEN_GENERATE_HTML YES)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(DOXYGEN_GENERATE_MAN YES)
  endif()
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)

  doxygen_add_docs(doxygen_mlbookproc
    ALL
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    COMMENT "Generating ${PROJECT_NAME} documentation"
  )
endif()

include(GNUInstallDirs)

install(TARGETS MLBookProc EXPORT "${PROJECT_NAME}Targets"
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(DIRECTORY include/
  EXPORT "${PROJECT_NAME}Targets"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT "${PROJECT_NAME}Targets"
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

if(BUILD_MLBOOKPROC_DOCS)
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}/html"
  )

  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/man/"
    DESTINATION "${CMAKE_INSTALL_MANDIR}"
  )
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${MLBookProc_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

