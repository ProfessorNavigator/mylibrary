cmake_minimum_required(VERSION 3.16)

project(OmpTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

if(OpenMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_link_options("${OpenMP_CXX_FLAGS}")
endif()
add_executable(omp_test main.cpp)

