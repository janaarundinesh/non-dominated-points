#include "sorting.hpp"
#include <algorithm>

void SortByLastCoordinate(std::vector<Item>& items)
{
    std::sort(items.begin(), items.end(), 
        [](const Item& a, const Item& b)
        {
            return a.coords.back() < b.coords.back();
        }
    );
}

void SortByCoordinate(std::vector<Item>& items, size_t k)
{
    std::sort(items.begin(), items.end(),
        [k](const Item& a, const Item& b)
        {
            return a.coords[k] < b.coords[k];
        }
    );
}