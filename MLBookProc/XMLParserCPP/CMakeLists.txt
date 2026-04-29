cmake_minimum_required(VERSION 3.16)

project(XMLParserCPP VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(CREATE_DOCS_XMLPARSERCPP "Build html documentation for XMLParserCPP" OFF)

if(BUILD_SHARED_LIBS)
    add_library(XMLParserCPP SHARED)
    set_target_properties(XMLParserCPP PROPERTIES
      VERSION ${PROJECT_VERSION}
      SOVERSION ${PROJECT_VERSION_MAJOR}
    )
else()
    add_library(XMLParserCPP STATIC)
endif()

set_target_properties(XMLParserCPP PROPERTIES POSITION_INDEPENDENT_CODE True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(ICUUC REQUIRED IMPORTED_TARGET icu-uc)

if(CREATE_DOCS_XMLPARSERCPP)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

add_subdirectory(include)
add_subdirectory(src)

target_include_directories(XMLParserCPP
    PRIVATE include
)

target_link_libraries(XMLParserCPP
    PUBLIC PkgConfig::ICU
    PUBLIC PkgConfig::ICUUC
)

if(CREATE_DOCS_XMLPARSERCPP)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_GENERATE_MAN YES)
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)

  doxygen_add_docs(doxygen_xmlparsercpp
    ALL
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    COMMENT "Generating ${PROJECT_NAME} documentation"
  )  
endif()

include(GNUInstallDirs)

if(CREATE_DOCS_XMLPARSERCPP)
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}/html"
  )

  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/man/"
    DESTINATION "${CMAKE_INSTALL_MANDIR}"
  )
endif()

install(TARGETS XMLParserCPP EXPORT "${PROJECT_NAME}Targets"
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

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${XMLParserCPP_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
