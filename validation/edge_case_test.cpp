// Degenerate inputs that neither the random differential test nor the
// dataset-driven validator produces.
//
// The differential test starts at two points and two dimensions, because a
// single coordinate has no merge step to exercise. That leaves exactly the
// inputs where the recursion is not really a recursion: no points at all, one
// point, all points sharing the last coordinate (which sends MAXIMA2 to the
// brute-force base case), exact duplicates, and a single coordinate. Each of
// them has an obvious answer by hand, so the expected maxima are written out
// rather than only compared with the baseline -- two implementations agreeing
// on an empty answer would otherwise look like a pass.

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "brute_force_maxima.hpp"
#include "item.hpp"
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

Item Point(std::initializer_list<double> coordinates)
{
    return Item{std::vector<double>(coordinates)};
}

void Sort(std::vector<Item>& points)
{
    std::sort(points.begin(), points.end(),
        [](const Item& a, const Item& b)
        {
            return a.coords < b.coords;
        }
    );
}

std::string Describe(const std::vector<Item>& points)
{
    std::string text = "[";

    for (const auto& point : points)
    {
        text += " (";

        for (size_t i = 0; i < point.coords.size(); ++i)
        {
            text += std::to_string(point.coords[i]);

            if (i + 1 < point.coords.size())
            {
                text += ",";
            }
        }

        text += ")";
    }

    return text + " ]";
}

// Checks both algorithms against the hand-written answer, as multisets: the
// maxima are a set, but duplicates in the input are separate points and must
// all be reported.
void ExpectMaxima(
    const std::string& label,
    size_t dimensions,
    const std::vector<Item>& points,
    std::vector<Item> expected)
{
    const PointSet set{dimensions, points};

    const std::vector<Item> computed = MAXIMA2(set);
    const std::vector<Item> reference = BruteForceMaxima(set);

    std::vector<Item> sorted_computed = computed;
    std::vector<Item> sorted_reference = reference;

    Sort(sorted_computed);
    Sort(sorted_reference);
    Sort(expected);

    bool computed_matches = sorted_computed.size() == expected.size();

    for (size_t i = 0; computed_matches && i < expected.size(); ++i)
    {
        computed_matches = sorted_computed[i].coords == expected[i].coords;
    }

    Check(computed_matches,
          label + ": MAXIMA2 returns " + Describe(expected) +
              ", got " + Describe(sorted_computed));

    bool reference_matches = sorted_reference.size() == expected.size();

    for (size_t i = 0; reference_matches && i < expected.size(); ++i)
    {
        reference_matches = sorted_reference[i].coords == expected[i].coords;
    }

    Check(reference_matches,
          label + ": brute force returns " + Describe(expected) +
              ", got " + Describe(sorted_reference));
}

} // namespace

int main()
{
    // No points. A dimension is still known, so the set is valid; the answer is
    // empty and neither algorithm may read the buffer it was not given.
    ExpectMaxima("empty set", 3, {}, {});

    // One point: it is trivially maximal and must be copied out unchanged.
    ExpectMaxima("single point", 3, {Point({5.0, 5.0, 5.0})},
                 {Point({5.0, 5.0, 5.0})});

    // Exact duplicates. Neither copy dominates the other, so all four survive;
    // the fifth point is dominated by them. This is the case that a tolerance
    // or a `>=`-everywhere dominance test would get wrong.
    ExpectMaxima("duplicates",
                 2,
                 {Point({1.0, 1.0}), Point({1.0, 1.0}), Point({1.0, 1.0}),
                  Point({1.0, 1.0}), Point({0.0, 0.0})},
                 {Point({1.0, 1.0}), Point({1.0, 1.0}), Point({1.0, 1.0}),
                  Point({1.0, 1.0})});

    // Every point shares the last coordinate, so the split cannot separate the
    // set and MAXIMA2 falls back to the brute-force base case.
    ExpectMaxima("one tie group", 2,
                 {Point({1.0, 1.0}), Point({2.0, 1.0}), Point({3.0, 1.0})},
                 {Point({3.0, 1.0})});

    ExpectMaxima("all points identical", 2,
                 {Point({2.0, 2.0}), Point({2.0, 2.0}), Point({2.0, 2.0})},
                 {Point({2.0, 2.0}), Point({2.0, 2.0}), Point({2.0, 2.0})});

    // A chain: only the last point is maximal.
    ExpectMaxima("strict chain", 3,
                 {Point({1.0, 1.0, 1.0}), Point({2.0, 2.0, 2.0}),
                  Point({3.0, 3.0, 3.0})},
                 {Point({3.0, 3.0, 3.0})});

    // One coordinate, where the recursion has a dedicated base case.
    ExpectMaxima("1D, mixed", 1,
                 {Point({3.0}), Point({1.0}), Point({2.0}), Point({2.0})},
                 {Point({3.0})});

    ExpectMaxima("1D, tied maximum", 1, {Point({3.0}), Point({3.0})},
                 {Point({3.0}), Point({3.0})});

    ExpectMaxima("1D, single point", 1, {Point({7.0})}, {Point({7.0})});

    // Twelve dimensions: two incomparable maxima, one dominated point, one
    // point on the boundary between them.
    ExpectMaxima("12D", 12,
                 {Point({1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 1.0}),
                  Point({2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0,
                         2.0, 2.0}),
                  Point({1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 5.0}),
                  Point({0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0})},
                 {Point({2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0,
                         2.0, 2.0}),
                  Point({1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                         1.0, 5.0})});

    // --- the validity predicate ------------------------------------------

    Check(IsValid({3, {}}), "an empty set with a known dimension is valid");

    Check(!IsValid({0, {}}), "zero dimensions is invalid");

    Check(!IsValid({2, {Point({1.0})}}),
          "a point with too few coordinates is invalid");

    Check(!IsValid({2,
                    {Point({1.0, 2.0}),
                     Point({std::numeric_limits<double>::quiet_NaN(), 2.0})}}),
          "a NaN coordinate is invalid");

    Check(!IsValid({1, {Point({std::numeric_limits<double>::infinity()})}}),
          "an infinite coordinate is invalid");

    if (failures != 0)
    {
        std::cerr << failures << " edge case check(s) failed\n";

        return 1;
    }

    std::cout << "edge cases: all checks passed\n";

    return 0;
}
