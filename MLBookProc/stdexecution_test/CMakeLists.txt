cmake_minimum_required(VERSION 3.16)

project(ExecTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stdexecution_test main.cpp)

find_package(TBB QUIET)
if(TBB_FOUND)
    target_link_libraries(stdexecution_test PRIVATE TBB::tbb)
endif()
