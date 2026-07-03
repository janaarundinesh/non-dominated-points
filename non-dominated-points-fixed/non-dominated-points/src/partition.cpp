#include "partition.hpp"
#include <algorithm>

std::pair<std::vector<Item>,std::vector<Item>>
    PartitionV(std::vector<Item> V, size_t d)
{
    // Split V by the median of its d-th coordinate (coords[d-1]).
    // This must be the same axis ThresholdFromV1/SplitU use, since the
    // threshold computed from V1 is later applied to U along that
    // exact axis. It also must be coords[d-1] (not d-2): FILTER's
    // recursive call C = FILTER(U1, V2, d-1) checks coordinates
    // 0..d-2, which is exactly "all coordinates except the one this
    // split already guarantees" only when the guaranteed coordinate is
    // the last one, d-1.
    SortByCoordinate(V, d - 1);

    size_t mid = V.size()/2;

    std::vector<Item> V1(V.begin(),V.begin()+mid);

    std::vector<Item> V2(V.begin()+mid,V.end());

    return {V1,V2};
}
