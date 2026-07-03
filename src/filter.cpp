#include "filter.hpp"
#include "dominance.hpp"
#include "partition.hpp"
#include "threshold.hpp"
#include "split_u.hpp"
#include "intersection.hpp"

std::vector<Item> FILTER2(
    const std::vector<Item>& U,
    const std::vector<Item>& V
)
{
    std::vector<Item> Us = U;
    std::vector<Item> Vs = V;

    SortByLastCoordinate(Us);
    SortByLastCoordinate(Vs);

    std::vector<Item> survivors;

    int i = static_cast<int>(Us.size()) - 1;
    int j = static_cast<int>(Vs.size()) - 1;

    int maxXSeen = std::numeric_limits<int>::min();

    while(i >= 0)
    {
        while(j >= 0 &&
              Vs[j].coords[1] >= Us[i].coords[1])
        {
            maxXSeen =
                std::max(
                    maxXSeen,
                    Vs[j].coords[0]
                );

            j--;
        }

        if(maxXSeen < Us[i].coords[0])
        {
            survivors.push_back(Us[i]);
        }

        i--;
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

    if(d == 2)
    {
        return FILTER2(U,V);
    }

    auto [V1,V2] = PartitionV(V);

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