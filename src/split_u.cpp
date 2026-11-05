#include "split_u.hpp"

#include <algorithm>

size_t SplitU(Item* points, size_t count, double threshold, size_t d)
{
    Item* const split = std::partition(
        points,
        points + count,
        [threshold, d](const Item& point)
        {
            return point.coords[d - 1] <= threshold;
        }
    );

    return static_cast<size_t>(split - points);
}
