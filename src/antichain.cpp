#include "antichain.hpp"

#include <algorithm>
#include <random>
#include <vector>

PointSet BuildAllMaximaSet(size_t dimensions, size_t num_points, unsigned seed)
{
    std::mt19937_64 generator(seed);
    std::uniform_int_distribution<std::int64_t> cut_distribution(
        0, kAntichainCoordinateSum);

    std::vector<std::int64_t> cuts(dimensions - 1);

    PointSet set;
    set.dimensions = dimensions;
    set.points.resize(num_points);

    for (auto& point : set.points)
    {
        point.coords.resize(dimensions);

        for (auto& cut : cuts)
        {
            cut = cut_distribution(generator);
        }

        std::sort(cuts.begin(), cuts.end());

        point.coords[0] = static_cast<double>(cuts[0]);

        for (size_t j = 1; j + 1 < dimensions; ++j)
        {
            point.coords[j] =
                static_cast<double>(cuts[j] - cuts[j - 1]);
        }

        point.coords[dimensions - 1] =
            static_cast<double>(kAntichainCoordinateSum - cuts[dimensions - 2]);
    }

    return set;
}
