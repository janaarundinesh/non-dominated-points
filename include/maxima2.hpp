#pragma once

#include <vector>

#include "item.hpp"
#include "point_set.hpp"

// Computes the maxima (non-dominated points) of `set`.
//
// The point set is sorted by its last coordinate internally, so callers do not
// have to know about that requirement. Throws std::invalid_argument if the set
// is inconsistent (zero dimension or a point with the wrong coordinate count).
std::vector<Item> MAXIMA2(const PointSet& set);
