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