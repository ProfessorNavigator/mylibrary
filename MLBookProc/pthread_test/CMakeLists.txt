cmake_minimum_required(VERSION 3.16)

project(pthread_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pthread_test main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(pthread_test Threads::Threads)
