// The three helpers MAXIMA2 hands its merge step to.
//
// PartitionV, SplitU and FILTER are only ever called from inside the recursion,
// so the differential test sees their effect but never their contract. Each of
// them has one invariant that the recursion relies on, and each of them sorts or
// moves points around while doing it:
//
//   PartitionV  the lower half is at or below every point of the upper half;
//   SplitU      the first `k` points are at or below the threshold, the rest
//               are above it;
//   FILTER      the survivors are exactly the points of U that no point of V
//               weakly dominates on the first d coordinates, and V loses
//               nothing.
//
// The last one is where the tie semantics live: `<=` on every compared
// coordinate is what removes a point that merely ties, because the coordinate
// dropped from the comparison already carries the strictness.

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "dominance.hpp"
#include "filter.hpp"
#include "item.hpp"
#include "partition.hpp"
#include "split_u.hpp"

namespace {

int failures = 0;

// A broken tie comparison fails hundreds of the random cases at once, so only
// the first few are printed; the final count still says how widespread it is.
constexpr int kMaxReportedFailures = 10;

void Check(bool condition, const std::string& what)
{
    if (condition)
    {
        return;
    }

    ++failures;

    if (failures <= kMaxReportedFailures)
    {
        std::cerr << "FAILED: " << what << '\n';
    }
}

Item Point(std::initializer_list<double> coordinates)
{
    return Item{std::vector<double>(coordinates)};
}

std::vector<Item> Sorted(std::vector<Item> points)
{
    std::sort(points.begin(), points.end(),
        [](const Item& a, const Item& b)
        {
            return a.coords < b.coords;
        }
    );

    return points;
}

bool SameMultiset(const std::vector<Item>& first, const std::vector<Item>& second)
{
    if (first.size() != second.size())
    {
        return false;
    }

    const std::vector<Item> a = Sorted(first);
    const std::vector<Item> b = Sorted(second);

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (a[i].coords != b[i].coords)
        {
            return false;
        }
    }

    return true;
}

std::mt19937 rng(90210u);

// Values in {0, 1, 2}: enough for a strict order, few enough that ties on the
// last coordinate -- the case the partition has to survive -- are common.
std::vector<Item> RandomPoints(size_t count, size_t dimensions)
{
    std::uniform_int_distribution<int> value(0, 2);

    std::vector<Item> points(count);

    for (auto& point : points)
    {
        point.coords.resize(dimensions);

        for (auto& coordinate : point.coords)
        {
            coordinate = static_cast<double>(value(rng));
        }
    }

    return points;
}

void CheckPartition(size_t count, size_t dimensions)
{
    const std::vector<Item> points = RandomPoints(count, dimensions);

    std::vector<Item> buffer = points;

    const size_t lower = PartitionV(buffer.data(), buffer.size(), dimensions);

    const std::string label = "PartitionV N=" + std::to_string(count) +
                              " d=" + std::to_string(dimensions);

    Check(lower == count / 2, label + ": returns the size of the lower half");

    Check(SameMultiset(buffer, points), label + ": keeps every point");

    bool sorted = true;
    bool separated = true;

    for (size_t i = 0; i + 1 < buffer.size(); ++i)
    {
        if (buffer[i].coords[dimensions - 1] >
            buffer[i + 1].coords[dimensions - 1])
        {
            sorted = false;
        }
    }

    for (size_t i = 0; i < lower; ++i)
    {
        for (size_t j = lower; j < buffer.size(); ++j)
        {
            if (buffer[i].coords[dimensions - 1] >
                buffer[j].coords[dimensions - 1])
            {
                separated = false;
            }
        }
    }

    Check(sorted, label + ": sorts by the last coordinate");
    Check(separated, label + ": V1 is at or below V2");
}

void CheckSplit(size_t count, size_t dimensions, double threshold)
{
    const std::vector<Item> points = RandomPoints(count, dimensions);

    std::vector<Item> buffer = points;

    const size_t low = SplitU(buffer.data(), buffer.size(), threshold, dimensions);

    const std::string label = "SplitU N=" + std::to_string(count) +
                              " d=" + std::to_string(dimensions);

    Check(low <= count, label + ": the split is inside the range");

    Check(SameMultiset(buffer, points), label + ": keeps every point");

    bool correct = true;

    for (size_t i = 0; i < count; ++i)
    {
        const bool at_or_below = buffer[i].coords[dimensions - 1] <= threshold;

        if (at_or_below != (i < low))
        {
            correct = false;
        }
    }

    Check(correct, label + ": U1 at or below the threshold, U2 above it");
}

void CheckFilterCase(
    size_t d,
    const std::vector<Item>& u_points,
    const std::vector<Item>& v_points,
    const std::string& label)
{
    std::vector<Item> buffer;
    buffer.reserve(u_points.size() + v_points.size());
    buffer.insert(buffer.end(), u_points.begin(), u_points.end());
    buffer.insert(buffer.end(), v_points.begin(), v_points.end());

    if (buffer.empty())
    {
        // Keep the pointers non-null for the empty case; the point placed here
        // is outside both ranges, so it is never read.
        buffer.push_back(Item{{0.0}});
    }

    Item* const u = buffer.data();
    Item* const v = buffer.data() + u_points.size();

    const size_t survivors =
        FILTER(u, u_points.size(), v, v_points.size(), d);

    // The contract, written as the brute-force predicate: a point survives when
    // no point of V weakly dominates it on the first d coordinates.
    std::vector<Item> expected;

    for (const Item& candidate : u_points)
    {
        bool dominated = false;

        for (const Item& other : v_points)
        {
            if (WeaklyDominates(other, candidate, d))
            {
                dominated = true;

                break;
            }
        }

        if (!dominated)
        {
            expected.push_back(candidate);
        }
    }

    const std::vector<Item> survivors_at_front(u, u + survivors);

    Check(SameMultiset(survivors_at_front, expected),
          label + ": the front of U holds exactly the points V does not "
                  "dominate");

    const std::vector<Item> v_after(v, v + v_points.size());

    Check(SameMultiset(v_after, v_points), label + ": V keeps all its points");
}

} // namespace

int main()
{
    // --- PartitionV -------------------------------------------------------

    for (size_t dimensions = 2; dimensions <= 4; ++dimensions)
    {
        for (size_t count = 2; count <= 9; ++count)
        {
            CheckPartition(count, dimensions);
        }
    }

    {
        // Hand-written case: the tie at 2.0 must be split arbitrarily rather
        // than dropped, and the result must still be separated.
        const std::vector<Item> points = {
            Point({0.0, 3.0}), Point({4.0, 1.0}), Point({2.0, 2.0}),
            Point({5.0, 2.0}), Point({1.0, 2.0})
        };

        std::vector<Item> buffer = points;

        const size_t lower = PartitionV(buffer.data(), buffer.size(), 2);

        Check(lower == 2, "PartitionV: five points give a lower half of two");
        Check(SameMultiset(buffer, points), "PartitionV: the tie group is kept");
    }

    // --- SplitU -----------------------------------------------------------

    for (size_t dimensions = 2; dimensions <= 4; ++dimensions)
    {
        for (size_t count = 0; count <= 8; ++count)
        {
            for (double threshold = -1.0; threshold <= 3.0; threshold += 1.0)
            {
                CheckSplit(count, dimensions, threshold);
            }
        }
    }

    // --- FILTER -----------------------------------------------------------
    //
    // The single-point branch, the two-coordinate sweep and the recursive
    // partition are separate code paths, so each is exercised directly.

    CheckFilterCase(3,
                    {Point({1.0, 1.0, 1.0}), Point({2.0, 2.0, 2.0})},
                    {Point({1.0, 1.0, 1.0})},
                    "FILTER d=3, one point in V");

    CheckFilterCase(3,
                    {Point({1.0, 1.0, 1.0}), Point({1.0, 1.0, 2.0})},
                    {},
                    "FILTER with an empty V");

    CheckFilterCase(2,
                    {Point({1.0, 1.0}), Point({2.0, 1.0}), Point({0.0, 2.0})},
                    {Point({1.0, 1.0}), Point({0.0, 0.0})},
                    "FILTER d=2, tied coordinates");

    CheckFilterCase(1,
                    {Point({1.0}), Point({2.0}), Point({3.0})},
                    {Point({2.0})},
                    "FILTER d=1, three points in V");

    for (size_t d = 1; d <= 3; ++d)
    {
        for (size_t u_count = 0; u_count <= 5; ++u_count)
        {
            for (size_t v_count = 0; v_count <= 5; ++v_count)
            {
                for (int repetition = 0; repetition < 15; ++repetition)
                {
                    const std::string label =
                        "FILTER d=" + std::to_string(d) +
                        " U=" + std::to_string(u_count) +
                        " V=" + std::to_string(v_count);

                    CheckFilterCase(
                        d,
                        RandomPoints(u_count, d),
                        RandomPoints(v_count, d),
                        label);
                }
            }
        }
    }

    if (failures != 0)
    {
        std::cerr << failures << " structure check(s) failed\n";

        return 1;
    }

    std::cout << "partition / split / filter: all checks passed\n";

    return 0;
}
