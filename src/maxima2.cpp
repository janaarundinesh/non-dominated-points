#include "maxima2.hpp"
#include "sorting.hpp"
#include "filter.hpp"
#include "dominance.hpp"


static std::vector<Item> BruteForceMaxima(
    const std::vector<Item>& points,
    size_t d
)
{
    std::vector<Item> result;

    for(size_t i = 0; i < points.size(); i++)
    {
        bool dominated = false;

        for(size_t j = 0; j < points.size(); j++)
        {
            if(i == j) continue;

            if(Dominates(points[j], points[i], d))
            {
                dominated = true;
                break;
            }
        }

        if(!dominated)
        {
            result.push_back(points[i]);
        }
    }

    return result;
}

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

    while(mid < points.size() &&
          points[mid].coords[d-1] == points[mid-1].coords[d-1])
    {
        mid++;
    }

    if(mid == points.size())
    {
        return BruteForceMaxima(points, d);
    }

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

    if(d == 1)
    {
        return M2;
    }
    
    auto survivors = WeakCoverFilter(M1, M2, d - 1);

    survivors.insert(
        survivors.end(),
        M2.begin(),
        M2.end()
    );

    return survivors;
}
