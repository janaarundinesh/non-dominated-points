#include <iostream>
#include <vector>
#include "item.hpp"
#include "print.hpp"
#include "maxima2.hpp"
#include "benchmark.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <filesystem>

Benchmark runBenchmark(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "Could not open dataset: " << filename << '\n';

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

    std::vector<double> execution_times;
    const int repetitions = 10;
    std::vector<Item> maxima;

    for (int i = 0; i < repetitions; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();

        maxima = MAXIMA2(items, dimensions);

        auto stop = std::chrono::high_resolution_clock::now();

        double execution_time =
            std::chrono::duration<double, std::micro>(
                stop - start
            ).count();

        execution_times.push_back(execution_time);
    }

    // Calculate the median execution time
    std::sort(execution_times.begin(), execution_times.end());

    double median_time;
    if (repetitions % 2 == 0)
    {
        median_time = (execution_times[repetitions / 2 - 1] + execution_times[repetitions / 2]) / 2.0;
    }
    else
    {
        median_time = execution_times[repetitions / 2];
    }

    Benchmark result;

    result.dimensions = dimensions;
    result.num_points = num_points;
    result.execution_time = median_time;
    result.num_non_dominated_points = maxima.size();

    return result;
}


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <dimension_folder>\n";

        return 1;
    }

    std::string dimension_folder = argv[1];

    std::string base_directory = "../DataSets/" + dimension_folder;

    std::string output_file = "../results/maxima_benchmark.csv";

    initializeBenchmarkFile(output_file);

    for (int test = 1; test <= 5; ++test)
    {
        std::string filename = base_directory + "/Test" + std::to_string(test) + ".txt";

        Benchmark result = runBenchmark(filename);

        saveBenchmark(result, output_file);

        std::cout << "Test" << test << " completed: " << result.execution_time << " us\n";
    }

    return 0;
}