cmake_minimum_required(VERSION 3.16)

project(OmpTestOffloading LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(omp_test_offloading main.cpp)

find_package(OpenMP REQUIRED)
target_link_libraries(omp_test_offloading
    PRIVATE OpenMP::OpenMP_CXX
)

target_link_options(omp_test_offloading
    PRIVATE ${OpenMP_CXX_FLAGS}
)


