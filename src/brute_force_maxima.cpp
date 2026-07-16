#include "brute_force_maxima.hpp"
#include "dominance.hpp"

std::vector<Item> BruteForceMaxima(
    const std::vector<Item>& points,
    size_t d)
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
