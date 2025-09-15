cmake_minimum_required(VERSION 3.16)

project(MLPluginIfc VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CREATE_HTML_DOCS_PLUGINIFC "Build html documentation for plugins" OFF)
option(CREATE_PDF_DOCS_PLUGINIFC "Build pdf documentation for plugins" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

if(CREATE_HTML_DOCS_PLUGINIFC OR CREATE_PDF_DOCS_PLUGINIFC)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

if(BUILD_SHARED_LIBS)
  add_library(mlpluginifc SHARED)
else()
  add_library(mlpluginifc STATIC)
endif()

set_target_properties(mlpluginifc PROPERTIES POSITION_INDEPENDENT_CODE True)

if(BUILD_SHARED_LIBS)
  set_target_properties(mlpluginifc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif()

add_subdirectory(src)
add_subdirectory(include)

target_include_directories(mlpluginifc
  PRIVATE include
  PRIVATE ../MLBookProc/include
)

if(GTKMM_VERSION VERSION_LESS "4.10")
  target_compile_options(mlpluginifc
      PUBLIC -DML_GTK_OLD
  )
endif()

target_link_libraries(mlpluginifc  
  PRIVATE mlbookproc
  PUBLIC PkgConfig::GTKMM
)

if(DOXYGEN_FOUND)
  if(CREATE_HTML_DOCS_PLUGINIFC)
    set(DOXYGEN_GENERATE_HTML YES)
  endif()
  if(CREATE_PDF_DOCS_PLUGINIFC)
    set(DOXYGEN_GENERATE_LATEX YES)
    set(DOXYGEN_USE_PDFLATEX YES)
    set(DOXYGEN_PDF_HYPERLINKS YES)
  endif()
  set(DOXYGEN_EXAMPLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)

  doxygen_add_docs(doxygen_mlpluginifc
    ALL
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/include
    COMMENT "Generating documentation"
  )

  if(CREATE_PDF_DOCS_PLUGINIFC)
    add_custom_command(TARGET doxygen_mlpluginifc
      POST_BUILD
      COMMAND make
      ARGS pdf -C "${CMAKE_CURRENT_BINARY_DIR}/latex"
      COMMENT "Generating pdf documentation"
    )
  endif()
endif()

include(GNUInstallDirs)

install(TARGETS mlpluginifc EXPORT "${PROJECT_NAME}Targets"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(DIRECTORY include/
    EXPORT "${PROJECT_NAME}Targets"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.h"
)

if(CREATE_HTML_DOCS_PLUGINIFC)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/html
  )
endif()

if(CREATE_PDF_DOCS_PLUGINIFC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
      DESTINATION ${CMAKE_INSTALL_DOCDIR}/pdf
  )
endif()

install(EXPORT "${PROJECT_NAME}Targets"
    NAMESPACE "${PROJECT_NAME}::"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${mlpluginifc_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

