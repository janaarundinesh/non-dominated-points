#include "brute_force_maxima.hpp"
#include "dominance.hpp"
#include "point_set.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

std::vector<Item> BruteForceMaxima(const std::vector<Item>& points, size_t d)
{
    std::vector<Item> result;

    for (size_t i = 0; i < points.size(); ++i)
    {
        bool dominated = false;

        for (size_t j = 0; j < points.size(); ++j)
        {
            if (i == j) continue;

            if (Dominates(points[j], points[i], d))
            {
                dominated = true;
                break;
            }
        }

        if (!dominated)
        {
            result.push_back(points[i]);
        }
    }

    return result;
}

std::vector<Item> BruteForceMaxima(const PointSet& set)
{
    if (!IsValid(set))
    {
        throw std::invalid_argument(
            "BruteForceMaxima: point set has zero dimension or a point with the wrong coordinate count");
    }

    return BruteForceMaxima(set.points, set.dimensions);
}

size_t BruteForceMaximaInPlace(Item* points, size_t count, size_t d)
{
    std::vector<char> dominated(count, 0);

    for (size_t i = 0; i < count; ++i)
    {
        for (size_t j = 0; j < count; ++j)
        {
            if (i == j) continue;

            if (Dominates(points[j], points[i], d))
            {
                dominated[i] = 1;
                break;
            }
        }
    }

    size_t write = 0;

    for (size_t i = 0; i < count; ++i)
    {
        if (dominated[i])
        {
            continue;
        }

        if (write != i)
        {
            points[write] = std::move(points[i]);
        }

        ++write;
    }

    return write;
}
