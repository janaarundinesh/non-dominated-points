cmake_minimum_required(VERSION 3.10)

project(NonDominatedPoints)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(
    non_dominated_points
    src/main.cpp
    src/generate_data.cpp
    src/print.cpp
    src/sorting.cpp
)
