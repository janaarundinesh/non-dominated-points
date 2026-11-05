#pragma once

#include <cstddef>
#include <vector>

#include "item.hpp"

void SortByLastCoordinate(std::vector<Item>& items);

void SortByCoordinate(std::vector<Item>& items, size_t k);

// Range versions. The algorithms work on one buffer, so they sort in place
// instead of building sorted copies.
void SortByLastCoordinate(Item* first, size_t count);

void SortByCoordinate(Item* first, size_t count, size_t k);
