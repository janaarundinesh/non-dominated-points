#pragma once

#include <vector>
#include "item.hpp"

std::vector<Item> Intersect(
    const std::vector<Item>& A,
    const std::vector<Item>& B
);