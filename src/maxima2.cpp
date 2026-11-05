#include "maxima2.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

#include "brute_force_maxima.hpp"
#include "filter.hpp"
#include "point_set.hpp"
#include "sorting.hpp"

namespace {

// Recursive divide-and-conquer step. The maxima of the range are compacted to
// its front and their number is returned, so no level of the recursion has to
// allocate a subset: every operation partitions or compacts in place.
//
// Precondition: the range is sorted by the last coordinate and every point has
// d coordinates.
size_t MaximaOfSorted(Item* first, size_t count, size_t d)
{
    if (count <= 1) // Base case: if there's 0 or 1 point, return it as the maxima
    {
        return count;
    }

    size_t mid = count / 2; // Finds the midpoint of the sorted points

    // Move mid to the right until we find a point with a different last coordinate
    // This ensures that we don't split points with the same last coordinate

    while (mid < count && first[mid].coords[d - 1] == first[mid - 1].coords[d - 1])
    {
        ++mid;
    }

    // If mid reaches the end of the range,
    // it means all points have the same last coordinate
    //

    if (mid == count)
    {
        return BruteForceMaximaInPlace(first, count, d);
    }

    const size_t m1 = MaximaOfSorted(first, mid, d);
    const size_t m2 = MaximaOfSorted(first + mid, count - mid, d);

    if (d == 1)
    {
        // Every maximum is in the upper half, which was sorted to dominate.
        for (size_t i = 0; i < m2; ++i)
        {
            first[i] = std::move(first[mid + i]);
        }

        return m2;
    }

    // M1 is at [first, first + m1), M2 at [first + mid, first + mid + m2).
    const size_t survivors = FILTER(first, m1, first + mid, m2, d - 1);

    // Append M2 after the survivors of M1; M2 remains a complete set because
    // FILTER only reorders it.
    for (size_t i = 0; i < m2; ++i)
    {
        if (survivors + i != mid + i)
        {
            first[survivors + i] = std::move(first[mid + i]);
        }
    }

    return survivors + m2;
}

} // namespace

std::vector<Item> MAXIMA2(const PointSet& set)
{
    if (!IsValid(set))
    {
        throw std::invalid_argument(
            "MAXIMA2: point set has zero dimension or a point with the wrong coordinate count");
    }

    std::vector<Item> points = set.points;

    SortByLastCoordinate(points); // Sort points by the last coordinate

    const size_t count = MaximaOfSorted(points.data(), points.size(), set.dimensions);

    // The maxima are compacted at the front; dropping the rest avoids copying
    // them into a second result vector.
    points.resize(count);

    return points;
}
