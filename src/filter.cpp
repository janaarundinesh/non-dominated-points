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
    std::vector<Item> survivors;

    for(const auto& u : U)
    {
        bool dominated = false;

        for(const auto& v : V)
        {
            if(Dominates(v,u,2))
            {
                dominated = true;
                break;
            }
        }

        if(!dominated)
        {
            survivors.push_back(u);
        }
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