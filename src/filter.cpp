#include "filter.hpp"

#include <algorithm>
#include <limits>
#include <utility>

#include "dominance.hpp"
#include "partition.hpp"
#include "sorting.hpp"
#include "split_u.hpp"
#include "threshold.hpp"

namespace {

// Keeps the points of U that are not weakly dominated by a single point,
// compacting them to the front.
size_t RemoveDominatedByOne(Item* u, size_t u_count, const Item& v, size_t d)
{
    size_t write = 0;

    for (size_t i = 0; i < u_count; ++i)
    {
        if (WeaklyDominates(v, u[i], d))
        {
            continue;
        }

        if (write != i)
        {
            u[write] = std::move(u[i]);
        }

        ++write;
    }

    return write;
}

// Single coordinate: a point survives only if it is strictly better than every
// point of V, because the coordinate that is no longer compared already
// dominates strictly.
size_t RemoveDominatedByMaxCoordinate(
    Item* u,
    size_t u_count,
    const Item* v,
    size_t v_count)
{
    double maximum = v[0].coords[0];

    for (size_t i = 1; i < v_count; ++i)
    {
        maximum = std::max(maximum, v[i].coords[0]);
    }

    size_t write = 0;

    for (size_t i = 0; i < u_count; ++i)
    {
        if (u[i].coords[0] <= maximum)
        {
            continue;
        }

        if (write != i)
        {
            u[write] = std::move(u[i]);
        }

        ++write;
    }

    return write;
}

// Linear sweep for the two remaining coordinates. Both ranges are sorted by the
// second coordinate; walking them downwards keeps the best first coordinate
// seen so far, so every point needs only two comparisons.
size_t Filter2InPlace(Item* u, size_t u_count, Item* v, size_t v_count)
{
    SortByCoordinate(u, u_count, 1);
    SortByCoordinate(v, v_count, 1);

    double best_x_strict = std::numeric_limits<double>::lowest();

    size_t j = v_count;   // points of V above the current coordinate
    size_t i = u_count;
    size_t write = u_count; // survivors are written from the back

    while (i > 0)
    {
        const double y = u[i - 1].coords[1];

        // Absorb every v with strictly greater y into best_x_strict.
        while (j > 0 && v[j - 1].coords[1] > y)
        {
            best_x_strict = std::max(best_x_strict, v[j - 1].coords[0]);
            --j;
        }

        // Points of V tied at this y can still dominate through a larger x.
        size_t k = j;
        double best_x_equal = std::numeric_limits<double>::lowest();

        while (k > 0 && v[k - 1].coords[1] == y)
        {
            best_x_equal = std::max(best_x_equal, v[k - 1].coords[0]);
            --k;
        }

        while (i > 0 && u[i - 1].coords[1] == y)
        {
            const bool dominated =
                best_x_strict >= u[i - 1].coords[0] ||
                best_x_equal >= u[i - 1].coords[0];

            if (!dominated)
            {
                --write;

                if (write != i - 1)
                {
                    u[write] = std::move(u[i - 1]);
                }
            }

            --i;
        }

        best_x_strict = std::max(best_x_strict, best_x_equal);

        j = k;
    }

    const size_t survivors = u_count - write;

    // The survivors form a contiguous block at the back; move it to the front.
    for (size_t t = 0; t < survivors; ++t)
    {
        if (write + t != t)
        {
            u[t] = std::move(u[write + t]);
        }
    }

    return survivors;
}

} // namespace

size_t FILTER(Item* u, size_t u_count, Item* v, size_t v_count, size_t d)
{
    if (u_count == 0)
    {
        return 0;
    }

    if (v_count == 0)
    {
        return u_count;
    }

    if (v_count == 1)
    {
        return RemoveDominatedByOne(u, u_count, v[0], d);
    }

    if (d == 1)
    {
        return RemoveDominatedByMaxCoordinate(u, u_count, v, v_count);
    }

    if (d == 2)
    {
        return Filter2InPlace(u, u_count, v, v_count);
    }

    const size_t v1_count = PartitionV(v, v_count, d);
    const size_t v2_count = v_count - v1_count;

    const double threshold = ThresholdFromV1(v, v1_count, d);

    const size_t u1_count = SplitU(u, u_count, threshold, d);
    const size_t u2_count = u_count - u1_count;

    Item* const v2 = v + v1_count;

    // U2 against V2, and U1 against V1, on the full set of coordinates.
    const size_t a_count = FILTER(u + u1_count, u2_count, v2, v2_count, d);
    const size_t b_count = FILTER(u, u1_count, v, v1_count, d);

    // U1 against V2 with one fewer coordinate. Filtering the survivors of the
    // previous step is the same predicate as intersecting the two survivor
    // sets, without materialising either of them.
    const size_t c_count = FILTER(u, b_count, v2, v2_count, d - 1);

    for (size_t i = 0; i < a_count; ++i)
    {
        if (c_count + i != u1_count + i)
        {
            u[c_count + i] = std::move(u[u1_count + i]);
        }
    }

    return c_count + a_count;
}
