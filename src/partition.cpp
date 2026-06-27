#include "partition.hpp"
#include <algorithm>

std::pair<std::vector<Item>,std::vector<Item>>
    PartitionV(std::vector<Item> V)
{
    SortByLastCoordinate(std::vector<Item>& V);

    size_t mid = V.size()/2;

    std::vector<Item> V1(V.begin(),V.begin()+mid);

    std::vector<Item> V2(V.begin()+mid,V.end());

    return {V1,V2};
}  