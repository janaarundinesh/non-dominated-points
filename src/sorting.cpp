#include "sorting.hpp"
#include <algorithm>

std::vector<Item> Asc_Sort(std::vector<Item> items)
{
    std::sort(items.begin(), items.end(),
              [](const Item& a, const Item& b) {
                  return a.weight < b.weight;
              });

    return items;
}