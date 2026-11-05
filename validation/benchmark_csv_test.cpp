// Behaviour of the measurement pipeline, not of the algorithms.
//
// Two properties matter for the plots and are easy to break without noticing:
//
//   * a configuration occupies exactly one row, so re-running a dimension
//     replaces its measurements instead of plotting each of them twice;
//   * a dataset that cannot be read is skipped and reported, never recorded as
//     a zero timing that would look like an instantaneous run.
//
// The runner and the CSV writer are exercised directly, with a temporary
// dataset and a temporary CSV, so the test cannot touch results/.

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "benchmark.hpp"
#include "benchmark_runner.hpp"
#include "maxima2.hpp"

namespace fs = std::filesystem;

namespace {

int failures = 0;

void Check(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';

        ++failures;
    }
}

fs::path MakeTemporaryDirectory()
{
    const auto stamp = static_cast<unsigned long long>(
        std::chrono::steady_clock::now().time_since_epoch().count());

    const fs::path directory =
        fs::temp_directory_path() / ("ndp_csv_test_" + std::to_string(stamp));

    fs::create_directories(directory);

    return directory;
}

std::vector<std::string> DataRows(const fs::path& csv)
{
    std::ifstream file(csv);

    std::vector<std::string> rows;
    std::string line;

    while (std::getline(file, line))
    {
        if (!line.empty() && line.rfind("dimensions", 0) != 0)
        {
            rows.push_back(line);
        }
    }

    return rows;
}

size_t HeaderCount(const fs::path& csv)
{
    std::ifstream file(csv);

    size_t count = 0;
    std::string line;

    while (std::getline(file, line))
    {
        if (line.rfind("dimensions", 0) == 0)
        {
            ++count;
        }
    }

    return count;
}

Benchmark MakeResult(size_t dimensions, size_t points, double microseconds)
{
    Benchmark result;

    result.dimensions = dimensions;
    result.num_points = points;
    result.execution_time = microseconds;
    result.min_execution_time = microseconds * 0.9;
    result.max_execution_time = microseconds * 1.1;
    result.num_non_dominated_points = 1;

    return result;
}

} // namespace

int main()
{
    const fs::path directory = MakeTemporaryDirectory();

    const fs::path dataset = directory / "Test1.txt";
    const fs::path csv = directory / "benchmark.csv";

    {
        std::ofstream file(dataset);

        file << "3 6\n"
                "0 0 0\n"
                "1 0 0\n"
                "0 1 0\n"
                "0 0 1\n"
                "1 1 1\n"
                "2 0 0\n";
    }

    // --- the runner -------------------------------------------------------

    const auto measured = RunBenchmark(dataset.string(), Algorithm::Maxima2);
    const auto reference = RunBenchmark(dataset.string(), Algorithm::BruteForce);

    Check(measured.has_value(), "a readable dataset is measured");
    Check(reference.has_value(), "the baseline measures the same dataset");

    if (measured && reference)
    {
        Check(measured->dimensions == 3, "the row carries the dimension");
        Check(measured->num_points == 6, "the row carries the point count");
        Check(measured->min_execution_time <= measured->execution_time &&
                  measured->execution_time <= measured->max_execution_time,
              "min <= median <= max");
        Check(measured->num_non_dominated_points ==
                  reference->num_non_dominated_points,
              "both algorithms report the same number of maxima");
        Check(measured->num_non_dominated_points > 0,
              "the maxima count is not left at zero");
    }

    Check(!RunBenchmark((directory / "absent.txt").string(),
                        Algorithm::Maxima2)
               .has_value(),
          "an unreadable dataset yields no measurement");

    // --- the CSV ----------------------------------------------------------

    const Benchmark second = MakeResult(3, 4, 20.0);
    const Benchmark first = MakeResult(3, 6, 10.0);

    saveBenchmark(first, csv.string());

    Check(HeaderCount(csv) == 1, "the header is written exactly once");
    Check(DataRows(csv).size() == 1, "one configuration gives one row");

    saveBenchmark(second, csv.string());

    Check(DataRows(csv).size() == 2, "a new configuration is appended");
    Check(HeaderCount(csv) == 1, "appending does not repeat the header");

    // Re-measuring the first configuration must replace its row, not add one.
    const Benchmark remeasured = MakeResult(3, 6, 12.5);

    saveBenchmark(remeasured, csv.string());

    const std::vector<std::string> rows = DataRows(csv);

    Check(rows.size() == 2, "re-measuring does not duplicate the row");
    Check(HeaderCount(csv) == 1, "re-measuring does not repeat the header");

    size_t occurrences_of_six = 0;

    for (const auto& row : rows)
    {
        if (row.rfind("3,6,", 0) == 0)
        {
            ++occurrences_of_six;

            std::stringstream fields(row);
            std::string text;

            std::getline(fields, text, ','); // dimensions
            std::getline(fields, text, ','); // points
            std::getline(fields, text, ','); // median

            Check(text == "12.5", "the replaced row holds the new timing");
        }
    }

    Check(occurrences_of_six == 1, "the configuration appears exactly once");

    // --- the suite --------------------------------------------------------
    //
    // The suite walks Test1..Test6 inside a directory. A dataset that is absent
    // has to be skipped and reported: recording it as a zero timing would draw
    // an instantaneous run in every plot, and the exit status has to say that
    // the sweep was incomplete.

    const fs::path suite_csv = directory / "suite.csv";

    const int suite_status = RunBenchmarkSuite(
        directory.string(), suite_csv.string(), Algorithm::Maxima2);

    Check(suite_status == 1,
          "a suite with unreadable datasets reports that it is incomplete");

    const std::vector<std::string> suite_rows = DataRows(suite_csv);

    Check(suite_rows.size() == 1, "only the readable dataset is recorded");
    Check(HeaderCount(suite_csv) == 1, "the suite writes the header once");

    if (suite_rows.size() == 1)
    {
        std::stringstream fields(suite_rows[0]);
        std::string text;

        std::getline(fields, text, ','); // dimensions
        std::getline(fields, text, ','); // points

        Check(text == "6", "the recorded row is the dataset that exists");

        std::getline(fields, text, ','); // median

        Check(std::stod(text) > 0.0, "the recorded row holds a real timing");
    }

    fs::remove_all(directory);

    if (failures != 0)
    {
        std::cerr << failures << " benchmark pipeline check(s) failed\n";

        return 1;
    }

    std::cout << "benchmark pipeline: all checks passed\n";

    return 0;
}
