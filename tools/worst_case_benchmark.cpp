// Worst-case measurement: an input on which no point is dominated.
//
// MAXIMA2 and the quadratic baseline are both fastest when almost every point
// is pruned early. This tool builds the opposite input on purpose, an
// antichain, where every one of the N points is maximal:
//
//   * draw d-1 integer cuts uniformly in [0, C], sort them, and use the d gaps
//     (plus the two ends) as the coordinates. Every point then sums to exactly
//     C, with integers small enough to be represented exactly as doubles.
//   * a point can only dominate another if all its coordinates are >= and at
//     least one is >, which would make its coordinate sum larger. Equal sums
//     therefore rule out domination by construction, with no reliance on
//     floating-point luck and no tolerance to tune.
//
// The result is checked: the run aborts unless the number of maxima equals N.
//
// Usage: worst_case_benchmark <output.csv> <dimensions> <num_points> <algorithm>
//        algorithm is "maxima2" or "brute_force".
//
// Row format:
//   dimensions,num_points,algorithm,median_ms,min_ms,max_ms,maxima,maxima_fraction
//
// Rows are upserted on (dimensions, num_points, algorithm) so the sweep can be
// repeated without the plots double-counting a configuration.

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "antichain.hpp"
#include "brute_force_maxima.hpp"
#include "maxima2.hpp"
#include "point_set.hpp"

namespace {

constexpr int kRepetitions = 5;
constexpr int kFewerRepetitionsForLargeInputs = 3;
constexpr size_t kLargeInput = 100000;

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

bool MatchesConfiguration(
    const std::string& line,
    size_t dimensions,
    size_t num_points,
    const std::string& algorithm)
{
    std::stringstream fields(line);
    std::string dimensions_field;
    std::string num_points_field;
    std::string algorithm_field;

    if (!std::getline(fields, dimensions_field, ',') ||
        !std::getline(fields, num_points_field, ',') ||
        !std::getline(fields, algorithm_field, ','))
    {
        return false;
    }

    try
    {
        return std::stoul(dimensions_field) == dimensions &&
               std::stoul(num_points_field) == num_points &&
               algorithm_field == algorithm;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void WriteRow(
    const std::string& filename,
    size_t dimensions,
    size_t num_points,
    const std::string& algorithm,
    double median_ms,
    double min_ms,
    double max_ms,
    size_t maxima)
{
    std::vector<std::string> kept_rows;

    {
        std::ifstream existing(filename);
        std::string line;

        while (std::getline(existing, line))
        {
            if (line.empty() || line.rfind("dimensions", 0) == 0)
            {
                continue;
            }

            if (MatchesConfiguration(line, dimensions, num_points, algorithm))
            {
                continue;
            }

            kept_rows.push_back(line);
        }
    }

    std::ofstream output(filename, std::ios::trunc);

    if (!output)
    {
        std::cerr << "Could not open output file: " << filename << '\n';

        return;
    }

    output << "dimensions,num_points,algorithm,median_ms,min_ms,max_ms,"
              "maxima,maxima_fraction\n";

    for (const auto& row : kept_rows)
    {
        output << row << '\n';
    }

    output << dimensions << ','
           << num_points << ','
           << algorithm << ','
           << median_ms << ','
           << min_ms << ','
           << max_ms << ','
           << maxima << ','
           << static_cast<double>(maxima) / static_cast<double>(num_points)
           << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <output.csv> <dimensions> <num_points>"
                     " <maxima2|brute_force>\n";

        return 1;
    }

    const size_t dimensions = std::stoul(argv[2]);
    const size_t num_points = std::stoul(argv[3]);
    const std::string algorithm = argv[4];

    if (dimensions < 2 || num_points < 2)
    {
        std::cerr << "Need at least two dimensions and two points.\n";

        return 1;
    }

    if (algorithm != "maxima2" && algorithm != "brute_force")
    {
        std::cerr << "Unknown algorithm: " << algorithm << '\n';

        return 1;
    }

    const PointSet set = BuildAllMaximaSet(dimensions, num_points, 20260124u);

    if (!IsValid(set))
    {
        std::cerr << "Generated point set is invalid.\n";

        return 1;
    }

    const int repetitions =
        num_points >= kLargeInput ? kFewerRepetitionsForLargeInputs
                                  : kRepetitions;

    std::vector<double> milliseconds;
    milliseconds.reserve(static_cast<size_t>(repetitions));

    size_t maxima = 0;

    for (int repetition = 0; repetition < repetitions; ++repetition)
    {
        const auto start = std::chrono::steady_clock::now();

        maxima = algorithm == "maxima2" ? MAXIMA2(set).size()
                                        : BruteForceMaxima(set).size();

        const auto stop = std::chrono::steady_clock::now();

        milliseconds.push_back(
            std::chrono::duration<double, std::milli>(stop - start).count());
    }

    // The experiment is only meaningful if the input really is an antichain.
    if (maxima != num_points)
    {
        std::cerr << "Input is not an antichain: " << maxima << " of "
                  << num_points << " points are maximal.\n";

        return 1;
    }

    WriteRow(
        argv[1],
        dimensions,
        num_points,
        algorithm,
        Median(milliseconds),
        *std::min_element(milliseconds.begin(), milliseconds.end()),
        *std::max_element(milliseconds.begin(), milliseconds.end()),
        maxima);

    std::cout << dimensions << "D N=" << num_points << ' ' << algorithm
              << ": " << Median(milliseconds) << " ms, "
              << maxima << '/' << num_points << " maxima\n";

    return 0;
}
