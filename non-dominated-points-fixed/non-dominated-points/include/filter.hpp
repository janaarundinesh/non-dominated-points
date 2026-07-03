#pragma once

#include <vector>
#include "item.hpp"

std::vector<Item> FILTER(
    const std::vector<Item>& U,
    const std::vector<Item>& V,
    size_t d
);

std::vector<Item> FILTER2(
    const std::vector<Item>& U,
    const std::vector<Item>& V
);

// Removes from U any point weakly covered (>= on every one of the
// first k coordinates) by some point of V, EXCEPT that an exact
// duplicate (identical on all k coordinates checked) does not count.
//
// This is the right tool whenever a strict difference on some OTHER
// coordinate (outside this prefix) is already guaranteed by
// construction -- e.g. FILTER's own U1-vs-V2 branch, and MAXIMA2's
// M1-vs-M2 merge -- so only weak coverage on the remaining prefix is
// needed to conclude full domination.
std::vector<Item> WeakCoverFilter(
    const std::vector<Item>& U,
    const std::vector<Item>& V,
    size_t k
);