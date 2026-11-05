#pragma once

#include <optional>
#include <string>

#include "benchmark.hpp"

enum class Algorithm {
    Maxima2,
    BruteForce
};

// Loads one dataset and measures the chosen algorithm kRepetitions times,
// returning the median wall-clock time. Returns nothing if the dataset cannot
// be read or is inconsistent; the caller reports and skips it.
//
// MAXIMA2 is measured including the sort and working copy it needs, while the
// brute-force baseline works directly on the loaded points. Both are measured
// with a steady clock, from freshly loaded data, once per repetition.
std::optional<Benchmark> RunBenchmark(
    const std::string& filename,
    Algorithm algorithm
);

// Runs every dataset in [kFirstBenchmarkDataset, kLastBenchmarkDataset] found in
// base_directory and writes one row per configuration to output_file. Returns 0
// when every dataset was measured and 1 when any had to be skipped.
int RunBenchmarkSuite(
    const std::string& base_directory,
    const std::string& output_file,
    Algorithm algorithm
);
