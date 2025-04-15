cmake_minimum_required(VERSION 3.16)

project(MLBookProc VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(USE_OPENMP "Build with OpenMP support" ON)
option(CREATE_HTML_DOCS_MLBOOKPROC "Build html documentation for MLBookProc" OFF)
option(CREATE_PDF_DOCS_MLBOOKPROC "Build pdf documentation for MLBookProc" OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET poppler-cpp)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(ICUUC REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(Gcrypt REQUIRED IMPORTED_TARGET libgcrypt)
pkg_check_modules(GPG-ERROR REQUIRED IMPORTED_TARGET gpg-error)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    pkg_check_modules(DJVU REQUIRED IMPORTED_TARGET ddjvuapi)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_library(DJVU djvulibre REQUIRED)
endif()
find_package(LibArchive REQUIRED)

if(USE_OPENMP)
  try_compile(OMP_TEST "${CMAKE_BINARY_DIR}/MLBookProc/omp_test" "${PROJECT_SOURCE_DIR}/omp_test" OmpTest)
  if(OMP_TEST)
    find_package(OpenMP REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP ${OpenMP_CXX_FLAGS}")
    add_link_options("${OpenMP_CXX_FLAGS}")
    message(STATUS "OpenMP features are supported, ${PROJECT_NAME} will be built with OpenMP support")
  else()
    set(USE_OPENMP OFF)
    find_package(TBB QUIET)
    find_package(Threads QUIET)
    find_library(ATOMIC atomic)
    message(WARNING "Compiler does not support some OpenMP features. ${PROJECT_NAME} will be built without OpenMP support.")
  endif()
else()
  find_package(TBB QUIET)
  find_package(Threads QUIET)
  find_library(ATOMIC atomic)
endif()

if(CREATE_HTML_DOCS_MLBOOKPROC OR CREATE_PDF_DOCS_MLBOOKPROC)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

if(POPPLER_VERSION VERSION_LESS "22.05")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OLDPOPPLER")
endif()


if(BUILD_SHARED_LIBS)
  add_library(mlbookproc SHARED)
else()
  add_library(mlbookproc STATIC)
endif()


set_target_properties(mlbookproc PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

set_target_properties(mlbookproc PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

add_subdirectory(src)
add_subdirectory(include)

target_include_directories(mlbookproc
    PRIVATE include
    PRIVATE PkgConfig::POPPLER
    PRIVATE PkgConfig::ICU
    PRIVATE PkgConfig::ICUUC
    PRIVATE ${LibArchive_INCLUDE_DIRS}
    PRIVATE PkgConfig::Gcrypt
    PRIVATE PkgConfig::GPG-ERROR    
)

target_link_libraries(mlbookproc
    PUBLIC PkgConfig::POPPLER
    PUBLIC PkgConfig::ICU
    PUBLIC PkgConfig::ICUUC
    PUBLIC ${LibArchive_LIBRARIES}
    PUBLIC PkgConfig::Gcrypt
    PUBLIC PkgConfig::GPG-ERROR
)

if(ATOMIC_FOUND)
  target_link_libraries(mlbookproc PUBLIC ${ATOMIC})
endif()

if(Threads_FOUND)
  target_link_libraries(mlbookproc PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_include_directories(mlbookproc PRIVATE PkgConfig::DJVU)
    target_link_libraries(mlbookproc PUBLIC PkgConfig::DJVU)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_library(DJVU djvulibre REQUIRED)
    target_link_libraries(mlbookproc PUBLIC ${DJVU})
endif()

if(TBB_FOUND)
    target_link_libraries(mlbookproc PRIVATE TBB::tbb)
endif()

if(DOXYGEN_FOUND)
  if(CREATE_HTML_DOCS_MLBOOKPROC)
    set(DOXYGEN_GENERATE_HTML YES)
  endif()
  if(CREATE_PDF_DOCS_MLBOOKPROC)
    set(DOXYGEN_GENERATE_LATEX YES)
    set(DOXYGEN_USE_PDFLATEX YES)
    set(DOXYGEN_PDF_HYPERLINKS YES)
  endif()
  if(USE_OPENMP)
    set(DOXYGEN_PREDEFINED USE_OPENMP)
  endif()
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)

  doxygen_add_docs(doxygen_mlbookproc
    ALL
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    COMMENT "Generating documentation"
  )

  if(CREATE_PDF_DOCS_MLBOOKPROC)
    add_custom_command(TARGET mlbookproc
      POST_BUILD
      COMMAND make
      ARGS pdf -C "${CMAKE_CURRENT_BINARY_DIR}/latex"
      COMMENT "Building pdf documentation"
    )
  endif()
endif()

include(GNUInstallDirs)

if(CREATE_HTML_DOCS_MLBOOKPROC)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/html
  )
endif()

if(CREATE_PDF_DOCS_MLBOOKPROC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/pdf
  )
endif()

install(TARGETS mlbookproc EXPORT MLBookProcTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(DIRECTORY MLBookProc DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")

install(DIRECTORY include/
  EXPORT MLBookProcTargets
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT "${PROJECT_NAME}Targets"
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${mlbookproc_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

