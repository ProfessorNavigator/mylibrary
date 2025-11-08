cmake_minimum_required(VERSION 3.16)

project(MLBookProc VERSION 1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(USE_OPENMP "Build with OpenMP support" ON)
option(CREATE_HTML_DOCS_MLBOOKPROC "Build html documentation for MLBookProc" OFF)
option(CREATE_PDF_DOCS_MLBOOKPROC "Build pdf documentation for MLBookProc" OFF)
option(BUILD_DEPRECATED_CLASSES "Build deprecated classes" OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET poppler-cpp)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(ICUUC REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(Gcrypt REQUIRED IMPORTED_TARGET libgcrypt)
pkg_check_modules(GPG-ERROR REQUIRED IMPORTED_TARGET gpg-error)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  pkg_check_modules(DJVU REQUIRED IMPORTED_TARGET ddjvuapi)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
  find_package(DJVU REQUIRED)
endif()
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(WARNING "Compiler type is ${CMAKE_CXX_COMPILER_ID}. It is recommended to disable OpenMP support.")
endif()

if(USE_OPENMP)
  try_compile(OMP_TEST "${CMAKE_BINARY_DIR}/MLBookProc/omp_test" "${PROJECT_SOURCE_DIR}/omp_test" OmpTest)
  if(OMP_TEST)
    include(CTest)
    add_subdirectory("${PROJECT_SOURCE_DIR}/omp_test")
    add_test(NAME OpenMPTest COMMAND omp_test WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/MLBookProc/omp_test")
    find_package(OpenMP REQUIRED)
    message(STATUS "OpenMP features are supported, ${PROJECT_NAME} will be built with OpenMP support")
  else()
    set(USE_OPENMP OFF)
    try_compile(EXEC_TEST "${CMAKE_BINARY_DIR}/MLBookProc/stdexecution_test"
      "${PROJECT_SOURCE_DIR}/stdexecution_test" ExecTest
    )
    if(EXEC_TEST)
      message(STATUS "${PROJECT_NAME} C++ parallel execution features are supported")
      find_package(TBB QUIET)
    else()
      message(STATUS "${PROJECT_NAME} C++ parallel features are not supported")
    endif()
    message(WARNING "Compiler does not support some OpenMP features. ${PROJECT_NAME} will be built without OpenMP support."
    )
  endif()
else()
  try_compile(EXEC_TEST "${CMAKE_BINARY_DIR}/MLBookProc/stdexecution_test" "${PROJECT_SOURCE_DIR}/stdexecution_test" ExecTest)
  if(EXEC_TEST)
    message(STATUS "${PROJECT_NAME} C++ parallel execution features are supported")
    find_package(TBB QUIET)
  else()
    message(STATUS "${PROJECT_NAME} C++ parallel features are not supported")    
  endif()
endif()

find_library(ATOMIC atomic)
find_package(Threads QUIET)

if(NOT USE_OPENMP)
  include(CTest)
  if(EXEC_TEST)
    add_subdirectory("${PROJECT_SOURCE_DIR}/stdexecution_test")
    add_test(NAME ParallelExecutionTest COMMAND stdexecution_test
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/MLBookProc/stdexecution_test"
    )
  else()
    add_subdirectory("${PROJECT_SOURCE_DIR}/vector_test")
    try_compile(VECTOR_TEST "${CMAKE_BINARY_DIR}/MLBookProc/vector_test" "${PROJECT_SOURCE_DIR}/vector_test" vector_test)
    add_test(NAME VectorTest COMMAND vector_test
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/MLBookProc/vector_test"
    )
  endif()
endif()

if(CREATE_HTML_DOCS_MLBOOKPROC OR CREATE_PDF_DOCS_MLBOOKPROC)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

if(BUILD_SHARED_LIBS)
  add_library(mlbookproc SHARED)
else()
  add_library(mlbookproc STATIC)
endif()

set_target_properties(mlbookproc PROPERTIES POSITION_INDEPENDENT_CODE True)

if(BUILD_SHARED_LIBS)
  set_target_properties(mlbookproc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif()

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(XMLParserCPP)

target_include_directories(mlbookproc
  PRIVATE include
  PRIVATE XMLParserCPP/include
)

if(POPPLER_VERSION VERSION_LESS "22.05")
  target_compile_options(mlbookproc
    PRIVATE -D_OLDPOPPLER
  )
endif()

target_link_libraries(mlbookproc
  PUBLIC PkgConfig::LIBARCHIVE
  PUBLIC PkgConfig::POPPLER
  PUBLIC PkgConfig::ICU
  PUBLIC PkgConfig::ICUUC
  PUBLIC PkgConfig::Gcrypt
  PUBLIC PkgConfig::GPG-ERROR
  PRIVATE XMLParserCPP
)

if(USE_OPENMP)
  target_compile_options(mlbookproc
    PUBLIC -DUSE_OPENMP
  ) 
  target_link_libraries(mlbookproc
    PUBLIC OpenMP::OpenMP_CXX
  )
  target_link_options(mlbookproc
    PUBLIC ${OpenMP_CXX_FLAGS}
  )
else()  
  if(EXEC_TEST)
    target_compile_options(mlbookproc
      PRIVATE -DUSE_PE
    )
  endif()
endif()

if(ATOMIC)
  target_link_libraries(mlbookproc
    PUBLIC ${ATOMIC}
  )
endif()

if(Threads_FOUND)
  target_link_libraries(mlbookproc
    PUBLIC Threads::Threads
  )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(mlbookproc
      PUBLIC PkgConfig::DJVU
    )
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_include_directories(mlbookproc PUBLIC ${DJVU_INCLUDE_DIR})
    target_link_libraries(mlbookproc PUBLIC ${DJVU_DLL})
endif()

if(TBB_FOUND) 
    target_link_libraries(mlbookproc PUBLIC TBB::tbb) 
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
  set(DOXYGEN_ENABLE_PREPROCESSING YES)
  set(DOXYGEN_MACRO_EXPANSION YES)
  set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
  set(DOXYGEN_PREDEFINED "__attribute__(x)=")
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)

  doxygen_add_docs(doxygen_mlbookproc
    ALL
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    COMMENT "Generating ${PROJECT_NAME} html documentation"
  )

  if(CREATE_PDF_DOCS_MLBOOKPROC)
    add_custom_command(TARGET doxygen_mlbookproc
      POST_BUILD
      COMMAND make
      ARGS pdf -C "${CMAKE_CURRENT_BINARY_DIR}/latex"
      COMMENT "Generating ${PROJECT_NAME} pdf documentation"
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
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}/pdf"
  )
endif()

install(TARGETS mlbookproc EXPORT "${PROJECT_NAME}Targets"
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(DIRECTORY "${PROJECT_NAME}" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}")

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
