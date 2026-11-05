#pragma once

#include <cstddef>
#include <string>

// Benchmark datasets are named Test<N>.txt inside DataSets/<dimension>D_Data/.
// Both drivers cover the same range so that their results can be compared
// configuration by configuration. Test7 holds 1,000,000 points and is reserved
// for the parallel experiments: the quadratic baseline cannot process it.
constexpr int kFirstBenchmarkDataset = 1;
constexpr int kLastBenchmarkDataset = 6;

struct Benchmark {
    size_t dimensions = 0;
    size_t num_points = 0;

    // Median of the repetitions, alongside the fastest and slowest one, so that
    // plots can show how much a single measurement actually varies.
    double execution_time = 0.0;
    double min_execution_time = 0.0;
    double max_execution_time = 0.0;

    size_t num_non_dominated_points = 0;
};

// Replaces the row for this (dimension, point count) configuration, or appends
// one if the configuration has not been measured yet, so that re-running a
// dimension updates its measurements instead of duplicating them.
void saveBenchmark (
    const Benchmark& results,
    const std::string& filename
);
