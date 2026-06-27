#include "maxima2.hpp"

std::vector<Item> MAXIMA2(
    std::vector<Item> points,
    size_t d
)
{
    if(points.size() <= 1)
    {
        return points;
    }

    size_t mid = points.size()/2;

    std::vector<Item> S1(
        points.begin(),
        points.begin()+mid
    );

    std::vector<Item> S2(
        points.begin()+mid,
        points.end()
    );

    auto M1 = MAXIMA2(S1,d);
    auto M2 = MAXIMA2(S2,d);

    return points; // temporary
}