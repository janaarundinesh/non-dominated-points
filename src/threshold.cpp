#include "threshold.hpp"

int ThresholdFromV1(
    const std::vector<Item>& V1,
    size_t d
)
{
    int xmax = V1[0].coords[d-2];

    for(const auto& p : V1)
    {
        if(p.coords[d-2] > xmax)
        {
            xmax = p.coords[d-2];
        }
    }

    return xmax;
}