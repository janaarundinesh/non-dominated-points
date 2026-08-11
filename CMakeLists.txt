cmake_minimum_required(VERSION 3.10)

project(NonDominatedPoints)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

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
target_compile_options(non_dominated_points PRIVATE
    -O3
    -g
    -fno-omit-frame-pointer
)

add_executable(
    brute_force
    src/main_bruteforce.cpp
    src/benchmark.cpp
    src/brute_force_maxima.cpp
    src/dominance.cpp
)

add_executable(
    generate_data
    src/main_gen_data.cpp
    src/generate_data.cpp
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