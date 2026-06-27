#pragma once

#include <vector>
#include "item.hpp"

std::pair<
    std::vector<Item>,
    std::vector<Item>
>
SplitU(
    const std::vector<Item>& U,
    int threshold,
    size_t d
);