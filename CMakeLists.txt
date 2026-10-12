cmake_minimum_required(VERSION 3.10)

project(NonDominatedPoints)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_compile_options(
    -O3
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(
    non_dominated_points
    src/main.cpp
    src/benchmark.cpp
    src/print.cpp
    src/sorting.cpp
    src/dominance.cpp
    src/maxima2.cpp
    src/brute_force_maxima.cpp
    src/partition.cpp
    src/threshold.cpp
    src/split_u.cpp
    src/intersection.cpp
    src/filter.cpp
)

add_executable(
    brute_force
    src/main_bruteforce.cpp
    src/benchmark.cpp
    src/brute_force_maxima.cpp
    src/dominance.cpp
)

add_executable(
    validator
    validation/validation_main.cpp
    validation/validator.cpp
    src/print.cpp
    src/maxima2.cpp
    src/brute_force_maxima.cpp
    src/filter.cpp
    src/dominance.cpp
    src/sorting.cpp
    src/partition.cpp
    src/threshold.cpp
    src/split_u.cpp
    src/intersection.cpp
)

# MPI
find_package(MPI REQUIRED)

add_executable(
    MPI_Marrygomethod
    mpi_src/mpi_marry_go_round_method.cpp
)

target_link_libraries(
    MPI_Marrygomethod
    PRIVATE MPI::MPI_CXX
)