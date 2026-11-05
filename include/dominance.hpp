#pragma once

#include "item.hpp"

bool Dominates(
    const Item& a,
    const Item& b,
    size_t d
);

// Weak dominance: a is greater than or equal to b in every one of the first d
// coordinates. Used by FILTER, where the coordinate that was dropped from the
// comparison already supplies the strictness of the dominance relation.
bool WeaklyDominates(
    const Item& a,
    const Item& b,
    size_t d
);