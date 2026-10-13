cmake_minimum_required(VERSION 3.15)

project(NonDominatedPoints
    VERSION 0.1.0
    DESCRIPTION "Maxima (non-dominated points) of a d-dimensional point set"
    LANGUAGES CXX
)

# ----------------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------------
option(NDP_ENABLE_MPI   "Build the MPI programs (requires an MPI installation)" ON)
option(NDP_BUILD_TESTS  "Build the validator and register it with CTest"        ON)
option(NDP_SANITIZE     "Enable AddressSanitizer + UndefinedBehaviorSanitizer"  OFF)
option(NDP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"               OFF)

# ----------------------------------------------------------------------------
# Language standard and default build type
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# ----------------------------------------------------------------------------
# Shared compile options (warnings / sanitizers) as an interface target.
# Optimisation level comes from CMAKE_BUILD_TYPE (Release => -O3 -DNDEBUG).
# ----------------------------------------------------------------------------
add_library(ndp_options INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(ndp_options INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion
    )
    if(NDP_WARNINGS_AS_ERRORS)
        target_compile_options(ndp_options INTERFACE -Werror)
    endif()
    if(NDP_SANITIZE)
        target_compile_options(ndp_options INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(ndp_options INTERFACE
            -fsanitize=address,undefined)
    endif()
elseif(MSVC)
    target_compile_options(ndp_options INTERFACE /W4)
    if(NDP_WARNINGS_AS_ERRORS)
        target_compile_options(ndp_options INTERFACE /WX)
    endif()
endif()

# ----------------------------------------------------------------------------
# Core library
# ----------------------------------------------------------------------------
add_library(nondom STATIC
    src/benchmark.cpp
    src/brute_force_maxima.cpp
    src/dominance.cpp
    src/filter.cpp
    src/intersection.cpp
    src/maxima2.cpp
    src/partition.cpp
    src/print.cpp
    src/sorting.cpp
    src/split_u.cpp
    src/threshold.cpp
)
target_include_directories(nondom PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(nondom PUBLIC ndp_options)

# ----------------------------------------------------------------------------
# Serial executables
# ----------------------------------------------------------------------------
add_executable(Maxima_method src/main.cpp)
target_link_libraries(Maxima_method PRIVATE nondom)

add_executable(brute_force src/main_bruteforce.cpp)
target_link_libraries(brute_force PRIVATE nondom)

# ----------------------------------------------------------------------------
# Validation / tests
# ----------------------------------------------------------------------------
if(NDP_BUILD_TESTS)
    enable_testing()

    add_executable(validator
        validation/validation_main.cpp
        validation/validator.cpp
    )
    target_link_libraries(validator PRIVATE nondom)

    # The validator loads "../validation/test_datasets/TestN.txt", so run it
    # from <source>/validation. This works for any build directory location.
    add_test(
        NAME validator
        COMMAND validator
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/validation
    )
    # validator's main() always returns 0, so judge success by its summary.
    set_tests_properties(validator PROPERTIES
        PASS_REGULAR_EXPRESSION "Passed : 13 / 13"
    )
endif()

# ----------------------------------------------------------------------------
# MPI executables (optional)
# ----------------------------------------------------------------------------
if(NDP_ENABLE_MPI)
    find_package(MPI COMPONENTS CXX)

    if(MPI_CXX_FOUND)
        # Standalone: flat-array implementation, does not use the core library.
        add_executable(MPI_Marrygomethod mpi_src/mpi_marry_go_round_method.cpp)
        target_link_libraries(MPI_Marrygomethod PRIVATE MPI::MPI_CXX ndp_options)

        # These use Item / Dominates / BruteForceMaxima from the core library.
        add_executable(MPI_Ringmethod mpi_src/mpi_ring_method.cpp)
        target_link_libraries(MPI_Ringmethod PRIVATE MPI::MPI_CXX nondom)

        add_executable(MPI_FanInOut mpi_src/mpi_fan_in_out.cpp)
        target_link_libraries(MPI_FanInOut PRIVATE MPI::MPI_CXX nondom)
    else()
        message(WARNING
            "MPI not found: skipping MPI programs. "
            "Install MPI or configure with -DNDP_ENABLE_MPI=OFF.")
    endif()
endif()