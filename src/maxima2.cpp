#include "maxima2.hpp"
#include "sorting.hpp"
#include "filter.hpp"
#include "brute_force_maxima.hpp"

std::vector<Item> MAXIMA2(std::vector<Item> points, size_t d)
{
    SortByLastCoordinate(points);

    if(points.size() <= 1)
    {
        return points;
    }

    size_t mid = points.size()/2;

    while(mid < points.size() &&
          points[mid].coords[d-1] == points[mid-1].coords[d-1])
    {
        mid++;
    }

    if(mid == points.size())
    {
        return BruteForceMaxima(points, d);
    }

    std::vector<Item> S1(points.begin(), points.begin()+mid);

    std::vector<Item> S2(points.begin()+mid, points.end());

    auto M1 = MAXIMA2(S1,d);
    auto M2 = MAXIMA2(S2,d);

    if(d == 1)
    {
        return M2;
    }
    
    auto survivors = FILTER(M1, M2, d - 1);

    survivors.insert(survivors.end(), M2.begin(), M2.end());
    return survivors;
}