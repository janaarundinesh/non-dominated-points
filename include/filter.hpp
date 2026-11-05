#pragma once

#include <cstddef>

#include "item.hpp"

// Removes from U the points that V weakly dominates on the first d coordinates.
//
// U and V are disjoint contiguous ranges of one buffer. Survivors are compacted
// to the front of U and their number is returned. V is reordered but never
// loses elements, because MAXIMA2 still needs M2 after the merge.
//
// Weak dominance is what the merge requires: the coordinate that is no longer
// compared already accounts for the strict improvement, so a point that merely
// ties on the compared coordinates is dominated.
//
// Precondition: every point in both ranges has at least d coordinates.
size_t FILTER(Item* u, size_t u_count, Item* v, size_t v_count, size_t d);
