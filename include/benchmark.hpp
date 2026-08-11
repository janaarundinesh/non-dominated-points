#pragma once

#include <string>

struct Benchmark {
    size_t dimensions;
    size_t num_points;
    double execution_time;
    size_t num_non_dominated_points;
};

void initializeBenchmarkFile(
    const std::string& filename
);

void saveBenchmark (
    const Benchmark& results,
    const std::string& filename
);