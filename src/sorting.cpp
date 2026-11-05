#include "sorting.hpp"

#include <algorithm>

void SortByLastCoordinate(Item* first, size_t count)
{
    std::sort(first, first + count, [](const Item& a, const Item& b)
        {
            return a.coords.back() < b.coords.back();
        }
    );
}

void SortByCoordinate(Item* first, size_t count, size_t k)
{
    std::sort(first, first + count, [k](const Item& a, const Item& b)
        {
            return a.coords[k] < b.coords[k];
        }
    );
}

void SortByLastCoordinate(std::vector<Item>& items)
{
    SortByLastCoordinate(items.data(), items.size());
}

void SortByCoordinate(std::vector<Item>& items, size_t k)
{
    SortByCoordinate(items.data(), items.size(), k);
}
