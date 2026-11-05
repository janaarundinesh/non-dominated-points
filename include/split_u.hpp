#pragma once

#include <cstddef>

#include "item.hpp"

// Partitions U in place around `threshold`: the first `returned` points have
// coordinate d-1 at or below the threshold (U1), the remaining points are above
// it (U2).
//
// Precondition: every point has d coordinates.
size_t SplitU(Item* points, size_t count, double threshold, size_t d);
