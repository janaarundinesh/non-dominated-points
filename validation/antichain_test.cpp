// The worst-case input has to be what it claims to be.
//
// The antichain benchmark only says something if every point really is maximal;
// a generator that quietly produced a few dominated points would make the
// "nothing can be pruned" measurements optimistic without failing anywhere.
// This test checks the construction directly: equal coordinate sums, no
// dominance between any two points, and agreement with both algorithms.

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "antichain.hpp"
#include "brute_force_maxima.hpp"
#include "dominance.hpp"
#include "maxima2.hpp"
#include "point_set.hpp"

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

std::string Describe(size_t dimensions, size_t points)
{
    return std::to_string(dimensions) + "D, N=" + std::to_string(points);
}

} // namespace

int main()
{
    const std::vector<std::pair<size_t, size_t>> configurations = {
        {2, 1}, {2, 5}, {3, 7}, {5, 40}, {10, 120}
    };

    for (const auto& configuration : configurations)
    {
        const size_t dimensions = configuration.first;
        const size_t points = configuration.second;

        const PointSet set = BuildAllMaximaSet(dimensions, points, 12345u);

        const std::string label = Describe(dimensions, points);

        Check(IsValid(set), label + ": the set is valid");
        Check(set.dimensions == dimensions, label + ": dimension is kept");
        Check(set.points.size() == points, label + ": point count is kept");

        if (!IsValid(set))
        {
            continue;
        }

        // Exact integer sums: equal sums are the whole argument for the set
        // being an antichain, so they are checked exactly rather than to within
        // a tolerance.
        bool sums_equal = true;

        for (const auto& point : set.points)
        {
            std::int64_t sum = 0;

            for (const double coordinate : point.coords)
            {
                sum += static_cast<std::int64_t>(coordinate);
            }

            if (sum != kAntichainCoordinateSum)
            {
                sums_equal = false;
            }
        }

        Check(sums_equal, label + ": every coordinate sum equals the constant");

        // Direct pairwise check; these sets are small enough to afford it.
        bool incomparable = true;

        for (size_t i = 0; i < set.points.size() && incomparable; ++i)
        {
            for (size_t j = i + 1; j < set.points.size(); ++j)
            {
                if (Dominates(set.points[i], set.points[j], dimensions) ||
                    Dominates(set.points[j], set.points[i], dimensions))
                {
                    incomparable = false;
                    break;
                }
            }
        }

        Check(incomparable, label + ": no point dominates another");

        Check(BruteForceMaxima(set).size() == points,
              label + ": brute force returns every point");
        Check(MAXIMA2(set).size() == points,
              label + ": MAXIMA2 returns every point");
    }

    // Same seed, same set; different seed, different set.
    const PointSet first = BuildAllMaximaSet(4, 20, 7u);
    const PointSet again = BuildAllMaximaSet(4, 20, 7u);
    const PointSet other = BuildAllMaximaSet(4, 20, 8u);

    bool reproducible = true;

    for (size_t i = 0; i < first.points.size() && reproducible; ++i)
    {
        for (size_t j = 0; j < first.points[i].coords.size(); ++j)
        {
            if (first.points[i].coords[j] != again.points[i].coords[j])
            {
                reproducible = false;
            }
        }
    }

    Check(reproducible, "the same seed produces the same set");

    bool differs = false;

    for (size_t i = 0; i < first.points.size() && !differs; ++i)
    {
        for (size_t j = 0; j < first.points[i].coords.size(); ++j)
        {
            if (first.points[i].coords[j] != other.points[i].coords[j])
            {
                differs = true;
            }
        }
    }

    Check(differs, "a different seed produces a different set");

    if (failures != 0)
    {
        std::cerr << failures << " antichain check(s) failed\n";

        return 1;
    }

    std::cout << "antichain generator: all checks passed\n";

    return 0;
}
