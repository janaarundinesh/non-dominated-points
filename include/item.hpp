#pragma once

#include <vector>

// A point in d-dimensional space.
//
// Coordinates are compared exactly (no tolerance), so inputs are expected to
// carry the values that should be compared; the algorithms are not designed for
// values that are only approximately equal.
struct Item {
    std::vector<double> coords; // coords[0]..coords[d-1]
};
