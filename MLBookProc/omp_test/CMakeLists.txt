cmake_minimum_required(VERSION 3.16)

project(OmpTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(omp_test main.cpp)

target_link_libraries(omp_test
    PRIVATE OpenMP::OpenMP_CXX
)

target_link_options(omp_test
    PRIVATE ${OpenMP_CXX_FLAGS}
)
