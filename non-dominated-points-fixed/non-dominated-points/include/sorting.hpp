#pragma once

#include "item.hpp"
#include <vector>

void SortByLastCoordinate(std::vector<Item>& items);

// Sorts items in ascending order by coords[k].
// Used by PartitionV/FILTER2 to sort along the "active" dimension
// of the current recursive call, which is not necessarily the
// item's true last coordinate once d < coords.size().
void SortByCoordinate(std::vector<Item>& items, size_t k);