#pragma once

#include <vector>
#include "item.hpp"

std::vector<Item> FILTER(
    const std::vector<Item>& U,
    const std::vector<Item>& V,
    size_t d
);

std::vector<Item> FILTER2(
    const std::vector<Item>& U,
    const std::vector<Item>& V
);