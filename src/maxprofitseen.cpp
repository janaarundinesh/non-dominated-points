#include <vector>
#include "maxprofitseen.hpp"

std::vector<Item> nondominated;

int maxProfitSeen = -1;

std::vector<Item> MaxProfitSeen(std::vector<Item> items)
{
    for (const Item& item : items)
    {
        if (item.profit > maxProfitSeen)
        {
            nondominated.push_back(item);
            maxProfitSeen = item.profit;
        }
    }
    return nondominated;
}
