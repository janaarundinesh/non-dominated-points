#include "benchmark.hpp"

#include <iostream>
#include <fstream>

void initializeBenchmarkFile(const std::string& filename)
{
    std::ifstream existing_file(filename);
    if (existing_file.good())
    {
        return;
    }
    
    std::ofstream file(filename);

    if (!file)
    {
        std::cerr << "Could not create benchmark file: "
                  << filename << '\n';
        return;
    }

    file << "dimensions,"
         << "num_points,"
         << "execution_time,"
         << "num_non_dominated_points\n";
}

void saveBenchmark (
    const Benchmark& results,
    const std::string& filename
)
{
    std::ofstream file(filename, std::ios::app);
    if (!file)
    {
        std::cerr << "Could not open benchmark file: " << filename << '\n';
        return;
    }   

    file << results.dimensions << ","
         << results.num_points << ","
         << results.execution_time << ","
         << results.num_non_dominated_points << '\n';

}