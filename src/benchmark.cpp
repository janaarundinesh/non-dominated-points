#include "benchmark.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* const kHeader =
    "dimensions,num_points,execution_time,min_execution_time,"
    "max_execution_time,num_non_dominated_points";

bool IsHeader(const std::string& line)
{
    return line.rfind("dimensions", 0) == 0;
}

// A row is identified by the configuration it measured. Re-running a
// configuration must replace its previous measurement rather than append a
// second one, so that plots see exactly one point per dataset.
bool MatchesConfiguration(
    const std::string& line,
    size_t dimensions,
    size_t num_points
)
{
    std::stringstream fields(line);
    std::string dimensions_field;
    std::string num_points_field;

    if (!std::getline(fields, dimensions_field, ',') ||
        !std::getline(fields, num_points_field, ','))
    {
        return false;
    }

    try
    {
        return std::stoul(dimensions_field) == dimensions &&
               std::stoul(num_points_field) == num_points;
    }
    catch (const std::exception&)
    {
        // Keep rows this program cannot interpret.
        return false;
    }
}

} // namespace

void saveBenchmark (
    const Benchmark& results,
    const std::string& filename
)
{
    // Collect the rows that belong to other configurations.
    std::vector<std::string> kept_rows;

    {
        std::ifstream existing_file(filename);
        std::string line;

        while (std::getline(existing_file, line))
        {
            if (line.empty() || IsHeader(line))
            {
                continue;
            }

            if (MatchesConfiguration(line, results.dimensions, results.num_points))
            {
                continue;
            }

            kept_rows.push_back(line);
        }
    }

    std::ofstream file(filename, std::ios::trunc);

    if (!file)
    {
        std::cerr << "Could not open benchmark file: " << filename << '\n';
        return;
    }

    file << kHeader << '\n';

    for (const auto& row : kept_rows)
    {
        file << row << '\n';
    }

    file << results.dimensions << ","
         << results.num_points << ","
         << results.execution_time << ","
         << results.min_execution_time << ","
         << results.max_execution_time << ","
         << results.num_non_dominated_points << '\n';
}
