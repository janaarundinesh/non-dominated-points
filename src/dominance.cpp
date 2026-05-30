#include "dominance.hpp"

bool dominates(const Item& a, const Item& b)
{
    return 
    a.weight <= b.weight &&
    a.profit >= b.profit &&
    (
        a.weight < b.weight ||
        a.profit > b.profit
    );
}