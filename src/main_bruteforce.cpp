#include "benchmark.hpp"
#include "brute_force_maxima.hpp"
#include "item.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

Benchmark runBenchmark(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "Could not open dataset: "
                  << filename << '\n';

        return {};
    }

    size_t dimensions;
    size_t num_points;

    file >> dimensions >> num_points;

    std::vector<Item> items(num_points);

    for (size_t i = 0; i < num_points; ++i)
    {
        items[i].coords.resize(dimensions);

        for (size_t d = 0; d < dimensions; ++d)
        {
            file >> items[i].coords[d];
        }
    }

    file.close();

    auto start = std::chrono::high_resolution_clock::now();

    auto maxima = BruteForceMaxima(items, dimensions);

    auto stop = std::chrono::high_resolution_clock::now();

    double execution_time =
        std::chrono::duration<double, std::micro>(
            stop - start
        ).count();

    Benchmark result;

    result.dimensions = dimensions;
    result.num_points = num_points;
    result.execution_time = execution_time;
    result.num_non_dominated_points = maxima.size();

    return result;
}


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: "
                  << argv[0]
                  << " <dimension_folder>\n";

        return 1;
    }

    std::string dimension_folder = argv[1];

    std::string base_directory =
        "../DataSets/" + dimension_folder;

    std::string output_file =
        "../results/brute_force_benchmark.csv";

    initializeBenchmarkFile(output_file);

    for (int test = 1; test <= 5; ++test)
    {
        std::string filename =
            base_directory +
            "/Test" +
            std::to_string(test) +
            ".txt";

        Benchmark result = runBenchmark(filename);

        saveBenchmark(result, output_file);

        std::cout << "Test" << test
                  << " completed: "
                  << result.execution_time
                  << " us\n";
    }

    return 0;
}