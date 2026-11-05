#pragma once

#include <cstddef>

#include "item.hpp"

// Largest coordinate d-1 among the first `count` points, used as the threshold
// that separates U1 from U2.
//
// Precondition: count >= 1 and every point has d coordinates.
double ThresholdFromV1(const Item* points, size_t count, size_t d);
