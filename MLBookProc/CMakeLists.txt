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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP")
    message(STATUS "OpenMP features are supported, ${PROJECT_NAME} will be built with OpenMP support")
  else()
    set(USE_OPENMP OFF)
    try_compile(EXEC_TEST "${CMAKE_BINARY_DIR}/MLBookProc/stdexecution_test" "${PROJECT_SOURCE_DIR}/stdexecution_test" ExecTest)
    if(EXEC_TEST)
      message(STATUS "${PROJECT_NAME} C++ parallel execution features are supported")
      find_package(TBB QUIET)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PE")
    else()
      message(STATUS "${PROJECT_NAME} C++ parallel features are not supported")
    endif()
    find_package(Threads QUIET)
    find_library(ATOMIC atomic)
    message(WARNING "Compiler does not support some OpenMP features. ${PROJECT_NAME} will be built without OpenMP support.")
  endif()
else()
  try_compile(EXEC_TEST "${CMAKE_BINARY_DIR}/MLBookProc/stdexecution_test" "${PROJECT_SOURCE_DIR}/stdexecution_test" ExecTest)
  if(EXEC_TEST)
    message(STATUS "${PROJECT_NAME} C++ parallel execution features are supported")
    find_package(TBB QUIET)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PE")
  else()
    message(STATUS "${PROJECT_NAME} C++ parallel features are not supported")
  endif()
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

set_target_properties(mlbookproc PROPERTIES POSITION_INDEPENDENT_CODE True)

if(BUILD_SHARED_LIBS)
  set_target_properties(mlbookproc PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif()

add_subdirectory(src)
add_subdirectory(include)

target_include_directories(mlbookproc
  PRIVATE include
  PUBLIC ${LibArchive_INCLUDE_DIRS}
  PUBLIC ${POPPLER_INCLUDE_DIRS}
  PUBLIC ${ICU_INCLUDE_DIRS}
  PUBLIC ${ICUUC_INCLUDE_DIRS}
  PUBLIC ${Gcrypt_INCLUDE_DIRS}
  PUBLIC ${GPG-ERROR_INCLUDE_DIRS}
)

target_compile_options(mlbookproc  
  PUBLIC ${POPPLER_CFLAGS_OTHER}
  PUBLIC ${ICU_CFLAGS_OTHER}
  PUBLIC ${ICUUC_CFLAGS_OTHER}
  PUBLIC ${Gcrypt_CFLAGS_OTHER}
  PUBLIC ${GPG-ERROR_CFLAGS_OTHER}
)

target_link_libraries(mlbookproc
  PUBLIC ${LibArchive_LIBRARIES}
  PUBLIC ${POPPLER_LINK_LIBRARIES}
  PUBLIC ${ICU_LINK_LIBRARIES}
  PUBLIC ${ICUUC_LINK_LIBRARIES}
  PUBLIC ${Gcrypt_LINK_LIBRARIES}
  PUBLIC ${GPG-ERROR_LINK_LIBRARIES}
)

target_link_options(mlbookproc
  PUBLIC ${POPPLER_LDFLAGS_OTHER}
  PUBLIC ${ICU_LDFLAGS_OTHER}
  PUBLIC ${ICUUC_LDFLAGS_OTHER}
  PUBLIC ${Gcrypt_LDFLAGS_OTHER}
  PUBLIC ${GPG-ERROR_LDFLAGS_OTHER}
)

if(USE_OPENMP)
  target_include_directories(mlbookproc PUBLIC ${OpenMP_CXX_INCLUDE_DIRS})
  target_compile_options(mlbookproc PUBLIC ${OpenMP_CXX_FLAGS})
  target_link_libraries(mlbookproc PUBLIC ${OpenMP_CXX_LIBRARIES})
else()
  if(ATOMIC)
    target_link_libraries(mlbookproc PUBLIC ${ATOMIC})
  endif()
endif()

if(Threads_FOUND)
  target_link_libraries(mlbookproc PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_include_directories(mlbookproc PUBLIC ${DJVU_INCLUDE_DIRS})
    target_compile_options(mlbookproc PUBLIC ${DJVU_CFLAGS_OTHER})
    target_link_libraries(mlbookproc PUBLIC ${DJVU_LINK_LIBRARIES})
    target_link_options(mlbookproc PUBLIC ${DJVU_LDFLAGS_OTHER})
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")    
    target_link_libraries(mlbookproc PUBLIC ${DJVU})
endif()

if(TBB_FOUND)
  if(BUILD_SHARED_LIBS)
    target_link_libraries(mlbookproc PRIVATE TBB::tbb)
  else()
    target_link_libraries(mlbookproc PUBLIC TBB::tbb)
  endif()
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
    add_custom_command(TARGET doxygen_mlbookproc
      POST_BUILD
      COMMAND make
      ARGS pdf -C "${CMAKE_CURRENT_BINARY_DIR}/latex"
      COMMENT "Generating pdf documentation"
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

