#include "dominance.hpp"

bool Dominates(const Item& a, const Item& b, size_t d)
{
    bool strict = false;

    for (size_t i=0; i < d; i++)
    {
        if(a.coords[i] < b.coords[i])
        {
            return false;
        }

        if(a.coords[i] > b.coords[i])
        {
            strict = true;
        }
    }

    return strict;
}