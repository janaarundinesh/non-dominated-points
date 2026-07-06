#pragma once

#include <vector>
#include <string>

#include "item.hpp"

struct ValidationDataset
{
    size_t dimension;
    std::vector<Item> inputPoints;
    std::vector<Item> expectedMaxima;
};

ValidationDataset LoadDataset(const std::string& filename);