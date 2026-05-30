cmake_minimum_required(VERSION 3.10)

project(ParetoKnapsack)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(
    pareto_knapsack
    src/main.cpp
    src/generate_data.cpp
    src/dominance.cpp
)
