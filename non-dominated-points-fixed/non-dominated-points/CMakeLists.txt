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
    src/dominance.cpp
    src/maxima2.cpp
    src/partition.cpp
    src/threshold.cpp
    src/split_u.cpp
    src/intersection.cpp
    src/filter.cpp
)
