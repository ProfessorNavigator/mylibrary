cmake_minimum_required(VERSION 3.16)

project(MLBookProc VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(USE_OPENMP "Build with openmp support" OFF)
option(USE_TBB "Build with tbb support" OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET poppler-cpp)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-i18n)
pkg_check_modules(ICUUC REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(LIBARCHIVE REQUIRED IMPORTED_TARGET libarchive)
pkg_check_modules(Gcrypt REQUIRED IMPORTED_TARGET libgcrypt)
pkg_check_modules(GPG-ERROR REQUIRED IMPORTED_TARGET gpg-error)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    pkg_check_modules(DJVU REQUIRED IMPORTED_TARGET ddjvuapi)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_library(DJVU djvulibre REQUIRED)
endif()

if(USE_OPENMP)    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_OPENMP -fopenmp")
    add_link_options("-fopenmp")
endif()

if(USE_TBB)
    find_package(TBB REQUIRED)
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
    PRIVATE PkgConfig::LIBARCHIVE
    PRIVATE PkgConfig::Gcrypt
    PRIVATE PkgConfig::GPG-ERROR    
)

target_link_libraries(mlbookproc
    PUBLIC PkgConfig::POPPLER
    PUBLIC PkgConfig::ICU
    PUBLIC PkgConfig::ICUUC
    PUBLIC PkgConfig::LIBARCHIVE
    PUBLIC PkgConfig::Gcrypt
    PUBLIC PkgConfig::GPG-ERROR    
    PUBLIC atomic
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_include_directories(mlbookproc PRIVATE PkgConfig::DJVU)
    target_link_libraries(mlbookproc PUBLIC PkgConfig::DJVU)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_library(DJVU djvulibre REQUIRED)
    target_link_libraries(mlbookproc PUBLIC ${DJVU})
endif()

if(USE_TBB)
    target_link_libraries(mlbookproc PUBLIC TBB::tbb)
endif()

include(GNUInstallDirs)

install(TARGETS mlbookproc EXPORT MLBookProcTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

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
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

