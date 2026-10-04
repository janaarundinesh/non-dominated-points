#pragma once

#include <vector>
#include "item.hpp"

std::vector<Item> BruteForceMaxima(
    const std::vector<Item>& points,
    size_t d
);
