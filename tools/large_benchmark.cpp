// Measures MAXIMA2 on datasets the benchmark drivers do not cover: the
// million-point Test7 files, and the peak memory the algorithm needs on top of
// the input it was handed.
//
// Usage: large_benchmark <output.csv> <dataset.txt>
//
// One dataset per process, so the resident-memory high-water mark belongs to
// that run alone. Rows are appended; the header is written when the file does
// not exist yet, so a shell loop over datasets produces one tidy table.
//
// The row format is
//   dimensions,num_points,median_ms,min_ms,max_ms,peak_rss_mb,extra_rss_mb,maxima
//
// peak_rss_mb is the process high-water mark after loading and running, so it
// includes the input representation. extra_rss_mb is what the run added on top
// of the loaded set, which is the part the algorithm itself controls.

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <sys/resource.h>

#include "dataset.hpp"
#include "maxima2.hpp"
#include "point_set.hpp"

namespace {

constexpr int kRepetitions = 5;

// Current process high-water mark. Linux reports kilobytes, macOS bytes.
double PeakResidentMegabytes()
{
    struct rusage usage
    {
    };

    if (getrusage(RUSAGE_SELF, &usage) != 0)
    {
        return 0.0;
    }

#if defined(__APPLE__)
    return static_cast<double>(usage.ru_maxrss) / (1024.0 * 1024.0);
#else
    return static_cast<double>(usage.ru_maxrss) / 1024.0;
#endif
}

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

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <output.csv> <dataset.txt>\n";

        return 1;
    }

    const std::optional<PointSet> set = LoadPointSet(argv[2]);

    if (!set)
    {
        return 1;
    }

    const double loaded_megabytes = PeakResidentMegabytes();

    std::vector<double> milliseconds;
    milliseconds.reserve(kRepetitions);

    size_t maxima = 0;

    for (int repetition = 0; repetition < kRepetitions; ++repetition)
    {
        const auto start = std::chrono::steady_clock::now();

        maxima = MAXIMA2(*set).size();

        const auto stop = std::chrono::steady_clock::now();

        milliseconds.push_back(
            std::chrono::duration<double, std::milli>(stop - start).count()
        );
    }

    const double peak_megabytes = PeakResidentMegabytes();

    const bool file_exists = std::ifstream(argv[1]).good();

    std::ofstream output(argv[1], std::ios::app);

    if (!output)
    {
        std::cerr << "Could not open output file: " << argv[1] << '\n';

        return 1;
    }

    if (!file_exists)
    {
        output << "dimensions,num_points,median_ms,min_ms,max_ms,"
                  "peak_rss_mb,extra_rss_mb,maxima\n";
    }

    output << set->dimensions << ','
           << set->points.size() << ','
           << Median(milliseconds) << ','
           << *std::min_element(milliseconds.begin(), milliseconds.end()) << ','
           << *std::max_element(milliseconds.begin(), milliseconds.end()) << ','
           << peak_megabytes << ','
           << std::max(0.0, peak_megabytes - loaded_megabytes) << ','
           << maxima << '\n';

    std::cout << argv[2] << ": " << set->points.size() << " points, "
              << set->dimensions << "D, " << Median(milliseconds) << " ms, peak "
              << peak_megabytes << " MB, " << maxima << " maxima\n";

    return 0;
}
