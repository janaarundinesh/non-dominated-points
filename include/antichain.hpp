#pragma once

#include <cstddef>
#include <cstdint>

#include "point_set.hpp"

// An antichain: no point dominates another, so every point is maximal. This is
// the worst case for MAXIMA2's pruning and for the brute-force baseline's early
// exit, and the benchmark tool and its test both need to build one.
//
// Every point is drawn on the hyperplane whose coordinates sum to
// kAntichainCoordinateSum. A point that dominated another would need all
// coordinates at least as large and one strictly larger, which would make its
// sum larger, so domination is impossible by construction rather than by a
// tolerance. The coordinates are integers small enough to be exact in a double,
// so the sums are exactly equal too.
inline constexpr std::int64_t kAntichainCoordinateSum = 1000000;

// `dimensions` must be at least 2 and `num_points` at least 1. The seed makes
// the set reproducible.
PointSet BuildAllMaximaSet(size_t dimensions, size_t num_points, unsigned seed);
