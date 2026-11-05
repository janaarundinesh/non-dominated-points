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
    src/antichain.cpp
    src/benchmark.cpp
    src/benchmark_runner.cpp
    src/brute_force_maxima.cpp
    src/dataset.cpp
    src/dominance.cpp
    src/filter.cpp
    src/maxima2.cpp
    src/partition.cpp
    src/point_set.cpp
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

# Peak-memory and million-point measurements; the quadratic baseline cannot run
# these datasets, so they are measured on their own.
add_executable(large_benchmark tools/large_benchmark.cpp)
target_link_libraries(large_benchmark PRIVATE nondom)

# Worst case for both algorithms: an antichain, where every point is maximal
# and nothing can be pruned early.
add_executable(worst_case_benchmark tools/worst_case_benchmark.cpp)
target_link_libraries(worst_case_benchmark PRIVATE nondom)

# ----------------------------------------------------------------------------
# Validation / tests
# ----------------------------------------------------------------------------
if(NDP_BUILD_TESTS)
    enable_testing()

    # Every test gets a time bound. A hung MPI runtime (or a deadlocked
    # algorithm) must show up as one failed test within seconds, not as a suite
    # that never returns.
    function(ndp_add_test name timeout_seconds)
        add_test(NAME ${name} COMMAND ${ARGN})
        set_tests_properties(${name} PROPERTIES TIMEOUT ${timeout_seconds})
    endfunction()

    add_executable(validator
        validation/validation_main.cpp
        validation/validator.cpp
    )
    target_link_libraries(validator PRIVATE nondom)

    # The validator loads "../validation/test_datasets/TestN.txt", so run it
    # from <source>/validation. This works for any build directory location.
    ndp_add_test(validator 120 validator)
    set_tests_properties(validator PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/validation
    )
    # validator exits non-zero on failure; also check its case count so that a
    # dataset that silently fails to load cannot reduce the coverage unnoticed.
    set_tests_properties(validator PROPERTIES
        PASS_REGULAR_EXPRESSION "Passed : 16 / 16"
    )

    # Randomized differential test against the brute-force reference. Runs in
    # memory, so it needs no data files and no particular working directory.
    add_executable(differential_test validation/differential_test.cpp)
    target_link_libraries(differential_test PRIVATE nondom)

    ndp_add_test(differential_test 60 differential_test)

    # The dominance predicate, on its own. Both algorithms share it, so a
    # difference between them cannot expose a mistake in it.
    add_executable(dominance_test validation/dominance_test.cpp)
    target_link_libraries(dominance_test PRIVATE nondom)
    ndp_add_test(dominance_relation 60 dominance_test)

    # Inputs the random test never generates: empty, single point, duplicates,
    # one dimension, everything in one tie group.
    add_executable(edge_case_test validation/edge_case_test.cpp)
    target_link_libraries(edge_case_test PRIVATE nondom)
    ndp_add_test(edge_cases 60 edge_case_test)

    # The merge helpers (PartitionV / SplitU / FILTER) with their contracts
    # spelled out, including the weak-dominance tie rule FILTER depends on.
    add_executable(partition_filter_test validation/partition_filter_test.cpp)
    target_link_libraries(partition_filter_test PRIVATE nondom)
    ndp_add_test(partition_filter 60 partition_filter_test)

    # Pipeline tests: the loader's refusal paths, the CSV upsert behaviour the
    # plots depend on, the suite's handling of missing datasets, and the
    # antichain generator behind the worst-case measurements. All of them write
    # to a temporary directory only.
    add_executable(dataset_test validation/dataset_test.cpp)
    target_link_libraries(dataset_test PRIVATE nondom)
    ndp_add_test(dataset_loader 60 dataset_test)

    add_executable(benchmark_csv_test validation/benchmark_csv_test.cpp)
    target_link_libraries(benchmark_csv_test PRIVATE nondom)
    ndp_add_test(benchmark_pipeline 60 benchmark_csv_test)

    add_executable(antichain_test validation/antichain_test.cpp)
    target_link_libraries(antichain_test PRIVATE nondom)
    ndp_add_test(antichain_generator 60 antichain_test)
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

        # Correctness of the distributed programs. They only ever printed their
        # result, so nothing checked them automatically; each is now run on an
        # 8-point fixture whose answer is 4 maxima, at the process counts where
        # the token passing wraps around. The marry-go-round program has a
        # separate code path at one process, which is covered as well.
        if(NDP_BUILD_TESTS AND MPIEXEC_EXECUTABLE)
            set(MPI_TEST_DATASET
                ${CMAKE_CURRENT_SOURCE_DIR}/validation/test_datasets/MPI_Test.txt)

            # The marry-go-round program writes its CSV relative to the parent
            # of the working directory.
            file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mpi_work)
            file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/results)

            # The bound is generous for eight points, but it is what turns a
            # deadlock (or an MPI runtime that hangs in MPI_Finalize) into a
            # failed test instead of a suite that has to be interrupted.
            foreach(count 1 2 3)
                ndp_add_test(mpi_marry_go_round_p${count} 20
                    ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG}
                    ${count} ${MPIEXEC_PREFLAGS}
                    $<TARGET_FILE:MPI_Marrygomethod>
                    ${MPI_TEST_DATASET})
                set_tests_properties(mpi_marry_go_round_p${count} PROPERTIES
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mpi_work
                    PASS_REGULAR_EXPRESSION "maxima=4")
            endforeach()

            foreach(program MPI_Ringmethod MPI_FanInOut)
                foreach(count 2 3)
                    ndp_add_test(${program}_p${count} 20
                        ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG}
                        ${count} ${MPIEXEC_PREFLAGS}
                        $<TARGET_FILE:${program}>
                        ${MPI_TEST_DATASET})
                    set_tests_properties(${program}_p${count} PROPERTIES
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mpi_work
                        PASS_REGULAR_EXPRESSION "Final maxima: 4")
                endforeach()
            endforeach()
        elseif(NDP_BUILD_TESTS)
            message(WARNING
                "mpiexec not found: the MPI correctness tests are not registered.")
        endif()
    else()
        message(WARNING
            "MPI not found: skipping MPI programs. "
            "Install MPI or configure with -DNDP_ENABLE_MPI=OFF.")
    endif()
endif()