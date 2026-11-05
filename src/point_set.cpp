#include "point_set.hpp"

#include <cmath>

bool IsValid(const PointSet& set)
{
    if (set.dimensions == 0)
    {
        return false;
    }

    for (const auto& point : set.points)
    {
        if (point.coords.size() != set.dimensions)
        {
            return false;
        }

        // NaN compares false against everything, which would make a point
        // undominated by accident; infinities have no meaningful ordering
        // against each other in this problem.
        for (const double coordinate : point.coords)
        {
            if (!std::isfinite(coordinate))
            {
                return false;
            }
        }
    }

    return true;
}
