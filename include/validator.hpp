#pragma once

#include <optional>
#include <string>
#include <vector>

#include "item.hpp"

struct ValidationDataset
{
    size_t dimension = 0;
    std::vector<Item> inputPoints;
    std::vector<Item> expectedMaxima;
};

// Loads a test dataset (dimension, points, expected maxima). Returns nothing
// when the file cannot be opened or does not follow the documented format, and
// reports the reason on stderr.
std::optional<ValidationDataset> LoadDataset(const std::string& filename);
