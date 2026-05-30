#include "sorting.hpp"
#include <algorithm>

void Asc_Sort(std::vector<Item>& items)
{
    std::sort(items.begin(), items.end(),
    [](const Item& a, const Item& b)
    {
        if (a.weight == b.weight)
            return a.profit > b.profit;

        return a.weight < b.weight;
    });
}