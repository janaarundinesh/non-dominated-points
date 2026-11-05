#include "benchmark_runner.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#include "brute_force_maxima.hpp"
#include "dataset.hpp"
#include "item.hpp"
#include "maxima2.hpp"
#include "point_set.hpp"

namespace {

constexpr int kRepetitions = 10;

double Median(std::vector<double> samples)
{
    std::sort(samples.begin(), samples.end());

    const size_t middle = samples.size() / 2;

    if (samples.size() % 2 == 0)
    {
        return (samples[middle - 1] + samples[middle]) / 2.0;
    }

    return samples[middle];
}

} // namespace

std::optional<Benchmark> RunBenchmark(
    const std::string& filename,
    Algorithm algorithm
)
{
    const std::optional<PointSet> set = LoadPointSet(filename);

    if (!set)
    {
        return std::nullopt;
    }

    std::vector<double> execution_times;
    execution_times.reserve(kRepetitions);

    size_t num_maxima = 0;

    for (int repetition = 0; repetition < kRepetitions; ++repetition)
    {
        const auto start = std::chrono::steady_clock::now();

        const std::vector<Item> maxima =
            algorithm == Algorithm::Maxima2
                ? MAXIMA2(*set)
                : BruteForceMaxima(*set);

        const auto stop = std::chrono::steady_clock::now();

        execution_times.push_back(
            std::chrono::duration<double, std::micro>(stop - start).count()
        );

        num_maxima = maxima.size();
    }

    Benchmark result;

    result.dimensions = set->dimensions;
    result.num_points = set->points.size();
    result.execution_time = Median(execution_times);
    result.min_execution_time =
        *std::min_element(execution_times.begin(), execution_times.end());
    result.max_execution_time =
        *std::max_element(execution_times.begin(), execution_times.end());
    result.num_non_dominated_points = num_maxima;

    return result;
}

int RunBenchmarkSuite(
    const std::string& base_directory,
    const std::string& output_file,
    Algorithm algorithm
)
{
    int failed = 0;

    for (int test = kFirstBenchmarkDataset; test <= kLastBenchmarkDataset; ++test)
    {
        const std::string filename =
            base_directory + "/Test" + std::to_string(test) + ".txt";

        const std::optional<Benchmark> result = RunBenchmark(filename, algorithm);

        if (!result)
        {
            std::cerr << "Test" << test << " skipped\n";

            ++failed;

            continue;
        }

        saveBenchmark(*result, output_file);

        std::cout << "Test" << test << " completed: "
                  << result->execution_time << " us\n";
    }

    if (failed != 0)
    {
        std::cerr << failed << " dataset(s) skipped, results are incomplete\n";

        return 1;
    }

    return 0;
}
