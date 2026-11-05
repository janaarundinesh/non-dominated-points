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

    return RunBenchmarkSuite(
        argv[1],
        "../results/brute_force_benchmark.csv",
        Algorithm::BruteForce
    );
}
