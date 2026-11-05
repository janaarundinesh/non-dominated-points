#pragma once

#include <cstddef>
#include <vector>

#include "item.hpp"
#include "point_set.hpp"

// Brute-force maxima, O(N^2 d). Throws std::invalid_argument if the set is
// inconsistent (zero dimension or a point with the wrong coordinate count).
std::vector<Item> BruteForceMaxima(const PointSet& set);

// Same computation for data that has already been validated. Used by the MPI
// programs on their local share of the data.
std::vector<Item> BruteForceMaxima(const std::vector<Item>& points, size_t d);

// In-place variant: compacts the maxima of the range to its front and returns
// how many there are. Used by MAXIMA2 when the recursion cannot separate the
// points, where copying the block out would defeat the purpose of working on
// one buffer.
//
// Precondition: every point has d coordinates.
size_t BruteForceMaximaInPlace(Item* points, size_t count, size_t d);
