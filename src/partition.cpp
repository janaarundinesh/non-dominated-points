#include "partition.hpp"
#include "sorting.hpp"

size_t PartitionV(Item* points, size_t count, size_t d)
{
    SortByCoordinate(points, count, d - 1);

    return count / 2;
}
