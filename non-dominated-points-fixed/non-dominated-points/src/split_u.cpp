#include "split_u.hpp"

std::pair<
    std::vector<Item>,
    std::vector<Item>
>
SplitU(
    const std::vector<Item>& U,
    int threshold,
    size_t d
)
{
    std::vector<Item> U1;
    std::vector<Item> U2;

    for(const auto& p : U)
    {
        if(p.coords[d-1] <= threshold)
        {
            U1.push_back(p);
        }
        else
        {
            U2.push_back(p);
        }
    }

    return {U1,U2};
}