#pragma once
#include "sorting.hpp"
#include <vector>
#include "item.hpp"

std::pair<
    std::vector<Item>,
    std::vector<Item>
>
PartitionV(std::vector<Item> V, size_t d);