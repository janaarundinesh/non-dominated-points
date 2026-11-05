#include <iostream>
#include <string>

#include "benchmark_runner.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <dimension_folder>\n";

        return 1;
    }

    const std::string base_directory = "../DataSets/" + std::string(argv[1]);

    return RunBenchmarkSuite(
        base_directory,
        "../results/maxima_benchmark.csv",
        Algorithm::Maxima2
    );
}
