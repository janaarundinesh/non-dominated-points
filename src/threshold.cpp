#include "threshold.hpp"

double ThresholdFromV1(const Item* points, size_t count, size_t d)
{
    double maximum = points[0].coords[d - 1];

    for (size_t i = 1; i < count; ++i)
    {
        if (points[i].coords[d - 1] > maximum)
        {
            maximum = points[i].coords[d - 1];
        }
    }

    return maximum;
}
