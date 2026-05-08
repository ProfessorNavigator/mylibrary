cmake_minimum_required(VERSION 3.16)

project(MLPlugin VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall)

option(BUILD_MLPLUGIN_DOCS "Build documentation for plugin interface" OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if(BUILD_MLPLUGIN_DOCS)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

add_library(MLPlugin SHARED)

set_target_properties(MLPlugin PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

add_subdirectory(include)
add_subdirectory(src)

target_include_directories(MLPlugin
  PRIVATE
    include
)

target_link_libraries(MLPlugin
  PUBLIC
    Qt::Core
    Qt::Widgets
)

qt_add_resources(MLPlugin "genres"
    PREFIX "/"
    FILES "genres/genres.csv" "genres/genre_groups.csv"
)

if(BUILD_MLPLUGIN_DOCS)
  set(DOXYGEN_GENERATE_HTML YES)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(DOXYGEN_GENERATE_MAN YES)
  endif()
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)
  set(DOXYGEN_EXAMPLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

  doxygen_add_docs(doxygen_mlplugin
    ALL
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    COMMENT "Generating ${PROJECT_NAME} documentation"
  )
endif()

include(GNUInstallDirs)

install(TARGETS MLPlugin EXPORT "${PROJECT_NAME}Targets"
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

if(BUILD_MLPLUGIN_DOCS)
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}/html"
  )

  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/man/"
      DESTINATION "${CMAKE_INSTALL_MANDIR}"
    )
  endif()
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${MLPlugin_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

