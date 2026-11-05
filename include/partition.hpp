#pragma once

#include <cstddef>

#include "item.hpp"

// Sorts `count` points by coordinate `d-1` and returns the size of the lower
// half V1; the upper half V2 occupies the remaining points. The sort is what
// guarantees that every point of V1 is at or below every point of V2, which is
// the threshold argument FILTER relies on.
//
// Precondition: count >= 2 and every point has d coordinates.
size_t PartitionV(Item* points, size_t count, size_t d);
