#include "filter.hpp"
#include "dominance.hpp"
#include "partition.hpp"
#include "threshold.hpp"
#include "split_u.hpp"
#include "intersection.hpp"
#include <limits>
#include <algorithm>

std::vector<Item> FILTER2(
    const std::vector<Item>& U,
    const std::vector<Item>& V
)
{
    std::vector<Item> Us = U;
    std::vector<Item> Vs = V;

    SortByCoordinate(Us, 1);
    SortByCoordinate(Vs, 1);
    std::reverse(Us.begin(), Us.end());
    std::reverse(Vs.begin(), Vs.end());

    std::vector<Item> survivors;

    size_t j = 0; // next Vs index to absorb

    int bestX_strict = std::numeric_limits<int>::min();

    size_t i = 0;

    while(i < Us.size())
    {
        int y = Us[i].coords[1];

        // Absorb every v with strictly greater y into bestX_strict.
        while(j < Vs.size() && Vs[j].coords[1] > y)
        {
            bestX_strict = std::max(bestX_strict, Vs[j].coords[0]);
            j++;
        }

        int bestX_eq = std::numeric_limits<int>::min();

        size_t k = j;

        while(k < Vs.size() && Vs[k].coords[1] == y)
        {
            bestX_eq = std::max(bestX_eq, Vs[k].coords[0]);
            k++;
        }

        // Process every u tied at this same y using the same two
        // thresholds (bestX_strict, bestX_eq).
        while(i < Us.size() && Us[i].coords[1] == y)
        {
            bool dominated =
                (bestX_strict >= Us[i].coords[0]) ||
                (bestX_eq > Us[i].coords[0]);

            if(!dominated)
            {
                survivors.push_back(Us[i]);
            }

            i++;
        }

        bestX_strict = std::max(bestX_strict, bestX_eq);

        j = k;
    }

    return survivors;
}

std::vector<Item> FILTER(
    const std::vector<Item>& U,
    const std::vector<Item>& V,
    size_t d
)
{
    if(U.empty())
    {
        return {};
    }

    if(V.empty())
    {
        return U;
    }

    if(V.size() == 1)
    {
        const Item& v = V[0];

        std::vector<Item> survivors;

        for(const auto& u : U)
        {
            if(!Dominates(v, u, d))
            {
                survivors.push_back(u);
            }
        }

        return survivors;
    }

    if(d == 1)
    {
        // Single coordinate: u survives unless some v has coords[0] > u.coords[0]
        // (a strict improvement -- an exact tie does not dominate).
        int maxV = V[0].coords[0];

        for(const auto& v : V)
        {
            if(v.coords[0] > maxV)
            {
                maxV = v.coords[0];
            }
        }

        std::vector<Item> survivors;

        for(const auto& u : U)
        {
            if(u.coords[0] >= maxV)
            {
                survivors.push_back(u);
            }
        }

        return survivors;
    }

    if(d == 2)
    {
        return FILTER2(U,V);
    }

    auto [V1,V2] = PartitionV(V,d);

    int threshold = ThresholdFromV1(V1,d);

    auto [U1,U2] = SplitU(U,threshold,d);

    auto A = FILTER(U2,V2,d);

    auto B = FILTER(U1,V1,d);

    auto C = FILTER(U1,V2,d-1);

    auto BC = Intersect(B,C);

    A.insert(
        A.end(),
        BC.begin(),
        BC.end()
    );

    return A;
}