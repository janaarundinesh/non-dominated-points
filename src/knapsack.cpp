#include "knapsack.hpp"


std::vector<Item> ZeroOneKS(std::vector<Item> items)
{
    std::vector<Item> NonDominated;

    for (int i = 0; i < items.size(); i++)
    {
        bool dominatedFlag = false;

        for (int j = 0; j < items.size(); j++)
        {
            if (i == j)
                continue;

            if (dominates(items[j], items[i]))
            {
                dominatedFlag = true;
                break;
            }
        }

        if (!dominatedFlag)
        {
            NonDominated.push_back(items[i]);
        }
    }

    return NonDominated;
}
