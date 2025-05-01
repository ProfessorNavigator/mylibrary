cmake_minimum_required(VERSION 3.16)

project(OmpTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(omp_test main.cpp)

if(OpenMP_FOUND)
    target_include_directories(omp_test PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
    target_compile_options(omp_test PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(omp_test PRIVATE ${OpenMP_CXX_LIBRARIES})
endif()
