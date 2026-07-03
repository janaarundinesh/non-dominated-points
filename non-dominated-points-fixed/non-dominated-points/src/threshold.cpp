#include "threshold.hpp"

int ThresholdFromV1(
    const std::vector<Item>& V1,
    size_t d
)
{
    int xmax = V1[0].coords[d-1];

    for(const auto& p : V1)
    {
        if(p.coords[d-1] > xmax)
        {
            xmax = p.coords[d-1];
        }
    }

    return xmax;
}