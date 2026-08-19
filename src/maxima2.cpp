#include "maxima2.hpp"
#include "filter.hpp"
#include "brute_force_maxima.hpp"

std::vector<Item> MAXIMA2(std::vector<Item> points, size_t d)
{

    if(points.size() <= 1) // Base case: if there's 0 or 1 point, return it as the maxima
    {
        return points;
    }

    size_t mid = points.size()/2; // Finds the midpoint of the sorted points

    // Move mid to the right until we find a point with a different last coordinate
    // This ensures that we don't split points with the same last coordinate

    while(mid < points.size() && points[mid].coords[d-1] == points[mid-1].coords[d-1])
    {
        mid++;
    }

    // If mid reaches the end of the points vector, 
    // it means all points have the same last coordinate
    //

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