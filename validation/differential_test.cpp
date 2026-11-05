// Randomized differential test for MAXIMA2.
//
// MAXIMA2 is compared against BruteForceMaxima, which is the direct
// translation of the dominance definition and therefore serves as the
// reference implementation. The generated inputs use small coordinate ranges,
// both integer and fractional, so ties on individual coordinates and exact
// duplicate points are common; those are precisely the inputs that expose
// mistakes in the weak-dominance semantics of the FILTER merge step.
//
// The test is deterministic: a fixed seed is used, so a failing case can be
// reproduced from the dimension, point count and range printed with it.

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

#include "brute_force_maxima.hpp"
#include "item.hpp"
#include "maxima2.hpp"
#include "point_set.hpp"

namespace {

struct CoordinateRange {
    int steps = 2;          // number of distinct values
    bool fractional = false; // values spread over [0, 1] instead of 0..steps-1
};

// The maxima are an unordered set, so compare them as multisets. Duplicate
// output entries are significant and are therefore preserved.
bool SameMaxima(const std::vector<Item>& computed,
                const std::vector<Item>& expected)
{
    if (computed.size() != expected.size())
    {
        return false;
    }

    auto sorted = [](std::vector<Item> items)
    {
        std::sort(items.begin(), items.end(),
            [](const Item& a, const Item& b)
            {
                return a.coords < b.coords;
            }
        );

        return items;
    };

    const std::vector<Item> a = sorted(computed);
    const std::vector<Item> b = sorted(expected);

    return std::equal(a.begin(), a.end(), b.begin(),
        [](const Item& x, const Item& y)
        {
            return x.coords == y.coords;
        }
    );
}

void PrintPoints(const char* label, const std::vector<Item>& points)
{
    std::cout << "  " << label << ":";

    for (const auto& point : points)
    {
        std::cout << " (";

        for (size_t i = 0; i < point.coords.size(); ++i)
        {
            std::cout << point.coords[i]
                      << (i + 1 < point.coords.size() ? "," : "");
        }

        std::cout << ')';
    }

    std::cout << '\n';
}

std::vector<Item> GeneratePoints(
    size_t dimensions,
    size_t num_points,
    const CoordinateRange& range,
    std::mt19937& rng)
{
    std::uniform_int_distribution<int> value(0, range.steps - 1);

    std::vector<Item> points(num_points);

    for (auto& point : points)
    {
        point.coords.resize(dimensions);

        for (auto& coordinate : point.coords)
        {
            const int step = value(rng);

            coordinate = range.fractional
                ? static_cast<double>(step) / static_cast<double>(range.steps - 1)
                : static_cast<double>(step);
        }
    }

    return points;
}

// An inconsistent point set must be rejected by both entry points instead of
// being read out of bounds.
int CheckInputValidation()
{
    int failures = 0;

    const PointSet zero_dimension{0, {}};
    const PointSet ragged{2, {Item{{1.0, 2.0}}, Item{{3.0}}}};

    const PointSet not_finite{
        2,
        {Item{{std::numeric_limits<double>::quiet_NaN(), 1.0}}}
    };

    // MAXIMA2 must throw, so RejectsInvalidSet has to observe an exception.
    auto expect_rejected = [&failures](const PointSet& set, const char* label)
    {
        bool rejected = false;

        try
        {
            MAXIMA2(set);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        catch (...)
        {
            rejected = false;
        }

        if (!rejected)
        {
            std::cout << "Invalid input accepted by MAXIMA2: " << label << '\n';
            ++failures;
        }

        rejected = false;

        try
        {
            BruteForceMaxima(set);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        catch (...)
        {
            rejected = false;
        }

        if (!rejected)
        {
            std::cout << "Invalid input accepted by BruteForceMaxima: " << label << '\n';
            ++failures;
        }
    };

    expect_rejected(zero_dimension, "zero dimension");
    expect_rejected(ragged, "point with the wrong coordinate count");
    expect_rejected(not_finite, "non-finite coordinate");

    return failures;
}

} // namespace

int main()
{
    constexpr std::uint32_t seed = 20240211;

    // Small ranges produce heavy tying and many duplicate points; the larger
    // ranges approximate tie-free data. Fractional ranges cover the floating
    // point inputs that the hand-built integer datasets do not.
    const std::vector<CoordinateRange> ranges = {
        {2, false},
        {3, false},
        {5, false},
        {1000, false},
        {21, true},
        {101, true},
    };

    constexpr int repetitions_per_size = 8;
    constexpr size_t max_points = 40;

    std::mt19937 rng(seed);

    size_t cases = 0;
    size_t mismatches = 0;
    size_t reported = 0;

    // Start at dimension 2: a single coordinate is handled by a separate base
    // case and has no merge step to exercise.
    for (size_t d = 2; d <= 8; ++d)
    {
        for (const CoordinateRange& range : ranges)
        {
            for (size_t n = 2; n <= max_points; ++n)
            {
                for (int repetition = 0; repetition < repetitions_per_size;
                     ++repetition)
                {
                    const std::vector<Item> points =
                        GeneratePoints(d, n, range, rng);

                    const PointSet set{d, points};

                    const std::vector<Item> expected = BruteForceMaxima(set);

                    // MAXIMA2 sorts internally; the oracle keeps seeing the
                    // original order, which must not matter.
                    const std::vector<Item> computed = MAXIMA2(set);

                    ++cases;

                    if (SameMaxima(computed, expected))
                    {
                        continue;
                    }

                    ++mismatches;

                    if (reported < 5)
                    {
                        ++reported;

                        std::cout << "Mismatch: d=" << d
                                  << " n=" << n
                                  << " steps=" << range.steps
                                  << (range.fractional ? " fractional" : " integer")
                                  << " repetition=" << repetition
                                  << " (seed=" << seed << ")\n";

                        PrintPoints("points", points);
                        PrintPoints("MAXIMA2", computed);
                        PrintPoints("brute force", expected);
                    }
                }
            }
        }
    }

    const int validation_failures = CheckInputValidation();

    std::cout << '\n';
    std::cout << "Differential test: " << cases << " cases, "
              << mismatches << " mismatches (seed=" << seed << ")\n";
    std::cout << "Input validation: " << validation_failures
              << " failure(s)\n";

    if (mismatches != 0 || validation_failures != 0)
    {
        std::cout << "FAILED\n";
        return 1;
    }

    // Guard against a misconfigured run that silently compares nothing.
    if (cases < 1000)
    {
        std::cout << "FAILED: only " << cases << " cases were generated\n";
        return 1;
    }

    std::cout << "PASSED\n";
    return 0;
}
