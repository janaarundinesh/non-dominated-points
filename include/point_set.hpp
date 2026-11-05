#pragma once

#include <cstddef>
#include <vector>

#include "item.hpp"

// A point set carries its own dimension, so an algorithm can never be handed a
// dimension that does not match the data. Algorithms that index coordinates
// take a PointSet and validate it, which removes the whole class of
// out-of-bounds reads that an independent `d` parameter allowed.
struct PointSet {
    size_t dimensions = 0;
    std::vector<Item> points;
};

// True when the dimension is positive, every point has exactly that many
// coordinates, and every coordinate is finite. An empty point set is valid as
// long as its dimension is known.
bool IsValid(const PointSet& set);
